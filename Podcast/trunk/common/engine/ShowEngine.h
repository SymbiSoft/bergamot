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
	void AddDownload(CShowInfo *info);
	TBool RemoveDownload(TUint aUid);
	void RemoveAllDownloads();

	void StopDownloads();
	void ResumeDownloads();
	TBool DownloadsStopped();

	TInt GetNumDownloadingShowsL();
	CShowInfo* ShowDownloading();
	CShowInfo* GetShowByUidL(TUint aShowUid);
	CShowInfo* GetNextShowByTrackL(CShowInfo* aShowInfo);
	
	// show access methods
	void GetAllShows(RShowInfoArray &aArray);
	void GetShowsByFeed(RShowInfoArray &aArray, TUint aFeedUid);
	void GetShowsDownloaded(RShowInfoArray &aArray);
	void GetNewShows(RShowInfoArray &aArray);
	void GetShowsDownloading(RShowInfoArray &aArray);
	CShowInfo* DBGetShowByFileName(TFileName aFileName);
	
	void CompleteL(CHttpClient* aClient, TBool aSuccessful);
	TBool AddShow(CShowInfo *item);
	void DeletePlayedShows(RShowInfoArray &aShowInfoArray);
	void DeleteAllShowsByFeed(TUint aFeedUid,TBool aDeleteFiles=ETrue);
	void DeleteShow(TUint aShowUid, TBool aRemoveFile=ETrue);
	void DeleteOldShowsByFeed(TUint aFeedUid);
	
	void CheckFilesL();
	void AddObserver(MShowEngineObserver *observer);
	void RemoveObserver(MShowEngineObserver *observer);

	void NotifyShowListUpdated();
	void UpdateShow(CShowInfo *aInfo);
	
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
	void GetMimeType(const TDesC& aFileName, TDes& aMimeType);
private:
	CShowEngine(CPodcastModel& aPodcastModel);
	void ConstructL();

	void GetShowL(CShowInfo *info);

	void NotifyDownloadQueueUpdated();
	void NotifyShowDownloadUpdated(TInt aPercentOfCurrentDownload, TInt aBytesOfCurrentDownload, TInt aBytesTotal);

	void DownloadNextShow();
	void ListDirL(TFileName &folder);

	static TInt CompareShowsByDate(const CShowInfo &a, const CShowInfo &b);
	static TBool CompareShowsByUid(const CShowInfo &a, const CShowInfo &b);
	static TInt CompareShowsByTitle(const CShowInfo &a, const CShowInfo &b);
	static TInt CompareShowsByTrackNo(const CShowInfo &a, const CShowInfo &b);
	
private:
	// DB methods
	CShowInfo* DBGetShowByUid(TUint aUid);
	void DBFillShowInfoFromStmt(sqlite3_stmt *st, CShowInfo* showInfo);
	TBool DBAddShow(CShowInfo *aItem);
	TBool DBUpdateShow(CShowInfo *aItem);
	void DBGetShowsByFeed(RShowInfoArray& aShowArray, TUint aFeedUid);
	void DBGetAllShows(RShowInfoArray& aShowArray);
	void DBGetNewShows(RShowInfoArray& aShowArray);
	void DBGetDownloadedShows(RShowInfoArray& aShowArray);
	TBool DBDeleteAllShowsByFeed(TUint aFeedUid);
	TBool DBDeleteShow(TUint aUid);
	void DBRemoveAllDownloads();
	void DBRemoveDownload(TUint aUid);
	void DBGetAllDownloads(RShowInfoArray& aShowArray);
	TUint DBGetDownloadsCount();
	void DBAddDownload(TUint aUid);
	CShowInfo* DBGetNextDownload();
	
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

