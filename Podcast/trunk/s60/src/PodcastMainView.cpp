/**
 * This file is a part of Escarpod Podcast project
 * (c) 2008 The Bergamot Project
 * (c) 2008 Teknolog (Sebastian Brännström)
 * (c) 2008 Anotherguest (Lars Persson)
 */

#include "PodcastMainView.h"
#include "ShowEngine.h"
#include "PodcastAppUi.h"
#include "Podcast.hrh"
#include "SoundEngine.h"
#include "FeedEngine.h"
#include <aknnavide.h> 
#include <podcast.rsg>
#include <podcast.mbg>
#include <gulicon.h>
#include <eikenv.h>

CPodcastMainView* CPodcastMainView::NewL(CPodcastModel& aPodcastModel)
    {
    CPodcastMainView* self = CPodcastMainView::NewLC(aPodcastModel);
    CleanupStack::Pop( self );
    return self;
    }

CPodcastMainView* CPodcastMainView::NewLC(CPodcastModel& aPodcastModel)
    {
    CPodcastMainView* self = new ( ELeave ) CPodcastMainView(aPodcastModel);
    CleanupStack::PushL( self );
    self->ConstructL();
    return self;
    }

CPodcastMainView::CPodcastMainView(CPodcastModel& aPodcastModel):iPodcastModel(aPodcastModel)
{
	iCheckForQuedDownloads = ETrue;
	iPodcastModel.ShowEngine().AddObserver(this);
}

void CPodcastMainView::ConstructL()
{
	BaseConstructL(R_PODCAST_MAINVIEW);
	iMainMenuItems = iEikonEnv->ReadDesCArrayResourceL(R_PODCAST_MAINMENU_ARRAY);
	CPodcastListView::ConstructL();	// Create icon array with granularity of 1 icon
	
	CArrayPtr< CGulIcon >* icons = new(ELeave) CArrayPtrFlat< CGulIcon >(1);
	
	CleanupStack::PushL( icons );

	// Load the bitmap for play icon	
	CFbsBitmap* bitmap = iEikonEnv->CreateBitmapL( _L("*"),EMbmPodcastPlay_40x40);
	CleanupStack::PushL( bitmap );		
	// Load the mask for play icon	
	CFbsBitmap* mask = iEikonEnv->CreateBitmapL( _L("*"),EMbmPodcastPlay_40x40m );	
	CleanupStack::PushL( mask );
	// Append the play icon to icon array
	icons->AppendL( CGulIcon::NewL( bitmap, mask ) );
	CleanupStack::Pop(2); // bitmap, mask


	// Load the bitmap for shows icon	
	bitmap = iEikonEnv->CreateBitmapL( _L("*"),EMbmPodcastDownloaded_view_40x40);
	CleanupStack::PushL( bitmap );		
	// Load the mask for shows icon	
	mask = iEikonEnv->CreateBitmapL( _L("*"),EMbmPodcastDownloaded_view_40x40m );	
	CleanupStack::PushL( mask );
	// Append the shows icon to icon array
	icons->AppendL( CGulIcon::NewL( bitmap, mask ) );
	CleanupStack::Pop(2); // bitmap, mask


	// Load the bitmap for download icon	
	bitmap = iEikonEnv->CreateBitmapL( _L("*"),EMbmPodcastPending_view_40x40);
	CleanupStack::PushL( bitmap );		
	// Load the mask for download icon	
	mask = iEikonEnv->CreateBitmapL( _L("*"),EMbmPodcastPending_view_40x40m );	
	CleanupStack::PushL( mask );
	// Append the download icon to icon array
	icons->AppendL( CGulIcon::NewL( bitmap, mask ) );
	CleanupStack::Pop(2); // bitmap, mask


	// Load the bitmap for Feeds icon	
	bitmap = iEikonEnv->CreateBitmapL( _L("*"),EMbmPodcastFeeds_40x40);
	CleanupStack::PushL( bitmap );		
	// Load the mask for Feeds icon	
	mask = iEikonEnv->CreateBitmapL( _L("*"),EMbmPodcastFeeds_40x40m );	
	CleanupStack::PushL( mask );
	// Append the Feeds icon to icon array
	icons->AppendL( CGulIcon::NewL( bitmap, mask ) );
	CleanupStack::Pop(2); // bitmap, mask


	// Load the bitmap for audiobooks icon	
	bitmap = iEikonEnv->CreateBitmapL( _L("*"),EMbmPodcastAudiobooks_40x40);
	CleanupStack::PushL( bitmap );		
	// Load the mask for audiobooks icon	
	mask = iEikonEnv->CreateBitmapL( _L("*"),EMbmPodcastAudiobooks_40x40m );	
	CleanupStack::PushL( mask );
	// Append the audiobooks icon to icon array
	icons->AppendL( CGulIcon::NewL( bitmap, mask ) );
	CleanupStack::Pop(2); // bitmap, mask

	iListContainer->Listbox()->ItemDrawer()->FormattedCellData()->SetIconArrayL( icons );
	CleanupStack::Pop(icons); // icons

	iListContainer->Listbox()->SetListBoxObserver(this);	
}


void CPodcastMainView::UpdateListboxItemsL()
{
 // Update already existing items perhaps for special layout?
	if(iListContainer->Listbox() != NULL)
	{
	
		TBool playerActive = EFalse;
		TPtrC descriptionText(KNullDesC());

		if(iPodcastModel.PlayingPodcast() != NULL && (iPodcastModel.SoundEngine().State() == ESoundEnginePlaying || iPodcastModel.SoundEngine().State() == ESoundEnginePaused))
		{
			playerActive = ETrue;
			descriptionText.Set(iPodcastModel.PlayingPodcast()->Title());
		}
		iItemArray->Reset();
		TInt cnt = iMainMenuItems->Count();
		for(TInt loop = 0;loop<cnt; loop++)
		{
			switch(loop)
			{
			case 0:
				iListboxFormatbuffer.Format(iMainMenuItems->MdcaPoint(loop), &descriptionText);
				break;
			case 1:
				{
					TUint unplayed, total;
					iPodcastModel.ShowEngine().GetStatsForDownloaded(total, unplayed);
					iListboxFormatbuffer.Format(iMainMenuItems->MdcaPoint(loop), unplayed, total);				
				}
				break;
			case 2:
				if (iPodcastModel.ShowEngine().DownloadsStopped()) {
				
					HBufC *buf = iEikonEnv->AllocReadResourceLC(R_VIEW_DLING_SHOWS_SUSPENDED_CMD);
					iListboxFormatbuffer.Format(buf->Des(), iPodcastModel.ShowEngine().GetNumDownloadingShowsL());
					CleanupStack::PopAndDestroy(buf);
				} else {
					iListboxFormatbuffer.Format(iMainMenuItems->MdcaPoint(loop), iPodcastModel.ShowEngine().GetNumDownloadingShowsL());
				}
				break;
			case 3:
				iListboxFormatbuffer.Format(iMainMenuItems->MdcaPoint(loop), iPodcastModel.FeedEngine().GetSortedFeeds().Count());
				break;
			case 4:
				iListboxFormatbuffer.Format(iMainMenuItems->MdcaPoint(loop), iPodcastModel.FeedEngine().GetSortedBooks().Count());
				break;
			}
			iItemArray->AppendL(iListboxFormatbuffer);
		}

		iListContainer->Listbox()->HandleItemAdditionL();			
	}
}
    
CPodcastMainView::~CPodcastMainView()
    {
	delete iMainMenuItems;
    }

TUid CPodcastMainView::Id() const
{
	return TUid::Uid(1);
}
		
void CPodcastMainView::DoActivateL(const TVwsViewId& aPrevViewId,
	                                  TUid aCustomMessageId,
	                                  const TDesC8& aCustomMessage)
{
	CPodcastListView::DoActivateL(aPrevViewId, aCustomMessageId, aCustomMessage);
	if(iCheckForQuedDownloads)
	{
		iCheckForQuedDownloads = EFalse;
		iStartupCallBack = new (ELeave) CAsyncCallBack(TCallBack(StaticCheckForQuedDownloadsL, this), CActive::EPriorityIdle);
		iStartupCallBack->Call();
	}
}

void CPodcastMainView::DoDeactivate()
{
	CPodcastListView::DoDeactivate();
}

TInt CPodcastMainView::StaticCheckForQuedDownloadsL(TAny* aBaseView)
{
	static_cast<CPodcastMainView*>(aBaseView)->CheckForQuedDownloadsL();
	return KErrNone;
}

void CPodcastMainView::CheckForQuedDownloadsL()
{
	delete iStartupCallBack;
	iStartupCallBack = NULL;

	/*iPodcastModel.ShowEngine().SelectShowsDownloading();
	if (iPodcastModel.ShowEngine().GetSelectedShows().Count() > 0) {
		if(iEikonEnv->QueryWinL(R_PODCAST_ENABLE_DOWNLOADS_TITLE, R_PODCAST_ENABLE_DOWNLOADS_PROMPT))
		{
			iPodcastModel.ShowEngine().ResumeDownloads();
		} else {
			iPodcastModel.ShowEngine().StopDownloads();
		}
		UpdateListboxItemsL();
	}*/
}


void CPodcastMainView::ShowListUpdated()
	{
	TRAP_IGNORE(UpdateListboxItemsL());	
	}

void CPodcastMainView::DownloadQueueUpdated(TInt /*aDownloadingShows*/, TInt /*aQueuedShows*/)

	{
	TRAP_IGNORE(UpdateListboxItemsL());
	}

// From // MEikListBoxObserverClass
void CPodcastMainView::HandleListBoxEventL(CEikListBox* /*aListBox*/, TListBoxEvent aEventType)
{
	switch(aEventType)
	{
	case EEventEnterKeyPressed:
	case EEventItemClicked:
	case EEventItemActioned:
		{
			TUid newview = TUid::Uid(0);
			TUid messageUid = TUid::Uid(0);

			switch(iListContainer->Listbox()->CurrentItemIndex())
			{
			case 0:
				if (iPodcastModel.PlayingPodcast()) {
					newview = KUidPodcastPlayViewID;
				}
				break;
			case 1:
				newview = KUidPodcastShowsViewID;
				messageUid = TUid::Uid(EShowDownloadedShows);
				break;
			case 2:
				newview = KUidPodcastShowsViewID;
				messageUid = TUid::Uid(EShowPendingShows);
				break;
			case 3:
				newview = KUidPodcastFeedViewID;
				messageUid = TUid::Uid(EFeedsNormalMode);
				break;
			case 4:
				newview = KUidPodcastFeedViewID;
				messageUid = TUid::Uid(EFeedsAudioBooksMode);	
				break;
			}

			if(newview.iUid != 0)
			{			
				AppUi()->ActivateLocalViewL(newview,  messageUid, KNullDesC8());
			}
		}
		break;
	default:
		break;
		}


}



