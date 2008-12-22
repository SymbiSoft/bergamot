#ifndef SHOWENGINE_H_
#define SHOWENGINE_H_

#include <e32base.h>
#include <APGCLI.H>
#include "ShowInfo.h"
//#include "PodcastModel.h"
#include "HttpClient.h"
#include "ShowEngineObserver.h"
#include "MetaDataReader.h"
#include "sqlite3.h"

class CPodcastModel;

class CShowEngine : public CBase, public MHttpClientObserver, public MMetaDataReaderObserver
{
public:
	static CShowEngine* NewL(CPodcastModel& aPodcastModel);
	virtual ~CShowEngine();
	
public:
	
	// download management
	void AddDownload(TUint aUid);
	TBool RemoveDownload(TUint aUid);
	void RemoveAllDownloads();
	void StopDownloads();
	void ResumeDownloads();
	TBool DownloadsStopped();
	TUint GetNumDownloadingShows();
	CShowInfo* ShowDownloading();
	void GetStatsForDownloaded(TUint &aNumShows, TUint &aNumUnplayed );

	// show access methods
	CShowInfo* GetShowByUid(TUint aShowUid);
	CShowInfo* GetNextShowByTrack(CShowInfo* aShowInfo);
	TBool AddShow(CShowInfo *aItem);
	TBool UpdateShow(CShowInfo *aItem);
	TUint GetDownloadsCount();
	CShowInfo* GetNextDownload();
	void GetAllShows(RShowInfoArray &aArray);
	void GetShowsByFeed(RShowInfoArray &aArray, TUint aFeedUid);
	void GetShowsDownloaded(RShowInfoArray &aArray);
	void GetNewShows(RShowInfoArray &aArray);
	void GetShowsDownloading(RShowInfoArray &aArray);
	CShowInfo* GetShowByFileName(TFileName aFileName);

	// file management methods
	void DeletePlayedShows();
	void DeleteAllShowsByFeed(TUint aFeedUid,TBool aDeleteFiles=ETrue);
	TBool DeleteShow(TUint aUid, TBool aRemoveFile=ETrue);
	void CheckFilesL();


	// callback handling
	void AddObserver(MShowEngineObserver *observer);
	void RemoveObserver(MShowEngineObserver *observer);
	
	CMetaDataReader& MetaDataReader();

	void NotifyShowListUpdated();

protected:
	// from HttpClientObserver, dont have to be public
	void Connected(CHttpClient* aClient);
	void Disconnected(CHttpClient* aClient);
	void CompleteL(CHttpClient* aClient, TBool aSuccessful);
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

	void FillShowInfoFromStmt(sqlite3_stmt *st, CShowInfo* showInfo);
	
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
