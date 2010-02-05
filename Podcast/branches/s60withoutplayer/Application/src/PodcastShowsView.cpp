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

#include "PodcastShowsView.h"
#include "PodcastAppUi.h"
#include "ShowEngine.h"
#include "SettingsEngine.h"
#include "PodcastApp.h"
#include "Constants.h"
#include "imagehandler.h"

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
			EMbmPodcastShow_40x40,
			EMbmPodcastShow_40x40m,
			EMbmPodcastNew_40x40,
			EMbmPodcastNew_40x40m,
			EMbmPodcastShow_playing_40x40,
			EMbmPodcastShow_playing_40x40m,
			EMbmPodcastDownloading_40x40,
			EMbmPodcastDownloading_40x40m,
			EMbmPodcastSuspended_40x40,
			EMbmPodcastSuspended_40x40m,
			EMbmPodcastQueued_40x40,
			EMbmPodcastQueued_40x40m,
			EMbmPodcastOld_40x40,
			EMbmPodcastOld_40x40m,
			EMbmPodcastDownloaded_new_40x40,
			EMbmPodcastDownloaded_new_40x40m,
			EMbmPodcastShowwarning_40x40,
			EMbmPodcastShowwarning_40x40m,
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

CPodcastShowsView* CPodcastShowsView::NewL(CPodcastModel& aPodcastModel)
	{
	CPodcastShowsView* self = CPodcastShowsView::NewLC(aPodcastModel);
	CleanupStack::Pop(self);
	return self;
	}

CPodcastShowsView* CPodcastShowsView::NewLC(CPodcastModel& aPodcastModel)
	{
	CPodcastShowsView* self = new ( ELeave ) CPodcastShowsView(aPodcastModel);
	CleanupStack::PushL(self);
	self->ConstructL();
	return self;
	}

CPodcastShowsView::CPodcastShowsView(CPodcastModel& aPodcastModel) :
	iPodcastModel(aPodcastModel)
	{
	}

void CPodcastShowsView::ConstructL()
	{
	BaseConstructL(R_PODCAST_SHOWSVIEW);
	CPodcastListView::ConstructL();
	CArrayPtr< CGulIcon>* icons = new(ELeave) CArrayPtrFlat< CGulIcon>(1);
	CleanupStack::PushL(icons);
	TInt pos = 0;
	while (KIconArrayIds[pos] > 0)
		{
		// Load the bitmap for play icon	
		CFbsBitmap* bitmap= iEikonEnv->CreateBitmapL( _L("*"), KIconArrayIds[pos]);
		CleanupStack::PushL(bitmap);
		// Load the mask for play icon	
		CFbsBitmap* mask= iEikonEnv->CreateBitmapL( _L("*"), KIconArrayIds[pos+1] );
		CleanupStack::PushL(mask);
		// Append the play icon to icon array
		icons->AppendL(CGulIcon::NewL(bitmap, mask) );
		CleanupStack::Pop(2); // bitmap, mask
		pos+=2;
		}
	iListContainer->Listbox()->ItemDrawer()->FormattedCellData()->SetIconArrayL(icons);
	CleanupStack::Pop(icons); // icons
	iListContainer->Listbox()->SetListBoxObserver(this);
	
	iListContainer->SetKeyEventListener(this);
	iPodcastModel.FeedEngine().AddObserver(this);
	iPodcastModel.ShowEngine().AddObserver(this);
	
	iStylusPopupMenu = CAknStylusPopUpMenu::NewL( this , TPoint(0,0));
	TResourceReader reader;
	iCoeEnv->CreateResourceReaderLC(reader,R_SHOWVIEW_POPUP_MENU);
	iStylusPopupMenu->ConstructFromResourceL(reader);
	CleanupStack::PopAndDestroy();
	iImageHandler = CImageHandler::NewL(iEikonEnv->FsSession(), *this);
	}

TKeyResponse CPodcastShowsView::OfferKeyEventL(const TKeyEvent& aKeyEvent,TEventCode aType)
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
				if (iCurrentCategory == EShowPendingShows) {
					HandleCommandL(EPodcastRemoveDownloadHardware);
				} else {
					HandleCommandL(EPodcastDeleteShowHardware);
				}
				break;
			}
			UpdateToolbar();
		}
	}
		return EKeyWasNotConsumed;
	}

CPodcastShowsView::~CPodcastShowsView()
	{
	iPodcastModel.ShowEngine().RemoveObserver(this);
	iPodcastModel.FeedEngine().RemoveObserver(this);
    if(iStylusPopupMenu)
        delete iStylusPopupMenu, iStylusPopupMenu = NULL;
	}

TUid CPodcastShowsView::Id() const
	{
	return KUidPodcastShowsViewID;
	}

void CPodcastShowsView::DoActivateL(const TVwsViewId& aPrevViewId,
		TUid aCustomMessageId, const TDesC8& aCustomMessage)
	{
	DP("CPodcastShowsView::DoActivateL BEGIN");
	
	switch (aCustomMessageId.iUid)
		{
		case EShowPendingShows:
			iCurrentCategory
					= (TPodcastClientShowCategory) aCustomMessageId.iUid;
			SetEmptyTextL(R_PODCAST_EMPTY_QUEUE);
			break;
		case EShowFeedShows:
			iCurrentCategory = EShowFeedShows;
			SetEmptyTextL(R_PODCAST_EMPTY_LIST);
			break;
		}

	CPodcastListView::DoActivateL(aPrevViewId, aCustomMessageId, aCustomMessage);
	iPreviousView = TVwsViewId(KUidPodcast, KUidPodcastFeedViewID);
	
	UpdateFeedUpdateStateL();
	UpdateToolbar();
	DP("CPodcastShowsView::DoActivateL END");
	}

void CPodcastShowsView::DoDeactivate()
	{
	CPodcastListView::DoDeactivate();
	}

// Engine callback when new shows are available
void CPodcastShowsView::ShowListUpdatedL()
	{
	UpdateListboxItemsL();
	}

void CPodcastShowsView::ShowDownloadUpdatedL(TInt aPercentOfCurrentDownload,
		TInt aBytesOfCurrentDownload, TInt /*aBytesTotal*/)
	{
	if (aPercentOfCurrentDownload == KOneHundredPercent)
		{
		iProgressAdded = EFalse;

		if (iCurrentCategory == EShowPendingShows)
			{
			iPodcastModel.GetShowsDownloading();
			UpdateListboxItemsL();
			}
		}
	else 
		{
		CShowInfo *info = iPodcastModel.ShowEngine().ShowDownloading();
		if (info) 
			{
			UpdateShowItemL(info->Uid(), aBytesOfCurrentDownload);
			}
		}
	}


void CPodcastShowsView::FeedDownloadStartedL(TUint aFeedUid)
	{
	// TODO make use of the fact that we know that the feed download is
	// started instead of checking feed engine states in UpdateFeedUpdateStateL.
	if (iPodcastModel.ActiveFeedInfo() != NULL
			&& iPodcastModel.ActiveFeedInfo()->Uid() == aFeedUid)
		{
		TRAP_IGNORE(UpdateFeedUpdateStateL());
		}	
	}

void CPodcastShowsView::FeedDownloadFinishedL(TUint aFeedUid, TInt /*aError*/)
	{
	// TODO make use of the fact that we know that the feed download is
	// finished instead of checking feed engine states in UpdateFeedUpdateStateL.
	if (iPodcastModel.ActiveFeedInfo() != NULL
			&& iPodcastModel.ActiveFeedInfo()->Uid() == aFeedUid)
		{
		TRAP_IGNORE(UpdateFeedUpdateStateL());
		// Title might have changed
		//TRAP_IGNORE(UpdateListboxItemsL());
		}
	}

void CPodcastShowsView::HandleListBoxEventL(CEikListBox* /*aListBox*/,
		TListBoxEvent aEventType)
	{
	switch (aEventType)
		{
		case EEventEnterKeyPressed:		
		case EEventItemActioned:
		case EEventItemClicked:
			{
			RShowInfoArray &fItems = iPodcastModel.ActiveShowList();
			TInt index = iListContainer->Listbox()->CurrentItemIndex();
			if (index>=0 && index< fItems.Count())
				{
				DP2("Handle event for podcast %S, downloadState is %d", &(fItems[index]->Title()), fItems[index]->DownloadState())
				;

				TPckgBuf<TInt> showUid;
				showUid() = fItems[index]->Uid();
#pragma message("LAPER Replace activate playview with activate playback in mpx")
			/*	AppUi()->ActivateLocalViewL(KUidPodcastPlayViewID,
						TUid::Uid(KActiveShowUIDCmd), showUid);*/
				}
			}
			break;
		default:
			break;
		}
		UpdateToolbar();
	}

void CPodcastShowsView::GetShowIcons(CShowInfo* aShowInfo, TInt& aIconIndex)
	{
	TBool dlStop = iPodcastModel.ShowEngine().DownloadsStopped();
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
	

void CPodcastShowsView::UpdateFeedUpdateStateL()
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

void CPodcastShowsView::FormatShowInfoListBoxItemL(CShowInfo& aShowInfo, TInt aSizeDownloaded)
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

void CPodcastShowsView::UpdateShowItemDataL(CShowInfo* aShowInfo,TInt aIndex, TInt aSizeDownloaded)
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

void CPodcastShowsView::UpdateShowItemL(TUint aUid, TInt aSizeDownloaded)
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

void CPodcastShowsView::UpdateListboxItemsL()
	{
	_LIT(KPanicCategory, "CPodcastShowsView::UpdateListboxItemsL");

	if (iListContainer->IsVisible())
		{
		TListItemProperties itemProps;
		TInt len = 0;

		switch (iCurrentCategory)
			{
			case EShowPendingShows:
				iPodcastModel.GetShowsDownloading();
				break;
			case EShowFeedShows:
				iPodcastModel.GetShowsByFeed(iPodcastModel.ActiveFeedInfo()->Uid());
				break;
			default:
				// how did we end up here?
				User::Panic(KPanicCategory, 1);
				break;
			}

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
void CPodcastShowsView::HandleCommandL(TInt aCommand)
	{
	switch (aCommand)
		{
		case EPodcastMarkAsPlayed:
			{
			TInt index = iListContainer->Listbox()->CurrentItemIndex();
			
			if (index >= 0 && index < iPodcastModel.ActiveShowList().Count())
				{
				CShowInfo *info = iPodcastModel.ActiveShowList()[index];
				info->SetPlayState(EPlayed);
				iPodcastModel.ShowEngine().UpdateShow(*info);
				UpdateShowItemDataL(iPodcastModel.ActiveShowList()[index], index, 0);
				iListContainer->Listbox()->DrawItem(index);					
				}
			}
			break;
		case EPodcastMarkAsUnplayed:
			{
			TInt index = iListContainer->Listbox()->CurrentItemIndex();
			if (index >= 0 && index < iPodcastModel.ActiveShowList().Count())
				{
				CShowInfo *info = iPodcastModel.ActiveShowList()[index];
				info->SetPlayState(ENeverPlayed);
				iPodcastModel.ShowEngine().UpdateShow(*info);

				UpdateShowItemDataL(iPodcastModel.ActiveShowList()[index], index, 0);
				iListContainer->Listbox()->DrawItem(index);					
				}
			}
			break;
		case EPodcastDeleteShowHardware:
		case EPodcastDeleteShow:
			{
			TInt index = iListContainer->Listbox()->CurrentItemIndex();

			if (index >= 0 && index < iPodcastModel.ActiveShowList().Count())
				{
			/*	if (iPodcastModel.PlayingPodcast()
						== iPodcastModel.ActiveShowList()[index] && iPodcastModel.SoundEngine().State() != ESoundEngineNotInitialized)
					{
					iPodcastModel.SoundEngine().Stop();
					}*/

				if (iEikonEnv->QueryWinL(R_PODCAST_DELETE_SHOW_TITLE, R_PODCAST_DELETE_SHOW_PROMPT))
					{
					iPodcastModel.ShowEngine().DeleteShowL(iPodcastModel.ActiveShowList()[index]->Uid());
					
					// and mark as played, and not downloaded
					CShowInfo *info = iPodcastModel.ActiveShowList()[index];
					info->SetDownloadState(ENotDownloaded);
					info->SetPlayState(EPlayed);
					iPodcastModel.ShowEngine().UpdateShow(*info);
					
					UpdateShowItemDataL(iPodcastModel.ActiveShowList()[index], index, 0);
					iListContainer->Listbox()->DrawItem(index);					
					}
				}
			}
			break;
		case EPodcastMarkAllPlayed:
			iPodcastModel.MarkSelectionPlayed();
			UpdateListboxItemsL();
			break;
		case EPodcastDownloadShow:
			{
			TInt index = iListContainer->Listbox()->CurrentItemIndex();
			if (index >= 0 && index < iPodcastModel.ActiveShowList().Count())
				{
				iPodcastModel.ShowEngine().AddDownloadL(*iPodcastModel.ActiveShowList()[index]);
				UpdateShowItemL(iPodcastModel.ActiveShowList()[index]->Uid(),-1);
				}
			}
			break;
		case EPodcastDeleteAllPlayed:
			{
			if (iEikonEnv->QueryWinL(R_PODCAST_DELETE_PLAYED_TITLE, R_PODCAST_DELETE_PLAYED_PROMPT))
				{
				iPodcastModel.ShowEngine().DeletePlayedShows(iPodcastModel.ActiveShowList());
				UpdateListboxItemsL();
				}
			}
			break;
		case EPodcastUpdateFeed:
			{

			if (iPodcastModel.ActiveFeedInfo()->Url().Length()>0)
				{
				TRAPD(error, iPodcastModel.FeedEngine().UpdateFeedL(iPodcastModel.ActiveFeedInfo()->Uid()));

				if (error != KErrNone)
					{
					HBufC
							* str =
							iEikonEnv->AllocReadResourceLC(R_PODCAST_FEEDS_UPDATE_ERROR);
					User::InfoPrint(*str);
					CleanupStack::PopAndDestroy(str);
					}
				}
			}
			break;
		case EPodcastCancelUpdateAllFeeds:
			iPodcastModel.FeedEngine().CancelUpdateAllFeeds();
			break;
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
		case EPodcastRemoveDownloadHardware:
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
		case EPodcastStopDownloads:
			{
			iPodcastModel.ShowEngine().StopDownloads();
			UpdateListboxItemsL();
			}
			break;
		case EPodcastResumeDownloads:
			{
			iPodcastModel.ShowEngine().ResumeDownloadsL();
			UpdateListboxItemsL();
			}
			break;
		case EPodcastShowInfo:
			{
			DisplayShowInfoDialogL();
			}break;
		default:
			CPodcastListView::HandleCommandL(aCommand);
			break;
		}
	UpdateToolbar();
	}
	
	void CPodcastShowsView::DynInitMenuPaneL(TInt /*aResourceId*/,CEikMenuPane* /*aMenuPane*/)
	{

	}
	
void CPodcastShowsView::ImageOperationCompleteL(TInt aError)
	{
	iLastImageHandlerError = aError;
	CActiveScheduler::Stop();
	}
	
void CPodcastShowsView::DisplayShowInfoDialogL()
	{
	TInt index = iListContainer->Listbox()->CurrentItemIndex();
	if (index >= 0 && index < iPodcastModel.ActiveShowList().Count())
		{
		CShowInfo* info = iPodcastModel.ActiveShowList()[index];
		TUint32 feedUid = info->FeedUid();							
		CFeedInfo* feedInfo = iPodcastModel.FeedEngine().GetFeedInfoByUid(feedUid);
		
		CPodcastImageMessageQueryDialog* note = new ( ELeave ) CPodcastImageMessageQueryDialog( (TDesC*)&info->Description(), (TDesC*)&info->Title() );
							
		note->PrepareLC( R_SHOW_INFO_NOTE ); // Adds to CleanupStack
		
		if(feedInfo && feedInfo->ImageFileName().Length()>0)
			{
			CFbsBitmap * bitmap = new (ELeave) CFbsBitmap;
			CleanupStack::PushL(bitmap);
			
			TRAPD(loaderror, iImageHandler->LoadFileAndScaleL(bitmap, feedInfo->ImageFileName(), TSize(KPodcastImageWidth, KPodcastImageHeight)));
			
			if(loaderror == KErrNone)
				{
				CActiveScheduler::Start();
				if(iLastImageHandlerError == KErrNone)
					{	
					CEikImage* image = static_cast<CEikImage*>(note->ControlOrNull(EPodcastShowInfoImage));
					image->SetBitmap(bitmap);
					CleanupStack::Pop(bitmap);
					bitmap = NULL;
					}
				else
					{				
					CleanupStack::PopAndDestroy(bitmap);
					bitmap = NULL;
					}
				}
			else
				{
				CleanupStack::PopAndDestroy(bitmap);
				bitmap = NULL;
				}
			}												
		
		note->RunLD(); 
		}
	}

void CPodcastShowsView::UpdateToolbar()
{
	CAknToolbar* toolbar = Toolbar();
	
	RShowInfoArray &fItems = iPodcastModel.ActiveShowList();
	TInt itemCnt = fItems.Count();

	TBool hideDownloadShowCmd = ETrue;
	TBool hideSetPlayed = EFalse;
	TBool updatingState = (iCurrentCategory == EShowFeedShows && 
			iPodcastModel.FeedEngine().ClientState() != EIdle && 
			iPodcastModel.FeedEngine().ActiveClientUid() == iPodcastModel.ActiveFeedInfo()->Uid());
	
	if(iListContainer->Listbox() != NULL)
	{
		TInt index = iListContainer->Listbox()->CurrentItemIndex();
		
		if(index>= 0 && index < itemCnt)
		{							
			if(fItems[index]->DownloadState() == ENotDownloaded)
				{
					hideDownloadShowCmd = EFalse;
				}
				
			if(fItems[index]->PlayState() == EPlayed) {
				hideSetPlayed = ETrue;
			}
		}
	}
	
	switch(iCurrentCategory) {
	case EShowFeedShows:
		toolbar->HideItem(EPodcastUpdateFeed, updatingState, ETrue ); 
		toolbar->HideItem(EPodcastCancelUpdateAllFeeds, !updatingState, ETrue );
		
		if (hideDownloadShowCmd) {
			toolbar->HideItem(EPodcastDownloadShow, ETrue, ETrue );
			toolbar->HideItem(EPodcastDeleteShow, EFalse, ETrue);
			toolbar->SetItemDimmed(EPodcastDeleteShow, updatingState, ETrue);
		} else {
			toolbar->HideItem(EPodcastDownloadShow, EFalse, ETrue );
			toolbar->HideItem(EPodcastDeleteShow, ETrue, ETrue);
			toolbar->SetItemDimmed(EPodcastDownloadShow, updatingState, ETrue);	
		}
		if (hideSetPlayed) {
			toolbar->HideItem(EPodcastMarkAsPlayed, ETrue, ETrue );
			toolbar->HideItem(EPodcastMarkAsUnplayed, EFalse, ETrue );
			toolbar->SetItemDimmed(EPodcastMarkAsUnplayed, updatingState, ETrue);
		} else {
			toolbar->HideItem(EPodcastMarkAsPlayed, EFalse, ETrue );
			toolbar->HideItem(EPodcastMarkAsUnplayed, ETrue, ETrue );
			toolbar->SetItemDimmed(EPodcastMarkAsPlayed, updatingState, ETrue);
		}
		toolbar->HideItem(EPodcastShowInfo, ETrue, ETrue );
		toolbar->HideItem(EPodcastRemoveDownload, ETrue, ETrue);
		toolbar->HideItem(EPodcastRemoveAllDownloads, ETrue, ETrue);
		toolbar->HideItem(EPodcastStopDownloads,ETrue, ETrue);
		toolbar->HideItem(EPodcastResumeDownloads,ETrue, ETrue);


		break;
	case EShowPendingShows:
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
		toolbar->HideItem(EPodcastStopDownloads,iPodcastModel.ShowEngine().DownloadsStopped(), ETrue);
		toolbar->HideItem(EPodcastResumeDownloads,!iPodcastModel.ShowEngine().DownloadsStopped(), ETrue);
		break;
	}
}

void CPodcastShowsView::HandleLongTapEventL( const TPoint& aPenEventLocation, const TPoint& /* aPenEventScreenLocation */)
{
	DP("CPodcastShowsView::HandleLongTapEventL BEGIN");
	if (iCurrentCategory == EShowPendingShows) {
		DP("CPodcastShowsView::HandleLongTapEventL END");
		return;
	}
	
    if(iStylusPopupMenu)
    {
		iStylusPopupMenu->ShowMenu();
		iStylusPopupMenu->SetPosition(aPenEventLocation);
    }
	DP("CPodcastShowsView::HandleLongTapEventL END");
}
