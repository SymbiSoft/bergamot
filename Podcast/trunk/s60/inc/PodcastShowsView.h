/**
 * This file is a part of Escarpod Podcast project
 * (c) 2008 The Bergamot Project
 * (c) 2008 Teknolog (Sebastian Brännström)
 * (c) 2008 Anotherguest (Lars Persson)
 */


#ifndef PODCASTSHOWSVIEWH
#define PODCASTSHOWSVIEWH 

#include <aknview.h>
#include <aknlists.h> 
#include <eiklbo.h>
#include "ShowEngine.h"
#include "FeedEngine.h"
#include "PodcastModel.h"
#include "Podcast.hrh"
#include "PodcastListView.h"

class CPodcastShowsView : public CPodcastListView,public MEikListBoxObserver, public MFeedEngineObserver, public MShowEngineObserver, public MKeyEventListener
	{
public: 
	static CPodcastShowsView* NewL(CPodcastModel& aPodcastModel);
	static CPodcastShowsView* NewLC(CPodcastModel& aPodcastModel);
	~CPodcastShowsView();
protected:
	void ConstructL();
	CPodcastShowsView(CPodcastModel& aPodcastModel);

	/**
	 * Returns views id, intended for overriding by sub classes.
	 * @return id for this view.
	 */
	TUid Id() const;

	/** 
	 * Handles a view activation and passes the message of type 
	 * @c aCustomMessageId. This function is intended for overriding by 
	 * sub classes. This function is called by @c AknViewActivatedL().
	 * @param aPrevViewId Specifies the view previously active.
	 * @param aCustomMessageId Specifies the message type.
	 * @param aCustomMessage The activation message.
	 */
	void DoActivateL(const TVwsViewId& aPrevViewId,
			TUid aCustomMessageId,
			const TDesC8& aCustomMessage);

	/** 
	 * View deactivation function intended for overriding by sub classes. 
	 * This function is called by @c AknViewDeactivated().
	 */
	void DoDeactivate();

	void UpdateListboxItemsL();

	// From // MEikListBoxObserverClass
	void HandleListBoxEventL(CEikListBox* aListBox, TListBoxEvent aEventType);
	// From showengine observer
	void ShowListUpdated();  
	void ShowDownloadUpdatedL(TInt aPercentOfCurrentDownload, TInt aBytesOfCurrentDownload, TInt aBytesTotal);
	void DownloadQueueUpdated(TInt /*aDownloadingShows*/, TInt /*aQueuedShows*/) {};
	// From feedengineobserver
	void FeedInfoUpdated(TUint aFeedUid);
	void FeedUpdateCompleteL(TUint aFeedUid);
	void FeedUpdateAllCompleteL();
	void FeedDownloadUpdatedL(TUint /*aFeedUid*/, TInt /*aPercentOfCurrentDownload*/);

	/** 
	 * Command handling function intended for overriding by sub classes. 
	 * Default implementation is empty.  
	 * @param aCommand ID of the command to respond to. 
	 */
	void HandleCommandL(TInt aCommand);
	
	void UpdateFeedUpdateStateL();
	void UpdateShowItemL(TUint aUid, TInt aSizeDownloaded);
	void UpdateShowItemDataL(CShowInfo* aShowInfo,TInt aIndex, TInt aSizeDownloaded = KErrNotFound);
	void DynInitMenuPaneL(TInt aResourceId,CEikMenuPane* aMenuPane);

	TKeyResponse OfferKeyEventL(const TKeyEvent& aKeyEvent,TEventCode aType);

private:
	void GetShowIcons(CShowInfo* aShowInfo, TInt& aIconIndex);
	void UpdateNaviPaneL();

private:
	
	TPodcastClientShowCategory iCurrentCategory;
	CPodcastModel& iPodcastModel;
	TBool iProgressAdded;
	};

#endif // PODCASTSHOWSVIEWH


