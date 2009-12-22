#ifndef PODCASTCLIENTBASEVIEW_H
#define PODCASTCLIENTBASEVIEW_H
#include "PodcastClientView.h"
#include "ShowEngineObserver.h"

enum TBaseViewSelections
{
	EBaseViewPlayer = 0x100, 
	EBaseViewNewShows = 0x101,
	EBaseViewDownloadedShows = 0x102,
	EBaseViewPendingShows = 0x103,
	EBaseViewFeeds = 0x104,
	EBaseViewAudioBooks = 0x105
};

class CPodcastClientBaseView : public CPodcastClientView, MShowEngineObserver
{
public:
	static CPodcastClientBaseView* NewLC(CQikAppUi& aAppUi, CPodcastModel& aPodcastModel);
	~CPodcastClientBaseView();
protected:
	CPodcastClientBaseView(CQikAppUi& aAppUi, CPodcastModel& aPodcastModel);
	void UpdateListboxItemsL();
	void HandleListBoxEventL(CQikListBox *aListBox, TQikListBoxEvent aEventType, TInt aItemIndex, TInt aSlotId);
	TVwsViewId ViewId()const;
	void ViewConstructL();

	// from MQikCommandModelOwner	
	CQikCommand* DynInitOrDeleteCommandL(CQikCommand* aCommand, const CCoeControl& aControlAddingCommands);
	void ViewActivatedL(const TVwsViewId &aPrevViewId, TUid aCustomMessageId, const TDesC8 &aCustomMessage);
	static TInt StaticCheckForQuedDownloadsL(TAny* aBaseView);
	void CheckForQuedDownloadsL();
	
	// from MShowEngineObserver
	
	void ShowListUpdated();
	void DownloadQueueUpdated(TInt aDownloadingShows, TInt aQueuedShows);
	void ShowDownloadUpdatedL(TInt /*aPercentOfCurrentDownload*/, TInt /*aBytesOfCurrentDownload*/, TInt /*aBytesTotal*/) { }

private:
	TBool iCheckForQuedDownloads;
	CAsyncCallBack* iStartupCallBack;

};
#endif // baseview


