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
	iDB = aPodcastModel.DB();
	}

CShowEngine::~CShowEngine()
	{
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
	
	DownloadNextShow();
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
	if(iDownloadsSuspended)
		{
		iDownloadsSuspended = EFalse;
		iDownloadErrors = 0;
		DownloadNextShow();
		}
	}

TBool CShowEngine::DownloadsStopped()
	{
	return iDownloadsSuspended;
	}

void CShowEngine::RemoveAllDownloads() {
	if (!iDownloadsSuspended) {
		return;
	}
	
	DBRemoveAllDownloads();
}

TBool CShowEngine::RemoveDownload(TUint aUid) 
	{
	DP("CShowEngine::RemoveDownload\t Trying to remove download");
	
	TBool retVal = EFalse;
	// if trying to remove the present download, we first stop it
	if (!iDownloadsSuspended && iShowDownloading != NULL && iShowDownloading->Uid() == aUid) {
		DP("CShowEngine::RemoveDownload\t This is the active download, we suspend downloading");
		StopDownloads();
	} else if (iShowDownloading != NULL) {
		BaflUtils::DeleteFile(iFs, iShowDownloading->FileName());

		NotifyShowDownloadUpdated(-1,-1,-1);
		NotifyDownloadQueueUpdated();
		DownloadNextShow();
		retVal = ETrue;	
	}
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

void CShowEngine::GetStatsForDownloaded(TUint &aNumShows, TUint &aNumUnplayed )
	{
	DP("CShowEngine::GetStatsForDownloaded");
	TInt showsCount = 0;
	TInt unplayedCount = 0;
	
	RShowInfoArray array;
	DBGetDownloadedShows(array);
	
	
	for (TInt i=0;i<array.Count();i++) {
		if (!(array[i]->ShowType() == EAudioBook))
			{
			showsCount++;
			if (array[i]->PlayState() == ENeverPlayed) {
				unplayedCount++;
			}
			}
		}
	
	aNumShows = showsCount;
	aNumUnplayed = unplayedCount;
	array.ResetAndDestroy();
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
	CShowInfo *showInfo = DBGetShowByUid(item->Uid());
	
	
	if (showInfo == NULL) {
		DBAddShow(item);
	} else {
		delete showInfo;
	}

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
			DBRemoveDownload(iShowDownloading->Uid());
			
			//SaveShowsL();
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
	return DBGetShowByUid(aShowUid);
	}

CShowInfo* CShowEngine::DBGetShowByUid(TUint aUid)
	{
	DP("CShowEngine::DBGetShowByUid");
	CShowInfo *showInfo = NULL;
	iSqlBuffer.Format(_L("select url, title, description, filename, position, playtime, playstate, downloadstate, feeduid, uid, showsize, trackno, pubdate, showtype from shows where uid=%u"), aUid);

	sqlite3_stmt *st;
	 
	//DP1("SQL: %S", &iSqlBuffer.Left(KSqlDPLen));

	int rc = sqlite3_prepare16_v2(iDB, (const void*)iSqlBuffer.PtrZ() , -1, &st,	(const void**) NULL);
	
	if (rc==SQLITE_OK) {
		rc = sqlite3_step(st);
		if (rc == SQLITE_ROW) {
			TRAPD(err, showInfo = CShowInfo::NewL());
			DBFillShowInfoFromStmt(st, showInfo);
		}
			
		sqlite3_finalize(st);
	}
	
	return showInfo;
}

void CShowEngine::DBGetAllShows(RShowInfoArray& aShowArray)
	{
	DP("CShowEngine::DBGetAllShows");
	CShowInfo *showInfo = NULL;
	iSqlBuffer.Format(_L("select url, title, description, filename, position, playtime, playstate, downloadstate, feeduid, uid, showsize, trackno, pubdate, showtype from shows"));
	
	if (iPodcastModel.SettingsEngine().SelectUnplayedOnly()) {
		iSqlBuffer.Append(_L(" where playstate=0"));
	}
	//DP1("SQL: %S", &iSqlBuffer.Left(KSqlDPLen));

	sqlite3_stmt *st;

	int rc = sqlite3_prepare16_v2(iDB, (const void*)iSqlBuffer.PtrZ() , -1, &st,	(const void**) NULL);
	
	if (rc==SQLITE_OK) {
		rc = sqlite3_step(st);
		if (rc == SQLITE_ROW) {
			TRAPD(err, showInfo = CShowInfo::NewL());
			DBFillShowInfoFromStmt(st, showInfo);
			aShowArray.Append(showInfo);
		}
			
		sqlite3_finalize(st);
	}
	
	}

void CShowEngine::DBGetAllDownloads(RShowInfoArray& aShowArray)
	{
	DP("CShowEngine::DBGetAllDownloads");
	CShowInfo *showInfo = NULL;
	iSqlBuffer.Format(_L("select url, title, description, filename, position, playtime, playstate, downloadstate, feeduid, shows.uid, showsize, trackno, pubdate, showtype from downloads, shows where downloads.uid=shows.uid order by dl_index"));

	sqlite3_stmt *st;

	int rc = sqlite3_prepare16_v2(iDB, (const void*)iSqlBuffer.PtrZ() , -1, &st,	(const void**) NULL);
	
	if (rc==SQLITE_OK) {
		rc = sqlite3_step(st);
		while (rc == SQLITE_ROW) {
			TRAPD(err, showInfo = CShowInfo::NewL());
			DBFillShowInfoFromStmt(st, showInfo);
			aShowArray.Append(showInfo);
			rc = sqlite3_step(st);
		}
			
		sqlite3_finalize(st);
	}
	
	}

CShowInfo* CShowEngine::DBGetNextDownload()
	{
	DP("CShowEngine::DBGetNextDownload");
	CShowInfo *showInfo = NULL;
	iSqlBuffer.Format(_L("select url, title, description, filename, position, playtime, playstate, downloadstate, feeduid, shows.uid, showsize, trackno, pubdate, showtype from downloads, shows where downloads.uid=shows.uid order by dl_index limit 1"));

	sqlite3_stmt *st;

	int rc = sqlite3_prepare16_v2(iDB, (const void*)iSqlBuffer.PtrZ() , -1, &st,	(const void**) NULL);
	
	if (rc==SQLITE_OK) {
		rc = sqlite3_step(st);
		if (rc == SQLITE_ROW) {
			TRAPD(err, showInfo = CShowInfo::NewL());
			DBFillShowInfoFromStmt(st, showInfo);
		}
			
		sqlite3_finalize(st);
	}
	
	return showInfo;
	}

void CShowEngine::DBGetShowsByFeed(RShowInfoArray& aShowArray, TUint aFeedUid)
	{
	DP1("CShowEngine::DBGetShowsByFeed, feedUid=%u", aFeedUid);
	CShowInfo *showInfo = NULL;
	iSqlBuffer.Format(_L("select url, title, description, filename, position, playtime, playstate, downloadstate, feeduid, uid, showsize, trackno, pubdate, showtype from shows where feeduid=%u"), aFeedUid);

	if (iPodcastModel.SettingsEngine().SelectUnplayedOnly()) {
		iSqlBuffer.Append(_L(" and playstate=0"));
	}
	
	iSqlBuffer.Append(_L(" order by pubdate desc"));
	
	sqlite3_stmt *st;

	int rc = sqlite3_prepare16_v2(iDB, (const void*)iSqlBuffer.PtrZ() , -1, &st,	(const void**) NULL);
	
	if (rc==SQLITE_OK) {
		rc = sqlite3_step(st);
		while (rc == SQLITE_ROW) {
			TRAPD(err, showInfo = CShowInfo::NewL());
			DBFillShowInfoFromStmt(st, showInfo);
			aShowArray.Append(showInfo);
			rc = sqlite3_step(st);
		}
			
		sqlite3_finalize(st);
	}
	
}

TUint CShowEngine::DBGetDownloadsCount()
	{
	DP("CShowEngine::DBGetDownloadsCount");

	
	iSqlBuffer.Format(_L("select count(*) from downloads"));

	sqlite3_stmt *st;
	TUint count = 0;

	int rc = sqlite3_prepare16_v2(iDB, (const void*)iSqlBuffer.PtrZ() , -1, &st,	(const void**) NULL);
	
	if( rc==SQLITE_OK ){
	  	rc = sqlite3_step(st);
	  	
	  	if (rc == SQLITE_ROW) {
	  		count = sqlite3_column_int(st, 0);
	  	}
	}
		  
	sqlite3_finalize(st);

	return count;
}


void CShowEngine::DBGetDownloadedShows(RShowInfoArray& aShowArray)
	{
	DP("CShowEngine::DBGetDownloadedShows");
	CShowInfo *showInfo = NULL;
	iSqlBuffer.Format(_L("select url, title, description, filename, position, playtime, playstate, downloadstate, feeduid, uid, showsize, trackno, pubdate, showtype from shows where downloadstate=%u or showtype=%u"), EDownloaded, EAudioBook);

	if (iPodcastModel.SettingsEngine().SelectUnplayedOnly()) {
		iSqlBuffer.Append(_L(" and playstate=0"));
	}
	
	sqlite3_stmt *st;
	 
	//DP1("SQL: %S", &iSqlBuffer.Left(KSqlDPLen));

	int rc = sqlite3_prepare16_v2(iDB, (const void*)iSqlBuffer.PtrZ() , -1, &st,	(const void**) NULL);
	
	if (rc==SQLITE_OK) {
		rc = sqlite3_step(st);
		if (rc == SQLITE_ROW) {
			TRAPD(err, showInfo = CShowInfo::NewL());
			DBFillShowInfoFromStmt(st, showInfo);
			aShowArray.Append(showInfo);
		}
			
		sqlite3_finalize(st);
	}
	
}
void CShowEngine::DBGetNewShows(RShowInfoArray& aShowArray)
	{
	DP("CShowEngine::DBGetNewShows");
	CShowInfo *showInfo = NULL;
	iSqlBuffer.Format(_L("select url, title, description, filename, position, playtime, playstate, downloadstate, feeduid, uid, showsize, trackno, pubdate, showtype from shows where playstate=%u"), ENeverPlayed);
	
	sqlite3_stmt *st;
	 
	//DP1("SQL: %S", &iSqlBuffer.Left(KSqlDPLen));

	int rc = sqlite3_prepare16_v2(iDB, (const void*)iSqlBuffer.PtrZ() , -1, &st,	(const void**) NULL);
	
	if (rc==SQLITE_OK) {
		rc = sqlite3_step(st);
		if (rc == SQLITE_ROW) {
			TRAPD(err, showInfo = CShowInfo::NewL());
			DBFillShowInfoFromStmt(st, showInfo);
			aShowArray.Append(showInfo);
		}
			
		sqlite3_finalize(st);
	}
	
}


void CShowEngine::DBFillShowInfoFromStmt(sqlite3_stmt *st, CShowInfo* showInfo)
{
	const void *urlz = sqlite3_column_text16(st, 0);
	TPtrC16 url((const TUint16*)urlz);
	showInfo->SetUrlL(url);

	const void *titlez = sqlite3_column_text16(st, 1);
	TPtrC16 title((const TUint16*)titlez);
	showInfo->SetTitleL(title);

	const void *descz = sqlite3_column_text16(st, 2);
	TPtrC16 desc((const TUint16*)descz);
	showInfo->SetDescriptionL(desc);

	const void *filez = sqlite3_column_text16(st, 3);
	TPtrC16 file((const TUint16*)filez);
	showInfo->SetFileNameL(file);

	sqlite3_int64 pos = sqlite3_column_int64(st, 4);
	TTimeIntervalMicroSeconds position(pos);
	showInfo->SetPosition(position);
	
	TUint playtime = sqlite3_column_int(st, 5);
	showInfo->SetPlayTime(playtime);
	
	TUint playstate = sqlite3_column_int(st, 6);
	showInfo->SetPlayState((TPlayState)playstate);
	
	TUint downloadstate = sqlite3_column_int(st, 7);
	showInfo->SetDownloadState((TDownloadState)downloadstate);
				
	TUint feeduid = sqlite3_column_int(st, 8);
	showInfo->SetFeedUid(feeduid);
							
	TUint uid = sqlite3_column_int(st, 9);
	showInfo->SetUid(uid);

	TUint showsize = sqlite3_column_int(st, 10);
	showInfo->SetShowSize(showsize);

	TUint trackno = sqlite3_column_int(st, 11);
	showInfo->SetTrackNo((TShowType)trackno);
	
	sqlite3_int64 pubdate = sqlite3_column_int64(st, 12);
	TTime timepubdate(pubdate);
	showInfo->SetPubDate(timepubdate);
	
	TUint showtype = sqlite3_column_int(st, 13);
	showInfo->SetShowType((TShowType)showtype);
}

TBool CShowEngine::DBAddShow(CShowInfo *aItem)
	{
	DP2("CShowEngine::DBAddShow, title=%S, URL=%S", &aItem->Title(), &aItem->Url());

	iSqlBuffer.Format(_L("insert into shows (url, title, description, filename, position, playtime, playstate, downloadstate, feeduid, uid, showsize, trackno, pubdate, showtype)"
			" values (\"%S\",\"%S\", \"%S\", \"%S\", \"%Lu\", \"%u\", \"%u\", \"%u\", \"%u\", \"%u\", \"%u\", \"%u\", \"%Lu\", \"%u\")"),
			&aItem->Url(), &aItem->Title(), &aItem->Description(), &aItem->FileName(), aItem->Position().Int64(), aItem->PlayTime(),
			aItem->PlayState(), aItem->DownloadState(), aItem->FeedUid(), aItem->Uid(), aItem->ShowSize(), aItem->TrackNo(), aItem->PubDate().Int64(), aItem->ShowType());

	sqlite3_stmt *st;

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

void CShowEngine::DBAddDownload(TUint aUid)
	{
	DP1("CShowEngine::DBAddDownload, aUid=%u", aUid);

	iSqlBuffer.Format(_L("insert into downloads (uid) values (%u)"), aUid);
	sqlite3_stmt *st;
	 
	int rc = sqlite3_prepare16_v2(iDB, (const void*)iSqlBuffer.PtrZ() , -1, &st,	(const void**) NULL);
	
	if (rc==SQLITE_OK)
		{
		rc = sqlite3_step(st);
		}
	
	sqlite3_finalize(st);

	}

TBool CShowEngine::DBUpdateShow(CShowInfo *aItem)
	{
	DP2("CShowEngine::DBUpdateShow, title=%S, URL=%S", &aItem->Title(), &aItem->Url());

	iSqlBuffer.Format(_L("update shows set url=\"%S\", title=\"%S\", description=\"%S\", filename=\"%S\", position=\"%Lu\"," \
			"playtime=\"%u\", playstate=\"%u\", downloadstate=\"%u\", feeduid=\"%u\", showsize=\"%u\", trackno=\"%u\"," \
			"pubdate=\"%Lu\", showtype=\"%u\" where uid=\"%u\""),
			&aItem->Url(), &aItem->Title(), &aItem->Description(), &aItem->FileName(), aItem->Position().Int64(), aItem->PlayTime(),
			aItem->PlayState(), aItem->DownloadState(), aItem->FeedUid(), aItem->ShowSize(), aItem->TrackNo(), aItem->PubDate(), 
			aItem->ShowType(),aItem->Uid());

	sqlite3_stmt *st;
	 
	//DP1("SQL: %S", &iSqlBuffer.Left(KSqlDPLen));
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

TBool CShowEngine::DBDeleteShow(TUint aUid)
	{
	DP("CShowEngine::DBDeleteShow");

	iSqlBuffer.Format(_L("delete from shows where uid=%u"), aUid);

	sqlite3_stmt *st;
	 
	//DP1("SQL: %S", &iSqlBuffer.Left(KSqlDPLen));
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

TBool CShowEngine::DBDeleteAllShowsByFeed(TUint aFeedUid)
	{
	DP("CShowEngine::DBDeleteAllShowsByFeed");

	iSqlBuffer.Format(_L("delete from shows where feeduid=%u"), aFeedUid);

	sqlite3_stmt *st;
	 
	//DP1("SQL: %S", &iSqlBuffer.Left(KSqlDPLen));
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

void CShowEngine::DBRemoveAllDownloads()
	{
	DP("CShowEngine::DBRemoveAllDownloads");

	iSqlBuffer.Format(_L("delete from downloads"));

	sqlite3_stmt *st;
	 
	int rc = sqlite3_prepare16_v2(iDB, (const void*)iSqlBuffer.PtrZ() , -1, &st,	(const void**) NULL);
	
	if (rc==SQLITE_OK)
		{
		rc = sqlite3_step(st);
		sqlite3_finalize(st);
	} 
	}

void CShowEngine::DBRemoveDownload(TUint aUid)
	{
	DP("CShowEngine::DBRemoveDownload");

	iSqlBuffer.Format(_L("delete from downloads where uid=%u"), aUid);

	sqlite3_stmt *st;
	 
	int rc = sqlite3_prepare16_v2(iDB, (const void*)iSqlBuffer.PtrZ() , -1, &st,	(const void**) NULL);
	
	if (rc==SQLITE_OK)
		{
		rc = sqlite3_step(st);
		sqlite3_finalize(st);
	} 
	}

CShowInfo* CShowEngine::GetNextShowByTrackL(CShowInfo* aShowInfo)
	{
	CShowInfo* nextShow = NULL;
	RShowInfoArray array;
	DBGetShowsByFeed(array, aShowInfo->FeedUid());
	TUint diff = KMaxTInt;
	for(TInt loop = 0; loop<array.Count(); loop++)
		{
			if(aShowInfo->TrackNo() < array[loop]->TrackNo())
				{
				if((array[loop]->TrackNo() - aShowInfo->TrackNo()) < diff)
					{
					diff = array[loop]->TrackNo() - aShowInfo->TrackNo();
					nextShow = array[loop];
					}
				}
		}

	return nextShow;
	}


TBool CShowEngine::CompareShowsByUid(const CShowInfo &a, const CShowInfo &b)
{
	return a.Uid() == b.Uid();
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
	/*for (TInt i=0;i<iSelectedShows.Count();i++)
		{
		if (iSelectedShows[i]->PlayState() == EPlayed && iSelectedShows[i]->FileName().Length() > 0) {
			if (iPodcastModel.PlayingPodcast() == iSelectedShows[i] && iPodcastModel.SoundEngine().State() != ESoundEngineNotInitialized) {
				iPodcastModel.SoundEngine().Stop();
			}
			BaflUtils::DeleteFile(iFs, iSelectedShows[i]->FileName());
			iSelectedShows[i]->SetDownloadState(ENotDownloaded);
			}
		}
	UpdateSelectedShows();*/
	}

void CShowEngine::DeleteAllShowsByFeed(TUint aFeedUid, TBool aDeleteFiles)
	{
	RShowInfoArray array;
	DBGetShowsByFeed(array, aFeedUid);
	
	const TInt count = array.Count();
	
	for (TInt i=count-1 ; i >= 0; i--)
		{
		if (array[i]->FileName().Length() > 0) 
			{
			if (aDeleteFiles) {
				BaflUtils::DeleteFile(iFs, array[i]->FileName());
			}
			}
		}
	array.ResetAndDestroy();
	DBDeleteAllShowsByFeed(aFeedUid);
	}

void CShowEngine::DeleteShow(TUint aShowUid, TBool aRemoveFile)
	{
	
	CShowInfo *info = DBGetShowByUid(aShowUid);
	
	if (info != NULL) {
		if (info->FileName().Length() > 0 && aRemoveFile) 
				{
				BaflUtils::DeleteFile(iFs, info->FileName());			
				}
		}
	
	DBDeleteShow(aShowUid);
	delete info;
	}

TUint CShowEngine::GetGrossSelectionLength()
	{
	/*if (iPodcastModel.SettingsEngine().SelectUnplayedOnly()) {
		TUint count=0;
		for (int i=0;i<iSelectedShows.Count();i++) {
			if (iSelectedShows[i]->PlayState() == ENeverPlayed) {
				count++;
			}
		}
	} else {
		return iSelectedShows.Count();
	}*/
	return 0;
	}
/*
void CShowEngine::SelectShowsByFeed(TUint aFeedUid)
	{
	DP2("SelectShowsByFeed: %u, unplayed only=%d", aFeedUid, iPodcastModel.SettingsEngine().SelectUnplayedOnly());
	/*ResetSelection();

	DBGetShowsByFeed(iSelectedShows, aFeedUid);
		
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
		/*TInt count = iSelectedShows.Count();
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
	DP("CShowEngine::SelectNewShows");
	//DBGetNewShows(iSelectedShows);
	}

void CShowEngine::SelectShowsDownloaded()
	{
	DP("CShowEngine::SelectShowsDownloaded");
	//DBGetDownloadedShows(iSelectedShows);
	}


void CShowEngine::SelectShowsDownloading()
	{
	DP("CShowEngine::SelectShowsDownloading");
	
	const TInt count = iShowsDownloading.Count();
	for (TInt i=0 ; i < count ;i++) 
		{
	//	iSelectedShows.Append(iShowsDownloading[i]);
		}
	}*/

void CShowEngine::GetShowsByFeed(RShowInfoArray& aShowArray, TUint aFeedUid)
	{
	DP("CShowEngine::GetShowsByFeed");
	DBGetShowsByFeed(aShowArray, aFeedUid);
	}

void CShowEngine::GetAllShows(RShowInfoArray &aArray)
	{
	DP("CShowEngine::GetAllShows");
	DBGetAllShows(aArray);
	}

void CShowEngine::GetShowsDownloaded(RShowInfoArray &aArray)
	{
	DP("CShowEngine::GetShowsDownloaded");
	DBGetDownloadedShows(aArray);
	}

void CShowEngine::GetNewShows(RShowInfoArray &aArray)
	{
	DP("CShowEngine::GetNewShows");
	DBGetNewShows(aArray);
	}

void CShowEngine::GetShowsDownloading(RShowInfoArray &aArray)
	{
	DP("CShowEngine::GetShowsDownloading");
	DBGetAllDownloads(aArray);
	}

TInt CShowEngine::GetNumDownloadingShowsL() 
	{
	return (const TInt) DBGetDownloadsCount();
	}

void CShowEngine::AddDownload(CShowInfo *info)
	{
	DBAddDownload(info->Uid());
	DownloadNextShow();
	}


void CShowEngine::DownloadNextShow()
	{
	// Check if we have anything in the download queue
	const TInt count = DBGetDownloadsCount();
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
			CShowInfo *info = DBGetNextDownload();
			if (info != NULL) {
				DP1("CShowEngine::DownloadNextShow\tDownloading: %S", &(info->Title()));
				info->SetDownloadState(EDownloading);
				iShowDownloading = info;
				TRAPD(error,GetShowL(info));
				if (error != KErrNone) {
					iDownloadsSuspended = ETrue;
				}
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
		TRAP_IGNORE(iObservers[i]->DownloadQueueUpdated(1, DBGetDownloadsCount() - 1));
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

void CShowEngine::SetShowPlayState(CShowInfo* aShowInfo, TPlayState aPlayState)
	{
	aShowInfo->SetPlayState(aPlayState);
	DBUpdateShow(aShowInfo);
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
/*
			TBool exists = EFalse;
			for (TInt i=0;i<iSelectedShows.Count();i++) {
				if (iSelectedShows[i]->FileName().Compare(pathName) == 0) {
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
			
			iMetaDataReader->SubmitShowL(info);
			iSelectedShows.Append(info);
			DBAddShow(info);*/
		}
	}
	delete dirPtr;
	CleanupStack::PopAndDestroy(dirScan);
}

void CShowEngine::CheckFilesL()
	{
	// check to see if any downloaded files were removed
	//SelectShowsDownloaded();
	/*for (TInt i=0;i<iSelectedShows.Count();i++) {
		if(!BaflUtils::FileExists(iFs, iSelectedShows[i]->FileName())) {
			DP1("%S was removed, delisting", &iSelectedShows[i]->FileName()); 
			iSelectedShows[i]->SetDownloadState(ENotDownloaded);
		}
	}*/

	// check if any new files were added
	ListDirL(iPodcastModel.SettingsEngine().BaseDir());	

	NotifyShowListUpdated();
}

void CShowEngine::ReadMetaData(CShowInfo *aShowInfo)
	{
	//DP1("Read %S", &(aShowInfo->Title()));
	DBUpdateShow(aShowInfo);
	}

void CShowEngine::ReadMetaDataComplete()
	{
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
