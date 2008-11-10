/**
 * This file is a part of Escarpod Podcast project
 * (c) 2008 The Bergamot Project
 * (c) 2008 Teknolog (Sebastian Brännström)
 * (c) 2008 Anotherguest (Lars Persson)
 */

#ifndef PODCASTMAINVIEWH
#define PODCASTMAINVIEWH 

#include <aknview.h>

#include "PodcastListView.h"
#include "ShowEngineObserver.h"
#include "PodcastModel.h"
class CPodcastMainView : public CPodcastListView, MShowEngineObserver, MEikListBoxObserver
    {
    public: 
        static CPodcastMainView* NewL(CPodcastModel& aPodcastModel);
        static CPodcastMainView* NewLC(CPodcastModel& aPodcastModel);
        ~CPodcastMainView();
	protected:
		void ConstructL();
		CPodcastMainView(CPodcastModel& aPodcastModel);
		
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
		static TInt StaticCheckForQuedDownloadsL(TAny* aBaseView);
		void CheckForQuedDownloadsL();
		
		// from MShowEngineObserver	
		void ShowListUpdated();
		void DownloadQueueUpdated(TInt aDownloadingShows, TInt aQueuedShows);
		void ShowDownloadUpdatedL(TInt /*aPercentOfCurrentDownload*/, TInt /*aBytesOfCurrentDownload*/, TInt /*aBytesTotal*/) { }

		// From // MEikListBoxObserverClass
		void HandleListBoxEventL(CEikListBox* aListBox, TListBoxEvent aEventType);
	private:
		TBool iCheckForQuedDownloads;
		CAsyncCallBack* iStartupCallBack;
		CPodcastModel& iPodcastModel;
};
#endif // PODCASTBASEVIEWH

