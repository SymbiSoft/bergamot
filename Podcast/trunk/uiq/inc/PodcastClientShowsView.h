#ifndef PODCASTCLIENTPODCASTSVIEW_H
#define PODCASTCLIENTPODCASTSVIEW_H
#include <videoplayer.h>

#include "PodcastClientView.h"
#include "ShowEngineObserver.h"

class CQikCategoryModel;


class CPodcastClientShowsView : public CPodcastClientView, public MFeedEngineObserver, public MShowEngineObserver
{
public:
	static CPodcastClientShowsView* NewLC(CQikAppUi& aAppUi, CPodcastModel& aPodcastModel);
	~CPodcastClientShowsView();
protected:
	CPodcastClientShowsView(CQikAppUi& aAppUi, CPodcastModel& aPodcastModel);
	void UpdateListboxItemsL();
	void HandleListBoxEventL(CQikListBox *aListBox, TQikListBoxEvent aEventType, TInt aItemIndex, TInt aSlotId);
	TVwsViewId ViewId()const;
	void ViewConstructL();
	void ConstructL();
	void ShowListUpdated();
    void FeedInfoUpdated(TUint aFeedUid);
    void FeedUpdateCompleteL(TUint aFeedUid);
	void FeedUpdateAllCompleteL();
	void FeedDownloadUpdatedL(TUint /*aFeedUid*/, TInt /*aPercentOfCurrentDownload*/);
	void ShowDownloadUpdatedL(TInt aPercentOfCurrentDownload, TInt aBytesOfCurrentDownload, TInt aBytesTotal);
	void DownloadQueueUpdated(TInt /*aDownloadingShows*/, TInt /*aQueuedShows*/) {};
	void HandleCommandL(CQikCommand& aCommand);
	void UpdateShowItemL(CShowInfo* aShowInfo, TInt aSizeDownloaded = KErrNotFound);
	void UpdateShowItemDataL(CShowInfo* aShowInfo, MQikListBoxData* aListboxData, TInt aSizeDownloaded = KErrNotFound);
	// from MQikCommandModelOwner	
	CQikCommand* DynInitOrDeleteCommandL(CQikCommand* aCommand, const CCoeControl& aControlAddingCommands);
	void ViewActivatedL(const TVwsViewId &aPrevViewId, TUid aCustomMessageId, const TDesC8 &aCustomMessage);
	void UpdateCommandsL();
	void UpdateSelectCommandL();
	void GetShowIcons(CShowInfo* aShowInfo, TInt& aImageId, TInt& aMaskId);
	void UpdateFeedUpdateStateL();
	TKeyResponse OfferKeyEventL(const TKeyEvent& aKeyEvent,TEventCode aType);

private:
	TPodcastClientShowCategory iCurrentCategory;
	TBool iProgressAdded;
	CVideoPlayerUtility* iVideoPlayer;
	TBool iDisableCatchupMode;
};


#endif // Shows view


