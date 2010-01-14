#ifndef FEEDENGINEOBSERVER_H_
#define FEEDENGINEOBSERVER_H_
class MFeedEngineObserver {
public:
	virtual void FeedInfoUpdated(TUint aFeedUid) = 0;
	virtual void FeedDownloadUpdatedL(TUint aFeedUid, TInt aPercentOfCurrentDownload) = 0;
	virtual void FeedUpdateCompleteL(TUint aFeedUid) = 0;
	virtual void FeedUpdateAllCompleteL() = 0;
};
#endif /*FEEDENGINEOBSERVER_H_*/
