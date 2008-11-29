#include "FeedEngine.h"
#include <f32file.h>
#include <bautils.h>
#include <s32file.h>
#include "SettingsEngine.h"
#include "ShowEngine.h"
#include <e32hashtab.h>
#include "OpmlParser.h"

CFeedEngine* CFeedEngine::NewL(CPodcastModel& aPodcastModel)
	{
	CFeedEngine* self = new (ELeave) CFeedEngine(aPodcastModel);
	CleanupStack::PushL(self);
	self->ConstructL();
	CleanupStack::Pop();
	return self;
	}

void CFeedEngine::ConstructL()
	{
	iParser = new (ELeave) CFeedParser(*this);
	iFs.Connect();
	iFeedClient = CHttpClient::NewL(iPodcastModel, *this);
	iFeedTimer.ConstructL();
	
	RunFeedTimer();
	
	
    if (DBGetFeedCount() > 0) {
		DP("Loading feeds from DB");
		
		LoadFeedsL();

    } else {
    	DP("No feeds in DB, loading default feeds");
    	 TFileName defaultFile = iPodcastModel.SettingsEngine().DefaultFeedsFileName();

	    if (BaflUtils::FileExists(iFs, defaultFile)) {
	    	DP("Loading default feeds");
	    	ImportFeedsL(defaultFile);
	    }
    }
    
    TFileName importFile = iPodcastModel.SettingsEngine().ImportFeedsFileName();
    if (BaflUtils::FileExists(iFs, importFile)) {
    	DP("Importing feeds");
    	ImportFeedsL(importFile);
    }

	}

CFeedEngine::CFeedEngine(CPodcastModel& aPodcastModel) : iFeedTimer(this), iPodcastModel(aPodcastModel)
	{
	iClientState = ENotUpdating;
	iDB = aPodcastModel.DB();
	}

CFeedEngine::~CFeedEngine()
	{
	iObservers.Close();
	
	iFeedsUpdating.Close();
	iSortedFeeds.ResetAndDestroy();
	iSortedFeeds.Close();
	iSortedBooks.ResetAndDestroy();
	iSortedBooks.Close();
	iFs.Close();
	delete iParser;
	delete iFeedClient;
	}

/**
 * Returns the current internal state of the feed engine4
 */
TClientState CFeedEngine::ClientState()
	{
	return iClientState;
	}


/**
 * Returns the current updating client UID if clientstate is != ENotUpdateing
 * @return TUint
 */
TUint CFeedEngine::ActiveClientUid()
	{
		if(iActiveFeed != NULL)
			{
			return iActiveFeed->Uid();
			}
		return 0;	
	}

void CFeedEngine::RunFeedTimer()
	{
	iFeedTimer.Cancel();

	if (iPodcastModel.SettingsEngine().UpdateAutomatically() == EAutoUpdateAtTime) {
		iFeedTimer.SetPeriod(24*60);
		iFeedTimer.SetSyncTime(iPodcastModel.SettingsEngine().UpdateFeedTime());
	} else if (iPodcastModel.SettingsEngine().UpdateAutomatically() == EAutoUpdatePeriodically){
		int interval = iPodcastModel.SettingsEngine().UpdateFeedInterval();
	
		if (interval != 0) {
			iFeedTimer.SetPeriod(interval);
			iFeedTimer.RunPeriodically();
		}
	}
	
	}

void CFeedEngine::UpdateAllFeedsL()
	{
	if (iFeedsUpdating.Count() > 0) {
		DP("Cancelling update");
		iFeedClient->Stop();
		iFeedsUpdating.Reset();
		return;
	}

	TInt cnt = iSortedFeeds.Count();
	for (int i=0;i<cnt;i++) {
		iFeedsUpdating.Append(iSortedFeeds[i]);
	}

	UpdateNextFeedL();
	}

void CFeedEngine::CancelUpdateAllFeedsL()
	{
	if(iClientState != ENotUpdating)
	{
		iFeedsUpdating.Reset();
		iFeedClient->Stop();
	}
}

void CFeedEngine::UpdateNextFeedL()
	{

	DP1("UpdateNextFeed. %d feeds left to update", iFeedsUpdating.Count());
	if (iFeedsUpdating.Count() > 0) {
		CFeedInfo *info = iFeedsUpdating[0];
		iFeedsUpdating.Remove(0);
		TBool result = EFalse;
		//DP2("** UpdateNextFeed: %S, ID: %u", &(info->Url()), info->Uid());
		TRAPD(error, result = UpdateFeedL(info->Uid()));
		
		if (error != KErrNone || !result) {
			DP("Error while updating all feeds");
			for (TInt i=0;i<iObservers.Count();i++) 
				{
				TRAP_IGNORE(iObservers[i]->FeedUpdateAllCompleteL());
				}
		}
	} else {
		iClientState = ENotUpdating;
		for (TInt i=0;i<iObservers.Count();i++) 
		{
			TRAP_IGNORE(iObservers[i]->FeedUpdateAllCompleteL());
		}
	}
}

TBool CFeedEngine::UpdateFeedL(TUint aFeedUid)
	{
	iActiveFeed = GetFeedInfoByUid(aFeedUid);
	iCatchupCounter = 0;
	TFileName filePath;
	filePath.Copy (iPodcastModel.SettingsEngine().PrivatePath ());
	TBuf<20> feedUidNum;
	feedUidNum.Format(_L("%lu"), aFeedUid);
	filePath.Append(feedUidNum);
	filePath.Append(_L(".xml"));
	//filePath.Append(_L("feed.xml"));
	iUpdatingFeedFileName.Copy(filePath);

	if(iFeedClient->GetL(iActiveFeed->Url(), iUpdatingFeedFileName, iPodcastModel.SettingsEngine().SpecificIAP()))
	{
		iClientState = EUpdatingFeed;
		
		for (TInt i=0;i<iObservers.Count();i++) {
			TRAP_IGNORE(iObservers[i]->FeedDownloadUpdatedL(iActiveFeed->Uid(), 0));
		}
		DP("Update done");
		return ETrue;
	}
	else
	{
		return EFalse;
	}

	}

TBool CFeedEngine::NewShow(CShowInfo *item)
	{
	//DP4("\nTitle: %S\nURL: %S\nDescription length: %d\nFeed: %d", &(item->Title()), &(item->Url()), item->Description().Length(), item->FeedUid());
	
	HBufC* description = HBufC::NewLC(2048);
	TPtr ptr(description->Des());
	ptr.Copy(item->Description());
	CleanHtml(ptr);
	//DP1("New show has feed ID: %u") item->FeedUid());
	TRAP_IGNORE(item->SetDescriptionL(*description));
	CleanupStack::PopAndDestroy(description);
	//DP1("Description: %S", &description);

	if (iCatchupMode) {
		// in catchup mode, we let one show be unplayed
		if (++iCatchupCounter > 1) {
			item->SetPlayState(EPlayed);
		}
	}
	
	TBool isShowAdded = iPodcastModel.ShowEngine().AddShow(item);

	if (!iCatchupMode && isShowAdded && iPodcastModel.SettingsEngine().DownloadAutomatically()) 
		{
		iPodcastModel.ShowEngine().AddDownload(item);
		}	
	return isShowAdded;
	}

void CFeedEngine::GetFeedImageL(CFeedInfo *aFeedInfo)
	{
	DP("GetFeedImage");

	TFileName filePath;
	filePath.Copy(iPodcastModel.SettingsEngine().BaseDir());
	
	// create relative file name
	TFileName relPath;
	relPath.Copy(aFeedInfo->Title());
	relPath.Append(_L("\\"));

	TFileName fileName;
	FileNameFromUrl(aFeedInfo->ImageUrl(), fileName);
	relPath.Append(fileName);
	EnsureProperPathName(relPath);
	
	// complete file path is base dir + rel path
	filePath.Append(relPath);
	aFeedInfo->SetImageFileNameL(filePath);

	if(iFeedClient->GetL(aFeedInfo->ImageUrl(), filePath, ETrue))
		{
			iClientState = EUpdatingImage;
		}
	}

void CFeedEngine::FileNameFromUrl(const TDesC& aUrl, TFileName &aFileName)
	{
	TInt pos = aUrl.LocateReverse('/');
	
	if (pos != KErrNotFound) 
		{	
		TPtrC str = aUrl.Mid(pos+1);
		pos = str.Locate('?');
		if (pos != KErrNotFound) 
			{
			aFileName.Copy(str.Left(pos));
			} 
		else 
			{
			aFileName.Copy(str);
			}
		}
	DP2("FileNameFromUrl in: %S, out: %S", &aUrl, &aFileName);
	}

void CFeedEngine::EnsureProperPathName(TFileName &aPath)
	{
	
	// from the SDK: The following characters cannot occur in the path: < >: " / |*
	
	ReplaceString(aPath, _L("/"), _L("_")); // better not to add \\ in case we have multiple /
	ReplaceString(aPath, _L(":"), _L("_"));
	ReplaceString(aPath, _L("?"), _L("_"));
	ReplaceString(aPath, _L("|"), _L("_"));
	ReplaceString(aPath, _L("*"), _L("_"));
	ReplaceString(aPath, _L("<"), _L("_"));
	ReplaceString(aPath, _L(">"), _L("_"));
	ReplaceString(aPath, _L("\""), _L("_"));

	//buf.Append(_L("\\"));
	}

void CFeedEngine::ReplaceString(TDes & aString, const TDesC& aStringToReplace,
		const TDesC& aReplacement)
	{

	TInt pos=aString.Find(aStringToReplace);
	TUint offset = 0;
	while (pos != KErrNotFound)
		{
		aString.Replace(offset+pos, aStringToReplace.Length(), aReplacement);
		offset = pos + 1;
		if (offset > aString.Length()) {
			return;
		}
		

		pos=aString.Mid(offset).Find(aStringToReplace);
		}

	}

TBool CFeedEngine::AddFeed(CFeedInfo *aItem) 
	{
	DP2("CFeedEngine::AddFeed, title=%S, URL=%S", &aItem->Title(), &aItem->Url());
	for (TInt i=0;i<iSortedFeeds.Count();i++) 
		{
		if (iSortedFeeds[i]->Uid() == aItem->Uid()) 
			{
			DP1("Already have feed %S, discarding", &aItem->Url());
			delete aItem;
			aItem = NULL;
			return EFalse;
			}
		}
	
	TLinearOrder<CFeedInfo> sortOrder( CFeedEngine::CompareFeedsByTitle);
	iSortedFeeds.InsertInOrder(aItem, sortOrder);

	// Save the feeds into DB
	DBAddFeed(aItem);
	return ETrue;
	}

TBool CFeedEngine::DBAddFeed(CFeedInfo *aItem)
	{
	DP2("CFeedEngine::DBAddFeed, title=%S, URL=%S", &aItem->Title(), &aItem->Url());
	
	CFeedInfo *info;
	if ((info = DBGetFeedInfoByUid(aItem->Uid())) != NULL) {
		DP("Feed already exists!");
		delete info;
		return EFalse;
	}

	iSqlBuffer.Format(_L("insert into feeds (url, title, description, imageurl, imagefile, link, built, lastupdated, uid, feedtype, customtitle)"
			" values (\"%S\",\"%S\", \"%S\", \"%S\", \"%S\", \"%S\", \"%Ld\", \"%Ld\", \"%u\", \"%u\", \"%u\")"),
			&aItem->Url(), &aItem->Title(), &aItem->Description(), &aItem->ImageUrl(), &aItem->ImageFileName(), &aItem->Link(),
			aItem->BuildDate().Int64(), aItem->LastUpdated().Int64(), aItem->Uid(), aItem->IsBookFeed(), aItem->CustomTitle());

	sqlite3_stmt *st;
	 
	//DP1("SQL statement length=%d", iSqlBuffer.Length());
	int rc = sqlite3_prepare16_v2(iDB, (const void*)iSqlBuffer.PtrZ() , -1, &st,	(const void**) NULL);
	
	if (rc==SQLITE_OK)
		{
		rc = sqlite3_step(st);

		if (rc == SQLITE_DONE)
			{
			sqlite3_finalize(st);
			return ETrue;
			}
		else {
			sqlite3_finalize(st);
		}
	}

	return EFalse;
	}

void CFeedEngine::RemoveFeed(TUint aUid) 
	{
	for (int i=0;i<iSortedFeeds.Count();i++) 
		{
		if (iSortedFeeds[i]->Uid() == aUid) 
			{
			iPodcastModel.ShowEngine().DeleteAllShowsByFeed(aUid);
					
			CFeedInfo* feedToRemove = iSortedFeeds[i];
			
			//delete the image file if it exists
			if ( (feedToRemove->ImageFileName().Length() >0) && BaflUtils::FileExists(iFs, feedToRemove->ImageFileName() ))
				{
				iFs.Delete(feedToRemove->ImageFileName());
				}
				
			//delete the folder. It has the same name as the title.
			TFileName filePath;
			filePath.Copy(iPodcastModel.SettingsEngine().BaseDir());
			filePath.Append(feedToRemove->Title());
			filePath.Append(_L("\\"));
			iFs.RmDir(filePath);

			iSortedFeeds.Remove(i);
			delete feedToRemove;
			
			DP("Removed feed from array");
			
			// now remove it from DB
			DBRemoveFeed(aUid);

			return;
		}
	}
}


TBool CFeedEngine::DBRemoveFeed(TUint aUid)
	{
	DP("CFeedEngine::DBRemoveFeed");
	iSqlBuffer.Format(_L("delete from feeds where where uid=%u"), aUid);

	sqlite3_stmt *st;
	 
	int rc = sqlite3_prepare16_v2(iDB, (const void*)iSqlBuffer.PtrZ() , -1, &st,	(const void**) NULL);
	
	if (rc==SQLITE_OK)
		{
		rc = sqlite3_step(st);

		if (rc == SQLITE_DONE)
			{
			sqlite3_finalize(st);
			DP("Feed removed from DB");
			return ETrue;
			}
		else {
			sqlite3_finalize(st);
			DP("Error when removing feed from DB");

		}
	}
	return EFalse;	
	}

TBool CFeedEngine::DBUpdateFeed(CFeedInfo *aItem)
	{
	DP2("CShowEngine::DBUpdateFeed, title=%S, URL=%S", &aItem->Title(), &aItem->Url());

	iSqlBuffer.Format(_L("update feeds set url=\"%S\", title=\"%S\", description=\"%S\", imageurl=\"%S\", imagefile=\"%S\"," \
			"link=\"%S\", built=\"%Lu\", lastupdated=\"%Lu\", feedtype=\"%u\", customtitle=\"%u\" where uid=\"%u\""),
			&aItem->Url(), &aItem->Title(), &aItem->Description(), &aItem->ImageUrl(), &aItem->ImageFileName(), &aItem->Link(),
			aItem->BuildDate().Int64(), aItem->LastUpdated().Int64(), aItem->IsBookFeed(), aItem->CustomTitle(), aItem->Uid());

	sqlite3_stmt *st;
	 
	//DP1("SQL statement length=%d", iSqlBuffer.Length());
	int rc = sqlite3_prepare16_v2(iDB, (const void*)iSqlBuffer.PtrZ() , -1, &st,	(const void**) NULL);
	
	if (rc==SQLITE_OK)
		{
		rc = sqlite3_step(st);

		if (rc == SQLITE_DONE)
			{
			sqlite3_finalize(st);
			return ETrue;
			}
		else {
			sqlite3_finalize(st);
		}
	} else {
		DP1("SQLite rc=%d", rc);
	}

	return EFalse;
	
	}

void CFeedEngine::ParsingComplete(CFeedInfo *item)
	{
	TBuf<1024> title;
	title.Copy(item->Title());
	CleanHtml(title);
	TRAP_IGNORE(item->SetTitleL(title));
	DBUpdateFeed(item);
	for (int i=0;i<iObservers.Count();i++) {
		iObservers[i]->FeedInfoUpdated(item);
	}
	
	}


void CFeedEngine::AddObserver(MFeedEngineObserver *observer)
	{
	iObservers.Append(observer);
	}

void CFeedEngine::RemoveObserver(MFeedEngineObserver *observer)
	{
	TInt index = iObservers.Find(observer);
	
	if (index > KErrNotFound)
		{
		iObservers.Remove(index);
		}
	}

void CFeedEngine::Connected(CHttpClient* /*aClient*/)
	{
	
	}

void CFeedEngine::Progress(CHttpClient* /*aHttpClient*/, TInt aBytes, TInt aTotalBytes)
	{	
	
	TInt percent = 0;
	if (aTotalBytes > 0)
		{
		percent = (aBytes*100)/aTotalBytes;		
		}
	
	for (TInt i=0;i<iObservers.Count();i++) 
		{
		TRAP_IGNORE(iObservers[i]->FeedDownloadUpdatedL(iActiveFeed->Uid(), percent));
		}
	/*if (iClientState == EFeed) {
		int percent = -1;
		if (aTotalBytes != -1) {
			percent = (int) ((float)aBytes * 100.0 / (float)aTotalBytes) ;
		}

	}*/
}

void CFeedEngine::CompleteL(CHttpClient* /*aClient*/, TBool aSuccessful)
	{
	DP1("Complete, aSuccessful=%d", aSuccessful);
	if (iClientState == EUpdatingFeed) 
		{
		// Parse the feed. We need to trap this call since it could leave and then
		// the whole update chain will be broken
		TRAPD(parserErr, iParser->ParseFeedL(iUpdatingFeedFileName, iActiveFeed, iPodcastModel.SettingsEngine().MaxListItems()));
		if(parserErr)
			{
			// we do not need to any special action on this error.
			DP1("CFeedEngine::Complete()\t Failed to parse feed. Leave with error code=%d", parserErr);
			}
			
		// delete the downloaded XML file as it is no longer needed
		BaflUtils::DeleteFile(iFs,iUpdatingFeedFileName);

		// change client state
		iClientState = ENotUpdating;

		TTime time;
		time.HomeTime();
		iActiveFeed->SetLastUpdated(time);

		//iPodcastModel.ShowEngine().SaveShowsL();

		// if the feed has specified a image url. download it if we dont already have it
		if ( iActiveFeed->ImageUrl().Length() > 0 && ((iActiveFeed->ImageFileName().Length() == 0) || (!BaflUtils::FileExists(iFs,iActiveFeed->ImageFileName()) )))
 			{
			TRAPD(error, GetFeedImageL(iActiveFeed));
			if (error)
				{
				// we have failed in a very early stage to fetch the image.
				// continue with next Feed update
				NotifyFeedUpdateComplete();
				UpdateNextFeedL();
				}
			}
		else
			{
			// we do not have an image file
			NotifyFeedUpdateComplete();
			UpdateNextFeedL();		
			}

		// we will wait until the image has been downloaded to start the next feed update.
		}
	else	// iClientState == EUpdatingImage
		{
		// change client state to not updating
		iClientState = ENotUpdating;
		
		NotifyFeedUpdateComplete();
		UpdateNextFeedL();
		}
	}

void CFeedEngine::NotifyFeedUpdateComplete()
	{
	DBUpdateFeed(iActiveFeed);
	for (TInt i=0;i<iObservers.Count();i++) 
		{
		TRAP_IGNORE(iObservers[i]->FeedUpdateCompleteL(iActiveFeed->Uid()));
		}
	}

void CFeedEngine::Disconnected(CHttpClient* /*aClient*/)
	{
	
	}

void CFeedEngine::DownloadInfo(CHttpClient* /*aHttpClient */, int /*aTotalBytes*/)
	{	
	/*DP1("About to download %d bytes", aTotalBytes);
	if(aHttpClient == iShowClient && iShowDownloading != NULL && aTotalBytes != -1) {
		iShowDownloading->iShowSize = aTotalBytes;
		}*/
	}

void CFeedEngine::ImportFeedsL(const TDesC& aFile)
	{
	TFileName opmlPath;
	opmlPath.Copy(aFile);
	COpmlParser opmlParser(*this);
	
	opmlParser.ParseOpmlL(opmlPath);
	}

void CFeedEngine::ImportBookL(const TDesC& aTitle, const TDesC& aFile)
	{
	CDesCArrayFlat *files = new (ELeave) CDesCArrayFlat(5);
	CleanupStack::PushL(files);
	
	RFile rfile;
	TInt error = rfile.Open(iFs, aFile,  EFileRead | EFileStreamText);
	if (error != KErrNone) 
		{
		rfile.Close();
		DP("CFeedEngine::ImportBookL\tFailed to read M3U");
		User::Leave(KErrNotFound);
		}
	
	TFileName path;
	path.Copy(aFile.Left(aFile.LocateReverse('\\')));
	path.Append('\\');
	
	TBuf8<1> buf8;
	TBuf<1> buf16;
	TBuf<1024> line16;
	
	error = rfile.Read(buf8);
	buf16.Copy(buf8);
	DP("Parsing M3U");
	while (error == KErrNone && buf16.Length() != 0) 
		{
		if (buf16[0] == '\r' || buf16[0] == '\n') {
			if (line16.Length() > 0 && line16[0] != '#') {
				if (line16.Length() > 1 && line16[1] != ':') {
					TFileName tmp;
					tmp.Copy(path);
					tmp.Append(line16);
					line16.Copy(tmp);
				}
				
				DP1("File: %S", &line16);
				files->AppendL(line16);
			}
			line16.Zero();
		} else {
			line16.Append(buf16);
		}
			
		error = rfile.Read(buf8);
		buf16.Copy(buf8);
		//RDebug::Print(_L("buf: %S\nline: %S"), &buf16, &line16);
		}
	
	rfile.Close();
	iPodcastModel.ShowEngine().MetaDataReader().SetIgnoreTrackNo(ETrue);
	AddBookL(aTitle, files);
	CleanupStack::PopAndDestroy(files);
	}

TBool CFeedEngine::ExportFeedsL(TFileName& aFile)
	{
	RFile rfile;
	TBuf<1024> privatePath;
	iFs.PrivatePath(privatePath);
	TInt error = rfile.Temp(iFs, privatePath, aFile, EFileWrite);
	if (error != KErrNone) 
		{
		DP("CFeedEngine::ExportFeedsL()\tFailed to open file");
		return EFalse;
		}
	
	TFileText tft;
	tft.Set(rfile);
	
	TBuf<1024> templ;
	templ.Copy(KOpmlFeed());
	TBuf<1024> line;
			
	TBuf<512> url;

	tft.Write(KOpmlHeader());
	for (int i=0; i<iSortedFeeds.Count(); i++)
		{
		url.Copy(iSortedFeeds[i]->Url());
		ReplaceString(url, _L("&"), _L("&amp;"));

		line.Format(templ, &iSortedFeeds[i]->Title(), &url);
		tft.Write(line);
		}

	tft.Write(KOpmlFooter());
		
	rfile.Close();
	
	return ETrue;
	}

void CFeedEngine::LoadFeedsL()
{
	DBLoadFeeds();
}

CFeedInfo* CFeedEngine::GetFeedInfoByUid(TUint aFeedUid)
	{
	TInt cnt = iSortedFeeds.Count();
	for (TInt i=0;i<cnt;i++)
		{
		if (iSortedFeeds[i]->Uid() == aFeedUid)
			{
			return iSortedFeeds[i];
			}
		}
	
	cnt = iSortedBooks.Count();
	for (TInt i=0;i<cnt;i++)
		{
		if (iSortedBooks[i]->Uid() == aFeedUid)
			{
			return iSortedBooks[i];
			}
		}
	
	return NULL;
	}
		
const RFeedInfoArray& CFeedEngine::GetSortedFeeds() const
{
	return iSortedFeeds;
}

void CFeedEngine::AddBookChaptersL(CFeedInfo& aFeedInfo, CDesCArrayFlat* aFileNameArray)
{
	TInt cnt = aFileNameArray->Count();
	CShowInfo* showInfo = NULL;
	TEntry fileInfo;
	RShowInfoArray showArray;
	CleanupClosePushL(showArray);
	iPodcastModel.ShowEngine().GetShowsForFeedL(showArray, aFeedInfo.Uid());
	TInt offset = showArray.Count();
	CleanupStack::PopAndDestroy();// close showArray

	for(TInt loop = 0;loop<cnt;loop++)
	{	
		showInfo = CShowInfo::NewL();
		CleanupStack::PushL(showInfo);
		showInfo->SetTitleL(aFeedInfo.Title());		
		showInfo->SetDescriptionL(aFeedInfo.Title());	
		showInfo->SetDownloadState(EDownloaded);
		showInfo->SetUrlL(aFileNameArray->MdcaPoint(loop));
		showInfo->SetFeedUid(aFeedInfo.Uid());
		showInfo->SetPubDate(aFeedInfo.LastUpdated());
		showInfo->SetTrackNo(offset+loop+1);
		showInfo->SetShowType(EAudioBook);
		
		showInfo->SetFileNameL(aFileNameArray->MdcaPoint(loop));
		
		if(iFs.Entry(aFileNameArray->MdcaPoint(loop), fileInfo) == KErrNone)
		{
			showInfo->SetShowSize(fileInfo.iSize);
		}
		
		showInfo->SetPlayState(ENeverPlayed);
		iPodcastModel.ShowEngine().AddShow(showInfo);			
		iPodcastModel.ShowEngine().MetaDataReader().SubmitShowL(showInfo);
		
		CleanupStack::Pop(showInfo);
		showInfo = NULL;
	}

	// Save the shows	
	//iPodcastModel.ShowEngine().SaveShows();
}


void CFeedEngine::AddBookL(const TDesC& aBookTitle, CDesCArrayFlat* aFileNameArray)
	{
	
	if(aFileNameArray && aFileNameArray->Count() > 0)
		{
	
		TParsePtrC parser (aFileNameArray->MdcaPoint(0));
		CFeedInfo* item = CFeedInfo::NewLC();
		
		TFileName tempUrl;
		tempUrl.Copy(parser.DriveAndPath());
		tempUrl.Append(aBookTitle);
		item->SetUrlL(tempUrl);
		item->SetIsBookFeed();

		for (TInt i=0;i<iSortedBooks.Count();i++) 
			{
			if (iSortedBooks[i]->Uid() == item->Uid()) 
				{
				DP1("Already have book %S, discarding", &item->Url());
				CleanupStack::PopAndDestroy(item);
				return;
				}
		}
		item->SetTitleL(aBookTitle);
		item->SetDescriptionL(aBookTitle);
		TTime time;
		time.HomeTime();
		item->SetLastUpdated(time);
		
		TLinearOrder<CFeedInfo> sortOrder( CFeedEngine::CompareFeedsByTitle);
		iSortedBooks.InsertInOrder(item, sortOrder);
		CleanupStack::Pop(item);
	    
		// Save the feeds into DB
		//SaveBooksL();
		
		AddBookChaptersL(*item, aFileNameArray);				
		}
	}

void CFeedEngine::RemoveBookL(TUint aUid)
	{
		for (int i=0;i<iSortedBooks.Count();i++) 
		{
		if (iSortedBooks[i]->Uid() == aUid) 
			{
			iPodcastModel.ShowEngine().DeleteAllShowsByFeed(aUid, EFalse);
					
			CFeedInfo* feedToRemove = iSortedBooks[i];					
				
			//delete the folder. It has the same name as the title.
			TFileName filePath;
			filePath.Copy(iPodcastModel.SettingsEngine().BaseDir());
			filePath.Append(feedToRemove->Title());
			filePath.Append(_L("\\"));
			// not sure we should do this... files are added manually after all
			//iFs.RmDir(filePath);

			iSortedBooks.Remove(i);
			delete feedToRemove;
			
			DBRemoveFeed(aUid);
			return;
			}
		}
	}

const RFeedInfoArray& CFeedEngine::GetSortedBooks() const 
{
	return iSortedBooks;
}

void CFeedEngine::CleanHtml(TDes &str)
{
	ReplaceString(str, _L("\n"), _L(""));

//	DP2("CleanHtml %d, %S", str.Length(), &str);
	TInt startPos = str.Locate('<');
	TInt endPos = str.Locate('>');
	//DP3("length: %d, startPos: %d, endPos: %d", str.Length(), startPos, endPos);
	HBufC* tmpBuf = HBufC::NewLC(2048);
	TPtr tmp(tmpBuf->Des());
	while (startPos != KErrNotFound && endPos != KErrNotFound && endPos > startPos) {
		//DP1("Cleaning out %S", &str.Mid(startPos, endPos-startPos+1));
		tmp.Copy(str.Left(startPos));
		TPtrC ptr=str.Mid(startPos, endPos-startPos+1);
		if (ptr.CompareF(_L("<br>"))== 0) {
			tmp.Append('\r');
			tmp.Append('\n');
		} else if (ptr.CompareF(_L("<p>")) == 0) {
			tmp.Append('\r');
			tmp.Append('\n');
			tmp.Append('\r');
			tmp.Append('\n');
		}
		
		if (str.Length() > endPos+1) {
			tmp.Append(str.Mid(endPos+1));
		}
		
		str.Copy(tmp);
		startPos = str.Locate('<');
		endPos = str.Locate('>');
	}
	
	str.Trim();
	if(str.Locate('&') != KErrNotFound) {
		ReplaceString(str, _L("&amp;"), _L(""));
		ReplaceString(str, _L("&quot;"), _L(""));
		ReplaceString(str, _L("&nbsp;"), _L(""));
		ReplaceString(str, _L("&copy;"), _L("(c)"));
	}
	CleanupStack::PopAndDestroy(tmpBuf);
}

TInt CFeedEngine::CompareFeedsByTitle(const CFeedInfo &a, const CFeedInfo &b)
	{
		//DP2("Comparing %S to %S", &a.Title(), &b.Title());
		return a.Title().CompareF(b.Title());
	}

void CFeedEngine::SetCatchupMode(TBool aCatchup)
	{
	iCatchupMode = aCatchup;
	}


TUint CFeedEngine::DBGetFeedCount()
	{
	 DP("DBGetFeedCount BEGIN");
	 sqlite3_stmt *st;
	 char *sqlstr =  "select count(*) from feeds";
	 int rc = sqlite3_prepare_v2(iDB,(const char*)sqlstr , -1, &st, (const char**) NULL);
	 TUint size = 0;
	 
	 if( rc==SQLITE_OK ){
	  	rc = sqlite3_step(st);
	  	
	  	if (rc == SQLITE_ROW) {
	  		size = sqlite3_column_int(st, 0);
	  	}
	  }
	  
	  sqlite3_finalize(st);
	  DP1("DBGetFeedCount END=%d", size);
	  return size;
}

void CFeedEngine::DBLoadFeeds()
	{
	DP("DBLoadFeeds BEGIN");
	iSortedFeeds.Reset();
	CFeedInfo *feedInfo = NULL;
	iSqlBuffer.Format(_L("select url, title, description, imageurl, imagefile, link, built, lastupdated, uid, feedtype, customtitle from feeds order by title"));

	sqlite3_stmt *st;
	 
	TLinearOrder<CFeedInfo> sortOrder( CFeedEngine::CompareFeedsByTitle);

	int rc = sqlite3_prepare16_v2(iDB, (const void*)iSqlBuffer.PtrZ() , -1, &st,	(const void**) NULL);
	
	if (rc==SQLITE_OK) {
		rc = sqlite3_step(st);
		while(rc == SQLITE_ROW) {
			TRAPD(err, feedInfo = CFeedInfo::NewL());
			
			const void *urlz = sqlite3_column_text16(st, 0);
			TPtrC16 url((const TUint16*)urlz);
			feedInfo->SetUrlL(url);

			const void *titlez = sqlite3_column_text16(st, 1);
			TPtrC16 title((const TUint16*)titlez);
			feedInfo->SetTitleL(title);

			const void *descz = sqlite3_column_text16(st, 2);
			TPtrC16 desc((const TUint16*)descz);
			feedInfo->SetDescriptionL(desc);

			const void *imagez = sqlite3_column_text16(st, 3);
			TPtrC16 image((const TUint16*)imagez);
			feedInfo->SetImageUrlL(image);

			const void *imagefilez = sqlite3_column_text16(st, 4);
			TPtrC16 imagefile((const TUint16*)imagefilez);
			feedInfo->SetDescriptionL(imagefile);
			
			const void *linkz = sqlite3_column_text16(st, 5);
			TPtrC16 link((const TUint16*)linkz);
			feedInfo->SetDescriptionL(link);
					
			sqlite3_int64 built = sqlite3_column_int64(st, 6);
			TTime buildtime(built);
			feedInfo->SetBuildDate(buildtime);

			sqlite3_int64 lastupdated = sqlite3_column_int64(st, 7);
			TTime lastupdatedtime(lastupdated);
			feedInfo->SetLastUpdated(lastupdatedtime);

			sqlite3_int64 uid = sqlite3_column_int64(st, 8);
			// don't need to set UID, it will be set properly from URL
			
			sqlite3_int64 customtitle = sqlite3_column_int64(st, 10);
			if (customtitle) {
				feedInfo->SetCustomTitle();
			}
			
			sqlite3_int64 feedtype = sqlite3_column_int64(st, 9);
			if (feedtype) {
				feedInfo->SetIsBookFeed();
				// feeds are alredy sorted by SQLite
				iSortedBooks.Append(feedInfo);
			} else {
				// feeds are alredy sorted by SQLite
				iSortedFeeds.Append(feedInfo);		
			}
			
				
			rc = sqlite3_step(st);
		}
			
		sqlite3_finalize(st);
	}
	
	DP("DBLoadFeeds END");
	}

CFeedInfo* CFeedEngine::DBGetFeedInfoByUid(TUint aFeedUid)
	{
	DP("CFeedEngine::DBGetFeedInfoByUid");
	CFeedInfo *feedInfo = NULL;
	iSqlBuffer.Format(_L("select url, title, description, imageurl, imagefile, link, built, lastupdated, uid, feedtype, customtitle from feeds where uid=%u"), aFeedUid);

	sqlite3_stmt *st;
	 
	//DP1("SQL statement length=%d", iSqlBuffer.Length());

	int rc = sqlite3_prepare16_v2(iDB, (const void*)iSqlBuffer.PtrZ() , -1, &st,	(const void**) NULL);
	
	if (rc==SQLITE_OK) {
		rc = sqlite3_step(st);
		if (rc == SQLITE_ROW) {
			TRAPD(err, feedInfo = CFeedInfo::NewL());
			
			const void *urlz = sqlite3_column_text16(st, 0);
			TPtrC16 url((const TUint16*)urlz);
			feedInfo->SetUrlL(url);

			const void *titlez = sqlite3_column_text16(st, 1);
			TPtrC16 title((const TUint16*)titlez);
			feedInfo->SetTitleL(title);

			const void *descz = sqlite3_column_text16(st, 2);
			TPtrC16 desc((const TUint16*)descz);
			feedInfo->SetDescriptionL(desc);

			const void *imagez = sqlite3_column_text16(st, 3);
			TPtrC16 image((const TUint16*)imagez);
			feedInfo->SetImageUrlL(image);

			const void *imagefilez = sqlite3_column_text16(st, 4);
			TPtrC16 imagefile((const TUint16*)imagefilez);
			feedInfo->SetDescriptionL(imagefile);
			
			const void *linkz = sqlite3_column_text16(st, 5);
			TPtrC16 link((const TUint16*)linkz);
			feedInfo->SetDescriptionL(link);
					
			sqlite3_int64 built = sqlite3_column_int64(st, 6);
			TTime buildtime(built);
			feedInfo->SetBuildDate(buildtime);

			sqlite3_int64 lastupdated = sqlite3_column_int64(st, 7);
			TTime lastupdatedtime(lastupdated);
			feedInfo->SetLastUpdated(lastupdatedtime);

			sqlite3_int64 uid = sqlite3_column_int64(st, 8);
			// don't need to set UID, it will be set properly from URL
			
			sqlite3_int64 feedtype = sqlite3_column_int64(st, 9);
			if (feedtype) {
				feedInfo->SetIsBookFeed();
			}
			
			sqlite3_int64 customtitle = sqlite3_column_int64(st, 10);
			if (customtitle) {
				feedInfo->SetCustomTitle();
			}
		}
			
		sqlite3_finalize(st);
	}
	
	return feedInfo;
}

