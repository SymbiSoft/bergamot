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
#include <podcast.rsg>
#include <podcast.mbg>
#include <gulicon.h>

const TInt KSizeBufLen = 64;
const TInt KDefaultGran = 5;
_LIT(KSizeDownloadingOf, "(%.1f/%.1f MB)");
_LIT(KShowsSizeFormatS60, "(%.1f MB)");
_LIT(KChapterFormatting, "%03d");
_LIT(KShowFormat, "%d\t%S\t%S %S");

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
			0,
			0
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
	
	HBufC* emptyText =  iEikonEnv->AllocReadResourceLC(R_PODCAST_EMPTY_QUEUE);
	iListContainer->Listbox()->View()->SetListEmptyTextL(*emptyText);
	CleanupStack::PopAndDestroy(emptyText);	
	
	iListContainer->SetKeyEventListener(this);
	iPodcastModel.FeedEngine().AddObserver(this);
	iPodcastModel.ShowEngine().AddObserver(this);
	
    CAknToolbar *toolbar = Toolbar();
	if (toolbar)
		{
		toolbar->SetToolbarObserver(this);
		}

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
	}

TUid CPodcastShowsView::Id() const
	{
	return KUidPodcastShowsViewID;
	}

void CPodcastShowsView::DoActivateL(const TVwsViewId& aPrevViewId,
		TUid aCustomMessageId, const TDesC8& aCustomMessage)
	{
	switch (aCustomMessageId.iUid)
		{
		case EShowNewShows:
		case EShowAllShows:
		case EShowDownloadedShows:
		case EShowPendingShows:
			iCurrentCategory
					= (TPodcastClientShowCategory) aCustomMessageId.iUid;
			break;
		case EShowFeedShows:
			iCurrentCategory = EShowFeedShows;
			break;
		}

	CPodcastListView::DoActivateL(aPrevViewId, aCustomMessageId, aCustomMessage);
	
	if(aPrevViewId.iAppUid == KUidPodcast)
		{
		switch(aPrevViewId.iViewUid.iUid)
			{
			case 1: // BaseView
				iPreviousView = TVwsViewId(KUidPodcast, KUidPodcastBaseViewID);
			break;
			default:
				if (iCurrentCategory == EShowDownloadedShows) {
					iPreviousView = TVwsViewId(KUidPodcast, KUidPodcastBaseViewID);
				} else {
					iPreviousView = TVwsViewId(KUidPodcast, KUidPodcastFeedViewID);
				}
				break;
			}
		}
	
	UpdateFeedUpdateStateL();
	UpdateToolbar();
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

void CPodcastShowsView::FeedInfoUpdated(TUint aFeedUid)
	{
	if (iPodcastModel.ActiveFeedInfo() != NULL && aFeedUid == 
		iPodcastModel.ActiveFeedInfo()->Uid())
		{
		TRAP_IGNORE(UpdateFeedUpdateStateL());
		// Title might have changed
		TRAP_IGNORE(UpdateListboxItemsL());
		}
	}

void CPodcastShowsView::FeedUpdateCompleteL(TUint aFeedUid)
	{
	if (iPodcastModel.ActiveFeedInfo() != NULL && iPodcastModel.ActiveFeedInfo()->Uid() == aFeedUid)
		{
		UpdateFeedUpdateStateL();
		}
	}

void CPodcastShowsView::FeedUpdateAllCompleteL()
	{
	
	}

void CPodcastShowsView::FeedDownloadUpdatedL(TUint aFeedUid, TInt /*aPercentOfCurrentDownload*/)
	{
	if (iPodcastModel.ActiveFeedInfo() != NULL && iPodcastModel.ActiveFeedInfo()->Uid() == aFeedUid)
		{
		UpdateFeedUpdateStateL();
		}
	}

void CPodcastShowsView::HandleListBoxEventL(CEikListBox* /*aListBox*/,
		TListBoxEvent aEventType)
	{
	switch (aEventType)
		{
		case EEventEnterKeyPressed:
		case EEventItemDoubleClicked:
		case EEventItemActioned:
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
			}
		}
	}
	

void CPodcastShowsView::UpdateFeedUpdateStateL()
	{
	TBool listboxDimmed = EFalse;

	if (iPodcastModel.FeedEngine().ClientState() != ENotUpdating && iPodcastModel.ActiveFeedInfo()
			!= NULL && iPodcastModel.FeedEngine().ActiveClientUid() == iPodcastModel.ActiveFeedInfo()->Uid())
		{
		listboxDimmed = ETrue;
		}

	if ((iListContainer->Listbox()->IsDimmed() && !listboxDimmed) || (!iListContainer->Listbox()->IsDimmed() && listboxDimmed))
		{
		iListContainer->Listbox()->SetDimmed(listboxDimmed);
		}
	
	UpdateToolbar();
	}

void CPodcastShowsView::UpdateShowItemDataL(CShowInfo* aShowInfo,TInt aIndex, TInt aSizeDownloaded)
{
	TBuf<32> infoSize;
	TInt iconIndex = 0;	
	TBuf<KMaxShortDateFormatSpec*2> showDate;
	
	GetShowIcons(aShowInfo, iconIndex);		
	if(aShowInfo->ShowType() == EAudioBook)
	{		
		infoSize.Format(KChapterFormatting(), aShowInfo->TrackNo());	
					
		if(aShowInfo->PlayTime() != 0)
			{
			TInt playtime = aShowInfo->PlayTime();
			TInt hour = playtime/3600;
			playtime = playtime-(hour*3600);
			
			TInt sec = (playtime%60);
			TInt min = (playtime/60);
			showDate.Format(_L("%01d:%02d:%02d"),hour, min, sec);
			}
		else
			{
			HBufC* unknown =  iEikonEnv->AllocReadResourceLC(R_PODCAST_ONPHONE_STATUS_UNKNOWN);
			showDate = *unknown;
			CleanupStack::PopAndDestroy(unknown);
			}
	}
	else
	{
		if(aSizeDownloaded > 0)
		{
			infoSize.Format(KSizeDownloadingOf(), ((float) aSizeDownloaded / (float) KSizeMb),
				((float)aShowInfo->ShowSize() / (float)KSizeMb));
		}
		else if (aShowInfo->ShowSize() > 0)
		{
			infoSize.Format(KShowsSizeFormatS60(), (float)aShowInfo->ShowSize() / (float)KSizeMb);
		} 
		else {
			infoSize = KNullDesC();	
		}
		
		if (aShowInfo->PubDate().Int64() == 0)
			{
			showDate = KNullDesC();
			}
		else
			{
			aShowInfo->PubDate().FormatL(showDate, KDateFormatShort());
			}
	}
	
	iListboxFormatbuffer.Format(KShowFormat(), iconIndex, &aShowInfo->Title(), &showDate, &infoSize);
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
	if (iListContainer->IsVisible())
		{
		TListItemProperties itemProps;
		TBuf<KSizeBufLen> showSize;
		TBuf<KMaxShortDateFormatSpec*2> showDate;

		TInt len = 0;

		switch (iCurrentCategory)
			{
			case EShowAllShows:

				iPodcastModel.GetAllShows();
				break;
			case EShowNewShows:

				iPodcastModel.GetNewShows();
				break;
			case EShowDownloadedShows:

				iPodcastModel.GetShowsDownloaded();
				break;
			case EShowPendingShows:

				iPodcastModel.GetShowsDownloading();
				break;
			default:
				iPodcastModel.GetShowsByFeed(iPodcastModel.ActiveFeedInfo()->Uid());
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
					iListContainer->Listbox()->DrawItem(loop);					
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
						iItemIdArray.Append(si->Uid());

						if (si->ShowType() == EAudioBook)
							{
							showSize.Format(KChapterFormatting(), si->TrackNo());
							if (si->PlayTime() != 0)
								{
								TInt playtime = si->PlayTime();
								TInt hour = playtime/3600;
								playtime = playtime-(hour*3600);

								TInt sec = (playtime%60);
								TInt min = (playtime/60);
								showDate.Format(_L("%01d:%02d:%02d"),hour, min, sec);
								}
							else
								{
								HBufC* unknown=  iEikonEnv->AllocReadResourceLC(R_PODCAST_ONPHONE_STATUS_UNKNOWN);
								showDate = *unknown;
								CleanupStack::PopAndDestroy(unknown);
								}
							}
						else
							{
							if (si->ShowSize() == 0)
								{
								showSize = KNullDesC();
								}
							else
								{
								showSize.Format(KShowsSizeFormatS60(),
										(float)si->ShowSize()/ (float)KSizeMb);
								}

							if (si->PubDate().Int64() == 0)
								{
								showDate = KNullDesC();
								}
							else
								{
								si->PubDate().FormatL(showDate, KDateFormatShort());
								}
							}
						TInt iconIndex = 0;						
						GetShowIcons(si, iconIndex);
						iListboxFormatbuffer.Format(KShowFormat(), iconIndex, &si->Title(), &showDate, &showSize);
						iItemArray->AppendL(iListboxFormatbuffer);
						}
					}
				else
					{					
					iEikonEnv->ReadResourceL(showSize,R_PODCAST_SHOWS_NO_ITEMS);
					showSize.Insert(0, _L("0\t"));
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
				iPodcastModel.ShowEngine().UpdateShow(info);
				if (iPodcastModel.SettingsEngine().SelectUnplayedOnly())
					{
					iItemArray->Delete(index);
					iItemIdArray.Remove(index);
					iPodcastModel.ActiveShowList().Remove(index);
					delete info;
					iListContainer->Listbox()->HandleItemRemovalL();
					iListContainer->Listbox()->SetCurrentItemIndex(index - 1 > 0 ? index - 1 : 0);
					iListContainer->Listbox()->DrawNow();
					}
				else {
					UpdateShowItemDataL(iPodcastModel.ActiveShowList()[index], index, 0);
					iListContainer->Listbox()->DrawItem(index);					
				}
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
				iPodcastModel.ShowEngine().UpdateShow(info);

				UpdateShowItemDataL(iPodcastModel.ActiveShowList()[index], index, 0);
				iListContainer->Listbox()->DrawItem(index);					
				}
			}
			break;
		case EPodcastSetOrderAudioBook:
			{
			if (iPodcastModel.ActiveFeedInfo() != NULL)
				{
				/*CPodcastClientSetAudioBookOrderDlg* dlg = new (ELeave) CPodcastClientSetAudioBookOrderDlg(iPodcastModel, iPodcastModel.ActiveFeedInfo()->Uid());
				 dlg->ExecuteLD(R_PODCAST_AUDIOBOOK_PLAYORDERDLG);*/
				UpdateListboxItemsL();
				}
			}
			break;
		case EPodcastAddAudioBookChapter:
			{
			if (iPodcastModel.ActiveFeedInfo() != NULL)
				{

				CDesCArrayFlat* mimeArray= iEikonEnv->ReadDesCArrayResourceL(R_PODCAST_NEW_AUDIOBOOK_MIMEARRAY);
				CleanupStack::PushL(mimeArray);
				CDesCArrayFlat* fileNameArray = new (ELeave) CDesCArrayFlat(KDefaultGran);
				CleanupStack::PushL(fileNameArray);
				HBufC* dialogTitle= iEikonEnv->AllocReadResourceLC(R_PODCAST_NEW_AUDIOBOOK_SELECT_FILES);
				/*	TQikDefaultFolderDescription defaultFolder;
				 defaultFolder.SetDefaultFolder(EQikFileHandlingDefaultFolderAudio);

				 if(CQikSelectFileDlg::RunDlgLD(*mimeArray, *fileNameArray, defaultFolder, dialogTitle, EQikSelectFileDialogEnableMultipleSelect|EQikSelectFileDialogSortByName))*/
					{
					if (fileNameArray->Count() > 0)
						{
						iPodcastModel.FeedEngine().AddBookChaptersL(
								*iPodcastModel.ActiveFeedInfo(), fileNameArray);
						}
					}

				CleanupStack::PopAndDestroy(dialogTitle);
				CleanupStack::PopAndDestroy(fileNameArray);
				CleanupStack::PopAndDestroy(mimeArray);
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

				TBool isBook = (iPodcastModel.ActiveShowList()[index]->ShowType() == EAudioBook);
				if (iEikonEnv->QueryWinL(isBook?R_PODCAST_REMOVE_CHAPTER_TITLE:R_PODCAST_DELETE_SHOW_TITLE,
						isBook?R_PODCAST_REMOVE_CHAPTER_PROMPT:R_PODCAST_DELETE_SHOW_PROMPT))
					{
					if (isBook)
						{
						iPodcastModel.ShowEngine().DeleteShowL(iPodcastModel.ActiveShowList()[index]->Uid(), EFalse);
						}
					else
						{
						iPodcastModel.ShowEngine().DeleteShowL(iPodcastModel.ActiveShowList()[index]->Uid());
						}
					
					UpdateListboxItemsL();
					}
				}
			}
			break;
//		case EPodcastUpdateLibrary:
//			iPodcastModel.ShowEngine().CheckFilesL();
//			break;
		case EPodcastShowUnplayedOnlyOn:
			iPodcastModel.SettingsEngine().SetSelectUnplayedOnly(ETrue);
			UpdateListboxItemsL();
			break;
		case EPodcastShowUnplayedOnlyOff:
			iPodcastModel.SettingsEngine().SetSelectUnplayedOnly(EFalse);
			UpdateListboxItemsL();
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
				iPodcastModel.ShowEngine().AddDownloadL(iPodcastModel.ActiveShowList()[index]);
				UpdateShowItemL(iPodcastModel.ActiveShowList()[index]->Uid(),-1);
				}
			if (iPodcastModel.ShowEngine().DownloadsStopped()) {
				if(iEikonEnv->QueryWinL(R_PODCAST_ENABLE_DOWNLOADS_TITLE, R_PODCAST_ENABLE_DOWNLOADS_PROMPT))
				{
					iPodcastModel.ShowEngine().ResumeDownloadsL();
				} else {
					iPodcastModel.ShowEngine().StopDownloads();
				}
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
			iPodcastModel.ShowEngine().RemoveAllDownloads();
			UpdateListboxItemsL();
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
		default:
			CPodcastListView::HandleCommandL(aCommand);
			break;
		}
	UpdateToolbar();
	}
	
	void CPodcastShowsView::DynInitMenuPaneL(TInt /*aResourceId*/,CEikMenuPane* /*aMenuPane*/)
	{
//		if(aResourceId == R_PODCAST_SHOWSVIEW_MENU)
//		{
//			
//			RShowInfoArray &fItems = iPodcastModel.ActiveShowList();
//			TInt itemCnt = fItems.Count();
//			TBool removeDeleteShowCmd = ETrue;
//			TBool removeDownloadShowCmd = ETrue;
//			TBool removeRemoveSuspendCmd = EFalse;
//			TBool updatingState = (iCurrentCategory != EShowDownloadedShows && iPodcastModel.FeedEngine().ClientState() != ENotUpdating && iPodcastModel.FeedEngine().ActiveClientUid() == iPodcastModel.ActiveFeedInfo()->Uid());
//			TBool removeSetPlayed = EFalse;
//			
//			if(iListContainer->Listbox() != NULL)
//			{
//				TInt index = iListContainer->Listbox()->CurrentItemIndex();
//				
//				if(index>= 0 && index < itemCnt)
//				{							
//					if(fItems[index]->DownloadState() != EQueued &&  fItems[index]->DownloadState() != EDownloading)
//					{			
//					}
//					else
//					{
//						if(fItems[index]->DownloadState() == EQueued)
//						{
//							aMenuPane->SetItemTextL(EPodcastRemoveDownload, R_PODCAST_PLAYER_REMOVE_DL_CMD);							
//						}
//						else
//						{					
//							aMenuPane->SetItemTextL(EPodcastRemoveDownload, R_PODCAST_PLAYER_SUSPEND_DL_CMD);
//							removeRemoveSuspendCmd = ETrue;
//						}
//					}
//					
//					removeDeleteShowCmd = fItems[index]->DownloadState() != EDownloaded || updatingState || 
//						(iPodcastModel.PlayingPodcast() != NULL && fItems[index] == iPodcastModel.PlayingPodcast() /*&& iPodcastModel.SoundEngine().State() == ESoundEnginePlaying*/);
//#pragma message("LAPER FIX SOUNDENGINE HANDLING HERE")
//					if(fItems[index]->DownloadState() == ENotDownloaded)
//					{
//						removeDownloadShowCmd = EFalse;
//					}
//					
//					if(fItems[index]->PlayState() == EPlayed) {
//						removeSetPlayed = ETrue;
//					}
//					
//				}
//				
//			}		
//			
//			TBool iUpdateRunning = iPodcastModel.FeedEngine().ClientState() != ENotUpdating && iPodcastModel.ActiveFeedInfo() != NULL && iPodcastModel.FeedEngine().ActiveClientUid() == iPodcastModel.ActiveFeedInfo()->Uid();
//			
//			aMenuPane->SetItemDimmed(EPodcastUpdateFeed, iUpdateRunning || (iCurrentCategory != EShowFeedShows || iPodcastModel.ActiveFeedInfo()->IsBookFeed()));
//			aMenuPane->SetItemDimmed(EPodcastCancelUpdateAllFeeds, !iUpdateRunning);
//			
//			aMenuPane->SetItemDimmed(EPodcastDownloadShow, removeDownloadShowCmd || updatingState);
//			
//			aMenuPane->SetItemDimmed(EPodcastMarkAllPlayed, updatingState);
//			
//			aMenuPane->SetItemDimmed(EPodcastDeleteShow, removeDeleteShowCmd);					
//			TBool isOrdinaryList = iCurrentCategory == EShowDownloadedShows || (iCurrentCategory == EShowFeedShows && !(iPodcastModel.ActiveFeedInfo()?iPodcastModel.ActiveFeedInfo()->IsBookFeed():ETrue));
//			aMenuPane->SetItemDimmed(EPodcastDeleteAllPlayed, (!(!updatingState && !iPodcastModel.SettingsEngine().SelectUnplayedOnly()) || !isOrdinaryList));						
//			
//			aMenuPane->SetItemDimmed(EPodcastSetOrderAudioBook, !(EShowFeedShows == iCurrentCategory && (iPodcastModel.ActiveFeedInfo()?iPodcastModel.ActiveFeedInfo()->IsBookFeed(): ETrue)));
//			aMenuPane->SetItemDimmed(EPodcastAddAudioBookChapter, !(EShowFeedShows == iCurrentCategory && (iPodcastModel.ActiveFeedInfo()?iPodcastModel.ActiveFeedInfo()->IsBookFeed(): ETrue)));
//			
//			aMenuPane->SetItemDimmed(EPodcastRemoveAllDownloads, ETrue);
//			aMenuPane->SetItemDimmed(EPodcastMarkAsPlayed, removeSetPlayed || !itemCnt);
//			aMenuPane->SetItemDimmed(EPodcastMarkAsUnplayed, !removeSetPlayed || !itemCnt);
//			
//			
//			switch(iCurrentCategory)
//			{	
//			case EShowPendingShows:
//				{
//					aMenuPane->SetItemDimmed(EPodcastMarkAllPlayed, ETrue);
//					aMenuPane->SetItemDimmed(EPodcastUpdateLibrary, ETrue);
//					aMenuPane->SetItemDimmed(EPodcastRemoveDownload, (!itemCnt|| removeRemoveSuspendCmd));				
//					aMenuPane->SetItemDimmed(EPodcastRemoveAllDownloads, (!iPodcastModel.ShowEngine().DownloadsStopped() || !itemCnt));				
//					aMenuPane->SetItemDimmed(EPodcastStopDownloads, (iPodcastModel.ShowEngine().DownloadsStopped()));
//					aMenuPane->SetItemDimmed(EPodcastResumeDownloads, (!iPodcastModel.ShowEngine().DownloadsStopped()));			
//					aMenuPane->SetItemDimmed(EPodcastShowUnplayedOnly, ETrue);				
//					aMenuPane->SetItemDimmed(EPodcastDownloadShow, ETrue);
//					aMenuPane->SetItemDimmed(EPodcastMarkAsPlayed, ETrue);
//					aMenuPane->SetItemDimmed(EPodcastMarkAsUnplayed, ETrue);
//					aMenuPane->SetItemDimmed(EPodcastDeleteShow, ETrue);
//				}break;
//			case EShowDownloadedShows:
//				{
//					aMenuPane->SetItemDimmed(EPodcastMarkAllPlayed, updatingState);
//					aMenuPane->SetItemDimmed(EPodcastUpdateLibrary, EFalse);
//					aMenuPane->SetItemDimmed(EPodcastRemoveDownload, ETrue);
//					aMenuPane->SetItemDimmed(EPodcastStopDownloads, ETrue);
//					aMenuPane->SetItemDimmed(EPodcastResumeDownloads, ETrue);
//					aMenuPane->SetItemDimmed(EPodcastShowUnplayedOnly, EFalse);
//				}break;
//			default:
//				{
//					aMenuPane->SetItemDimmed(EPodcastRemoveDownload, ETrue);					
//					aMenuPane->SetItemDimmed(EPodcastStopDownloads, ETrue);
//					aMenuPane->SetItemDimmed(EPodcastResumeDownloads, ETrue);
//					aMenuPane->SetItemDimmed(EPodcastMarkAllPlayed, updatingState);
//					aMenuPane->SetItemDimmed(EPodcastUpdateLibrary, ETrue);
//					aMenuPane->SetItemDimmed(EPodcastShowUnplayedOnly, EFalse);
//				}break;
//			}	
//			
//			
//	} else if (aResourceId == R_ONOFF_MENU) {
//		if (iPodcastModel.SettingsEngine().SelectUnplayedOnly())
//		{
//		aMenuPane->SetItemButtonState(EPodcastShowUnplayedOnlyOff, EEikMenuItemSymbolIndeterminate);
//		aMenuPane->SetItemButtonState(EPodcastShowUnplayedOnlyOn, EEikMenuItemSymbolOn);
//		} else {
//		aMenuPane->SetItemButtonState(EPodcastShowUnplayedOnlyOn, EEikMenuItemSymbolIndeterminate);
//		aMenuPane->SetItemButtonState(EPodcastShowUnplayedOnlyOff, EEikMenuItemSymbolOn);
//		}
//
//	}
}

void CPodcastShowsView::OfferToolbarEventL(TInt aCommand)
	{
	HandleCommandL(aCommand);
	}

void CPodcastShowsView::DynInitToolbarL (TInt /*aResourceId*/, CAknToolbar * /*aToolbar*/)
	{
	DP("CPodcastShowsView::DynInitToolbarL");
	UpdateToolbar();
	}

void CPodcastShowsView::UpdateToolbar()
{
	CAknToolbar* toolbar = Toolbar();
	
	RShowInfoArray &fItems = iPodcastModel.ActiveShowList();
	TInt itemCnt = fItems.Count();

	TBool hideDownloadShowCmd = ETrue;
	TBool hideSetPlayed = EFalse;
	TBool updatingState = (iCurrentCategory != EShowDownloadedShows && iPodcastModel.FeedEngine().ClientState() != ENotUpdating && iPodcastModel.FeedEngine().ActiveClientUid() == iPodcastModel.ActiveFeedInfo()->Uid());
	
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
	case EShowAllShows:
	case EShowNewShows:
	case EShowFeedShows:
	case EShowDownloadedShows:
		toolbar->HideItem(EPodcastUpdateFeed, updatingState, ETrue ); 
		toolbar->HideItem(EPodcastCancelUpdateAllFeeds, !updatingState, ETrue );
		toolbar->HideItem(EPodcastDownloadShow, hideDownloadShowCmd, ETrue );
		toolbar->HideItem(EPodcastDeleteShow, !hideDownloadShowCmd, ETrue);
		toolbar->HideItem(EPodcastMarkAsPlayed, hideSetPlayed, ETrue );
		toolbar->HideItem(EPodcastMarkAsUnplayed, !hideSetPlayed, ETrue );
		
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
		
		toolbar->SetItemDimmed(EPodcastRemoveDownload, itemCnt == 0, ETrue);
		toolbar->SetItemDimmed(EPodcastRemoveAllDownloads, itemCnt == 0, ETrue);
		toolbar->HideItem(EPodcastStopDownloads,iPodcastModel.ShowEngine().DownloadsStopped(), ETrue);
		toolbar->HideItem(EPodcastResumeDownloads,!iPodcastModel.ShowEngine().DownloadsStopped(), ETrue);
		break;
	}
}
