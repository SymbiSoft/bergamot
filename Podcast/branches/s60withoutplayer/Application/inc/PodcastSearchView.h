/*
* Copyright (c) 2007-2010 Sebastian Brannstrom, Lars Persson, EmbedDev AB
*
* All rights reserved.
* This component and the accompanying materials are made available
* under the terms of the License "Eclipse Public License v1.0"
* which accompanies this distribution, and is available
* at the URL "http://www.eclipse.org/legal/epl-v10.html".
*
* Initial Contributors:
* EmbedDev AB - initial contribution.
*
* Contributors:
*
* Description:
*
*/

#ifndef PODCASTSEARCHVIEW_H
#define PODCASTSEARCHVIEW_H 

#include <aknview.h>
#include <AknToolbarObserver.h>
#include <AknToolbar.h>
#include <aknlongtapdetector.h>
#include <aknstyluspopupmenu.h>
#include "FeedEngine.h"
#include "PodcastModel.h"
#include "PodcastListView.h"
#include "Podcast.hrh"

class CPodcastSearchView : public CPodcastListView, public MEikListBoxObserver, 
	public MFeedEngineObserver, public MKeyEventListener, public MAknToolbarObserver,
	public MPointerListener, public MAknLongTapDetectorCallBack
    {
    public: 
        static CPodcastSearchView* NewL(CPodcastModel& aPodcastModel);
        static CPodcastSearchView* NewLC(CPodcastModel& aPodcastModel);
        ~CPodcastSearchView();
        
	protected:
	    void ConstructL();
		CPodcastSearchView(CPodcastModel& aPodcastModel);

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

		// From MAknLongTapDetectorCallBack
		virtual void HandleLongTapEventL( const TPoint& aPenEventLocation, const TPoint& aPenEventScreenLocation );
		
		//From MEikMenuObserver
		void ProcessCommandL(TInt aCommandId);
		void SetEmphasis(CCoeControl* /*aMenuControl*/,TBool /*aEmphasis*/)
		{
		}
	 
		void PointerEventL(const TPointerEvent& aPointerEvent);

		
		// from MFeedEngineObserver
		void FeedInfoUpdated(TUint /* aFeedUid */) {}
		void FeedDownloadUpdatedL(TUint /* aFeedUid */, TInt /*aPercentOfCurrentDownload*/) {}
		void FeedUpdateCompleteL(TUint /*aFeeidUid*/) {}
		void FeedUpdateAllCompleteL() {}
		void FeedSearchResultsUpdated();

		/** 
		 * Command handling function intended for overriding by sub classes. 
		 * Default implementation is empty.  
		 * @param aCommand ID of the command to respond to. 
		 */
		void HandleCommandL(TInt aCommand);
		void DynInitMenuPaneL(TInt aResourceId,CEikMenuPane* aMenuPane) {}
		TKeyResponse OfferKeyEventL(const TKeyEvent& aKeyEvent,TEventCode aType);

		void OfferToolbarEventL(TInt aCommand);
		void DynInitToolbarL (TInt aResourceId, CAknToolbar *aToolbar);
		void UpdateToolbar();
		void CloseToolbarExtension();
		
	private:
		CPodcastModel& iPodcastModel;		
		
		CAknStylusPopUpMenu* iStylusPopupMenu;
		CAknLongTapDetector* iLongTapDetector;
		TBool iSearchRunning;
};

#endif // PODCASTSEARCHVIEW_H
