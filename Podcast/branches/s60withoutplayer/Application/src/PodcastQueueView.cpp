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

#include "PodcastQueueView.h"
#include "PodcastAppUi.h"
#include "ShowEngine.h"
#include "SettingsEngine.h"
#include "PodcastApp.h"
#include "Constants.h"
#include "imagehandler.h"

#include <akntitle.h>
#include <podcast.rsg>
#include <podcast.mbg>
#include <gulicon.h>
#include <barsread.h>
#include <aknnotedialog.h>
#include <aknmessagequerydialog.h>

const TInt KSizeBufLen = 64;
const TInt KDefaultGran = 5;
_LIT(KSizeDownloadingOf, "(%.1f/%.1f MB)");
_LIT(KShowsSizeFormatS60, "(%.1f MB)");
_LIT(KChapterFormatting, "%03d");
_LIT(KShowFormat, "%d\t%S\t%S %S\t");
_LIT(KShowErrorFormat, "%d\t%S\t%S\t");
#define KMaxMessageLength 200

const TUint KIconArrayIds[] =
	{
			EMbmPodcastEmptyimage,
			EMbmPodcastEmptyimage,
			EMbmPodcastAudio,
			EMbmPodcastAudio_mask,
			EMbmPodcastNew_showstate_right,// new
			EMbmPodcastNew_showstate_right_mask,
			EMbmPodcastAudio_green, // playing
			EMbmPodcastAudio_green_mask,
			EMbmPodcastNot_downloaded_right, //downloading
			EMbmPodcastNot_downloaded_right_mask,
			EMbmPodcastNot_downloaded_right,
			EMbmPodcastNot_downloaded_right_mask,
			EMbmPodcastQueue_downloaded_right,
			EMbmPodcastQueue_downloaded_right_mask,
			EMbmPodcastBroken_right,
			EMbmPodcastBroken_right_mask,  // old
			EMbmPodcastQueue_downloaded_right,
			EMbmPodcastQueue_downloaded_right_mask,
			EMbmPodcastRed_right,
			EMbmPodcastRed_right_mask,
			0,
			0
	};

#define KPodcastImageWidth 160
#define KPodcastImageHeight 120
#define KPodcastDialogOffset 2

/** 
 *  This is an interal class to display a message query dialog with an image at the bottm
 */
class CPodcastImageMessageQueryDialog:public CAknMessageQueryDialog
	{
	public:
		/**
		 * C++ default constructor.
		 *
		 * @param aMessage Dialog box text.
		 * @param aHeader Header for the dialog.
		 * @deprecated 
		 */ 
		CPodcastImageMessageQueryDialog(TDesC* aMessage, TDesC* aHeader):CAknMessageQueryDialog(aMessage, aHeader)
    		{

    		}    

		~CPodcastImageMessageQueryDialog()
			{

			}

		void SetSizeAndPosition(const TSize& aSize)
			{
			CAknMessageQueryDialog::SetSizeAndPosition(aSize);

			TPoint pos = Position();		 
			TSize size = Size();

			CAknDialog::SetSizeAndPosition(aSize);		 		 

			pos.iY-=((aSize.iHeight-size.iHeight)-KPodcastDialogOffset);
			SetPosition(pos);	
			SetSize(aSize);
			}

	};

CPodcastQueueView* CPodcastQueueView::NewL(CPodcastModel& aPodcastModel)
	{
	CPodcastQueueView* self = CPodcastQueueView::NewLC(aPodcastModel);
	CleanupStack::Pop(self);
	return self;
	}

CPodcastQueueView* CPodcastQueueView::NewLC(CPodcastModel& aPodcastModel)
	{
	CPodcastQueueView* self = new ( ELeave ) CPodcastQueueView(aPodcastModel);
	CleanupStack::PushL(self);
	self->ConstructL();
	return self;
	}

CPodcastQueueView::CPodcastQueueView(CPodcastModel& aPodcastModel) :
	iPodcastModel(aPodcastModel)
	{
	}

void CPodcastQueueView::ConstructL()
	{
	BaseConstructL(R_PODCAST_SHOWSVIEW);
	CPodcastListView::ConstructL();
	CArrayPtr< CGulIcon>* icons = new(ELeave) CArrayPtrFlat< CGulIcon>(1);
	CleanupStack::PushL(icons);
	TInt pos = 0;
	while (KIconArrayIds[pos] > 0)
		{
		// Load the bitmap for play icon	
		CFbsBitmap* bitmap= NULL;//iEikonEnv->CreateBitmapL( _L("*"), KIconArrayIds[pos]);
		CFbsBitmap* mask=  NULL;////iEikonEnv->CreateBitmapL( _L("*"), KIconArrayIds[pos+1] );
		AknIconUtils::CreateIconL(bitmap,
					                          mask,
					                          iEikonEnv->EikAppUi()->Application()->BitmapStoreName(),
					                          KIconArrayIds[pos],
					                          KIconArrayIds[pos+1]);	
		CleanupStack::PushL(bitmap);
		// Load the mask for play icon			
		CleanupStack::PushL(mask);
		// Append the play icon to icon array
		icons->AppendL(CGulIcon::NewL(bitmap, mask) );
		CleanupStack::Pop(2); // bitmap, mask
		pos+=2;
		}
	iListContainer->Listbox()->ItemDrawer()->FormattedCellData()->SetIconArrayL(icons);
	CleanupStack::Pop(icons); // icons
	iListContainer->Listbox()->SetListBoxObserver(this);
	
	iPodcastModel.FeedEngine().AddObserver(this);
	iPodcastModel.ShowEngine().AddObserver(this);
	
	iStylusPopupMenu = CAknStylusPopUpMenu::NewL( this , TPoint(0,0));
	TResourceReader reader;
	iCoeEnv->CreateResourceReaderLC(reader,R_QUEUEVIEW_POPUP_MENU);
	iStylusPopupMenu->ConstructFromResourceL(reader);
	CleanupStack::PopAndDestroy();	
	}

TKeyResponse CPodcastQueueView::OfferKeyEventL(const TKeyEvent& aKeyEvent,TEventCode aType)
	{
	if (aType == EEventKey)
		{
		CShowInfo *activeShow = NULL;

		TInt index = iListContainer->Listbox()->CurrentItemIndex();
		if(index >= 0 && index < iPodcastModel.ActiveShowList().Count())
		{
			activeShow = iPodcastModel.ActiveShowList()[index];
		}
		
		if (activeShow != NULL) {
			DP1("aKeyEvent.iCode=%d", aKeyEvent.iCode);
			switch (aKeyEvent.iCode) {
			case 117: 
			case '*':
			case EKeySpace:
				if (activeShow->PlayState() == EPlayed) {
					HandleCommandL(EPodcastMarkAsUnplayed);
				} else {
					HandleCommandL(EPodcastMarkAsPlayed);
				}
				break;
			case 106:
			case '#':
				if (activeShow->DownloadState() == ENotDownloaded) {
					HandleCommandL(EPodcastDownloadShow);
				}
				break;
			case EKeyBackspace:
			case EKeyDelete:
				HandleCommandL(EPodcastRemoveDownload);
				break;
			default:
				break;
			}
			UpdateToolbar();
		}
	}
		return CPodcastListView::OfferKeyEventL(aKeyEvent, aType);
	}

CPodcastQueueView::~CPodcastQueueView()
	{
	iPodcastModel.ShowEngine().RemoveObserver(this);
	iPodcastModel.FeedEngine().RemoveObserver(this);
	
    if(iStylusPopupMenu)
        delete iStylusPopupMenu, iStylusPopupMenu = NULL;
	}


TUid CPodcastQueueView::Id() const
	{
	return KUidPodcastQueueViewID;
	}

void CPodcastQueueView::DoActivateL(const TVwsViewId& aPrevViewId,
		TUid aCustomMessageId, const TDesC8& aCustomMessage)
	{
	DP("CPodcastQueueView::DoActivateL BEGIN");
	
	CPodcastListView::DoActivateL(aPrevViewId, aCustomMessageId, aCustomMessage);
	iPreviousView = TVwsViewId(KUidPodcast, KUidPodcastFeedViewID);
	
	UpdateFeedUpdateStateL();
	UpdateToolbar();
	DP("CPodcastQueueView::DoActivateL END");
	}

void CPodcastQueueView::DoDeactivate()
	{
	CAknTitlePane* titlePane = static_cast<CAknTitlePane*>
		     ( StatusPane()->ControlL( TUid::Uid( EEikStatusPaneUidTitle ) ) );
	
	CPodcastListView::DoDeactivate();
	}

// Engine callback when new shows are available
void CPodcastQueueView::ShowListUpdatedL()
	{
	UpdateListboxItemsL();
	}

void CPodcastQueueView::ShowDownloadUpdatedL(TInt aBytesOfCurrentDownload, TInt /*aBytesTotal*/)
	{	
		CShowInfo *info = iPodcastModel.ShowEngine().ShowDownloading();
		if (info) 
			{
			UpdateShowItemL(info->Uid(), aBytesOfCurrentDownload);
			}		
	}

void CPodcastQueueView::ShowDownloadFinishedL(TUint aFeedUid, TInt aError)
	{
	iProgressAdded = EFalse;

	iPodcastModel.GetShowsDownloading();
	UpdateListboxItemsL();
	UpdateToolbar();
	
	switch(aError)
		{
		case KErrCouldNotConnect:
			{
			TBuf<KMaxMessageLength> message;
			iEikonEnv->ReadResourceL(message, R_PODCAST_CONNECTION_ERROR);
			ShowErrorMessage(message);
			}
			break;
		default: // Do nothing
			break;
		}
	}


void CPodcastQueueView::FeedDownloadStartedL(TFeedState aState,TUint aFeedUid)
	{
	// TODO make use of the fact that we know that the feed download is
	// started instead of checking feed engine states in UpdateFeedUpdateStateL.
	if (iPodcastModel.ActiveFeedInfo() != NULL
			&& iPodcastModel.ActiveFeedInfo()->Uid() == aFeedUid)
		{
		TRAP_IGNORE(UpdateFeedUpdateStateL());
		UpdateToolbar();
		}	
	}

void CPodcastQueueView::FeedDownloadFinishedL(TFeedState aState,TUint aFeedUid, TInt /*aError*/)
	{
	DP("CPodcastQueueView::FeedDownloadFinishedL BEGIN");
	// TODO make use of the fact that we know that the feed download is
	// finished instead of checking feed engine states in UpdateFeedUpdateStateL.
	if (iPodcastModel.ActiveFeedInfo() != NULL
			&& iPodcastModel.ActiveFeedInfo()->Uid() == aFeedUid)
		{
		TRAP_IGNORE(UpdateFeedUpdateStateL());
		}
	DP("CPodcastQueueView::FeedDownloadFinishedL END");
	}

void CPodcastQueueView::HandleListBoxEventL(CEikListBox* /*aListBox*/,
		TListBoxEvent aEventType)
	{
	switch (aEventType)
		{
		case EEventEnterKeyPressed:		
		case EEventItemActioned:
		case EEventItemDoubleClicked:
			break;
		default:
			break;
		}
		UpdateToolbar();
	}

void CPodcastQueueView::GetShowIcons(CShowInfo* aShowInfo, TInt& aIconIndex)
	{
	TBool dlStop = iPodcastModel.SettingsEngine().DownloadSuspended();
	TUint showDownloadingUid = iPodcastModel.ShowEngine().ShowDownloading() ? iPodcastModel.ShowEngine().ShowDownloading()->Uid() : 0;
	TUint showPlayingUid = iPodcastModel.PlayingPodcast() ? iPodcastModel.PlayingPodcast()->Uid() : 0;
	TBool playingOrPaused = EFalse;/*iPodcastModel.SoundEngine().State() == ESoundEnginePlaying || 
							iPodcastModel.SoundEngine().State() == ESoundEnginePaused;*/
#pragma message("LAPER Need to rework playing api if it should still exist")
	
	if (showDownloadingUid == aShowInfo->Uid())
		{
		aIconIndex = dlStop ? ESuspendedShowIcon : EDownloadingShowIcon;		
		}
	else if (showPlayingUid == aShowInfo->Uid() && playingOrPaused)
		{
		aIconIndex = EPlayingShowIcon;
		}
	else
		{
		switch (aShowInfo->DownloadState())
			{
			case EDownloaded:
				if (aShowInfo->PlayState() == ENeverPlayed) {
					aIconIndex = EDownloadedNewShowIcon;
				} else {
					aIconIndex = EShowIcon;
				}
				break;
			case ENotDownloaded:
				if (aShowInfo->PlayState() == ENeverPlayed) {
					aIconIndex = ENewShowIcon;
				} else {
					aIconIndex = EOldShowIcon;
				}
				break;
			case EQueued:
				aIconIndex = dlStop ? ESuspendedShowIcon : EQuedShowIcon;
				break;
			case EDownloading:
				aIconIndex = dlStop ? ESuspendedShowIcon : EDownloadingShowIcon;		
				break;
			case EFailedDownload:
				aIconIndex = EFailedShowIcon;
				break;
			}
		}
	}

void CPodcastQueueView::UpdateFeedUpdateStateL()
	{
	TBool listboxDimmed = EFalse;

	if (iPodcastModel.FeedEngine().ClientState() != EIdle && iPodcastModel.ActiveFeedInfo()
			!= NULL && iPodcastModel.FeedEngine().ActiveClientUid() == iPodcastModel.ActiveFeedInfo()->Uid())
		{
		listboxDimmed = ETrue;
		}

	if ((iListContainer->Listbox()->IsDimmed() && !listboxDimmed) || (!iListContainer->Listbox()->IsDimmed() && listboxDimmed))
		{
		iListContainer->Listbox()->SetDimmed(listboxDimmed);
		}
	UpdateListboxItemsL();
	UpdateToolbar();
	}

void CPodcastQueueView::FormatShowInfoListBoxItemL(CShowInfo& aShowInfo, TInt aSizeDownloaded)
	{
	TBuf<32> infoSize;
	TInt iconIndex = 0;	
	TBuf<KMaxShortDateFormatSpec*2> showDate;
	GetShowIcons(&aShowInfo, iconIndex);	
	
	if(aSizeDownloaded > 0)
		{
		if (aShowInfo.ShowSize() > 0)
			{
				infoSize.Format(KSizeDownloadingOf(), ((float) aSizeDownloaded / (float) KSizeMb),
						((float)aShowInfo.ShowSize() / (float)KSizeMb));
			}
		else
			{
			infoSize.Format(KShowsSizeFormatS60(), (float)aSizeDownloaded / (float)KSizeMb);
			}
		}
	else if (aShowInfo.ShowSize() > 0)
		{
		infoSize.Format(KShowsSizeFormatS60(), (float)aShowInfo.ShowSize() / (float)KSizeMb);
		} 
	else {
		infoSize = KNullDesC();	
	}

	if (aShowInfo.PubDate().Int64() == 0)
		{
		showDate = KNullDesC();
		}
	else
		{
		aShowInfo.PubDate().FormatL(showDate, KDateFormatShort());
		}

	if(aShowInfo.LastError() != KErrNone)
		{
		TBuf<KSizeBufLen> errorBuffer;
		iEikonEnv->GetErrorText(errorBuffer, aShowInfo.LastError());
		iListboxFormatbuffer.Format(KShowErrorFormat(), iconIndex, &aShowInfo.Title(), &errorBuffer);
		}
	else	
		{
		if(aShowInfo.DownloadState() == EDownloaded)
			{
			iListboxFormatbuffer.Format(KShowFormat(), iconIndex, &aShowInfo.Title(), &showDate, &aShowInfo.Description());
			}
		else
			{
			iListboxFormatbuffer.Format(KShowFormat(), iconIndex, &aShowInfo.Title(), &showDate, &infoSize);
			}
		}
	}

void CPodcastQueueView::UpdateShowItemDataL(CShowInfo* aShowInfo,TInt aIndex, TInt aSizeDownloaded)
{
	FormatShowInfoListBoxItemL(*aShowInfo, aSizeDownloaded);
	iItemArray->Delete(aIndex);
	if(aIndex>= iItemArray->MdcaCount())
		{
		iItemArray->AppendL(iListboxFormatbuffer);
		}
	else
		{
		iItemArray->InsertL(aIndex, iListboxFormatbuffer);
		}
}

void CPodcastQueueView::UpdateShowItemL(TUint aUid, TInt aSizeDownloaded)
{
	RShowInfoArray& array = iPodcastModel.ActiveShowList();
	
	for (int i=0;i<array.Count();i++) {
		if (array[i]->Uid() == aUid) {
			UpdateShowItemDataL(array[i], i, aSizeDownloaded);
			if (iListContainer->Listbox()->TopItemIndex() <= i &&
				iListContainer->Listbox()->BottomItemIndex() >= i) {
					iListContainer->Listbox()->DrawItem(i);
			}
		}
	}
}

void CPodcastQueueView::UpdateListboxItemsL()
	{
	_LIT(KPanicCategory, "CPodcastQueueView::UpdateListboxItemsL");

	if (iListContainer->IsVisible())
		{
		TListItemProperties itemProps;
		TInt len = 0;

		iPodcastModel.GetShowsDownloading();
	
		RShowInfoArray &fItems = iPodcastModel.ActiveShowList();
		len = fItems.Count();

		if (iListContainer->Listbox() != NULL)
			{
			TBool allUidsMatch = EFalse;

			if (len == iListContainer->Listbox()->Model()->NumberOfItems())
				{
				allUidsMatch = ETrue;
				TUint itemId = 0;
				for (TInt loop = 0; loop< len; loop++)
					{
					itemId = iItemIdArray[loop];
					if (fItems[loop]->Uid() != itemId)
						{
						allUidsMatch = EFalse;
						break;
						}
					}
				}

			if (allUidsMatch && len > 0)
				{
				for (TInt loop = 0; loop< len; loop++)
					{					
					UpdateShowItemDataL(fItems[loop], loop);	
					}
				}
			else
				{
				iListContainer->Listbox()->ItemDrawer()->ClearAllPropertiesL();
				iListContainer->Listbox()->Reset();
				iItemIdArray.Reset();
				iItemArray->Reset();

				if (len > 0)
					{
					for (TInt i=0; i<len; i++)
						{
						CShowInfo *si = fItems[i];
						FormatShowInfoListBoxItemL(*si);
						iItemIdArray.Append(si->Uid());						
						iItemArray->AppendL(iListboxFormatbuffer);
						}
					}
				else
					{
					iItemArray->Reset();
					iItemIdArray.Reset();
					
					itemProps.SetDimmed(ETrue);
					itemProps.SetHiddenSelection(ETrue);
					}
				iListContainer->Listbox()->HandleItemAdditionL();
				}				
			}
		}
	iListContainer->ScrollToVisible();
	}

/** 
 * Command handling function intended for overriding by sub classes. 
 * Default implementation is empty.  
 * @param aCommand ID of the command to respond to. 
 */
void CPodcastQueueView::HandleCommandL(TInt aCommand)
	{
	switch (aCommand)
		{
		case EPodcastRemoveAllDownloads:
			{
			TBuf<KMaxMessageLength> msg;
			iEikonEnv->ReadResourceL(msg, R_CLEAR_QUERY);
												
			if(ShowQueryMessage(msg))
				{
				iPodcastModel.ShowEngine().RemoveAllDownloads();
				UpdateListboxItemsL();
				}
			}
			break;
		case EPodcastRemoveDownload:
			{
			TInt index = iListContainer->Listbox()->CurrentItemIndex();
			if (index >= 0 && index < iPodcastModel.ActiveShowList().Count())
				{
				if (iPodcastModel.ShowEngine().RemoveDownloadL(iPodcastModel.ActiveShowList()[index]->Uid()))
					{
						iItemArray->Delete(index);
						iItemIdArray.Remove(index);						
						iListContainer->Listbox()->HandleItemRemovalL();
						iListContainer->Listbox()->SetCurrentItemIndex(index - 1 > 0 ? index - 1 : 0);
						iListContainer->Listbox()->DrawNow();
						
						delete iPodcastModel.ActiveShowList()[index];
						iPodcastModel.ActiveShowList().Remove(index);
					}
				}
			}
			break;
		case EPodcastSuspendDownloads:
			{
			iPodcastModel.ShowEngine().SuspendDownloads();
			UpdateListboxItemsL();
			}
			break;
		case EPodcastResumeDownloads:
			{
			iPodcastModel.ShowEngine().ResumeDownloadsL();
			UpdateListboxItemsL();
			}
			break;
		default:
			CPodcastListView::HandleCommandL(aCommand);
			break;
		}
	UpdateToolbar();
	}
	
void CPodcastQueueView::DynInitMenuPaneL(TInt aResourceId,CEikMenuPane* aMenuPane)
{
	if(aResourceId == R_PODCAST_SHOWSVIEW_MENU)
		{
		TBool updatingState = iPodcastModel.FeedEngine().ClientState() != EIdle && iPodcastModel.FeedEngine().ActiveClientUid() == iPodcastModel.ActiveFeedInfo()->Uid();
		aMenuPane->SetItemDimmed(EPodcastMarkAllPlayed, ETrue);
		}
	else if (aResourceId == R_SHOWVIEW_POPUP_MENU)
		{
		TInt index = iListContainer->Listbox()->CurrentItemIndex();
		if (index >= 0 && index < iPodcastModel.ActiveShowList().Count())
			{
			CShowInfo *info = iPodcastModel.ActiveShowList()[index];
			}

		}

}

void CPodcastQueueView::UpdateToolbar()
{
	CAknToolbar* toolbar = Toolbar();
	
	RShowInfoArray &fItems = iPodcastModel.ActiveShowList();
	TInt itemCnt = fItems.Count();

	TBool hideDownloadShowCmd = EFalse;
	TBool dimDownloadShowCmd = EFalse;
	TBool hideSetPlayed = EFalse;
	TBool updatingState = EFalse;
	
	if(iListContainer->Listbox() != NULL)
	{
		TInt index = iListContainer->Listbox()->CurrentItemIndex();
		
		if(index>= 0 && index < itemCnt)
		{
			switch(fItems[index]->DownloadState())
				{
				case ENotDownloaded:
				case EFailedDownload:
					hideDownloadShowCmd = EFalse;
					dimDownloadShowCmd = EFalse;
					break;
				case EQueued:
				case EDownloading:
					hideDownloadShowCmd = EFalse;
					dimDownloadShowCmd = ETrue;
					break;
				case EDownloaded:
					hideDownloadShowCmd = ETrue;
					break;
				}
				
			if(fItems[index]->PlayState() == EPlayed) {
				hideSetPlayed = ETrue;
			}
		}
	}
	
	toolbar->HideItem(EPodcastUpdateFeed, ETrue, ETrue );
	toolbar->HideItem(EPodcastCancelUpdateAllFeeds, ETrue, ETrue );
	toolbar->HideItem(EPodcastDownloadShow, ETrue, ETrue );
	toolbar->HideItem(EPodcastDeleteShow, ETrue, ETrue);
	toolbar->HideItem(EPodcastMarkAsPlayed, ETrue, ETrue );
	toolbar->HideItem(EPodcastMarkAsUnplayed, ETrue, ETrue );
	toolbar->HideItem(EPodcastShowInfo, ETrue, ETrue );
	
	toolbar->HideItem(EPodcastRemoveDownload, EFalse, ETrue);
	toolbar->HideItem(EPodcastRemoveAllDownloads, EFalse, ETrue);
	toolbar->SetItemDimmed(EPodcastRemoveDownload, itemCnt == 0, ETrue);
	toolbar->SetItemDimmed(EPodcastRemoveAllDownloads, itemCnt == 0, ETrue);
	toolbar->HideItem(EPodcastSuspendDownloads,iPodcastModel.SettingsEngine().DownloadSuspended(), ETrue);
	toolbar->HideItem(EPodcastResumeDownloads,!iPodcastModel.SettingsEngine().DownloadSuspended(), ETrue);
	
}

void CPodcastQueueView::HandleLongTapEventL( const TPoint& aPenEventLocation, const TPoint& /* aPenEventScreenLocation */)
{
	DP("CPodcastQueueView::HandleLongTapEventL BEGIN");
	
    if(iStylusPopupMenu)
    {
		iStylusPopupMenu->ShowMenu();
		iStylusPopupMenu->SetPosition(aPenEventLocation);
    }
	DP("CPodcastQueueView::HandleLongTapEventL END");
}

void CPodcastQueueView::DownloadQueueUpdatedL(TInt aDownloadingShows, TInt aQueuedShows)
	{
	((CPodcastAppUi*)AppUi())->UpdateQueueTab(aDownloadingShows+aQueuedShows);
	}

void CPodcastQueueView::FeedUpdateAllCompleteL(TFeedState aState)
	{
	UpdateListboxItemsL();
	UpdateToolbar();
	}
