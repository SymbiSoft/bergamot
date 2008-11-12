/**
 * This file is a part of Escarpod Podcast project
 * (c) 2008 The Bergamot Project
 * (c) 2008 Teknolog (Sebastian Brännström)
 * (c) 2008 Anotherguest (Lars Persson)
 */

#ifndef PODCASTFEEDVIEWH
#define PODCASTFEEDVIEWH 

#include <aknview.h>
#include "FeedEngine.h"
#include "PodcastModel.h"
#include "PodcastListView.h"
#include "Podcast.hrh"

class CPodcastFeedView : public CPodcastListView, MEikListBoxObserver, public MFeedEngineObserver
    {
    public: 
        static CPodcastFeedView* NewL(CPodcastModel& aPodcastModel);
        static CPodcastFeedView* NewLC(CPodcastModel& aPodcastModel);
        ~CPodcastFeedView();
	protected:
	    void ConstructL();
		CPodcastFeedView(CPodcastModel& aPodcastModel);

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
		

		// From // MEikListBoxObserverClass
		void HandleListBoxEventL(CEikListBox* aListBox, TListBoxEvent aEventType);
		void UpdateListboxItemsL();

		
		// from MFeedEngineObserver
		void FeedInfoUpdated(CFeedInfo* aFeedInfo);
		void FeedDownloadUpdatedL(TUint aFeedUid, TInt aPercentOfCurrentDownload);
		void FeedUpdateCompleteL(TUint aFeeidUid);
		void FeedUpdateAllCompleteL();
		
		/** 
		 * Command handling function intended for overriding by sub classes. 
		 * Default implementation is empty.  
		 * @param aCommand ID of the command to respond to. 
		 */
		void HandleCommandL(TInt aCommand);
	private:
		CPodcastModel& iPodcastModel;
		CDesCArrayFlat iFeeds;
		TBool iUpdatingAllRunning;
		TBool iProgressAdded;
		TFeedsViewMode iCurrentViewMode;
		HBufC* iBooksFormat;
		HBufC* iFeedsFormat;
		HBufC* iNeverUpdated;
};

#endif // PODCASTFEEDVIEWH