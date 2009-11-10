#include <QikContent.h>
#include <qikcommand.h>
#include <PodcastClient.mbg>
#include <PodcastClient.rsg>
#include "PodcastClientShowsView.h"
#include "PodcastClientBaseView.h"
#include "PodcastClientFeedView.h"
#include "PodcastModel.h"
#include "SoundEngine.h"
#include "ShowEngine.h"
#include "debug.h"

/**
Creates and constructs the view.

@param aAppUi Reference to the AppUi
@return Pointer to a CPodcastClientView object
*/
CPodcastClientBaseView* CPodcastClientBaseView::NewLC(CQikAppUi& aAppUi, CPodcastModel& aPodcastModel)
	{
	CPodcastClientBaseView* self = new (ELeave) CPodcastClientBaseView(aAppUi, aPodcastModel);
	CleanupStack::PushL(self);
	self->ConstructL();
	return self;
	}

/**
Returns the view Id
@return Returns the Uid of the view
*/
TVwsViewId CPodcastClientBaseView::ViewId()const
	{
	return TVwsViewId(KUidPodcastClientID, KUidPodcastBaseViewID);
	}

void CPodcastClientBaseView::ViewActivatedL(const TVwsViewId &aPrevViewId, TUid aCustomMessageId, const TDesC8 &aCustomMessage)
{
	CPodcastClientView::ViewActivatedL(aPrevViewId, aCustomMessageId, aCustomMessage);

	if(iCheckForQuedDownloads)
	{
		iCheckForQuedDownloads = EFalse;
		iStartupCallBack = new (ELeave) CAsyncCallBack(TCallBack(StaticCheckForQuedDownloadsL, this), CActive::EPriorityIdle);
		iStartupCallBack->Call();
	}
}

TInt CPodcastClientBaseView::StaticCheckForQuedDownloadsL(TAny* aBaseView)
{
	static_cast<CPodcastClientBaseView*>(aBaseView)->CheckForQuedDownloadsL();
	return KErrNone;
}

void CPodcastClientBaseView::CheckForQuedDownloadsL()
{
/*	delete iStartupCallBack;
	iStartupCallBack = NULL;

	iPodcastModel.ShowEngine().SelectShowsDownloading();
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

void CPodcastClientBaseView::ViewConstructL()
{
    DP("ViewConstructL");
    ViewConstructFromResourceL(R_PODCAST_MAINVIEW_UI_CONFIGURATIONS);

	ViewContext()->AddTextL(EPodcastListViewContextLabel, KNullDesC(), EHCenterVCenter);

	// Get the list box and the list box model
	iListbox = LocateControlByUniqueHandle<CQikListBox>(EPodcastListViewListCtrl);
	iListbox->SetListBoxObserver(this);
	MQikListBoxModel& model(iListbox->Model());
	model.ModelBeginUpdateLC();
	
	MQikListBoxData* data = model.RetrieveDataL(0);	
	data->SetItemId(EBaseViewPlayer);
	data->Close();
	
/*	data = model.RetrieveDataL(1);	
	data->SetItemId(EBaseViewNewShows);
	data->Close();*/
	
	data = model.RetrieveDataL(1);	
	data->SetItemId(EBaseViewDownloadedShows);
	data->Close();
	
	data = model.RetrieveDataL(2);	
	data->SetItemId(EBaseViewPendingShows);
	data->Close();

	data = model.RetrieveDataL(3);	
	data->SetItemId(EBaseViewFeeds);
	data->Close();

	data = model.RetrieveDataL(4);	
	data->SetItemId(EBaseViewAudioBooks);
	data->Close();


	model.ModelEndUpdateL();

    UpdateListboxItemsL();
}


CPodcastClientBaseView::CPodcastClientBaseView(CQikAppUi& aAppUi, CPodcastModel& aPodcastModel):CPodcastClientView(aAppUi, aPodcastModel)
{
	iCheckForQuedDownloads = ETrue;
	iPodcastModel.ShowEngine().AddObserver(this);
}

CPodcastClientBaseView::~CPodcastClientBaseView()
{
	delete iStartupCallBack;
}

CQikCommand* CPodcastClientBaseView::DynInitOrDeleteCommandL(CQikCommand* aCommand, const CCoeControl& /*aControlAddingCommands*/)
{
	switch(aCommand->Id())
	{
	default:
		break;
	}
	
	return aCommand;
}

void CPodcastClientBaseView::UpdateListboxItemsL()
{
 // Update already existing items perhaps for special layout?
	if(iListbox != NULL)
	{
		MQikListBoxModel& model(iListbox->Model());
		TInt cnt = model.Count();

		model.ModelBeginUpdateLC();
		TPtrC descriptionText(KNullDesC());
		TBool playerActive = EFalse;
		TBuf<32> formatting;
		TBuf<64> statusText;
		if(iPodcastModel.PlayingPodcast() != NULL && (iPodcastModel.SoundEngine().State() == ESoundEnginePlaying || iPodcastModel.SoundEngine().State() == ESoundEnginePaused))
		{
			playerActive = ETrue;
			descriptionText.Set(iPodcastModel.PlayingPodcast()->Title());
		}
		
		for(TInt loop = 0;loop <cnt;loop++)
		{
			MQikListBoxData* data = model.RetrieveDataL(loop);	
			TInt itemId = data->ItemId();
			
			switch (itemId)
			{
			case EBaseViewPlayer:
				{
					data->SetDimmed(!playerActive);					
					data->SetTextL(descriptionText, EQikListBoxSlotText2);				
					model.DataUpdatedL(loop);
					
				}break;
			case EBaseViewDownloadedShows:
				{
					TUint unplayed, total;
					iPodcastModel.FeedEngine().GetDownloadedStats(total, unplayed);
					/*if (total == -1) {
						iEikonEnv->ReadResourceL(formatting, R_PODCAST_ONPHONE_STATUS_UNKNOWN);
						statusText.Copy(formatting);
					} else {*/
						iEikonEnv->ReadResourceL(formatting, R_PODCAST_ONPHONE_STATUS);
						statusText.Format(formatting, unplayed, total);
					//}
					data->SetTextL(statusText, EQikListBoxSlotText2);				
					model.DataUpdatedL(loop);
					
				}break;				
			case EBaseViewPendingShows:
				{
					if(iPodcastModel.ShowEngine().DownloadsStopped()) {
						iEikonEnv->ReadResourceL(formatting, R_PODCAST_PENDING_STATUS_SUSPENDED);
										
					} else {
						iEikonEnv->ReadResourceL(formatting, R_PODCAST_PENDING_STATUS_ACTIVE);				
					}
					statusText.Format(formatting, iPodcastModel.ShowEngine().GetNumDownloadingShowsL());
					data->SetTextL(statusText, EQikListBoxSlotText2);				
					model.DataUpdatedL(loop);
				}break;
			case EBaseViewFeeds:
				{
					iEikonEnv->ReadResourceL(formatting, R_PODCAST_FEEDS_STATUS);
					statusText.Format(formatting, iPodcastModel.FeedEngine().GetSortedFeeds().Count());
					data->SetTextL(statusText, EQikListBoxSlotText2);				
					model.DataUpdatedL(loop);
				}break;
			case EBaseViewAudioBooks:
				{
					iEikonEnv->ReadResourceL(formatting, R_PODCAST_BOOKS_STATUS);
					statusText.Format(formatting, iPodcastModel.FeedEngine().GetSortedBooks().Count());
					data->SetTextL(statusText, EQikListBoxSlotText2);				
					model.DataUpdatedL(loop);
				}break;
			default:
				{
				}break;
			}
			
			data->Close();
		}	
		
		model.ModelEndUpdateL();
		
	}
	TBuf<100> titleBuffer;
	iEikonEnv->ReadResourceL(titleBuffer, R_PODCAST_SUBTITLE);
	ViewContext()->ChangeTextL(EPodcastListViewContextLabel, titleBuffer);

}


void CPodcastClientBaseView::HandleListBoxEventL(CQikListBox * /*aListBox*/, TQikListBoxEvent aEventType, TInt aItemIndex, TInt aSlotId)
{
	DP3("HandleListBoxEvent, itemIndex=%d, slotId=%d, aEventType=%d", aItemIndex, aSlotId, aEventType);

	switch (aEventType)
	{
	default:
		break;
	case EEventHighlightMoved:
		break;
	case EEventItemHighlighted:
	case EEventItemConfirmed:
	case EEventItemTapped:
		{
			MQikListBoxModel& model(iListbox->Model());
			MQikListBoxData* data = model.RetrieveDataL(aItemIndex);	
			TInt itemId = data->ItemId();
			data->Close();
			
			TUid newview = TUid::Uid(0);
			TUid messageUid = TUid::Uid(0);
			switch(itemId)
			{
			case EBaseViewPlayer:
				{
					if (iPodcastModel.PlayingPodcast()) {
						newview = KUidPodcastPlayViewID;
					}
				
				}break;
			case EBaseViewNewShows:
				{
					newview = KUidPodcastShowsViewID;
					messageUid = TUid::Uid(EShowNewShows);
				}break;
			case EBaseViewDownloadedShows:
				{
					newview = KUidPodcastShowsViewID;
					messageUid = TUid::Uid(EShowDownloadedShows);
				}break;
			case EBaseViewPendingShows:
				{
					newview = KUidPodcastShowsViewID;
					messageUid = TUid::Uid(EShowPendingShows);
				}break;
			case EBaseViewFeeds:
				{
					newview = KUidPodcastFeedViewID;
					messageUid = TUid::Uid(EFeedsNormalMode);
				}break;
			case EBaseViewAudioBooks:
				{
					newview = KUidPodcastFeedViewID;
					messageUid = TUid::Uid(EFeedsAudioBooksMode);				
				}break;
			default:
				break;
			}
			if(newview.iUid != 0)
			{
				TVwsViewId viewId = TVwsViewId(KUidPodcastClientID, newview);
				iQikAppUi.ActivateViewL(viewId,  messageUid, KNullDesC8());
			}
			
		}break;
	}
	
}

void CPodcastClientBaseView::ShowListUpdated()
	{
	TRAP_IGNORE(UpdateListboxItemsL());	
	}

void CPodcastClientBaseView::DownloadQueueUpdated(TInt /*aDownloadingShows*/, TInt /*aQueuedShows*/)

	{
	TRAP_IGNORE(UpdateListboxItemsL());
	}
