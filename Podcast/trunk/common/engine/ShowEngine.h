#ifndef SHOWENGINE_H_
#define SHOWENGINE_H_

#include <e32base.h>
#include <APGCLI.H>
#include "ShowInfo.h"
#include "PodcastModel.h"
#include "HttpClient.h"
#include "ShowEngineObserver.h"
#include "MetaDataReader.h"

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

	TInt GetNumDownloadingShowsL() const;
	CShowInfo* ShowDownloading();
	CShowInfo* GetShowByUidL(TUint aShowUid);
	CShowInfo* GetNextShowByTrackL(CShowInfo* aShowInfo);
	
	// show selection methods
	void SelectAllShows();
	void SelectShowsByFeed(TUint aFeedUid);
	void SelectShowsDownloaded();
	void SelectNewShows();
	void SelectShowsDownloading();
	void GetShowsForFeedL(RShowInfoArray& aShowArray, TUint aFeedUid);

	void CompleteL(CHttpClient* aClient, TBool aSuccessful);
	TBool AddShow(CShowInfo *item);
	void SaveShowsL();
	void SaveShows();
	void DeletePlayedShows();
	void DeleteAllShowsByFeed(TUint aFeedUid,TBool aDeleteFiles=ETrue);
	void DeleteShow(TUint aShowUid, TBool aRemoveFile=ETrue);
	
	RShowInfoArray& GetSelectedShows();
	void SetSelectionPlayed();
	void CheckFilesL();
	void GetStatsByFeed(TUint aFeedUid, TUint &aNumShows, TUint &aNumUnplayed, TBool aIsBookFeed );
	void GetStatsForDownloaded(TUint &aNumShows, TUint &aNumUnplayed );
	void AddObserver(MShowEngineObserver *observer);
	void RemoveObserver(MShowEngineObserver *observer);

	void NotifyShowListUpdated();
	
	TUint GetGrossSelectionLength();
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
	void LoadShowsL(TBool aUseBackup = EFalse);

	void NotifyDownloadQueueUpdated();
	void NotifyShowDownloadUpdated(TInt aPercentOfCurrentDownload, TInt aBytesOfCurrentDownload, TInt aBytesTotal);

	void DownloadNextShow();
	void ListDirL(TFileName &folder);

	static TInt CompareShowsByDate(const CShowInfo &a, const CShowInfo &b);
	static TBool CompareShowsByUid(const CShowInfo &a, const CShowInfo &b);
	static TInt CompareShowsByTitle(const CShowInfo &a, const CShowInfo &b);
	static TInt CompareShowsByTrackNo(const CShowInfo &a, const CShowInfo &b);
private:
	CHttpClient* iShowClient;

	// the complete database of shows
	RShowInfoArray iShows;

	// the current selection of shows
	RShowInfoArray iSelectedShows;
	
	// list of shows waiting to download
	RShowInfoArray iShowsDownloading;
	
	// the file session used to read and write settings
	RFs iFs;
	
	// The show we are currently downloading
	CShowInfo* iShowDownloading;
	
	CPodcastModel& iPodcastModel;

	// observers that will receive callbacks
    RArray<MShowEngineObserver*> iObservers;

    TBool iDownloadsSuspended;
        
    TUint iDownloadErrors;
    
    CMetaDataReader* iMetaDataReader;
    
    TUint iGrossSelectionLength;
    RApaLsSession iApaSession;
	TBuf8<512> iRecogBuffer;
};

#endif /*SHOWENGINE_H_*/
