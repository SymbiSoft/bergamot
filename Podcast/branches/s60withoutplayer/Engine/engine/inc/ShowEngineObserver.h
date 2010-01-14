#ifndef SHOWENGINEOBSERVER_H_
#define SHOWENGINEOBSERVER_H_
class MShowEngineObserver {
public:
	virtual void ShowListUpdated() = 0;
	virtual void DownloadQueueUpdated(TInt aDownloadingShows, TInt aQueuedShows) = 0;
	virtual void ShowDownloadUpdatedL(TInt aPercentOfCurrentDownload, TInt aBytesOfCurrentDownload, TInt aBytesTotal) = 0;
};
#endif /*SHOWENGINEOBSERVER_H_*/
