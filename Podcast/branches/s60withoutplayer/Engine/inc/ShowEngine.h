#ifndef SHOWENGINE_H_
#define SHOWENGINE_H_

#include <e32base.h>
#include <APGCLI.H>
#include "ShowInfo.h"
#include "PodcastModel.h"
#include "HttpClient.h"
#include "ShowEngineObserver.h"
#include "MetaDataReader.h"
#include "sqlite3.h"

class CShowEngine : public CBase, public MHttpClientObserver, public MMetaDataReaderObserver
{
public:
	static CShowEngine* NewL(CPodcastModel& aPodcastModel);
	virtual ~CShowEngine();
	
public:
	IMPORT_C void AddDownloadL(CShowInfo *info);
	IMPORT_C TBool RemoveDownloadL(TUint aUid);
	IMPORT_C void RemoveAllDownloads();

	IMPORT_C void StopDownloads();
	IMPORT_C void ResumeDownloadsL();
	IMPORT_C TBool DownloadsStopped();

	IMPORT_C TInt GetNumDownloadingShowsL();
	IMPORT_C CShowInfo* ShowDownloading();
	IMPORT_C CShowInfo* GetShowByUidL(TUint aShowUid);
	CShowInfo* GetNextShowByTrackL(CShowInfo* aShowInfo);
	
	// show access methods
	void GetAllShowsL(RShowInfoArray &aArray);
	void GetShowsByFeedL(RShowInfoArray &aArray, TUint aFeedUid);
	void GetShowsDownloadedL(RShowInfoArray &aArray);
	void GetNewShowsL(RShowInfoArray &aArray);
	void GetShowsDownloadingL(RShowInfoArray &aArray);
	CShowInfo* DBGetShowByFileNameL(TFileName aFileName);
	
	void CompleteL(CHttpClient* aClient, TBool aSuccessful);
	TBool AddShowL(CShowInfo *item);
	IMPORT_C void DeletePlayedShows(RShowInfoArray &aShowInfoArray);
	IMPORT_C void DeleteAllShowsByFeedL(TUint aFeedUid,TBool aDeleteFiles=ETrue);
	IMPORT_C void DeleteShowL(TUint aShowUid, TBool aRemoveFile=ETrue);
	void DeleteOldShowsByFeed(TUint aFeedUid);
	
	IMPORT_C void CheckFilesL();
	IMPORT_C void AddObserver(MShowEngineObserver *observer);
	IMPORT_C void RemoveObserver(MShowEngineObserver *observer);

	void NotifyShowListUpdated();
	IMPORT_C void UpdateShow(CShowInfo *aInfo);

	IMPORT_C void GetMimeType(const TDesC& aFileName, TDes& aMimeType);

	CMetaDataReader& MetaDataReader();
protected:
	// from HttpClientObserver, dont have to be public
	void Connected(CHttpClient* aClient);
	void Disconnected(CHttpClient* aClient);
	void Progress(CHttpClient* aHttpClient, int aBytes, int aTotalBytes);
	void DownloadInfo(CHttpClient* aClient, int aSize);
	void FileError(TUint aError);
	// from MetaDataReaderObserver
	void ReadMetaData(CShowInfo *aShowInfo);
	void ReadMetaDataComplete();
private:
	CShowEngine(CPodcastModel& aPodcastModel);
	void ConstructL();

	void GetShowL(CShowInfo *info);

	void NotifyDownloadQueueUpdated();
	void NotifyShowDownloadUpdated(TInt aPercentOfCurrentDownload, TInt aBytesOfCurrentDownload, TInt aBytesTotal);

	void DownloadNextShowL();
	void ListDirL(TFileName &folder);

	static TInt CompareShowsByDate(const CShowInfo &a, const CShowInfo &b);
	static TBool CompareShowsByUid(const CShowInfo &a, const CShowInfo &b);
	static TInt CompareShowsByTitle(const CShowInfo &a, const CShowInfo &b);
	static TInt CompareShowsByTrackNo(const CShowInfo &a, const CShowInfo &b);
	
private:
	// DB methods
	CShowInfo* DBGetShowByUidL(TUint aUid);
	void DBFillShowInfoFromStmtL(sqlite3_stmt *st, CShowInfo* showInfo);
	TBool DBAddShow(CShowInfo *aItem);
	TBool DBUpdateShow(CShowInfo *aItem);
	void DBGetShowsByFeedL(RShowInfoArray& aShowArray, TUint aFeedUid);
	void DBGetAllShowsL(RShowInfoArray& aShowArray);
	void DBGetNewShowsL(RShowInfoArray& aShowArray);
	void DBGetDownloadedShowsL(RShowInfoArray& aShowArray);
	TBool DBDeleteAllShowsByFeed(TUint aFeedUid);
	TBool DBDeleteShow(TUint aUid);
	void DBRemoveAllDownloads();
	void DBRemoveDownload(TUint aUid);
	void DBGetAllDownloadsL(RShowInfoArray& aShowArray);
	TUint DBGetDownloadsCount();
	void DBAddDownload(TUint aUid);
	CShowInfo* DBGetNextDownloadL();
	
private:
	CHttpClient* iShowClient;
		
	// the file session used to read and write settings
	RFs iFs;
		
	CPodcastModel& iPodcastModel;

	// observers that will receive callbacks
    RArray<MShowEngineObserver*> iObservers;

	// The show we are currently downloading
	CShowInfo* iShowDownloading;
    TBool iDownloadsSuspended;
    TUint iDownloadErrors;
    
    CMetaDataReader* iMetaDataReader;
    
    RApaLsSession iApaSession;
	TBuf8<512> iRecogBuffer;
	
	sqlite3* iDB;
    TBuf<2048> iSqlBuffer;
};

#endif /*SHOWENGINE_H_*/

