#include "ShowEngine.h"
#include "FeedEngine.h"
#include "FeedInfo.h"
#include <bautils.h>
#include <s32file.h>
#include "SettingsEngine.h"
#include <e32hashtab.h>
#include "SoundEngine.h"
#include "debug.h"

const TUint KMaxDownloadErrors = 3;
const TInt KMimeBufLength = 100;

CShowEngine::CShowEngine(CPodcastModel& aPodcastModel) :
	iPodcastModel(aPodcastModel)
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

EXPORT_C void CShowEngine::GetMimeType(const TDesC& aFileName, TDes& aMimeType)
	{
	aMimeType.Zero();
	RFile file;
	if (file.Open(iFs, aFileName, 0) == KErrNone)
		{
		if (file.Read(iRecogBuffer) == KErrNone)
			{
			file.Close();
			TDataRecognitionResult result;
			if (iApaSession.RecognizeData(aFileName, iRecogBuffer, result)
					== KErrNone)
				{
				aMimeType.Copy(result.iDataType.Des());
				}

			}
		}
	file.Close();
	}

void CShowEngine::ConstructL()
	{
	User::LeaveIfError(iFs.Connect());
	iShowClient = CHttpClient::NewL(iPodcastModel, *this);
	iShowClient->SetResumeEnabled(ETrue);
	iMetaDataReader = new (ELeave) CMetaDataReader(*this, iPodcastModel.FsSession());
	iMetaDataReader->ConstructL();
	User::LeaveIfError(iApaSession.Connect());
	CheckFilesL();
	DownloadNextShowL();
	}

EXPORT_C void CShowEngine::StopDownloads()
	{
	DP("StopDownloads");
	iDownloadsSuspended = ETrue;
	if (iShowClient != NULL)
		{
		iShowClient->Stop();
		}
	}

EXPORT_C void CShowEngine::ResumeDownloadsL()
	{
	DP("ResumeDownloads");
	if (iDownloadsSuspended)
		{
		iDownloadsSuspended = EFalse;
		iDownloadErrors = 0;
		DownloadNextShowL();
		}
	}

EXPORT_C TBool CShowEngine::DownloadsStopped()
	{
	return iDownloadsSuspended;
	}

EXPORT_C void CShowEngine::RemoveAllDownloads()
	{
	if (!iDownloadsSuspended)
		{
		return;
		}

	DBRemoveAllDownloads();
	}

EXPORT_C TBool CShowEngine::RemoveDownloadL(TUint aUid)
	{
	DP("CShowEngine::RemoveDownload\t Trying to remove download");

	TBool retVal = EFalse;
	// if trying to remove the present download, we first stop it
	if (!iDownloadsSuspended && iShowDownloading != NULL
			&& iShowDownloading->Uid() == aUid)
		{
		DP("CShowEngine::RemoveDownload\t This is the active download, we suspend downloading");
		StopDownloads();
		}
	else
		{
		CShowInfo *info = DBGetShowByUidL(aUid);
		if (info != NULL)
			{
			info->SetDownloadState(ENotDownloaded);
			DBUpdateShow(info);
			delete info;
			}
		DBRemoveDownload(aUid);

		// partial downloads should be removed
		if (iShowDownloading)
			{
			BaflUtils::DeleteFile(iFs, iShowDownloading->FileName());
			}

		NotifyShowDownloadUpdated(-1, -1, -1);
		NotifyDownloadQueueUpdated();
		DownloadNextShowL();
		retVal = ETrue;
		}
	return retVal;
	}

void CShowEngine::Connected(CHttpClient* /*aClient*/)
	{

	}

void CShowEngine::Progress(CHttpClient* /*aHttpClient */, TInt aBytes,
		TInt aTotalBytes)
	{
	TInt percent = -1;
	if (aTotalBytes > 0)
		{
		percent = (TInt) ((float) aBytes * 100.0 / (float) aTotalBytes);
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
	if (aHttpClient == iShowClient && iShowDownloading != NULL && aTotalBytes
			!= -1)
		{
		iShowDownloading->SetShowSize(aTotalBytes);
		}
	}

void CShowEngine::GetShowL(CShowInfo *info)
	{
	CFeedInfo *feedInfo = iPodcastModel.FeedEngine().GetFeedInfoByUid(
			info->FeedUid());
	if (feedInfo == NULL)
		{
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

TBool CShowEngine::AddShowL(CShowInfo *item)
	{
	CShowInfo *showInfo = DBGetShowByUidL(item->Uid());

	if (showInfo == NULL)
		{
		DBAddShow(item);
		}
	else
		{
		delete showInfo;
		return EFalse;
		}

	return ETrue;
	}

EXPORT_C void CShowEngine::AddObserver(MShowEngineObserver *observer)
	{
	iObservers.Append(observer);
	}

EXPORT_C void CShowEngine::RemoveObserver(MShowEngineObserver *observer)
	{
	TInt index = iObservers.Find(observer);

	if (index > KErrNotFound)
		{
		iObservers.Remove(index);
		}
	}

void CShowEngine::CompleteL(CHttpClient* /*aHttpClient*/, TBool aSuccessful)
	{
	if (iShowDownloading != NULL)
		{
		DP1("CShowEngine::Complete\tDownload of file: %S is complete", &iShowDownloading->FileName());

		// decide what kind of file this is
		TBuf<KMimeBufLength> mimeType;
		GetMimeType(iShowDownloading->FileName(), mimeType);

		if (mimeType.Left(5) == _L("audio"))
			{
			iShowDownloading->SetShowType(EAudioPodcast);
			}
		else if (mimeType.Left(5) == _L("video"))
			{
			iShowDownloading->SetShowType(EVideoPodcast);
			}

		if (aSuccessful)
			{
			iShowDownloading->SetDownloadState(EDownloaded);
			DBUpdateShow(iShowDownloading);
			DBRemoveDownload(iShowDownloading->Uid());

			NotifyShowDownloadUpdated(100, 0, 1);

			delete iShowDownloading;
			}
		else
			{
			iShowDownloading->SetDownloadState(EQueued);
			DBUpdateShow(iShowDownloading);
			iDownloadErrors++;
			if (iDownloadErrors > KMaxDownloadErrors)
				{
				DP("Too many downloading errors, suspending downloads");
				iDownloadsSuspended = ETrue;
				NotifyShowDownloadUpdated(-1, -1, -1);
				}
			}
		}
	DownloadNextShowL();
	}

EXPORT_C CShowInfo* CShowEngine::ShowDownloading()
	{
	return iShowDownloading;
	}

EXPORT_C CShowInfo* CShowEngine::GetShowByUidL(TUint aShowUid)
	{
	return DBGetShowByUidL(aShowUid);
	}

CShowInfo* CShowEngine::DBGetShowByUidL(TUint aUid)
	{
	DP("CShowEngine::DBGetShowByUid");
	CShowInfo *showInfo = NULL;
	iSqlBuffer.Format(
			_L("select url, title, description, filename, position, playtime, playstate, downloadstate, feeduid, uid, showsize, trackno, pubdate, showtype from shows where uid=%u"), aUid);

	sqlite3_stmt *st;

	//DP1("SQL: %S", &iSqlBuffer.Left(KSqlDPLen));

	int rc = sqlite3_prepare16_v2(iDB, (const void*) iSqlBuffer.PtrZ(), -1,
			&st, (const void**) NULL);

	if (rc == SQLITE_OK)
		{
		rc = sqlite3_step(st);
		if (rc == SQLITE_ROW)
			{
			TRAPD(err, showInfo = CShowInfo::NewL());
			DBFillShowInfoFromStmtL(st, showInfo);
			}

		sqlite3_finalize(st);
		}

	return showInfo;
	}

CShowInfo* CShowEngine::DBGetShowByFileNameL(TFileName aFileName)
	{
	DP("CShowEngine::DBGetShowByUid");
	CShowInfo *showInfo = NULL;
	iSqlBuffer.Format(
			_L("select url, title, description, filename, position, playtime, playstate, downloadstate, feeduid, uid, showsize, trackno, pubdate, showtype from shows where filename=\"%S\""), &aFileName);

	sqlite3_stmt *st;

	int rc = sqlite3_prepare16_v2(iDB, (const void*) iSqlBuffer.PtrZ(), -1,
			&st, (const void**) NULL);

	if (rc == SQLITE_OK)
		{
		rc = sqlite3_step(st);
		if (rc == SQLITE_ROW)
			{
			TRAPD(err, showInfo = CShowInfo::NewL());
			DBFillShowInfoFromStmtL(st, showInfo);
			}

		sqlite3_finalize(st);
		}

	return showInfo;
	}

void CShowEngine::DBGetAllShowsL(RShowInfoArray& aShowArray)
	{
	DP("CShowEngine::DBGetAllShows");
	CShowInfo *showInfo = NULL;
	iSqlBuffer.Format(
			_L("select url, title, description, filename, position, playtime, playstate, downloadstate, feeduid, uid, showsize, trackno, pubdate, showtype from shows"));

	if (iPodcastModel.SettingsEngine().SelectUnplayedOnly())
		{
		iSqlBuffer.Append(_L(" where playstate=0"));
		}
	//DP1("SQL: %S", &iSqlBuffer.Left(KSqlDPLen));

	sqlite3_stmt *st;

	int rc = sqlite3_prepare16_v2(iDB, (const void*) iSqlBuffer.PtrZ(), -1,
			&st, (const void**) NULL);

	if (rc == SQLITE_OK)
		{
		rc = sqlite3_step(st);
		if (rc == SQLITE_ROW)
			{
			TRAPD(err, showInfo = CShowInfo::NewL());
			DBFillShowInfoFromStmtL(st, showInfo);
			aShowArray.Append(showInfo);
			}

		sqlite3_finalize(st);
		}

	}

void CShowEngine::DBGetAllDownloadsL(RShowInfoArray& aShowArray)
	{
	DP("CShowEngine::DBGetAllDownloads");
	CShowInfo *showInfo = NULL;
	iSqlBuffer.Format(
			_L("select url, title, description, filename, position, playtime, playstate, downloadstate, feeduid, shows.uid, showsize, trackno, pubdate, showtype from downloads, shows where downloads.uid=shows.uid"));

#ifndef DONT_SORT_SQL
	iSqlBuffer.Append(_L(" order by dl_index"));
#endif
	sqlite3_stmt *st;

	int rc = sqlite3_prepare16_v2(iDB, (const void*) iSqlBuffer.PtrZ(), -1,
			&st, (const void**) NULL);

	if (rc == SQLITE_OK)
		{
		rc = sqlite3_step(st);
		while (rc == SQLITE_ROW)
			{
			TRAPD(err, showInfo = CShowInfo::NewL());
			DBFillShowInfoFromStmtL(st, showInfo);
			aShowArray.Append(showInfo);
			rc = sqlite3_step(st);
			}

		sqlite3_finalize(st);
		}

	// delete downloads that don't have a show
	
	iSqlBuffer.Format(_L("delete from downloads where uid not in (select downloads.uid from shows, downloads where shows.uid=downloads.uid)"));

	rc = sqlite3_prepare16_v2(iDB, (const void*) iSqlBuffer.PtrZ(), -1,	&st, (const void**) NULL);

	if (rc == SQLITE_OK)
		{
		rc = sqlite3_step(st);
		sqlite3_finalize(st);
		}

	}

CShowInfo* CShowEngine::DBGetNextDownloadL()
	{
	DP("CShowEngine::DBGetNextDownload");
	CShowInfo *showInfo = NULL;
	iSqlBuffer.Format(
			_L("select url, title, description, filename, position, playtime, playstate, downloadstate, feeduid, shows.uid, showsize, trackno, pubdate, showtype from downloads, shows where downloads.uid=shows.uid"));

#ifndef DONT_SORT_SQL
	iSqlBuffer.Append(_L(" limit 1"));
#else
	iSqlBuffer.Append(_L(" order by dl_index limit 1"));
#endif

	sqlite3_stmt *st;

	int rc = sqlite3_prepare16_v2(iDB, (const void*) iSqlBuffer.PtrZ(), -1,
			&st, (const void**) NULL);

	if (rc == SQLITE_OK)
		{
		rc = sqlite3_step(st);
		if (rc == SQLITE_ROW)
			{
			TRAPD(err, showInfo = CShowInfo::NewL());
			DBFillShowInfoFromStmtL(st, showInfo);
			}

		sqlite3_finalize(st);
		}

	return showInfo;
	}

void CShowEngine::DBGetShowsByFeedL(RShowInfoArray& aShowArray, TUint aFeedUid)
	{
	DP1("CShowEngine::DBGetShowsByFeed, feedUid=%u", aFeedUid);
	CShowInfo *showInfo = NULL;
	iSqlBuffer.Format(
			_L("select url, title, description, filename, position, playtime, playstate, downloadstate, feeduid, uid, showsize, trackno, pubdate, showtype from shows where feeduid=%u"), aFeedUid);

	if (iPodcastModel.SettingsEngine().SelectUnplayedOnly())
		{
		iSqlBuffer.Append(_L(" and playstate=0"));
		}

#ifndef DONT_SORT_SQL
	CFeedInfo *feedInfo = iPodcastModel.FeedEngine().GetFeedInfoByUid(aFeedUid);
	if (feedInfo->IsBookFeed())
		{
		iSqlBuffer.Append(_L(" order by trackno"));
		}
	else
		{
		iSqlBuffer.Append(_L(" order by pubdate desc"));
		}
#endif

/*	TBuf<100> limit;
	limit.Format(_L(" limit %u"), iPodcastModel.SettingsEngine().MaxListItems());
	iSqlBuffer.Append(limit);
*/
	sqlite3_stmt *st;

	int rc = sqlite3_prepare16_v2(iDB, (const void*) iSqlBuffer.PtrZ(), -1,
			&st, (const void**) NULL);

	if (rc == SQLITE_OK)
		{
		rc = sqlite3_step(st);
		while (rc == SQLITE_ROW)
			{
			TRAPD(err, showInfo = CShowInfo::NewL());
			DBFillShowInfoFromStmtL(st, showInfo);
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

	int rc = sqlite3_prepare16_v2(iDB, (const void*) iSqlBuffer.PtrZ(), -1,
			&st, (const void**) NULL);

	if (rc == SQLITE_OK)
		{
		rc = sqlite3_step(st);

		if (rc == SQLITE_ROW)
			{
			count = sqlite3_column_int(st, 0);
			}
		}

	sqlite3_finalize(st);

	return count;
	}

void CShowEngine::DBGetDownloadedShowsL(RShowInfoArray& aShowArray)
	{
	DP("CShowEngine::DBGetDownloadedShows");
	CShowInfo *showInfo = NULL;
	iSqlBuffer.Format(
			_L("select url, title, description, filename, position, playtime, playstate, downloadstate, feeduid, uid, showsize, trackno, pubdate, showtype from shows where downloadstate=%u and showtype!=%u"), EDownloaded,
			EAudioBook);

	if (iPodcastModel.SettingsEngine().SelectUnplayedOnly())
		{
		iSqlBuffer.Append(_L(" and playstate=0"));
		}

#ifndef DONT_SORT_SQL
	iSqlBuffer.Append(_L(" order by title"));
#endif	

	sqlite3_stmt *st;

	int rc = sqlite3_prepare16_v2(iDB, (const void*) iSqlBuffer.PtrZ(), -1,
			&st, (const void**) NULL);

	if (rc == SQLITE_OK)
		{
		rc = sqlite3_step(st);
		while (rc == SQLITE_ROW)
			{
			TRAPD(err, showInfo = CShowInfo::NewL());
			DBFillShowInfoFromStmtL(st, showInfo);
			aShowArray.Append(showInfo);
			rc = sqlite3_step(st);
			}

		sqlite3_finalize(st);
		}

	}

void CShowEngine::DBGetNewShowsL(RShowInfoArray& aShowArray)
	{
	DP("CShowEngine::DBGetNewShows");
	CShowInfo *showInfo = NULL;
	iSqlBuffer.Format(
			_L("select url, title, description, filename, position, playtime, playstate, downloadstate, feeduid, uid, showsize, trackno, pubdate, showtype from shows where playstate=%u"), ENeverPlayed);

	sqlite3_stmt *st;

	int rc = sqlite3_prepare16_v2(iDB, (const void*) iSqlBuffer.PtrZ(), -1,
			&st, (const void**) NULL);

	if (rc == SQLITE_OK)
		{
		rc = sqlite3_step(st);
		if (rc == SQLITE_ROW)
			{
			TRAPD(err, showInfo = CShowInfo::NewL());
			DBFillShowInfoFromStmtL(st, showInfo);
			aShowArray.Append(showInfo);
			}

		sqlite3_finalize(st);
		}

	}

void CShowEngine::DeleteOldShowsByFeed(TUint aFeedUid)
	{
	DP("CShowEngine::DeleteOldShows");
	
	// what we do:
	// 1. sort shows by pubdate
	// 2. select the first MaxListItems shows
	// 3. delete the rest if downloadstate is ENotDownloaded
	
	CShowInfo *showInfo = NULL;
	iSqlBuffer.Format(_L("delete from shows where feeduid=%u and downloadstate=0 and uid not in " \
			"(select uid from shows where feeduid=%u order by pubdate desc limit %u)"), 
			aFeedUid, aFeedUid, iPodcastModel.SettingsEngine().MaxListItems());

	sqlite3_stmt *st;

	int rc = sqlite3_prepare16_v2(iDB, (const void*) iSqlBuffer.PtrZ(), -1,
			&st, (const void**) NULL);

	if (rc == SQLITE_OK)
		{
		rc = sqlite3_step(st);
		sqlite3_finalize(st);
		}
	
	iSqlBuffer.Format(_L("delete from downloads where uid not in (select downloads.uid from shows, downloads where shows.uid=downloads.uid)"));

	rc = sqlite3_prepare16_v2(iDB, (const void*) iSqlBuffer.PtrZ(), -1,	&st, (const void**) NULL);

	if (rc == SQLITE_OK)
		{
		rc = sqlite3_step(st);
		sqlite3_finalize(st);
		}

	}

void CShowEngine::DBFillShowInfoFromStmtL(sqlite3_stmt *st, CShowInfo* showInfo)
	{
	const void *urlz = sqlite3_column_text16(st, 0);
	TPtrC16 url((const TUint16*) urlz);
	showInfo->SetUrlL(url);

	const void *titlez = sqlite3_column_text16(st, 1);
	TPtrC16 title((const TUint16*) titlez);
	showInfo->SetTitleL(title);

	const void *descz = sqlite3_column_text16(st, 2);
	TPtrC16 desc((const TUint16*) descz);
	showInfo->SetDescriptionL(desc);

	const void *filez = sqlite3_column_text16(st, 3);
	TPtrC16 file((const TUint16*) filez);
	showInfo->SetFileNameL(file);

	sqlite3_int64 pos = sqlite3_column_int64(st, 4);
	TTimeIntervalMicroSeconds position(pos);
	showInfo->SetPosition(position);

	TUint playtime = sqlite3_column_int(st, 5);
	showInfo->SetPlayTime(playtime);

	TUint playstate = sqlite3_column_int(st, 6);
	showInfo->SetPlayState((TPlayState) playstate);

	TUint downloadstate = sqlite3_column_int(st, 7);
	showInfo->SetDownloadState((TDownloadState) downloadstate);

	TUint feeduid = sqlite3_column_int(st, 8);
	showInfo->SetFeedUid(feeduid);

	TUint uid = sqlite3_column_int(st, 9);
	showInfo->SetUid(uid);

	TUint showsize = sqlite3_column_int(st, 10);
	showInfo->SetShowSize(showsize);

	TUint trackno = sqlite3_column_int(st, 11);
	showInfo->SetTrackNo((TShowType) trackno);

	sqlite3_int64 pubdate = sqlite3_column_int64(st, 12);
	TTime timepubdate(pubdate);
	showInfo->SetPubDate(timepubdate);

	TUint showtype = sqlite3_column_int(st, 13);
	showInfo->SetShowType((TShowType) showtype);
	}

TBool CShowEngine::DBAddShow(CShowInfo *aItem)
	{
	DP2("CShowEngine::DBAddShow, title=%S, URL=%S", &aItem->Title(), &aItem->Url());

	iSqlBuffer.Format(
			_L("insert into shows (url, title, description, filename, position, playtime, playstate, downloadstate, feeduid, uid, showsize, trackno, pubdate, showtype)"
					" values (\"%S\",\"%S\", \"%S\", \"%S\", \"%Lu\", \"%u\", \"%u\", \"%u\", \"%u\", \"%u\", \"%u\", \"%u\", \"%Lu\", \"%u\")"),
			&aItem->Url(), &aItem->Title(), &aItem->Description(),
			&aItem->FileName(), aItem->Position().Int64(), aItem->PlayTime(),
			aItem->PlayState(), aItem->DownloadState(), aItem->FeedUid(),
			aItem->Uid(), aItem->ShowSize(), aItem->TrackNo(),
			aItem->PubDate().Int64(), aItem->ShowType());

	sqlite3_stmt *st;

	int rc = sqlite3_prepare16_v2(iDB, (const void*) iSqlBuffer.PtrZ(), -1,
			&st, (const void**) NULL);
	if (rc == SQLITE_OK)
		{
		rc = sqlite3_step(st);
		if (rc == SQLITE_DONE)
			{
			sqlite3_finalize(st);
			return ETrue;
			}
		else
			{
			sqlite3_finalize(st);
			}
		}
	else
		{
		DP1("SQLite rc=%d", rc);
		}

	return EFalse;

	}

void CShowEngine::DBAddDownload(TUint aUid)
	{
	DP1("CShowEngine::DBAddDownload, aUid=%u", aUid);

	iSqlBuffer.Format(_L("insert into downloads (uid) values (%u)"), aUid);
	sqlite3_stmt *st;

	int rc = sqlite3_prepare16_v2(iDB, (const void*) iSqlBuffer.PtrZ(), -1,
			&st, (const void**) NULL);

	if (rc == SQLITE_OK)
		{
		rc = sqlite3_step(st);
		}

	sqlite3_finalize(st);

	}

TBool CShowEngine::DBUpdateShow(CShowInfo *aItem)
	{
	DP1("CShowEngine::DBUpdateShow, title='%S'", &aItem->Title());

	iSqlBuffer.Format(
			_L("update shows set url=\"%S\", title=\"%S\", description=\"%S\", filename=\"%S\", position=\"%Lu\","
					"playtime=\"%u\", playstate=\"%u\", downloadstate=\"%u\", feeduid=\"%u\", showsize=\"%u\", trackno=\"%u\","
					"pubdate=\"%Lu\", showtype=\"%u\" where uid=\"%u\""),
			&aItem->Url(), &aItem->Title(), &aItem->Description(),
			&aItem->FileName(), aItem->Position().Int64(), aItem->PlayTime(),
			aItem->PlayState(), aItem->DownloadState(), aItem->FeedUid(),
			aItem->ShowSize(), aItem->TrackNo(), aItem->PubDate().Int64(),
			aItem->ShowType(), aItem->Uid());

	sqlite3_stmt *st;

	//DP1("SQL: %S", &iSqlBuffer.Left(KSqlDPLen));
	int rc = sqlite3_prepare16_v2(iDB, (const void*) iSqlBuffer.PtrZ(), -1,
			&st, (const void**) NULL);

	if (rc == SQLITE_OK)
		{
		rc = sqlite3_step(st);

		if (rc == SQLITE_DONE)
			{
			sqlite3_finalize(st);
			return ETrue;
			}
		else
			{
			sqlite3_finalize(st);
			}
		}
	else
		{
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
	int rc = sqlite3_prepare16_v2(iDB, (const void*) iSqlBuffer.PtrZ(), -1,
			&st, (const void**) NULL);

	if (rc == SQLITE_OK)
		{
		rc = sqlite3_step(st);

		if (rc == SQLITE_DONE)
			{
			sqlite3_finalize(st);
			return ETrue;
			}
		else
			{
			sqlite3_finalize(st);
			}
		}
	else
		{
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
	int rc = sqlite3_prepare16_v2(iDB, (const void*) iSqlBuffer.PtrZ(), -1,
			&st, (const void**) NULL);

	if (rc == SQLITE_OK)
		{
		rc = sqlite3_step(st);

		if (rc == SQLITE_DONE)
			{
			sqlite3_finalize(st);
			return ETrue;
			}
		else
			{
			sqlite3_finalize(st);
			}
		}
	else
		{
		DP1("SQLite rc=%d", rc);
		}

	return EFalse;

	}

void CShowEngine::DBRemoveAllDownloads()
	{
	DP("CShowEngine::DBRemoveAllDownloads");

	iSqlBuffer.Format(_L("delete from downloads"));

	sqlite3_stmt *st;

	int rc = sqlite3_prepare16_v2(iDB, (const void*) iSqlBuffer.PtrZ(), -1,
			&st, (const void**) NULL);

	if (rc == SQLITE_OK)
		{
		rc = sqlite3_step(st);
		sqlite3_finalize(st);
		}

	iSqlBuffer.Format(_L("update shows set downloadstate=0 where downloadstate=1"));

	rc = sqlite3_prepare16_v2(iDB, (const void*) iSqlBuffer.PtrZ(), -1, &st,
			(const void**) NULL);

	if (rc == SQLITE_OK)
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

	int rc = sqlite3_prepare16_v2(iDB, (const void*) iSqlBuffer.PtrZ(), -1,
			&st, (const void**) NULL);

	if (rc == SQLITE_OK)
		{
		rc = sqlite3_step(st);
		sqlite3_finalize(st);
		}
	}

CShowInfo* CShowEngine::GetNextShowByTrackL(CShowInfo* aShowInfo)
	{
	CShowInfo* nextShow = NULL;
	RShowInfoArray array;
	DBGetShowsByFeedL(array, aShowInfo->FeedUid());
	TUint diff = KMaxTInt;
	for (TInt loop = 0; loop < array.Count(); loop++)
		{
		if (aShowInfo->TrackNo() < array[loop]->TrackNo())
			{
			if ((array[loop]->TrackNo() - aShowInfo->TrackNo()) < diff)
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

EXPORT_C void CShowEngine::DeletePlayedShows(RShowInfoArray &aShowInfoArray)
	{
	for (TInt i = 0; i < aShowInfoArray.Count(); i++)
		{
		if (aShowInfoArray[i]->PlayState() == EPlayed
				&& aShowInfoArray[i]->FileName().Length() > 0)
			{
			if (iPodcastModel.PlayingPodcast() == aShowInfoArray[i]
					&& iPodcastModel.SoundEngine().State()
							!= ESoundEngineNotInitialized)
				{
				iPodcastModel.SoundEngine().Stop();
				}
			BaflUtils::DeleteFile(iFs, aShowInfoArray[i]->FileName());
			aShowInfoArray[i]->SetDownloadState(ENotDownloaded);
			DBUpdateShow(aShowInfoArray[i]);
			}
		}
	}

EXPORT_C void CShowEngine::DeleteAllShowsByFeedL(TUint aFeedUid, TBool aDeleteFiles)
	{
	RShowInfoArray array;
	DBGetShowsByFeedL(array, aFeedUid);

	const TInt count = array.Count();

	for (TInt i = count - 1; i >= 0; i--)
		{
		if (array[i]->FileName().Length() > 0)
			{
			if (aDeleteFiles)
				{
				BaflUtils::DeleteFile(iFs, array[i]->FileName());
				}
			}
		}
	array.ResetAndDestroy();
	DBDeleteAllShowsByFeed(aFeedUid);
	}

EXPORT_C void CShowEngine::DeleteShowL(TUint aShowUid, TBool aRemoveFile)
	{

	CShowInfo *info = DBGetShowByUidL(aShowUid);

	if (info != NULL)
		{
		if (info->FileName().Length() > 0 && aRemoveFile)
			{
			BaflUtils::DeleteFile(iFs, info->FileName());
			}
		
		info->SetDownloadState(ENotDownloaded);
		DBUpdateShow(info);
		delete info;
		}
	}

void CShowEngine::GetShowsByFeedL(RShowInfoArray& aShowArray, TUint aFeedUid)
	{
	DP("CShowEngine::GetShowsByFeed");
	DBGetShowsByFeedL(aShowArray, aFeedUid);
	}

void CShowEngine::GetAllShowsL(RShowInfoArray &aArray)
	{
	DP("CShowEngine::GetAllShows");
	DBGetAllShowsL(aArray);
	}

void CShowEngine::GetShowsDownloadedL(RShowInfoArray &aArray)
	{
	DP("CShowEngine::GetShowsDownloaded");
	DBGetDownloadedShowsL(aArray);
	}

void CShowEngine::GetNewShowsL(RShowInfoArray &aArray)
	{
	DP("CShowEngine::GetNewShows");
	DBGetNewShowsL(aArray);
	}

void CShowEngine::GetShowsDownloadingL(RShowInfoArray &aArray)
	{
	DP("CShowEngine::GetShowsDownloading");
	DBGetAllDownloadsL(aArray);
	}

EXPORT_C TInt CShowEngine::GetNumDownloadingShowsL()
	{
	return (const TInt) DBGetDownloadsCount();
	}

EXPORT_C void CShowEngine::AddDownloadL(CShowInfo *info)
	{
	info->SetDownloadState(EQueued);
	DBUpdateShow(info);
	DBAddDownload(info->Uid());
	DownloadNextShowL();
	}

void CShowEngine::DownloadNextShowL()
	{
	// Check if we have anything in the download queue
	const TInt count = DBGetDownloadsCount();
	DP("CShowEngine::DownloadNextShow\tTrying to start new download");DP1("CShowEngine::DownloadNextShow\tShows in download queue %d", count);

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
			CShowInfo *info = DBGetNextDownloadL();
			if (info != NULL)
				{
				DP1("CShowEngine::DownloadNextShow\tDownloading: %S", &(info->Title()));
				info->SetDownloadState(EDownloading);
				DBUpdateShow(info);
				iShowDownloading = info;
				TRAPD(error,GetShowL(info));
				if (error != KErrNone)
					{
					iDownloadsSuspended = ETrue;
					}
				}
			}
		}
	else
		{
		iShowDownloading = NULL;DP("CShowEngine::DownloadNextShow\tNothing to download");
		}
	}

void CShowEngine::NotifyDownloadQueueUpdated()
	{
	const TInt count = iObservers.Count();
	for (TInt i = 0; i < count; i++)
		{
			TRAP_IGNORE(iObservers[i]->DownloadQueueUpdated(1, DBGetDownloadsCount() - 1));
		}
	}

void CShowEngine::NotifyShowDownloadUpdated(TInt aPercentOfCurrentDownload,
		TInt aBytesOfCurrentDownload, TInt aBytesTotal)
	{
	const TInt count = iObservers.Count();
	for (TInt i = 0; i < count; i++)
		{
			TRAP_IGNORE(iObservers[i]->ShowDownloadUpdatedL(aPercentOfCurrentDownload, aBytesOfCurrentDownload, aBytesTotal));
		}
	}

void CShowEngine::NotifyShowListUpdated()
	{
	for (TInt i = 0; i < iObservers.Count(); i++)
		{
		iObservers[i]->ShowListUpdated();
		}
	}

void CShowEngine::ListDirL(TFileName &folder)
	{
	CDirScan *dirScan = CDirScan::NewLC(iFs);
	//DP1("Listing dir: %S", &folder);
	BaflUtils::EnsurePathExistsL(iFs, folder);
	dirScan ->SetScanDataL(folder, KEntryAttDir, ESortByName);

	CDir *dirPtr;
	dirScan->NextL(dirPtr);
	for (TInt i = 0; i < dirPtr->Count(); i++)
		{
		const TEntry &entry = (TEntry) (*dirPtr)[i];
		if (entry.IsDir())
			{
			TFileName subFolder;
			subFolder.Copy(folder);
			subFolder.Append(entry.iName);
			subFolder.Append(_L("\\"));
			ListDirL(subFolder);
			}
		else
			{
			TFileName fileName;
			fileName.Copy(entry.iName);
			TFileName pathName;
			pathName.Copy(folder);
			pathName.Append(entry.iName);

			TShowType showType;

			// decide what kind of file this is
			TBuf<KMimeBufLength> mimeType;
			GetMimeType(pathName, mimeType);DP2("'%S' has mime: '%S'", &pathName, &mimeType);
#ifdef __WINS__
			if (mimeType.Length() == 0)
				{
				mimeType = _L("audio");
				}

#endif
			if (mimeType.Left(5) == _L("audio"))
				{
				showType = EAudioPodcast;
				}
			else if (mimeType.Left(5) == _L("video"))
				{
				showType = EVideoPodcast;
				}
			else
				{
				continue;
				}

			CShowInfo *exists = DBGetShowByFileNameL(pathName);

			if (exists)
				{
				delete exists;
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
			DBAddShow(info);

			delete info;
			}
		}
	delete dirPtr;
	CleanupStack::PopAndDestroy(dirScan);
	}

EXPORT_C void CShowEngine::CheckFilesL()
	{
	// check to see if any downloaded files were removed

	RShowInfoArray array;
	GetShowsDownloadedL(array);
	for (TInt i = 0; i < array.Count(); i++)
		{
		if (!BaflUtils::FileExists(iFs, array[i]->FileName()))
			{
			DP1("%S was removed, delisting", &array[i]->FileName());
			if (array[i]->FeedUid() == 0)
				{
				DBDeleteShow(array[i]->Uid());
				}
			else
				{
				array[i]->SetDownloadState(ENotDownloaded);
				DBUpdateShow(array[i]);
				}
			}
		}

	array.ResetAndDestroy();

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

EXPORT_C void CShowEngine::UpdateShow(CShowInfo *aInfo)
	{
	DBUpdateShow(aInfo);
	}

CMetaDataReader& CShowEngine::MetaDataReader()
	{
	return *iMetaDataReader;
	}

void CShowEngine::FileError(TUint /*aError*/)
	{
	//TODO: Error dialog
	//StopDownloads();
	iDownloadErrors = KMaxDownloadErrors;
	}

