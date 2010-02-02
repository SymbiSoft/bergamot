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

#ifndef PODCASTSHOWSVIEWH
#define PODCASTSHOWSVIEWH 

#include <aknview.h>
#include <aknlists.h> 
#include <eiklbo.h>
#include <AknToolbarObserver.h>
#include <AknToolbar.h>
#include "ShowEngine.h"
#include "FeedEngine.h"
#include "PodcastModel.h"
#include "Podcast.hrh"
#include "PodcastListView.h"

class CPodcastShowsView : public CPodcastListView, public MEikListBoxObserver, 
	public MFeedEngineObserver, public MShowEngineObserver, public MKeyEventListener
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
	void ShowListUpdatedL();  
	void ShowDownloadUpdatedL(TInt aPercentOfCurrentDownload, TInt aBytesOfCurrentDownload, TInt aBytesTotal);
	void DownloadQueueUpdatedL(TInt /*aDownloadingShows*/, TInt /*aQueuedShows*/) {};
	// From feedengineobserver
	void FeedInfoUpdated(TUint aFeedUid);
	void FeedUpdateCompleteL(TUint aFeedUid);
	void FeedUpdateAllCompleteL();
	void FeedDownloadUpdatedL(TUint /*aFeedUid*/, TInt /*aPercentOfCurrentDownload*/);
	void FeedSearchResultsUpdated() {}

	/** 
	 * Command handling function intended for overriding by sub classes. 
	 * Default implementation is empty.  
	 * @param aCommand ID of the command to respond to. 
	 */
	void HandleCommandL(TInt aCommand);
	TKeyResponse OfferKeyEventL(const TKeyEvent& aKeyEvent,TEventCode aType);

	void UpdateFeedUpdateStateL();
	void UpdateShowItemL(TUint aUid, TInt aSizeDownloaded);
	void UpdateShowItemDataL(CShowInfo* aShowInfo,TInt aIndex, TInt aSizeDownloaded = KErrNotFound);
	void DynInitMenuPaneL(TInt aResourceId,CEikMenuPane* aMenuPane);
	void FormatFeedInfoListBoxItemL(CShowInfo& aShowInfo, TInt aSizeDownloaded = 0);
private:
	void GetShowIcons(CShowInfo* aShowInfo, TInt& aIconIndex);
	void UpdateToolbar();
private:
	
	TPodcastClientShowCategory iCurrentCategory;
	CPodcastModel& iPodcastModel;
	TBool iProgressAdded;
	};

#endif // PODCASTSHOWSVIEWH


