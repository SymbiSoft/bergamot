#include "ShowEngine.h"
#include "FeedEngine.h"
#include "FeedInfo.h"
#include <bautils.h>
#include <s32file.h>
#include "SettingsEngine.h"
#include <e32hashtab.h>
#include "SoundEngine.h"
#include "debug.h"

const TUint KMaxDownloadErrors=3;

CShowEngine::CShowEngine(CPodcastModel& aPodcastModel) : iPodcastModel(aPodcastModel)
	{
	iDownloadsSuspended = ETrue;
	}

CShowEngine::~CShowEngine()
	{
	iShows.ResetAndDestroy();
	iShows.Close();
	iSelectedShows.Close();
	iShowsDownloading.Close();
	iFs.Close();
	delete iShowClient;
	iObservers.Close();
	delete iMetaDataReader;
	iApaSession.Close();
	}

CShowEngine* CShowEngine::NewL(CPodcastModel& aPodcastModel)
	{
	CShowEngine* self = new (ELeave) CShowEngine(aPodcastModel);
	CleanupStack::PushL(self);
	self->ConstructL();
	CleanupStack::Pop();
	return self;
	}

void CShowEngine::GetMimeType(const TDesC& aFileName, TDes& aMimeType)
{
	aMimeType.Zero();
	RFile file;
	if(file.Open(iFs, aFileName, 0) == KErrNone)
	{
		if(file.Read(iRecogBuffer) == KErrNone)
		{
			file.Close();
			TDataRecognitionResult result;
			if(iApaSession.RecognizeData(aFileName, iRecogBuffer, result) == KErrNone)
			{
				aMimeType.Copy(result.iDataType.Des());
			}
			
		}
	}
	file.Close();
}

void CShowEngine::ConstructL()
	{
	iFs.Connect();
	iShowClient = CHttpClient::NewL(iPodcastModel, *this);
	iShowClient->SetResumeEnabled(ETrue);
	iMetaDataReader = new (ELeave) CMetaDataReader(*this);
	iMetaDataReader->ConstructL();
	iApaSession.Connect();
	
	// Try to load the database.
	TRAPD(err, LoadShowsL());
	
	// if failure, try to load backup
	if (err != KErrNone) {
		DP("Loading show database backup");
		iShows.Reset();
		TRAP(err,LoadShowsL(ETrue));
		if( err == KErrNone) {
			// and if successfull, save the backup as the real thing
			SaveShowsL();
		}
	}

	DownloadNextShow();
	
	// maybe this is a bad idea?
	if (iShowsDownloading.Count() == 0) 
		{
		iDownloadsSuspended = EFalse;
		}
	}

void CShowEngine::StopDownloads() 
	{
	DP("StopDownloads");
	iDownloadsSuspended = ETrue;
	if (iShowClient != NULL) {
		iShowClient->Stop();
	}
	}

void CShowEngine::ResumeDownloads() 
	{
	DP("ResumeDownloads");
	iDownloadsSuspended = EFalse;
	iDownloadErrors = 0;
	DownloadNextShow();
	}

TBool CShowEngine::DownloadsStopped()
	{
	return iDownloadsSuspended;
	}

void CShowEngine::RemoveAllDownloads() {
	if (!iDownloadsSuspended) {
		return;
	}
	
	for (int i=0;i<iShowsDownloading.Count();i++) {
		iShowsDownloading[i]->SetDownloadState(ENotDownloaded);
	}
	
	iShowsDownloading.Reset();
	SaveShows();
}

TBool CShowEngine::RemoveDownload(TUint aUid) 
	{
	//DP("CShowEngine::RemoveDownload\t Trying to remove download");
	//DP1("Title: %S", &iPodcastModel.ShowEngine().GetShowByUidL(aUid)->Title());
	TBool retVal = EFalse;
	// if trying to remove the present download, we first stop it
	if (!iDownloadsSuspended && iShowDownloading != NULL && iShowDownloading->Uid() == aUid) {
		DP("CShowEngine::RemoveDownload\t This is the active download, we suspend downloading");
		StopDownloads();
	} else {
		const TInt count = iShowsDownloading.Count();
		for (TInt i=0 ; i < count; i++) 
			{
			//DP2("Comparing %u (%S) to %u", iShowsDownloading[i]->Uid(), &iShowsDownloading[i]->Title(), aUid );
			if (iShowsDownloading[i] != NULL && iShowsDownloading[i]->Uid() == aUid) 
				{
				//DP1("Removing by title: %S", &iShowsDownloading[i]->Title());
				iShowsDownloading[i]->SetDownloadState(ENotDownloaded);
				BaflUtils::DeleteFile(iFs, iShowsDownloading[i]->FileName());
				iShowsDownloading.Remove(i);
	
				DP("CShowEngine::RemoveDownload\tDownload removed.");
				
				NotifyShowDownloadUpdated(-1,-1,-1);
				NotifyDownloadQueueUpdated();
				SaveShows();
				DownloadNextShow();
				retVal = ETrue;	
				break;
				}
			}
	}

	DP("CShowEngine::RemoveDownload\tCould not find downloading show to remove");
	return retVal;
	}

void CShowEngine::Connected(CHttpClient* /*aClient*/)
	{
	
	}

void CShowEngine::Progress(CHttpClient* /*aHttpClient */, TInt aBytes, TInt aTotalBytes)
	{	
	TInt percent = -1;
	if (aTotalBytes > 0) 
		{
		percent = (TInt) ((float)aBytes * 100.0 / (float)aTotalBytes) ;
		}
	
	iShowDownloading->SetShowSize(aTotalBytes);
	NotifyShowDownloadUpdated(percent, aBytes, aTotalBytes);
	}

void CShowEngine::Disconnected(CHttpClient* /*aClient */)
	{
	}

void CShowEngine::DownloadInfo(CHttpClient* aHttpClient, TInt aTotalBytes)
	{
	DP1("About to download %d bytes", aTotalBytes);
	if(aHttpClient == iShowClient && iShowDownloading != NULL && aTotalBytes != -1) {
		iShowDownloading->SetShowSize(aTotalBytes);
		}
	}

void CShowEngine::GetStatsByFeed(TUint aFeedUid, TUint &aNumShows, TUint &aNumUnplayed, TBool aIsBookFeed)
	{
	TInt showsCount = 0;
	TInt unplayedCount = 0;
	
	for (TInt i=0;i<iShows.Count();i++) {
		if (iShows[i]->FeedUid() == aFeedUid)
			{
			showsCount++;
			if (iShows[i]->PlayState() == ENeverPlayed) {
				unplayedCount++;
			}
			}
		}

	if(aIsBookFeed)
		{
		aNumShows = showsCount;
		aNumUnplayed = unplayedCount;
		}
	else
		{
		TInt max = iPodcastModel.SettingsEngine().MaxListItems();
		aNumShows = showsCount < max ? showsCount : max;
		aNumUnplayed = unplayedCount < max ? unplayedCount : max;
		}
	
	}

void CShowEngine::GetStatsForDownloaded(TUint &aNumShows, TUint &aNumUnplayed )
	{
	TInt showsCount = 0;
	TInt unplayedCount = 0;
	
	for (TInt i=0;i<iShows.Count();i++) {
		if (iShows[i]->DownloadState() == EDownloaded && !(iShows[i]->ShowType() == EAudioBook))
			{
			showsCount++;
			if (iShows[i]->PlayState() == ENeverPlayed) {
				unplayedCount++;
			}
			}
		}
	
	aNumShows = showsCount;
	aNumUnplayed = unplayedCount;
	
	}

void CShowEngine::GetShowL(CShowInfo *info)
	{
	CFeedInfo *feedInfo = iPodcastModel.FeedEngine().GetFeedInfoByUid(info->FeedUid());
	if (feedInfo == NULL) {
		DP("Feed not found for this show!");
		return;
	}
	
	TFileName filePath;
	filePath.Copy(iPodcastModel.SettingsEngine().BaseDir());
	
	// create relative file name
	TFileName relPath;
	relPath.Copy(feedInfo->Title());
	relPath.Append(_L("\\"));

	TFileName fileName;
	iPodcastModel.FeedEngine().FileNameFromUrl(info->Url(), fileName);
	relPath.Append(fileName);
	iPodcastModel.FeedEngine().EnsureProperPathName(relPath);
	
	// complete file path is base dir + rel path
	filePath.Append(relPath);
	info->SetFileNameL(filePath);
	
	iShowClient->GetL(info->Url(), filePath);
	}

TBool CShowEngine::AddShow(CShowInfo *item) 
	{
	const TInt count = iShows.Count();
	for (TInt i=0; i<count; i++) 
		{
		if (iShows[i]->Url().Compare(item->Url()) == 0) 
			{
			return EFalse;
			}
		}
	iShows.Append(item);

	return ETrue;
	}

void CShowEngine::AddObserver(MShowEngineObserver *observer)
	{
	iObservers.Append(observer);
	}

void CShowEngine::RemoveObserver(MShowEngineObserver *observer)
	{
	TInt index = iObservers.Find(observer);
	
	if (index > KErrNotFound)
		{
		iObservers.Remove(index);
		}
	}

void CShowEngine::CompleteL(CHttpClient* /*aHttpClient*/, TBool aSuccessful)
	{
	if(iShowDownloading != NULL)
	{
		DP1("CShowEngine::Complete\tDownload of file: %S is complete", &iShowDownloading->FileName());
			
		// decide what kind of file this is
		TBuf<100> mimeType;
		GetMimeType(iShowDownloading->FileName(), mimeType);
		
		if (mimeType.Left(5) == _L("audio")) {
			iShowDownloading->SetShowType(EAudioPodcast);
		} else if (mimeType.Left(5) == _L("video")) {
			iShowDownloading->SetShowType(EVideoPodcast);		
		}
		
		if (aSuccessful) 
		{
			iShowDownloading->SetDownloadState(EDownloaded);
			iShowsDownloading.Remove(0);
			
			SaveShowsL();
			NotifyShowDownloadUpdated(100,0,1);
		}
		else 
		{
			iShowDownloading->SetDownloadState(EQueued);
			iDownloadErrors++;
			if (iDownloadErrors > KMaxDownloadErrors) 
			{
				DP("Too many downloading errors, suspending downloads");
				iDownloadsSuspended = ETrue;
				NotifyShowDownloadUpdated(-1,-1,-1);
			}
		}
	}
	DownloadNextShow();
	}

CShowInfo* CShowEngine::ShowDownloading()
	{
		return iShowDownloading;
	}


CShowInfo* CShowEngine::GetShowByUidL(TUint aShowUid)
	{
	CShowInfo* infoToFind = CShowInfo::NewL();
	CleanupStack::PushL(infoToFind);
	infoToFind->SetUid(aShowUid);
	TIdentityRelation<CShowInfo> comparison(CompareShowsByUid);
	TInt index = iShows.Find(infoToFind, comparison);
	CleanupStack::PopAndDestroy(infoToFind);
	if(index != KErrNotFound)
		{
		return iShows[index];
		}

	return NULL;
	}

CShowInfo* CShowEngine::GetNextShowByTrackL(CShowInfo* aShowInfo)
	{
	TInt cnt = iShows.Count();
	CShowInfo* nextShow = NULL;
	TUint diff = KMaxTInt;
	for(TInt loop = 0; loop<cnt; loop++)
		{
			if(aShowInfo->FeedUid() == iShows[loop]->FeedUid() && aShowInfo->TrackNo() < iShows[loop]->TrackNo())
				{
				if((iShows[loop]->TrackNo() - aShowInfo->TrackNo()) < diff)
					{
					diff = iShows[loop]->TrackNo() - aShowInfo->TrackNo();
					nextShow = iShows[loop];
					}
				}
		}

	return nextShow;
	}


TBool CShowEngine::CompareShowsByUid(const CShowInfo &a, const CShowInfo &b)
{
	return a.Uid() == b.Uid();
}

void CShowEngine::LoadShowsL(TBool aUseBackup)
	{
	DP("CShowEngine::LoadShowsL\tLoad shows from database file");
	TFileName path;
	TParse	filestorename;
	
	TBuf<100> privatePath;
	iFs.PrivatePath(privatePath);
	BaflUtils::EnsurePathExistsL(iFs, privatePath);
	privatePath.Append(KShowDB);
	
	if (aUseBackup) {
		privatePath.Append(_L(".old"));
	}
	
	iFs.Parse(privatePath, filestorename);

	if (!BaflUtils::FileExists(iFs, privatePath)) 
		{
		DP("The show database does not exist");	
		User::Leave(KErrNotFound);
		}
	
	CFileStore* store = NULL;
	TRAPD(error, store = CDirectFileStore::OpenL(iFs,filestorename.FullName(),EFileRead));
	CleanupStack::PushL(store);
	
	if (error != KErrNone) 
		{
		User::Leave(error);
		}
	
	RStoreReadStream instream;
	instream.OpenLC(*store, store->Root());

	
	TInt version = instream.ReadInt32L();
	DP1("CShowEngine::LoadShowsL\tVersion of database file = %d", version);

	
	if (version != KShowInfoVersion) 
		{
		DP("CShowEngine::LoadShowsL\tWrong version, discarding");
		User::Leave(KErrGeneral);
		return;
		}
	
	CShowInfo *readData;
	//TUint lastUid = 0;
	TInt count = instream.ReadInt32L();
	DP1("CShowEngine::LoadShowsL\t%d Shows present in database", count);
		
	for (TInt i=0 ; i < count ; i++) 
		{
		readData = CShowInfo::NewL(version);
		instream  >> *readData;
		
		TBool isAdded = AddShow(readData);
		
		if (isAdded == EFalse)
			{
			delete readData;
			readData = NULL;
			}
		else
			{
			if ((readData->DownloadState() == EQueued) || (readData->DownloadState() == EDownloading)) 
				{
				readData->SetDownloadState(EQueued);
				iShowsDownloading.Append(readData);
				}
			}
		}
	CleanupStack::PopAndDestroy(2); // instream and store
	}

void CShowEngine::SaveShows()
	{
	TRAP_IGNORE(SaveShowsL());
	}

void CShowEngine::SaveShowsL()
	{
	DP("void CShowEngine::SaveShows\tAttempt to store shows to db.");
	
	TParse	filestorename;

	TBuf<100> privatePath;
	iFs.PrivatePath(privatePath);
	BaflUtils::EnsurePathExistsL(iFs, privatePath);
	privatePath.Append(KShowDB);
	
	DP("Saving backup...");
	TFileName backupFile;
	backupFile.Copy(privatePath);
	backupFile.Append(_L(".old"));
	BaflUtils::CopyFile(iFs,privatePath,backupFile);

	
	//DP1("File: %S", &privatePath);
	iFs.Parse(privatePath, filestorename);
	CFileStore* store = CDirectFileStore::ReplaceLC(iFs, filestorename.FullName(), EFileWrite);
	store->SetTypeL(KDirectFileStoreLayoutUid);
	
	RStoreWriteStream outstream;
	TStreamId id = outstream.CreateLC(*store);
	
	outstream.WriteInt32L(KShowInfoVersion);
	
	const TInt numberOfShows = iShows.Count();
	DP1("CShowEngine::SaveShows\tDatabase has %d shows entries", numberOfShows);
	
	//We need to purge the array for entries that should not be saved.
	RShowInfoArray tempArray;
	CleanupClosePushL(tempArray);
	
	// Move all non delete entries into the new array
	for (TInt j = 0; j < numberOfShows ; j++)
		{
		if (!iShows[j]->Delete() && iPodcastModel.FeedEngine().GetFeedInfoByUid(iShows[j]->FeedUid()) != NULL)
			{
			tempArray.Append(iShows[j]);
			}
		}
	
	const TInt countTempArray = tempArray.Count();
	DP1("CShowEngine::SaveShows\tDatabase has %d entries marked for delete", numberOfShows - countTempArray);	
	DP1("CShowEngine::SaveShows\tSaving %d show entries", countTempArray);
	
	outstream.WriteInt32L(countTempArray);
	for (TInt i=0 ; i < countTempArray ; i++) 
		{
		outstream  << *tempArray[i];
		}
	
	CleanupStack::PopAndDestroy(&tempArray);
	
	outstream.CommitL();
	store->SetRootL(id);
	store->CommitL();
	CleanupStack::PopAndDestroy(); // outstream
	CleanupStack::PopAndDestroy(store);	
	}

void CShowEngine::SelectAllShows()
	{
	iSelectedShows.Reset();
	
	const TInt count = iShows.Count();
	for (TInt i=0;i<count;i++)
		{
			iSelectedShows.Append(iShows[i]);
		}
	}

TInt CShowEngine::CompareShowsByDate(const CShowInfo &a, const CShowInfo &b)
	{
	if (a.PubDate() > b.PubDate()) 
		{
//		DP2("Sorting %S less than %S", &a.iTitle, &b.iTitle);
		return -1;
		} 
	else if (a.PubDate() == b.PubDate()) 
		{
//		DP2("Sorting %S equal to %S", &a.iTitle, &b.iTitle);
		return 0;
		}
	else 
		{
//		DP2("Sorting %S greater than %S", &a.iTitle, &b.iTitle);
		return 1;
		}
	}

TInt CShowEngine::CompareShowsByTrackNo(const CShowInfo &a, const CShowInfo &b)
	{
	if (a.TrackNo() < b.TrackNo()) 
		{
		return -1;
		} 
	else if (a.TrackNo() == b.TrackNo()) 
		{
		return 0;
		}
	else 
		{
		return 1;
		}
	}

TInt CShowEngine::CompareShowsByTitle(const CShowInfo &a, const CShowInfo &b)
	{
	if (a.Title() < b.Title()) 
		{
//		DP2("Sorting %S less than %S", &a.iTitle, &b.iTitle);
		return -1;
		} 
	else if (a.Title() == b.Title()) 
		{
//		DP2("Sorting %S equal to %S", &a.iTitle, &b.iTitle);
		return 0;
		}
	else 
		{
//		DP2("Sorting %S greater than %S", &a.iTitle, &b.iTitle);
		return 1;
		}
	}

void CShowEngine::DeletePlayedShows()
	{
	for (TInt i=0;i<iSelectedShows.Count();i++)
		{
		if (iSelectedShows[i]->PlayState() == EPlayed && iSelectedShows[i]->FileName().Length() > 0) {
			if (iPodcastModel.PlayingPodcast() == iSelectedShows[i] && iPodcastModel.SoundEngine().State() != ESoundEngineNotInitialized) {
				iPodcastModel.SoundEngine().Stop();
			}
			BaflUtils::DeleteFile(iFs, iSelectedShows[i]->FileName());
			iSelectedShows[i]->SetDownloadState(ENotDownloaded);
			}
		}
	SaveShows();
	}

void CShowEngine::DeleteAllShowsByFeed(TUint aFeedUid, TBool aDeleteFiles)
	{
	const TInt count = iShows.Count();
	
	for (TInt i=count-1 ; i >= 0; i--)
		{
		if (iShows[i]->FeedUid() == aFeedUid)
			{
			if (iShows[i]->FileName().Length() > 0) 
				{
				if (aDeleteFiles) {
					BaflUtils::DeleteFile(iFs, iShows[i]->FileName());
				}
				iShows[i]->SetDownloadState(ENotDownloaded);
				}
			CShowInfo* show = iShows[i];
			iShows.Remove(i);
			delete show;
			}
		}
	SaveShows();
	}

void CShowEngine::DeleteShow(TUint aShowUid, TBool aRemoveFile)
	{
	const TInt count = iShows.Count();
	
	for (TInt i=count-1 ; i >= 0; i--)
		{
		if (iShows[i]->Uid() == aShowUid)
			{
			if (iShows[i]->FileName().Length() > 0 && aRemoveFile) 
				{
				BaflUtils::DeleteFile(iFs, iShows[i]->FileName());			
				}

			iShows[i]->SetDownloadState(ENotDownloaded);
			iShows[i]->SetPlayState(EPlayed);
			break;
			}
		}
	
	SaveShows();
	}

TUint CShowEngine::GetGrossSelectionLength()
	{
	int max = iPodcastModel.SettingsEngine().MaxListItems();	
	return iGrossSelectionLength < max ? iGrossSelectionLength : max;;
	}

void CShowEngine::SelectShowsByFeed(TUint aFeedUid)
	{
	DP2("SelectShowsByFeed: %u, unplayed only=%d", aFeedUid, iPodcastModel.SettingsEngine().SelectUnplayedOnly());
	iSelectedShows.Reset();
	iGrossSelectionLength = 0;
	for (TInt i=0;i<iShows.Count();i++)
		{
		if (iShows[i]->FeedUid() == aFeedUid)
			{
			iGrossSelectionLength++;
			if (!iPodcastModel.SettingsEngine().SelectUnplayedOnly() || (iPodcastModel.SettingsEngine().SelectUnplayedOnly() && iShows[i]->PlayState() == ENeverPlayed) ) {
				iSelectedShows.Append(iShows[i]);
			}
			}
		}
		
	if (iSelectedShows.Count() == 0) {
		return;
	}
	
	if (iSelectedShows[0]->ShowType() == EAudioBook) {
		// sort by track number
		TLinearOrder<CShowInfo> sortOrder(CShowEngine::CompareShowsByTrackNo);
		iSelectedShows.Sort(sortOrder);	 
	} else {
		// sort by date falling
		TLinearOrder<CShowInfo> sortOrder(CShowEngine::CompareShowsByDate);
		iSelectedShows.Sort(sortOrder);
	
		// now purge if more than limit
		TInt count = iSelectedShows.Count();
		while (count > iPodcastModel.SettingsEngine().MaxListItems()) {
			DP("Too many items, Removing");
			//delete iSelectedShows[count-1];
			iSelectedShows[count-1]->SetDelete();
			iSelectedShows[count-1]->SetPlayState(EPlayed);
			iSelectedShows.Remove(count-1);
			iGrossSelectionLength--;
			count = iSelectedShows.Count();
		}
	}
}

void CShowEngine::SelectNewShows()
	{
	iSelectedShows.Reset();
	for (TInt i=0;i<iShows.Count();i++)
		{
		if (iShows[i]->PlayState() == ENeverPlayed)
			{
			iSelectedShows.Append(iShows[i]);
			}
		}
	
	TLinearOrder<CShowInfo> sortOrder(CShowEngine::CompareShowsByDate);
	iSelectedShows.Sort(sortOrder);

	iGrossSelectionLength = iSelectedShows.Count();
	}

void CShowEngine::SelectShowsDownloaded()
	{
	iSelectedShows.Reset();
	iGrossSelectionLength = 0;
	for (TInt i=0;i<iShows.Count();i++)
		{
		if (iShows[i]->DownloadState() == EDownloaded && !(iShows[i]->ShowType() == EAudioBook))
			{
			iGrossSelectionLength++;
			if (!iPodcastModel.SettingsEngine().SelectUnplayedOnly() || (iPodcastModel.SettingsEngine().SelectUnplayedOnly() && iShows[i]->PlayState() == ENeverPlayed) ) {
				iSelectedShows.Append(iShows[i]);
				}			
			}
		}
	
	TLinearOrder<CShowInfo> sortOrder(CShowEngine::CompareShowsByTitle);
	iSelectedShows.Sort(sortOrder);
	}


void CShowEngine::SelectShowsDownloading()
	{
	iSelectedShows.Reset();
	
	/*for (TInt i=0;i<iShows.Count();i++)
		{
		if (iShows[i]->DownloadState() == EDownloading)
				{
				iSelectedShows.Append(iShows[i]);
				}
		}

	for (TInt i=0;i<iShows.Count();i++)
		{
		if (iShows[i]->DownloadState() == EQueued)
				{
				iSelectedShows.Append(iShows[i]);
				}
		}
*/
	const TInt count = iShowsDownloading.Count();
	for (TInt i=0 ; i < count ;i++) 
		{
		iSelectedShows.Append(iShowsDownloading[i]);
		}
	iGrossSelectionLength = iSelectedShows.Count();
	}

void CShowEngine::GetShowsForFeedL(RShowInfoArray& aShowArray, TUint aFeedUid)
	{
	TInt cnt = iShows.Count();
	aShowArray.Reset();

	for(TInt loop = 0;loop<cnt;loop++)
		{
		if(iShows[loop]->FeedUid() == aFeedUid)
			{
			aShowArray.AppendL(iShows[loop]);
			}
		}	
	}

TInt CShowEngine::GetNumDownloadingShowsL() const 
	{
	return iShowsDownloading.Count();
	}

RShowInfoArray& CShowEngine::GetSelectedShows()
	{
	return iSelectedShows;
	}

void CShowEngine::AddDownload(CShowInfo *info)
	{
	if (iShowsDownloading.Count() > iPodcastModel.SettingsEngine().MaxListItems()) {
		return;
	}
	
	info->SetDownloadState(EQueued);
	iShowsDownloading.Append(info);
	SaveShows();
	DownloadNextShow();
	}


void CShowEngine::DownloadNextShow()
	{
	// Check if we have anything in the download queue
	const TInt count = iShowsDownloading.Count();
	DP("CShowEngine::DownloadNextShow\tTrying to start new download");
	DP1("CShowEngine::DownloadNextShow\tShows in download queue %d", count);
	
	// Inform the observers
	NotifyDownloadQueueUpdated();
	
	if (count > 0)
		{
		if (iDownloadsSuspended)
			{
			DP("CShowEngine::DownloadNextShow\tDownload process is suspended, ABORTING");
			return;
			}
		else if (iShowClient->IsActive())
			{
			DP("CShowEngine::DownloadNextShow\tDownload process is already active.");
			return;
			}
		else
			{
			
			// Start the download
			CShowInfo *info = iShowsDownloading[0];
			DP1("CShowEngine::DownloadNextShow\tDownloading: %S", &(info->Title()));
			info->SetDownloadState(EDownloading);
			iShowDownloading = info;
			TRAPD(error,GetShowL(info));
			if (error != KErrNone) {
				iDownloadsSuspended = ETrue;
			}
			}
		}
	else
		{
		iShowDownloading = NULL;
		DP("CShowEngine::DownloadNextShow\tNothing to download");
		}
	}


void CShowEngine::NotifyDownloadQueueUpdated()
	{
	const TInt count = iObservers.Count();
	for (TInt i=0;i < count; i++) 
		{
		TRAP_IGNORE(iObservers[i]->DownloadQueueUpdated(1, iShowsDownloading.Count() -1));
		}	
	}


void CShowEngine::NotifyShowDownloadUpdated(TInt aPercentOfCurrentDownload, TInt aBytesOfCurrentDownload, TInt aBytesTotal)
	{
	const TInt count = iObservers.Count();
	for (TInt i=0; i < count ; i++) 
		{
		TRAP_IGNORE(iObservers[i]->ShowDownloadUpdatedL(aPercentOfCurrentDownload, aBytesOfCurrentDownload, aBytesTotal));
		}
	}

void CShowEngine::NotifyShowListUpdated()
	{
	for (TInt i=0;i<iObservers.Count();i++) {
		iObservers[i]->ShowListUpdated();
		}
	}

void CShowEngine::SetSelectionPlayed()
	{
	DP("CShowEngine::SetSelectionPlayed");
	const TInt count = iSelectedShows.Count();
	for (TInt i=0 ; i < count ; i++)
		{
		//DP1("Setting %d played", iSelectedShows[i]->Uid());
		iSelectedShows[i]->SetPlayState(EPlayed);
		}
	SaveShows();
	}


void CShowEngine::ListDirL(TFileName &folder) {
	CDirScan *dirScan = CDirScan::NewLC(iFs);
	//DP1("Listing dir: %S", &folder);
	dirScan ->SetScanDataL(folder, KEntryAttDir, ESortByName);
	
	CDir *dirPtr;
	dirScan->NextL(dirPtr);
	for (TInt i=0;i<dirPtr->Count();i++) {
		const TEntry &entry = (TEntry) (*dirPtr)[i];
		if (entry.IsDir())  {
			TFileName subFolder;
			subFolder.Copy(folder);
			subFolder.Append(entry.iName);
			subFolder.Append(_L("\\"));
			ListDirL(subFolder);
		} else {
			TFileName fileName;
			fileName.Copy(entry.iName);
			TFileName pathName;
			pathName.Copy(folder);
			pathName.Append(entry.iName);

			TShowType showType;
			
			// decide what kind of file this is
			TBuf<100> mimeType;
			GetMimeType(pathName, mimeType);
			DP2("'%S' has mime: '%S'", &pathName, &mimeType);
#ifdef __WINS__
			if(mimeType.Length() == 0)
			{
				mimeType = _L("audio");
			}
			
#endif
			if (mimeType.Left(5) == _L("audio")) {
				showType = EAudioPodcast;
			} else if (mimeType.Left(5) == _L("video")) {
				showType = EVideoPodcast;
			} else {
				continue;
			}


			TBool exists = EFalse;
			for (TInt i=0;i<iShows.Count();i++) {
				if (iShows[i]->FileName().Compare(pathName) == 0) {
					DP1("Already listed %S", &pathName);
					exists = ETrue;
					break;
				}
			}
			
			if (exists) {
				continue;
			}
			
			//DP1("We found a new file: %S", &fileName);
			
			CShowInfo *info = CShowInfo::NewL();
			info->SetFileNameL(pathName);
			info->SetTitleL(fileName);
			info->SetDownloadState(EDownloaded);
			info->SetUid(DefaultHash::Des16(fileName));
			info->SetShowType(showType);
			info->SetFeedUid(0);
			TEntry entry;
			iFs.Entry(pathName, entry);
			info->SetShowSize(entry.iSize);
			info->SetPubDate(entry.iModified);
			info->SetDelete();  			// so that we do not save the entry in the DB.
			
			iMetaDataReader->SubmitShowL(info);
			iShows.Append(info);
		}
	}
	delete dirPtr;
	CleanupStack::PopAndDestroy(dirScan);
}

void CShowEngine::CheckFilesL()
	{
	// check to see if any files were removed
	for (TInt i=0;i<iShows.Count();i++) {
		if(iShows[i]->DownloadState() == EDownloaded) {
			if(!BaflUtils::FileExists(iFs, iShows[i]->FileName())) {
				DP1("%S was removed, delisting", &iShows[i]->FileName());
				if (iShows[i]->FeedUid() == 0) {
					iShows[i]->SetDelete();
				} 
				iShows[i]->SetDownloadState(ENotDownloaded);
			}
		}
	}

	// check if any new files were added
	ListDirL(iPodcastModel.SettingsEngine().BaseDir());	
	SaveShows();
	NotifyShowListUpdated();
}

void CShowEngine::ReadMetaData(CShowInfo *aShowInfo)
	{
	DP1("Read %S", &(aShowInfo->Title()));
	}

void CShowEngine::ReadMetaDataComplete()
	{
	SaveShows();
	NotifyShowListUpdated();
	MetaDataReader().SetIgnoreTrackNo(EFalse);
	}

CMetaDataReader& CShowEngine::MetaDataReader()
	{
	return *iMetaDataReader;
	}

void CShowEngine::FileError(TUint /*aError*/)
	{
	//TODO: Error dialog
	//StopDownloads();
	iDownloadErrors=KMaxDownloadErrors;
	}
