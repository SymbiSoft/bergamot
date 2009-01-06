/**
 * This file is a part of Escarpod Podcast project
 * (c) 2008 The Bergamot Project
 * (c) 2008 Teknolog (Sebastian Brännström)
 * (c) 2008 Anotherguest (Lars Persson)
 */

#include "PodcastShowsView.h"
#include "PodcastAppUi.h"
#include "PodcastPlayView.h"
#include "ShowEngine.h"
#include "SettingsEngine.h"
#include "SoundEngine.h"
#include "PodcastApp.h"
#include "Constants.h"
#include <aknnavide.h> 
#include <podcast.rsg>
#include <podcast.mbg>
#include <gulicon.h>
const TInt KSizeBufLen = 64;
const TInt KDefaultGran = 5;
_LIT(KSizeDownloadingOf, "%S/%S");
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
			EMbmPodcastAudiobookchapter_40x40,
			EMbmPodcastAudiobookchapter_40x40m,
			0,
			0
	};

class CPodcastShowsContainer : public CCoeControl
	{
public:
	CPodcastShowsContainer();
	~CPodcastShowsContainer();
	void ConstructL(const TRect& aRect);
protected:
	CAknNavigationDecorator* iNaviDecorator;
	CAknNavigationControlContainer* iNaviPane;
	};

CPodcastShowsContainer::CPodcastShowsContainer()
	{
	}

void CPodcastShowsContainer::ConstructL(const TRect& aRect)
	{
	CreateWindowL();

	// Set the windows size
	SetRect(aRect);

	// Activate the window, which makes it ready to be drawn
	ActivateL();
	}

CPodcastShowsContainer::~CPodcastShowsContainer()
	{
	delete iNaviDecorator;
	}

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
	}

void CPodcastShowsView::DoDeactivate()
	{
	CPodcastListView::DoDeactivate();
	}

// Engine callback when new shows are available
void CPodcastShowsView::ShowListUpdated()
	{
	TRAP_IGNORE(UpdateListboxItemsL());
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
		case EEventItemClicked:
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
				AppUi()->ActivateLocalViewL(KUidPodcastPlayViewID,
						TUid::Uid(KActiveShowUIDCmd), showUid);
				}
			}
			break;
		default:
			break;
		}
	}

void CPodcastShowsView::GetShowIcons(CShowInfo* aShowInfo, TInt& aIconIndex)
	{
	TBool dlStop = iPodcastModel.ShowEngine().DownloadsStopped();
	TUint showDownloadingUid = iPodcastModel.ShowEngine().ShowDownloading() ? iPodcastModel.ShowEngine().ShowDownloading()->Uid() : 0;
	TUint showPlayingUid = iPodcastModel.PlayingPodcast() ? iPodcastModel.PlayingPodcast()->Uid() : 0;
	TBool playingOrPaused = iPodcastModel.SoundEngine().State() == ESoundEnginePlaying || 
							iPodcastModel.SoundEngine().State() == ESoundEnginePaused;
	
	if (aShowInfo->ShowType() == EAudioBook)
		{
		aIconIndex = EAudiobookChapterIcon;
		}
	else if (showDownloadingUid == aShowInfo->Uid())
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
				aIconIndex = EShowIcon;
				break;
			case ENotDownloaded:
				aIconIndex = ENewShowIcon;
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
			TBuf<KSizeBufLen> dlSize;
			TBuf<KSizeBufLen> totSize;
			
			totSize.Format(KShowsSizeFormat(), (float)aShowInfo->ShowSize() / (float)KSizeMb);
			dlSize.Format(KShowsSizeFormat(), (float) aSizeDownloaded / (float) KSizeMb);
			infoSize.Format(KSizeDownloadingOf(), &dlSize, &totSize);
			infoSize.Append(KShowsSizeUnit());
		}
		else if (aShowInfo->ShowSize() > 0)
		{
			infoSize.Format(KShowsSizeFormat(), (float)aShowInfo->ShowSize() / (float)KSizeMb);
			infoSize.Append(KShowsSizeUnit());
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
			aShowInfo->PubDate().FormatL(showDate, KDateFormat());
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
	TListItemProperties itemProps;	
	itemProps.SetUnderlined(aShowInfo->PlayState() == ENeverPlayed);
	iListContainer->Listbox()->ItemDrawer()->SetPropertiesL(aIndex, itemProps);						
}

void CPodcastShowsView::UpdateShowItemL(TUint aUid, TInt aSizeDownloaded)
{
	RShowInfoArray& array = iPodcastModel.ActiveShowList();
	
	for (int i=0;i<array.Count();i++) {
		if (array[i]->Uid() == aUid) {
			UpdateShowItemDataL(array[i], i, aSizeDownloaded);		
			iListContainer->Listbox()->DrawItem(i);			
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
								showSize.Format(KShowsSizeFormat(),
										(float)si->ShowSize()/ (float)KSizeMb);
								showSize.Append(KShowsSizeUnit());
								}

							if (si->PubDate().Int64() == 0)
								{
								showDate = KNullDesC();
								}
							else
								{
								si->PubDate().FormatL(showDate, KDateFormat());
								}
							}
						TInt iconIndex = 0;						
						GetShowIcons(si, iconIndex);
						iListboxFormatbuffer.Format(KShowFormat(), iconIndex, &si->Title(), &showDate, &showSize);
						iItemArray->AppendL(iListboxFormatbuffer);

						itemProps.SetUnderlined(si->PlayState() == ENeverPlayed);
						iListContainer->Listbox()->ItemDrawer()->SetPropertiesL(i, itemProps);																	
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
	UpdateNaviPaneL();
	iListContainer->ScrollToVisible();
	}

	void CPodcastShowsView::UpdateNaviPaneL()
		{
		if (iCurrentCategory == EShowPendingShows)
			{
			HBufC* titleBuffer= NULL;

			RShowInfoArray &fItems = iPodcastModel.ActiveShowList();
			TUint len = fItems.Count();
			if (iPodcastModel.ShowEngine().DownloadsStopped())
				{
				HBufC* titleFormat=  iEikonEnv->AllocReadResourceLC(R_PODCAST_SHOWS_DOWNLOADS_SUSPENDED);
				titleBuffer = HBufC::NewL(titleFormat->Length()+8);
				titleBuffer->Des().Format(*titleFormat, len);
				CleanupStack::PopAndDestroy(titleFormat);
				CleanupStack::PushL(titleBuffer);
				}
			else
				{
				HBufC* titleFormat=  iEikonEnv->AllocReadResourceLC(R_PODCAST_SHOWS_TITLE_DOWNLOAD);
				titleBuffer = HBufC::NewL(titleFormat->Length()+8);
				titleBuffer->Des().Format(*titleFormat, len);
				CleanupStack::PopAndDestroy(titleFormat);
				CleanupStack::PushL(titleBuffer);
				}

			if (iNaviPane != NULL)
				{
				iNaviPane->Pop(iNaviDecorator);
				delete iNaviDecorator;
				iNaviDecorator = NULL;
				}

			iNaviDecorator	= iNaviPane->CreateNavigationLabelL(*titleBuffer);
			iNaviPane->PushL(*iNaviDecorator);
			CleanupStack::PopAndDestroy(titleBuffer);
			}
		else
			{
			TUint numUnplayed = 0;
			TUint numShows = 0;
	
			HBufC* titleFormat=  iEikonEnv->AllocReadResourceLC(R_PODCAST_SHOWS_TITLE_FORMAT);
			HBufC* titleBuffer = HBufC::NewL(titleFormat->Length()+8);
			if (iCurrentCategory == EShowDownloadedShows) {
				iPodcastModel.FeedEngine().GetDownloadedStats(numShows, numUnplayed);
			} else if (iPodcastModel.ActiveFeedInfo()){
				iPodcastModel.FeedEngine().GetStatsByFeed(iPodcastModel.ActiveFeedInfo()->Uid(), numShows, numUnplayed, EFalse);
			} 
			
			titleBuffer->Des().Format(*titleFormat, numUnplayed, numShows);
			CleanupStack::PopAndDestroy(titleFormat);
			CleanupStack::PushL(titleBuffer);

			if (iNaviPane != NULL)
				{
				iNaviPane->Pop(iNaviDecorator);
				delete iNaviDecorator;
				iNaviDecorator = NULL;
				}
			
			iNaviDecorator = iNaviPane->CreateNavigationLabelL(*titleBuffer);
			iNaviPane->PushL(*iNaviDecorator);
			CleanupStack::PopAndDestroy(titleBuffer);
		}
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
					iListContainer->Listbox()->HandleItemRemovalL();
					iListContainer->Listbox()->SetCurrentItemIndex(index - 1 > 0 ? index - 1 : 0);
					iListContainer->Listbox()->DrawNow();
					UpdateNaviPaneL();
					}
				else {
					UpdateShowItemDataL(iPodcastModel.ActiveShowList()[index], index, 0);
					iListContainer->Listbox()->DrawItem(index);					
				}
				UpdateNaviPaneL();
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
				UpdateNaviPaneL();
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
				if (iPodcastModel.PlayingPodcast()
						== iPodcastModel.ActiveShowList()[index] && iPodcastModel.SoundEngine().State() != ESoundEngineNotInitialized)
					{
					iPodcastModel.SoundEngine().Stop();
					}

				TBool isBook = (iPodcastModel.ActiveShowList()[index]->ShowType() == EAudioBook);
				if (iEikonEnv->QueryWinL(isBook?R_PODCAST_REMOVE_CHAPTER_TITLE:R_PODCAST_DELETE_SHOW_TITLE,
						isBook?R_PODCAST_REMOVE_CHAPTER_PROMPT:R_PODCAST_DELETE_SHOW_PROMPT))
					{
					if (isBook)
						{
						iPodcastModel.ShowEngine().DeleteShow(iPodcastModel.ActiveShowList()[index]->Uid(), EFalse);
						}
					else
						{
						iPodcastModel.ShowEngine().DeleteShow(iPodcastModel.ActiveShowList()[index]->Uid());
						}
					
					UpdateListboxItemsL();
					}
				}
			}
			break;
		case EPodcastUpdateLibrary:
			HBufC* str = CEikonEnv::Static()->AllocReadResourceLC(R_PODCAST_FEEDS_UPDATE_MESSAGE);
			User::InfoPrint(*str);
			CleanupStack::PopAndDestroy(str);
			iPodcastModel.ShowEngine().CheckFilesL();
			break;
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
				iPodcastModel.ShowEngine().AddDownload(iPodcastModel.ActiveShowList()[index]);
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
				HBufC* str = CEikonEnv::Static()->AllocReadResourceLC(R_PODCAST_FEEDS_UPDATE_MESSAGE);
				User::InfoPrint(*str);
				CleanupStack::PopAndDestroy(str);
				TRAPD(error, iPodcastModel.FeedEngine().UpdateFeedL(iPodcastModel.ActiveFeedInfo()->Uid()))
				;

				if (error != KErrNone)
					{
					HBufC
							* str =
									CEikonEnv::Static()->AllocReadResourceLC(R_PODCAST_FEEDS_UPDATE_ERROR);
					User::InfoPrint(*str);
					CleanupStack::PopAndDestroy(str);
					}
				}
			}
			break;
		case EPodcastCancelUpdateAllFeeds:
			iPodcastModel.FeedEngine().CancelUpdateAllFeedsL();
			break;
		case EPodcastRemoveAllDownloads:
			{
			iPodcastModel.ShowEngine().RemoveAllDownloads();
			UpdateListboxItemsL();
			if (iProgressAdded)
				{
				//ViewContext()->RemoveAndDestroyProgressInfo();
				}
			}
			break;
		case EPodcastRemoveDownloadHardware:
		case EPodcastRemoveDownload:
			{
			TInt index = iListContainer->Listbox()->CurrentItemIndex();
			if (index >= 0 && index < iPodcastModel.ActiveShowList().Count())
				{
				if (iPodcastModel.ShowEngine().RemoveDownload(iPodcastModel.ActiveShowList()[index]->Uid()))
					{
						iItemArray->Delete(index);
						iItemIdArray.Remove(index);						
						iListContainer->Listbox()->HandleItemRemovalL();
						iListContainer->Listbox()->SetCurrentItemIndex(index - 1 > 0 ? index - 1 : 0);
						iListContainer->Listbox()->DrawNow();
						UpdateNaviPaneL();
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
			iPodcastModel.ShowEngine().ResumeDownloads();
			UpdateListboxItemsL();
			}
			break;
		default:
			CPodcastListView::HandleCommandL(aCommand);
			break;
		}
	}
	
	void CPodcastShowsView::DynInitMenuPaneL(TInt aResourceId,CEikMenuPane* aMenuPane)
	{
		if(aResourceId == R_PODCAST_SHOWSVIEW_MENU)
		{
			
			RShowInfoArray &fItems = iPodcastModel.ActiveShowList();
			TInt itemCnt = fItems.Count();
			TBool removeDeleteShowCmd = ETrue;
			TBool removeDownloadShowCmd = ETrue;
			TBool removeRemoveSuspendCmd = EFalse;
			TBool updatingState = (iCurrentCategory != EShowDownloadedShows && iPodcastModel.FeedEngine().ClientState() != ENotUpdating && iPodcastModel.FeedEngine().ActiveClientUid() == iPodcastModel.ActiveFeedInfo()->Uid());
			//TBool playingPodcast = (iPodcastModel.PlayingPodcast() != NULL && (iPodcastModel.SoundEngine().State() == ESoundEnginePlaying || iPodcastModel.SoundEngine().State() == ESoundEnginePaused));
			TBool removeSetPlayed = EFalse;
			//aMenuPane->SetItemDimmed(EPodcastViewPlayer, !playingPodcast);
			
			if(iListContainer->Listbox() != NULL)
			{
				TInt index = iListContainer->Listbox()->CurrentItemIndex();
				
				if(index>= 0 && index < itemCnt)
				{							
					if(fItems[index]->DownloadState() != EQueued &&  fItems[index]->DownloadState() != EDownloading)
					{			
					}
					else
					{
						if(fItems[index]->DownloadState() == EQueued)
						{
							aMenuPane->SetItemTextL(EPodcastRemoveDownload, R_PODCAST_PLAYER_REMOVE_DL_CMD);							
						}
						else
						{					
							aMenuPane->SetItemTextL(EPodcastRemoveDownload, R_PODCAST_PLAYER_SUSPEND_DL_CMD);
							removeRemoveSuspendCmd = ETrue;
						}
					}
					
					removeDeleteShowCmd = fItems[index]->DownloadState() != EDownloaded || updatingState || 
						(iPodcastModel.PlayingPodcast() != NULL && fItems[index] == iPodcastModel.PlayingPodcast() && iPodcastModel.SoundEngine().State() == ESoundEnginePlaying);
					
					if(fItems[index]->DownloadState() == ENotDownloaded)
					{
						removeDownloadShowCmd = EFalse;
					}
					
					if(fItems[index]->PlayState() == EPlayed) {
						removeSetPlayed = ETrue;
					}
					
				}
				
			}		
			
			TBool iUpdateRunning = iPodcastModel.FeedEngine().ClientState() != ENotUpdating && iPodcastModel.ActiveFeedInfo() != NULL && iPodcastModel.FeedEngine().ActiveClientUid() == iPodcastModel.ActiveFeedInfo()->Uid();
			
			aMenuPane->SetItemDimmed(EPodcastUpdateFeed, iUpdateRunning || (iCurrentCategory != EShowFeedShows || iPodcastModel.ActiveFeedInfo()->IsBookFeed()));
			aMenuPane->SetItemDimmed(EPodcastCancelUpdateAllFeeds, !iUpdateRunning);
			
			aMenuPane->SetItemDimmed(EPodcastDownloadShow, removeDownloadShowCmd || updatingState);
			
			aMenuPane->SetItemDimmed(EPodcastMarkAllPlayed, updatingState);
			
			aMenuPane->SetItemDimmed(EPodcastDeleteShow, removeDeleteShowCmd);					
			TBool isOrdinaryList = iCurrentCategory == EShowDownloadedShows || (iCurrentCategory == EShowFeedShows && !(iPodcastModel.ActiveFeedInfo()?iPodcastModel.ActiveFeedInfo()->IsBookFeed():ETrue));
			aMenuPane->SetItemDimmed(EPodcastDeleteAllPlayed, (!(!updatingState && !iPodcastModel.SettingsEngine().SelectUnplayedOnly()) || !isOrdinaryList));						
			
			aMenuPane->SetItemDimmed(EPodcastSetOrderAudioBook, !(EShowFeedShows == iCurrentCategory && (iPodcastModel.ActiveFeedInfo()?iPodcastModel.ActiveFeedInfo()->IsBookFeed(): ETrue)));
			aMenuPane->SetItemDimmed(EPodcastAddAudioBookChapter, !(EShowFeedShows == iCurrentCategory && (iPodcastModel.ActiveFeedInfo()?iPodcastModel.ActiveFeedInfo()->IsBookFeed(): ETrue)));
			
			aMenuPane->SetItemDimmed(EPodcastRemoveAllDownloads, ETrue);
			aMenuPane->SetItemDimmed(EPodcastMarkAsPlayed, removeSetPlayed || !itemCnt);
			aMenuPane->SetItemDimmed(EPodcastMarkAsUnplayed, !removeSetPlayed || !itemCnt);
			
			
			switch(iCurrentCategory)
			{	
			case EShowPendingShows:
				{
					aMenuPane->SetItemDimmed(EPodcastMarkAllPlayed, ETrue);
					aMenuPane->SetItemDimmed(EPodcastUpdateLibrary, ETrue);
					aMenuPane->SetItemDimmed(EPodcastRemoveDownload, (!itemCnt|| removeRemoveSuspendCmd));				
					aMenuPane->SetItemDimmed(EPodcastRemoveAllDownloads, (!iPodcastModel.ShowEngine().DownloadsStopped() || !itemCnt));				
					aMenuPane->SetItemDimmed(EPodcastStopDownloads, (iPodcastModel.ShowEngine().DownloadsStopped()));
					aMenuPane->SetItemDimmed(EPodcastResumeDownloads, (!iPodcastModel.ShowEngine().DownloadsStopped()));			
					aMenuPane->SetItemDimmed(EPodcastShowUnplayedOnly, ETrue);				
					aMenuPane->SetItemDimmed(EPodcastDownloadShow, ETrue);
					aMenuPane->SetItemDimmed(EPodcastMarkAsPlayed, ETrue);
					aMenuPane->SetItemDimmed(EPodcastMarkAsUnplayed, ETrue);
					aMenuPane->SetItemDimmed(EPodcastDeleteShow, ETrue);
				}break;
			case EShowDownloadedShows:
				{
					aMenuPane->SetItemDimmed(EPodcastMarkAllPlayed, updatingState);
					aMenuPane->SetItemDimmed(EPodcastUpdateLibrary, EFalse);
					aMenuPane->SetItemDimmed(EPodcastRemoveDownload, ETrue);
					aMenuPane->SetItemDimmed(EPodcastStopDownloads, ETrue);
					aMenuPane->SetItemDimmed(EPodcastResumeDownloads, ETrue);
					aMenuPane->SetItemDimmed(EPodcastShowUnplayedOnly, EFalse);
				}break;
			default:
				{
					aMenuPane->SetItemDimmed(EPodcastRemoveDownload, ETrue);					
					aMenuPane->SetItemDimmed(EPodcastStopDownloads, ETrue);
					aMenuPane->SetItemDimmed(EPodcastResumeDownloads, ETrue);
					aMenuPane->SetItemDimmed(EPodcastMarkAllPlayed, updatingState);
					aMenuPane->SetItemDimmed(EPodcastUpdateLibrary, ETrue);
					aMenuPane->SetItemDimmed(EPodcastShowUnplayedOnly, EFalse);
				}break;
			}	
			
			
	} else if (aResourceId == R_ONOFF_MENU) {
		if (iPodcastModel.SettingsEngine().SelectUnplayedOnly())
		{
		aMenuPane->SetItemButtonState(EPodcastShowUnplayedOnlyOff, EEikMenuItemSymbolIndeterminate);
		aMenuPane->SetItemButtonState(EPodcastShowUnplayedOnlyOn, EEikMenuItemSymbolOn);
		} else {
		aMenuPane->SetItemButtonState(EPodcastShowUnplayedOnlyOn, EEikMenuItemSymbolIndeterminate);
		aMenuPane->SetItemButtonState(EPodcastShowUnplayedOnlyOff, EEikMenuItemSymbolOn);
		}

	}
}
