/*
* Copyright (c) 2007-2010 Sebastian Brannstrom, Lars Persson, EmbedDev AB
*
* All rights reserved.
* This component and the accompanying materials are made available
* under the terms of the License "Eclipse Public License v1.0"
* which accompanies this distribution, and is available
* at the URL "http://www.eclipse.org/legal/epl-v10.html".
*
* Initial Contributors:
* EmbedDev AB - initial contribution.
*
* Contributors:
*
* Description:
*
*/

#include "FeedEngine.h"
#include <f32file.h>
#include <bautils.h>
#include <s32file.h>
#include "SettingsEngine.h"
#include "ShowEngine.h"
#include <e32hashtab.h>
#include <TXTETEXT.H> // for ELineBreak
#include "OpmlParser.h"

const TInt KMaxUidBufLen = 20;
const TInt KMaxDescriptionLength = 1024;
const TInt KMaxURLLength = 512;
const TInt KMaxLineLength = 4096;
// Cleanup stack macro for SQLite3
// TODO Move this to some common place.
static void Cleanup_sqlite3_finalize_wrapper(TAny* handle)
	{
	sqlite3_finalize(static_cast<sqlite3_stmt*>(handle));
	}
#define Cleanup_sqlite3_finalize_PushL(__handle) CleanupStack::PushL(TCleanupItem(&Cleanup_sqlite3_finalize_wrapper, __handle))


CFeedEngine* CFeedEngine::NewL(CPodcastModel& aPodcastModel)
	{
	CFeedEngine* self = new (ELeave) CFeedEngine(aPodcastModel);
	CleanupStack::PushL(self);
	self->ConstructL();
	CleanupStack::Pop(self);
	return self;
	}

void CFeedEngine::ConstructL()
	{
	iParser = new (ELeave) CFeedParser(*this, iPodcastModel.FsSession());
	
	iFeedClient = CHttpClient::NewL(iPodcastModel, *this);
	iFeedTimer.ConstructL();
	
	RunFeedTimer();
	
	
    if (DBGetFeedCount() > 0) {
		DP("Loading feeds from DB");
		
		DBLoadFeedsL();

		} else {
    	DP("No feeds in DB, loading default feeds");
    	TFileName defaultFile = iPodcastModel.SettingsEngine().DefaultFeedsFileName();
	    if (BaflUtils::FileExists(iPodcastModel.FsSession(), defaultFile)) {
	    	DP("Loading default feeds");
	    	ImportFeedsL(defaultFile);
	    }
    }
    
    TFileName importFile = iPodcastModel.SettingsEngine().ImportFeedsFileName();
    if (BaflUtils::FileExists(iPodcastModel.FsSession(), importFile)) {
    	DP("Importing feeds");
    	ImportFeedsL(importFile);
		}
	}

CFeedEngine::CFeedEngine(CPodcastModel& aPodcastModel)
		: iClientState(EIdle),
		  iFeedTimer(this),
		  iPodcastModel(aPodcastModel),
		  iDB(*aPodcastModel.DB())
	{
	}

CFeedEngine::~CFeedEngine()
	{
	iObservers.Close();
	
	iFeedsUpdating.Close();
	iSortedFeeds.ResetAndDestroy();
	iSearchResults.ResetAndDestroy();
	
	delete iParser;
	delete iFeedClient;
	delete iOpmlParser;
	}

/**
 * Returns the current internal state of the feed engine4
 */
EXPORT_C TClientState CFeedEngine::ClientState()
	{
	return iClientState;
	}


/**
 * Returns the current updating client UID if clientstate is != ENotUpdateing
 * @return TUint
 */
EXPORT_C TUint CFeedEngine::ActiveClientUid()
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

	if (iPodcastModel.SettingsEngine().UpdateAutomatically() == EAutoUpdateAtTime)
		{
		iFeedTimer.SetPeriod(24*60);
		iFeedTimer.SetSyncTime(iPodcastModel.SettingsEngine().UpdateFeedTime());
		}
	else if (iPodcastModel.SettingsEngine().UpdateAutomatically() == EAutoUpdatePeriodically)
		{
		TInt interval = iPodcastModel.SettingsEngine().UpdateFeedInterval();
	
		if (interval != 0)
			{
			iFeedTimer.SetPeriod(interval);
			iFeedTimer.RunPeriodically();
			}
		}
	}

EXPORT_C void CFeedEngine::UpdateAllFeedsL()
	{
	if (iFeedsUpdating.Count() > 0)
		{
		DP("Cancelling update");
		iFeedClient->Stop();
		iFeedsUpdating.Reset();
		return;
		}

	TInt cnt = iSortedFeeds.Count();
	for (int i=0;i<cnt;i++)
		{
		iFeedsUpdating.Append(iSortedFeeds[i]);
		}

	UpdateNextFeedL();
	}

EXPORT_C void CFeedEngine::CancelUpdateAllFeeds()
	{
	if(iClientState != EIdle)
		{
		iFeedsUpdating.Reset();
		iFeedClient->Stop();
		}
	}

void CFeedEngine::UpdateNextFeedL()
	{
	DP1("UpdateNextFeed. %d feeds left to update", iFeedsUpdating.Count());
	
	if (iFeedsUpdating.Count() > 0)
		{
		CFeedInfo *info = iFeedsUpdating[0];
		iFeedsUpdating.Remove(0);
		TBool result = EFalse;
		//DP2("** UpdateNextFeed: %S, ID: %u", &(info->Url()), info->Uid());
		TRAPD(error, result = UpdateFeedL(info->Uid()));
		
		if (error != KErrNone || !result)
			{
			DP("Error while updating all feeds");
			for (TInt i=0;i<iObservers.Count();i++) 
				{
				TRAP_IGNORE(iObservers[i]->FeedUpdateAllCompleteL());
				}
			}
		}
	else
		{
		iClientState = EIdle;
		for (TInt i=0;i<iObservers.Count();i++) 
			{
			TRAP_IGNORE(iObservers[i]->FeedUpdateAllCompleteL());
			}
		}
	}

EXPORT_C TBool CFeedEngine::UpdateFeedL(TUint aFeedUid)
	{
	DP("FeedEngine::UpdateFeedL BEGIN");
	iActiveFeed = GetFeedInfoByUid(aFeedUid);
	iCatchupCounter = 0;

	if (iActiveFeed->LastUpdated() == 0)
		{
		iCatchupMode = ETrue;
		}

	iUpdatingFeedFileName.Copy (iPodcastModel.SettingsEngine().PrivatePath ());
	_LIT(KFileNameFormat, "%lu.xml");
	iUpdatingFeedFileName.AppendFormat(KFileNameFormat, aFeedUid);
	
	if(iFeedClient->GetL(iActiveFeed->Url(), iUpdatingFeedFileName, iPodcastModel.SettingsEngine().SpecificIAP()))
		{
		iClientState = EUpdatingFeed;
		
		for (TInt i=0;i<iObservers.Count();i++)
			{
			TRAP_IGNORE(iObservers[i]->FeedDownloadUpdatedL(iActiveFeed->Uid(), 0));
			}

		DP("FeedEngine::UpdateFeedL END, return ETrue");
		return ETrue;
		}
	else
		{
		DP("FeedEngine::UpdateFeedL END, return EFalse");
		return EFalse;
		}
	}

void CFeedEngine::NewShowL(CShowInfo& aItem)
	{
	//DP4("\nTitle: %S\nURL: %S\nDescription length: %d\nFeed: %d", &(item->Title()), &(item->Url()), item->Description().Length(), item->FeedUid());
	
	HBufC* description = HBufC::NewLC(KMaxDescriptionLength);
	TPtr ptr(description->Des());
	ptr.Copy(aItem.Description());
	CleanHtmlL(ptr);
	//DP1("New show has feed ID: %u") item->FeedUid());
	TRAP_IGNORE(aItem.SetDescriptionL(*description));
	CleanupStack::PopAndDestroy(description);
	//DP1("Description: %S", &description);

	if (iCatchupMode) {
		// in catchup mode, we let one show be unplayed
		if (++iCatchupCounter > 1) {
			aItem.SetPlayState(EPlayed);
		}
	}
	
	TBool isShowAdded = iPodcastModel.ShowEngine().AddShowL(aItem);

	if (!iCatchupMode && isShowAdded && iPodcastModel.SettingsEngine().DownloadAutomatically()) 
		{
		iPodcastModel.ShowEngine().AddDownloadL(aItem);
		}	
	}

void CFeedEngine::GetFeedImageL(CFeedInfo *aFeedInfo)
	{
	DP("GetFeedImage");

	TFileName filePath;
	filePath.Copy(iPodcastModel.SettingsEngine().BaseDir());
	
	// create relative file name
	TFileName relPath;
	relPath.Copy(aFeedInfo->Title());
	relPath.Append('\\');

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
	
	ReplaceChar(aPath, '/', '_'); // better not to add \\ in case we have multiple /
	ReplaceChar(aPath, ':', '_');
	ReplaceChar(aPath, '?', '_');
	ReplaceChar(aPath, '|', '_');
	ReplaceChar(aPath, '*', '_');
	ReplaceChar(aPath, '<', '_');
	ReplaceChar(aPath, '>', '_');
	ReplaceChar(aPath, '"', '_');

	//buf.Append(_L("\\"));
	}

void CFeedEngine::ReplaceChar(TDes & aString, TUint aCharToReplace, TUint aReplacement)
	{
	TInt strLen=aString.Length();
	for (TInt i=0; i < strLen; i++)
		{
		if (aString[i] == aCharToReplace)
			{
			aString[i] = aReplacement;
			}
		}
	}

void CFeedEngine::ReplaceString(TDes & aString, const TDesC& aStringToReplace, const TDesC& aReplacement)
	{
	TInt pos=aString.Find(aStringToReplace);
	TInt offset = 0;
	while (pos != KErrNotFound)
		{
		aString.Replace(offset+pos, aStringToReplace.Length(), aReplacement);
		offset += pos + aStringToReplace.Length()+1;
		if (offset > aString.Length())
			{
			return;
			}

		pos=aString.Mid(offset).Find(aStringToReplace);
		}
	}

EXPORT_C TBool CFeedEngine::AddFeedL(const CFeedInfo&aItem) 
	{
	DP2("CFeedEngine::AddFeed, title=%S, URL=%S", &aItem.Title(), &aItem.Url());
	for (TInt i=0;i<iSortedFeeds.Count();i++) 
		{
		if (iSortedFeeds[i]->Uid() == aItem.Uid()) 
			{
			DP1("Already have feed %S, discarding", &aItem.Url());			
			return EFalse;
			}
		}
	
	TLinearOrder<CFeedInfo> sortOrder( CFeedEngine::CompareFeedsByTitle);
	CFeedInfo* newItem = aItem.CopyL();
	CleanupStack::PushL(newItem);
	User::LeaveIfError(iSortedFeeds.InsertInOrder(newItem, sortOrder));
	CleanupStack::Pop(newItem);
	

	// Save the feeds into DB
	DBAddFeedL(aItem);
	return ETrue;
	}

TBool CFeedEngine::DBAddFeedL(const CFeedInfo& aItem)
	{
	DP2("CFeedEngine::DBAddFeed, title=%S, URL=%S", &aItem.Title(), &aItem.Url());
	
	CFeedInfo *info = DBGetFeedInfoByUidL(aItem.Uid());
	if (info) {
		DP("Feed already exists!");
		delete info;
		return EFalse;
	}

	_LIT(KSqlStatement, "insert into feeds (url, title, description, imageurl, imagefile, link, built, lastupdated, uid, feedtype, customtitle, lasterror)"
			" values (\"%S\",\"%S\", \"%S\", \"%S\", \"%S\", \"%S\", \"%Ld\", \"%Ld\", \"%u\", \"%u\", \"%u\", \"%d\")");
	iSqlBuffer.Format(KSqlStatement,
			&aItem.Url(), &aItem.Title(), &aItem.Description(), &aItem.ImageUrl(), &aItem.ImageFileName(), &aItem.Link(),
			aItem.BuildDate().Int64(), aItem.LastUpdated().Int64(), aItem.Uid(), EAudioPodcast, aItem.CustomTitle(), aItem.LastError());

	sqlite3_stmt *st;
	 
	//DP1("SQL statement length=%d", iSqlBuffer.Length());
	int rc = sqlite3_prepare16_v2(&iDB, (const void*)iSqlBuffer.PtrZ() , -1, &st, (const void**) NULL);
	
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

EXPORT_C void CFeedEngine::RemoveFeedL(TUint aUid) 
	{
	for (int i=0;i<iSortedFeeds.Count();i++) 
		{
		if (iSortedFeeds[i]->Uid() == aUid) 
			{
			iPodcastModel.ShowEngine().DeleteAllShowsByFeedL(aUid);
					
			CFeedInfo* feedToRemove = iSortedFeeds[i];
			
			//delete the image file if it exists
			if ((feedToRemove->ImageFileName().Length() >0)
					&& BaflUtils::FileExists(iPodcastModel.FsSession(), feedToRemove->ImageFileName()))
				{
				iPodcastModel.FsSession().Delete(feedToRemove->ImageFileName());
				}
				
			//delete the folder. It has the same name as the title.
			TFileName filePath;
			filePath.Copy(iPodcastModel.SettingsEngine().BaseDir());
			filePath.Append(feedToRemove->Title());
			filePath.Append('\\');
			iPodcastModel.FsSession().RmDir(filePath);

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
	_LIT(KSqlStatement, "delete from feeds where uid=%u");
	iSqlBuffer.Format(KSqlStatement, aUid);

	sqlite3_stmt *st;
	 
	int rc = sqlite3_prepare16_v2(&iDB, (const void*)iSqlBuffer.PtrZ() , -1, &st,	(const void**) NULL);
	
	if (rc==SQLITE_OK)
		{
		rc = sqlite3_step(st);
		sqlite3_finalize(st);

		if (rc == SQLITE_DONE)
			{
			DP("Feed removed from DB");
			return ETrue;
			}
		else
			{
			DP("Error when removing feed from DB");
			}
		}
	return EFalse;	
	}

TBool CFeedEngine::DBUpdateFeed(const CFeedInfo &aItem)
	{
	DP2("CShowEngine::DBUpdateFeed, title=%S, URL=%S", &aItem.Title(), &aItem.Url());
	_LIT(KSqlStatement, "update feeds set url=\"%S\", title=\"%S\", description=\"%S\", imageurl=\"%S\", imagefile=\"%S\"," \
			"link=\"%S\", built=\"%Lu\", lastupdated=\"%Lu\", feedtype=\"%u\", customtitle=\"%u\", lasterror=\"%d\" where uid=\"%u\"");
	iSqlBuffer.Format(KSqlStatement,
			&aItem.Url(), &aItem.Title(), &aItem.Description(), &aItem.ImageUrl(), &aItem.ImageFileName(), &aItem.Link(),
			aItem.BuildDate().Int64(), aItem.LastUpdated().Int64(), EAudioPodcast, aItem.CustomTitle(), aItem.LastError(), aItem.Uid());

	sqlite3_stmt *st;
	 
	//DP1("SQL statement length=%d", iSqlBuffer.Length());
	int rc = sqlite3_prepare16_v2(&iDB, (const void*)iSqlBuffer.PtrZ() , -1, &st, (const void**) NULL);
	
	if (rc==SQLITE_OK)
		{
		rc = sqlite3_step(st);
		sqlite3_finalize(st);
		
		if (rc == SQLITE_DONE)
			{
			return ETrue;
			}
		}
	else
		{
		DP1("SQLite rc=%d", rc);
		}

	return EFalse;
	}

void CFeedEngine::ParsingCompleteL(CFeedInfo *item)
	{
	TBuf<KMaxLineLength> title;
	title.Copy(item->Title());
	TRAP_IGNORE(CleanHtmlL(title));
	TRAP_IGNORE(item->SetTitleL(title));
	DBUpdateFeed(*item);
	for (int i=0;i<iObservers.Count();i++)
		{
		iObservers[i]->FeedInfoUpdated(item->Uid());
		}
	}


EXPORT_C void CFeedEngine::AddObserver(MFeedEngineObserver *observer)
	{
	iObservers.Append(observer);
	}

EXPORT_C void CFeedEngine::RemoveObserver(MFeedEngineObserver *observer)
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
	
	if(iClientState != ESearching  && iActiveFeed != NULL)
		{
		for (TInt i=0;i<iObservers.Count();i++) 
			{
			TRAP_IGNORE(iObservers[i]->FeedDownloadUpdatedL(iActiveFeed->Uid(), percent));
			}
		}
	/*if (iClientState == EFeed) {
		int percent = -1;
		if (aTotalBytes != -1) {
			percent = (int) ((float)aBytes * 100.0 / (float)aTotalBytes) ;
		}

	}*/
}

void CFeedEngine::CompleteL(CHttpClient* /*aClient*/, TInt aError)
	{
	DP2("CFeedEngine::CompleteL BEGIN, iClientState=%d, aSuccessful=%d", iClientState, aError);

	switch(iClientState)
		{		
		default:
			if(iActiveFeed != NULL)
				{
				TTime time;
				time.HomeTime();
				iActiveFeed->SetLastUpdated(time);
				iActiveFeed->SetLastError(aError);
				NotifyFeedUpdateComplete();
				}
			break;
		case EUpdatingFeed: 
		{
		// Parse the feed. We need to trap this call since it could leave and then
		// the whole update chain will be broken
		// change client state
		iClientState = EIdle;
		iActiveFeed->SetLastError(aError);
		TTime time;
		time.HomeTime();
		iActiveFeed->SetLastUpdated(time);
		
		if( aError == KErrNone)
			{			
			TRAPD(parserErr, iParser->ParseFeedL(iUpdatingFeedFileName, iActiveFeed, iPodcastModel.SettingsEngine().MaxListItems()));

			if(parserErr)
				{
				// we do not need to any special action on this error.
				iActiveFeed->SetLastError(parserErr);
				DP1("CFeedEngine::Complete()\t Failed to parse feed. Leave with error code=%d", parserErr);
				}
			else
				{
				iPodcastModel.ShowEngine().DeleteOldShowsByFeed(iActiveFeed->Uid());
				}

			// delete the downloaded XML file as it is no longer needed
			BaflUtils::DeleteFile(iPodcastModel.FsSession(),iUpdatingFeedFileName);			

			// if the feed has specified a image url. download it if we dont already have it
			if((iActiveFeed->ImageUrl().Length() > 0))
				{
				if ( (iActiveFeed->ImageFileName().Length() == 0) || 
						(iActiveFeed->ImageFileName().Length() > 0 && 
								!BaflUtils::FileExists(iPodcastModel.FsSession(), 
										iActiveFeed->ImageFileName()) )
				   )
					{
					TRAPD(error, GetFeedImageL(iActiveFeed));
					if (error)
						{
						// we have failed in a very early stage to fetch the image.
						// continue with next Feed update	
						iActiveFeed->SetLastError(parserErr);
						iClientState = EIdle;							
						}
					}	
				}
			}

		// we will wait until the image has been downloaded to start the next feed update.
		if (iClientState == EIdle)
			{
			NotifyFeedUpdateComplete();
			UpdateNextFeedL();	
			}
		}break;
	case EUpdatingImage:
		{
		// change client state to not updating
		iClientState = EIdle;

		NotifyFeedUpdateComplete();
		UpdateNextFeedL();
		}break;
	case ESearching: 
		{
		iClientState = EIdle;

		DP1("Search complete, results in %S", &iSearchResultsFileName);

		if (!iOpmlParser) 
			{
			iOpmlParser = new COpmlParser(*this, iPodcastModel.FsSession());
			}

		DP("Parsing OPML");
		TRAPD(err, iOpmlParser->ParseOpmlL(iSearchResultsFileName, ETrue));

		if (err == KErrNone) 
			{
			NotifySearchComplete();
			} 
		else 
			{
			DP1("Parsing failed, err=%d", err);	
			}
		BaflUtils::DeleteFile(iPodcastModel.FsSession(), iSearchResultsFileName);
		}break;
		}
	DP("CFeedEngine::CompleteL END");
	}

void CFeedEngine::NotifyFeedUpdateComplete()
	{
	DBUpdateFeed(*iActiveFeed);
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

EXPORT_C void CFeedEngine::ImportFeedsL(const TDesC& aFile)
	{
	TFileName opmlPath;
	opmlPath.Copy(aFile);
	
	if (!iOpmlParser) {
		iOpmlParser = new COpmlParser(*this, iPodcastModel.FsSession());
	}
	
	iOpmlParser->ParseOpmlL(opmlPath, EFalse);
	}

EXPORT_C TBool CFeedEngine::ExportFeedsL(TFileName& aFile)
	{
	RFile rfile;
	TFileName privatePath;
	iPodcastModel.FsSession().PrivatePath(privatePath);
	TInt error = rfile.Temp(iPodcastModel.FsSession(), privatePath, aFile, EFileWrite);
	if (error != KErrNone) 
		{
		DP("CFeedEngine::ExportFeedsL()\tFailed to open file");
		return EFalse;
		}
	CleanupClosePushL(rfile);
	TFileText tft;
	tft.Set(rfile);
	HBufC* templ = HBufC::NewLC(KMaxLineLength);
	
	templ->Des().Copy(KOpmlFeed());
	
	HBufC* line = HBufC::NewLC(KMaxLineLength);
	
	HBufC* xmlUrl = HBufC::NewLC(KMaxURLLength);		
	HBufC* htmlUrl = HBufC::NewLC(KMaxURLLength);		

	HBufC* desc = HBufC::NewLC(KMaxDescriptionLength);

	tft.Write(KOpmlHeader());
	for (int i=0; i<iSortedFeeds.Count(); i++)
		{
		DP1("Exporting feed '%S'", &iSortedFeeds[i]->Title());
		
		xmlUrl->Des().Copy(iSortedFeeds[i]->Url());
		TPtr ptr(xmlUrl->Des());
		_LIT(KAnd, "&");
		_LIT(KAmp, "&amp;");
		ReplaceString(ptr, KAnd, KAmp);
		
		desc->Des().Zero();
		if (iSortedFeeds[i]->Description() != KNullDesC) {
			desc->Des().Copy(iSortedFeeds[i]->Description());
		}
		
		line->Des().Format(*templ, &iSortedFeeds[i]->Title(), desc, xmlUrl, htmlUrl);
		tft.Write(*line);
		}

	tft.Write(KOpmlFooter());
		
	CleanupStack::PopAndDestroy(6);//destroy 4 bufs & close rfile
	
	return ETrue;
	}

EXPORT_C CFeedInfo* CFeedEngine::GetFeedInfoByUid(TUint aFeedUid)
	{
	TInt cnt = iSortedFeeds.Count();
	for (TInt i=0;i<cnt;i++)
		{
		if (iSortedFeeds[i]->Uid() == aFeedUid)
			{
			return iSortedFeeds[i];
			}
		}
	
	return NULL;
	}
		
EXPORT_C const RFeedInfoArray& CFeedEngine::GetSortedFeeds()
{
	TLinearOrder<CFeedInfo> sortOrder( CFeedEngine::CompareFeedsByTitle);

	iSortedFeeds.Sort(sortOrder);
	return iSortedFeeds;
}

void CFeedEngine::CleanHtmlL(TDes &str)
{
#ifdef UIQ
	_LIT(KLineBreak, "\r\n");
#else
	const TChar KLineBreak(CEditableText::ELineBreak); 
#endif
	_LIT(KNewLine, "\n");
	ReplaceString(str, KNewLine, KNullDesC);

//	DP2("CleanHtml %d, %S", str.Length(), &str);
	TInt startPos = str.Locate('<');
	TInt endPos = str.Locate('>');
	//DP3("length: %d, startPos: %d, endPos: %d", str.Length(), startPos, endPos);
	HBufC* tmpBuf = HBufC::NewLC(KMaxDescriptionLength);
	TPtr tmp(tmpBuf->Des());
	while (startPos != KErrNotFound && endPos != KErrNotFound && endPos > startPos) {
		//DP1("Cleaning out %S", &str.Mid(startPos, endPos-startPos+1));
		tmp.Copy(str.Left(startPos));
		TPtrC ptr=str.Mid(startPos, endPos-startPos+1);
		_LIT(KHtmlBr, "<br>");
		_LIT(KHtmlBr2, "<br />");
		_LIT(KHtmlBr3, "<br/>");
		_LIT(KHtmlP, "<p>");
		if (ptr.CompareF(KHtmlBr)== 0 || ptr.CompareF(KHtmlBr2)== 0 || ptr.CompareF(KHtmlBr3)== 0)
			{
			tmp.Append(KLineBreak);
			}
		else if (ptr.CompareF(KHtmlP) == 0)
			{
			tmp.Append(KLineBreak);
			tmp.Append(KLineBreak);
			}
		
		if (str.Length() > endPos+1) {
			tmp.Append(str.Mid(endPos+1));
		}
		
		str.Copy(tmp);
		startPos = str.Locate('<');
		endPos = str.Locate('>');
	}
	
	str.Trim();
	_LIT(KAmp, "&amp;");
	_LIT(KQuot, "&quot;");
	_LIT(KNbsp, "&nbsp;");
	_LIT(KCopy, "&copy;");
	_LIT(KCopyReplacement, "(c)");
	if(str.Locate('&') != KErrNotFound) {
		ReplaceString(str, KAmp, KNullDesC);
		ReplaceString(str, KQuot, KNullDesC);
		ReplaceString(str, KNbsp, KNullDesC);
		ReplaceString(str, KCopy, KCopyReplacement);
	}
	ReplaceChar(str, '"', '\'');
	
	CleanupStack::PopAndDestroy(tmpBuf);
}

TInt CFeedEngine::CompareFeedsByTitle(const CFeedInfo &a, const CFeedInfo &b)
	{
		//DP2("Comparing %S to %S", &a.Title(), &b.Title());
		return a.Title().CompareF(b.Title());
	}

EXPORT_C void CFeedEngine::GetDownloadedStats(TUint &aNumShows, TUint &aNumUnplayed)
	{
	DP("CFeedEngine::GetDownloadedStats");
	_LIT(KSqlStatement, "select count(*) from shows where downloadstate=%u");
	iSqlBuffer.Format(KSqlStatement, EDownloaded);

	sqlite3_stmt *st;
	 
	int rc = sqlite3_prepare16_v2(&iDB, (const void*)iSqlBuffer.PtrZ() , -1, &st,	(const void**) NULL);
	
	if( rc==SQLITE_OK ){
	  	rc = sqlite3_step(st);
	  	
	  	if (rc == SQLITE_ROW) {
	  		aNumShows = sqlite3_column_int(st, 0);
	  	}
	}
		  
	sqlite3_finalize(st);

	_LIT(KSqlStatement2, "select count(*) from shows where downloadstate=%u and playstate=%u");
	iSqlBuffer.Format(KSqlStatement2, EDownloaded, ENeverPlayed);

	rc = sqlite3_prepare16_v2(&iDB, (const void*)iSqlBuffer.PtrZ() , -1, &st,	(const void**) NULL);
		
	if( rc==SQLITE_OK ){
	  	rc = sqlite3_step(st);
	  	
	  	if (rc == SQLITE_ROW) {
	  		aNumUnplayed = sqlite3_column_int(st, 0);
	  	}
	}
		  
	sqlite3_finalize(st);
	}

EXPORT_C void CFeedEngine::GetStatsByFeed(TUint aFeedUid, TUint &aNumShows, TUint &aNumUnplayed)
	{
	DP1("CFeedEngine::GetStatsByFeed, aFeedUid=%u", aFeedUid);
	DBGetStatsByFeed(aFeedUid, aNumShows, aNumUnplayed);
	}

void CFeedEngine::DBGetStatsByFeed(TUint aFeedUid, TUint &aNumShows, TUint &aNumUnplayed)
	{
	DP1("CFeedEngine::DBGetStatsByFeed, feedUid=%u", aFeedUid);
	_LIT(KSqlStatement, "select count(*) from shows where feeduid=%u");
	iSqlBuffer.Format(KSqlStatement, aFeedUid);

	sqlite3_stmt *st;
	 
	int rc = sqlite3_prepare16_v2(&iDB, (const void*)iSqlBuffer.PtrZ() , -1, &st,	(const void**) NULL);
	
	if( rc==SQLITE_OK ){
	  	rc = sqlite3_step(st);
	  	
	  	if (rc == SQLITE_ROW) {
	  		aNumShows = sqlite3_column_int(st, 0);
	  	}
	}
		  
	sqlite3_finalize(st);

	_LIT(KSqlStatement2, "select count(*) from shows where feeduid=%u and playstate=0");
	iSqlBuffer.Format(KSqlStatement2, aFeedUid);

	rc = sqlite3_prepare16_v2(&iDB, (const void*)iSqlBuffer.PtrZ() , -1, &st,	(const void**) NULL);
		
	if( rc==SQLITE_OK ){
	  	rc = sqlite3_step(st);
	  	
	  	if (rc == SQLITE_ROW) {
	  		aNumUnplayed = sqlite3_column_int(st, 0);
	  	}
	}
		  
	sqlite3_finalize(st);
}

TUint CFeedEngine::DBGetFeedCount()
	{
	 DP("DBGetFeedCount BEGIN");
	 sqlite3_stmt *st;
	 int rc = sqlite3_prepare_v2(&iDB,"select count(*) from feeds" , -1, &st, (const char**) NULL);
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

void CFeedEngine::DBLoadFeedsL()
	{
	DP("DBLoadFeeds BEGIN");
	iSortedFeeds.Reset();
	CFeedInfo *feedInfo = NULL;
	
	_LIT(KSqlStatement, "select url, title, description, imageurl, imagefile, link, built, lastupdated, uid, feedtype, customtitle from feeds");
	iSqlBuffer.Format(KSqlStatement);

	sqlite3_stmt *st;
	 
	TLinearOrder<CFeedInfo> sortOrder( CFeedEngine::CompareFeedsByTitle);

	int rc = sqlite3_prepare16_v2(&iDB, (const void*)iSqlBuffer.PtrZ() , -1, &st,	(const void**) NULL);
	Cleanup_sqlite3_finalize_PushL(st);
	
	if (rc==SQLITE_OK) {
		rc = sqlite3_step(st);
		while(rc == SQLITE_ROW) {
			feedInfo = CFeedInfo::NewLC();
			
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
			feedInfo->SetImageFileNameL(imagefile);
			
			const void *linkz = sqlite3_column_text16(st, 5);
			TPtrC16 link((const TUint16*)linkz);
			feedInfo->SetDescriptionL(link);
					
			sqlite3_int64 built = sqlite3_column_int64(st, 6);
			TTime buildtime(built);
			feedInfo->SetBuildDate(buildtime);

			sqlite3_int64 lastupdated = sqlite3_column_int64(st, 7);
			TTime lastupdatedtime(lastupdated);
			feedInfo->SetLastUpdated(lastupdatedtime);
			
			sqlite3_int64 customtitle = sqlite3_column_int64(st, 10);
			if (customtitle) {
				feedInfo->SetCustomTitle();
			}
			
			TInt lasterror = sqlite3_column_int(st, 11);
			feedInfo->SetLastError(lasterror);
			
			TLinearOrder<CFeedInfo> sortOrder( CFeedEngine::CompareFeedsByTitle);

			iSortedFeeds.InsertInOrder(feedInfo, sortOrder);
			
			CleanupStack::Pop(feedInfo);
				
			rc = sqlite3_step(st);
		}
	}

	CleanupStack::PopAndDestroy();//st

	DP("DBLoadFeeds END");
	}

CFeedInfo* CFeedEngine::DBGetFeedInfoByUidL(TUint aFeedUid)
	{
	DP("CFeedEngine::DBGetFeedInfoByUid");
	CFeedInfo *feedInfo = NULL;
	_LIT(KSqlStatement, "select url, title, description, imageurl, imagefile, link, built, lastupdated, uid, feedtype, customtitle, lasterror from feeds where uid=%u");
	iSqlBuffer.Format(KSqlStatement, aFeedUid);

	sqlite3_stmt *st;
	 
	//DP1("SQL statement length=%d", iSqlBuffer.Length());

	int rc = sqlite3_prepare16_v2(&iDB, (const void*)iSqlBuffer.PtrZ() , -1, &st,	(const void**) NULL);
	
	if (rc==SQLITE_OK) {
		Cleanup_sqlite3_finalize_PushL(st);
		rc = sqlite3_step(st);
		if (rc == SQLITE_ROW) {
			feedInfo = CFeedInfo::NewLC();
			
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
			
			sqlite3_int64 customtitle = sqlite3_column_int64(st, 10);
			if (customtitle) {
				feedInfo->SetCustomTitle();
			}
			
			TInt lasterror = sqlite3_column_int(st, 11);
			feedInfo->SetLastError(lasterror);
						
			CleanupStack::Pop(feedInfo);
		}
		CleanupStack::PopAndDestroy();//st	
	}
	
	return feedInfo;
}

EXPORT_C void CFeedEngine::UpdateFeed(CFeedInfo *aItem)
	{
	DBUpdateFeed(*aItem);
	}

EXPORT_C void CFeedEngine::SearchForFeedL(TDesC& aSearchString)
	{
	DP1("FeedEngine::SearchForFeedL BEGIN, aSearchString=%S", &aSearchString);
	
	if (iClientState != EIdle) {
		User::Leave(KErrInUse);
	}
	TBuf<KMaxURLLength> ssBuf;
	ssBuf.Copy(aSearchString);
	ReplaceString(ssBuf, _L(" "), _L("%20"));
	// prepare search URL
	HBufC* templ = HBufC::NewLC(KMaxLineLength);
	templ->Des().Copy(KSearchUrl());
		
	HBufC* url = HBufC::NewLC(KMaxURLLength);		
	url->Des().Format(*templ, &ssBuf);

	DP1("SearchURL: %S", url);
	
	// crate path to store OPML search results
	iPodcastModel.FsSession().PrivatePath(iSearchResultsFileName);
	
	iSearchResultsFileName.Append(KSearchResultsFileName);
	iSearchResults.ResetAndDestroy();
	// run search
	if(iFeedClient->GetL(*url, iSearchResultsFileName, iPodcastModel.SettingsEngine().SpecificIAP()))
		{
		iClientState = ESearching;
		}
	else
		{
		iClientState = EIdle;
		User::Leave(KErrAbort);
		}
	
	CleanupStack::PopAndDestroy(url);
	CleanupStack::PopAndDestroy(templ);
		
	DP("FeedEngine::SearchForFeedL END");
	}

EXPORT_C void CFeedEngine::AddSearchResultL(CFeedInfo *item)
	{
	DP1("CFeedEngine::AddSearchResultL, item->Title()=%S", &(item->Title()));
	iSearchResults.AppendL(item);
	}

EXPORT_C const RFeedInfoArray& CFeedEngine::GetSearchResults()
	{
	return iSearchResults;
	}


void CFeedEngine::NotifySearchComplete()
	{
	for (TInt i=0;i<iObservers.Count();i++) 
		{
		iObservers[i]->FeedSearchResultsUpdated();
		}
	}
