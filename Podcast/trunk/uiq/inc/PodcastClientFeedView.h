#ifndef PODCASTCLIENTFEEDVIEW_H
#define PODCASTCLIENTFEEDVIEW_H
#include "PodcastClientView.h"


enum TFeedsViewMode
{
	EFeedsNormalMode = 0x100,
	EFeedsAudioBooksMode = 0x101
};

class CPodcastClientFeedView : public CPodcastClientView, public MFeedEngineObserver
{
public:
	static CPodcastClientFeedView* NewLC(CQikAppUi& aAppUi, CPodcastModel& aPodcastModel);
	~CPodcastClientFeedView();
protected:
	CPodcastClientFeedView(CQikAppUi& aAppUi, CPodcastModel& aPodcastModel);
	void UpdateListboxItemsL();
	void HandleListBoxEventL(CQikListBox *aListBox, TQikListBoxEvent aEventType, TInt aItemIndex, TInt aSlotId);
	TVwsViewId ViewId()const;
	void ViewConstructL();
	void ViewActivatedL(const TVwsViewId &aPrevViewId, TUid aCustomMessageId, const TDesC8 &aCustomMessage);

	void ConstructL();
	void UpdateCommandsL();

    void FeedInfoUpdatedL(TUint aFeedUid);

	// from MFeedEngineObserver
	void UpdateFeedInfoDataL(CFeedInfo* aFeedInfo,  MQikListBoxData* aListboxData, TBool aIsUpdating = EFalse); 
    void FeedInfoUpdated(TUint aFeedUid);
	void UpdateFeedInfoStatusL(TUint aFeedUid, TBool aIsUpdating);
	void FeedDownloadUpdatedL(TUint aFeedUid, TInt aPercentOfCurrentDownload);
	void FeedUpdateCompleteL(TUint aFeeidUid);
	void FeedUpdateAllCompleteL();
	// from MQikCommandModelOwner	
	CQikCommand* DynInitOrDeleteCommandL(CQikCommand* aCommand, const CCoeControl& aControlAddingCommands);
	void HandleCommandL(CQikCommand& aCommand);
	void HandleAddNewAudioBookL();
private:
    TBool iProgressAdded;
	TBool iUpdatingAllRunning;
	TFeedsViewMode iCurrentViewMode;
	HBufC* iBooksFormat;
	HBufC* iFeedsFormat;
	HBufC* iNeverUpdated;
};


#endif
