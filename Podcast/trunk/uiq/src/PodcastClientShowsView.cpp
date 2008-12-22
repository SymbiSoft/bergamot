#include <PodcastClient.rsg>
#include <QikCategoryModel.h>
#include <qikcommand.h>
#include <qikcontent.h>
#include <QikListBoxLayoutPair.h>
#include <QikListBoxLayoutElement.h>
#include <QikListBoxLayout.h>
#include <QikListBoxLayoutProperties.h>
#include <eikdialg.h>
#include <QikMediaFileFolderUtils.h>
#include <QikSelectFileDlg.h>
#include <QikSaveFileDlg.h>

#include <podcastclient.mbg>
#include "PodcastClientShowsView.h"
#include "PodcastClientPlayView.h"
#include "PodcastClientSettingsDlg.h"
#include "PodcastModel.h"
#include "ShowEngine.h"
#include "SettingsEngine.h"
_LIT(KSizeDownloadingOf, "%S/%S");
_LIT(KChapterFormatting, "%03d");

const TInt KSizeBufLen = 64;
const TInt KDefaultGran = 5;

/**
Creates and constructs the view.

@param aAppUi Reference to the AppUi
@return Pointer to a CPodcastClientView object
*/
CPodcastClientShowsView* CPodcastClientShowsView::NewLC(CQikAppUi& aAppUi, CPodcastModel& aPodcastModel)
	{
	//RDebug::Print(_L("NewLC"));
	CPodcastClientShowsView* self = new (ELeave) CPodcastClientShowsView(aAppUi, aPodcastModel);
	CleanupStack::PushL(self);
	self->ConstructL();
	return self;
	}

CPodcastClientShowsView::CPodcastClientShowsView(CQikAppUi& aAppUi, CPodcastModel& aPodcastModel):CPodcastClientView(aAppUi, aPodcastModel)
{
	iCurrentCategory = EShowAllShows;
}

void CPodcastClientShowsView::ConstructL()
{
	CPodcastClientView::ConstructL();
	iPodcastModel.FeedEngine().AddObserver(this);
	iPodcastModel.ShowEngine().AddObserver(this);
}

CPodcastClientShowsView::~CPodcastClientShowsView()
{
	iPodcastModel.ShowEngine().RemoveObserver(this);
	iPodcastModel.FeedEngine().RemoveObserver(this);
}

/**
Returns the view Id
@return Returns the Uid of the view
*/
TVwsViewId CPodcastClientShowsView::ViewId()const
	{
	return TVwsViewId(KUidPodcastClientID, KUidPodcastShowsViewID);
	}

void CPodcastClientShowsView::ViewConstructL()
{
	iViewLabel = HBufC::NewL(1);

    ViewConstructFromResourceL(R_PODCAST_SHOWSVIEW_UI_CONFIGURATIONS);

	CPodcastClientView::ViewConstructL();

	SetCategoryModel(NULL);
	delete iCategories; 
	iCategories = NULL;
	iCategories = QikCategoryUtils::ConstructCategoriesLC(R_PODCAST_SHOWS_CATEGORIES);
	SetCategoryModel(iCategories);
	CleanupStack::Pop(iCategories);
}

void CPodcastClientShowsView::ViewActivatedL(const TVwsViewId &aPrevViewId, TUid aCustomMessageId, const TDesC8 &aCustomMessage)
{
	//TPodcastClientShowCategory previousCategory = iCurrentCategory;

	switch(aCustomMessageId.iUid)
	{
	case EShowNewShows:
	case EShowAllShows:
	case EShowDownloadedShows:
	case EShowPendingShows:
		iCurrentCategory = (TPodcastClientShowCategory) aCustomMessageId.iUid;
		break;
	case EShowFeedShows:
		iCurrentCategory = EShowFeedShows;
		break;
	}	

	CPodcastClientView::ViewActivatedL(aPrevViewId, aCustomMessageId, aCustomMessage);

	if(aPrevViewId == TVwsViewId(KUidPodcastClientID, KUidPodcastFeedViewID) && iCurrentCategory == EShowFeedShows)
	{
		SetParentView( TVwsViewId(KUidPodcastClientID, KUidPodcastFeedViewID));
	}
	else if(aPrevViewId.iAppUid == KUidPodcastClientID && iCurrentCategory != EShowFeedShows )
	{
		SetParentView( TVwsViewId(KUidPodcastClientID, KUidPodcastBaseViewID));
	}
	
	iProgressAdded = EFalse;
	ViewContext()->RemoveAndDestroyProgressInfo();
	
	UpdateFeedUpdateStateL();
}


void CPodcastClientShowsView::HandleCommandL(CQikCommand& aCommand)
{
	if(aCommand.Type() == EQikCommandTypeCategory)
	{
		iCurrentCategory = (TPodcastClientShowCategory) CategoryHandleForCommandId(aCommand.Id());
		UpdateListboxItemsL();
	}
	else
	{	
		switch(aCommand.Id())
		{
		case EPodcastMarkAsPlayed:
			{
			TInt index = iListbox->CurrentItemIndex();
			if(index >= 0 && index < iPodcastModel.ActiveShowList().Count())
			{
				//iPodcastModel.ShowEngine().SetShowPlayState(iPodcastModel.ActiveShowList()[index], EPlayed);
				if (iPodcastModel.SettingsEngine().SelectUnplayedOnly()) {
					MQikListBoxModel& model(iListbox->Model());
					model.ModelBeginUpdateLC();
					model.RemoveDataL(index);
					model.ModelEndUpdateL();
				}
				
				UpdateListboxItemsL();
			}
			}break;
		case EPodcastMarkAsUnplayed:
			{
			TInt index = iListbox->CurrentItemIndex();
			if(index >= 0 && index < iPodcastModel.ActiveShowList().Count())
			{
				//iPodcastModel.ShowEngine().SetShowPlayState(iPodcastModel.ActiveShowList()[index], ENeverPlayed);
				UpdateListboxItemsL();
			}
			}break;
		case EPodcastSetOrderAudioBook:
			{
				if(iPodcastModel.ActiveFeedInfo() != NULL)
					{
					CPodcastClientSetAudioBookOrderDlg* dlg = new (ELeave) CPodcastClientSetAudioBookOrderDlg(iPodcastModel, iPodcastModel.ActiveFeedInfo()->Uid());
					dlg->ExecuteLD(R_PODCAST_AUDIOBOOK_PLAYORDERDLG);
					UpdateListboxItemsL();
					}
			}break;
		case EPodcastAddAudioBookChapter:
			{
				if(iPodcastModel.ActiveFeedInfo() != NULL)
				{
					
					CDesCArrayFlat* mimeArray = iEikonEnv->ReadDesCArrayResourceL(R_PODCAST_NEW_AUDIOBOOK_MIMEARRAY);
					CleanupStack::PushL(mimeArray);
					CDesCArrayFlat* fileNameArray = new (ELeave) CDesCArrayFlat(KDefaultGran);
					CleanupStack::PushL(fileNameArray);
					HBufC* dialogTitle = iEikonEnv->AllocReadResourceLC(R_PODCAST_NEW_AUDIOBOOK_SELECT_FILES);
					TQikDefaultFolderDescription defaultFolder;
					defaultFolder.SetDefaultFolder(EQikFileHandlingDefaultFolderAudio);
					
					if(CQikSelectFileDlg::RunDlgLD(*mimeArray, *fileNameArray, defaultFolder, dialogTitle, EQikSelectFileDialogEnableMultipleSelect|EQikSelectFileDialogSortByName))
					{
						if(fileNameArray->Count() > 0)
						{
							iPodcastModel.FeedEngine().AddBookChaptersL(*iPodcastModel.ActiveFeedInfo(), fileNameArray);			
						}
					}
					
					CleanupStack::PopAndDestroy(dialogTitle);
					CleanupStack::PopAndDestroy(fileNameArray);
					CleanupStack::PopAndDestroy(mimeArray);
				}
			}break;
		case EPodcastDeleteShowHardware:
		case EPodcastDeleteShow:
			{				
				TInt index = iListbox->CurrentItemIndex();

				if(index >= 0 && index < iPodcastModel.ActiveShowList().Count())
					{
					if (iPodcastModel.PlayingPodcast() == iPodcastModel.ActiveShowList()[index] && iPodcastModel.SoundEngine().State() != ESoundEngineNotInitialized) {
						iPodcastModel.SoundEngine().Stop();
					}
					                               
					TBool isBook = (iPodcastModel.ActiveShowList()[index]->ShowType() == EAudioBook);
					if(iEikonEnv->QueryWinL(isBook?R_PODCAST_REMOVE_CHAPTER_TITLE:R_PODCAST_DELETE_SHOW_TITLE, 
											isBook?R_PODCAST_REMOVE_CHAPTER_PROMPT:R_PODCAST_DELETE_SHOW_PROMPT))				
						{											
							if(isBook)
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
			}break;
		case EPodcastUpdateLibrary:
			HBufC* str = CEikonEnv::Static()->AllocReadResourceLC(R_PODCAST_FEEDS_UPDATE_MESSAGE);
			User::InfoPrint(*str);
			CleanupStack::PopAndDestroy(str);
			iPodcastModel.ShowEngine().CheckFilesL();
			break;
		case EPodcastShowUnplayedOnly:
			{
			iPodcastModel.SettingsEngine().SetSelectUnplayedOnly(!iPodcastModel.SettingsEngine().SelectUnplayedOnly());
			UpdateListboxItemsL();
			}break;
		case EPodcastMarkAllPlayed:
			iPodcastModel.MarkSelectionPlayed();
			UpdateListboxItemsL();
			break;
		case EPodcastDownloadShow:
			{
				TInt index = iListbox->CurrentItemIndex();
				if(index >= 0 && index < iPodcastModel.ActiveShowList().Count())
				{
					//iPodcastModel.ShowEngine().AddDownload(iPodcastModel.ActiveShowList()[index]);
					UpdateShowItemL(iPodcastModel.ActiveShowList()[index]);
				
				}
			}break;
		case EPodcastDeleteAllPlayed:
			{
				if(iEikonEnv->QueryWinL(R_PODCAST_DELETE_PLAYED_TITLE, R_PODCAST_DELETE_PLAYED_PROMPT))				
				{
				iPodcastModel.ShowEngine().DeletePlayedShows();
				UpdateListboxItemsL();
				}
			}break;
		case EPodcastUpdateFeed:
			{
			// flag to know whether to disable catchup mode here or not,
			// otherwise we might disable an Update All initiated from
			// the feed view
			iDisableCatchupMode = EFalse;
				
			if (iPodcastModel.ActiveFeedInfo()->Url().Length()>0) {
					HBufC* str = CEikonEnv::Static()->AllocReadResourceLC(R_PODCAST_FEEDS_UPDATE_MESSAGE);
					User::InfoPrint(*str);
					CleanupStack::PopAndDestroy(str);
					TRAPD(error, iPodcastModel.FeedEngine().UpdateFeedL(iPodcastModel.ActiveFeedInfo()->Uid()));
					
					if (error != KErrNone) {
						HBufC* str = CEikonEnv::Static()->AllocReadResourceLC(R_PODCAST_FEEDS_UPDATE_ERROR);
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
			if (iProgressAdded) {
				ViewContext()->RemoveAndDestroyProgressInfo();
			}
			}
			break;
		case EPodcastRemoveDownloadHardware:
		case EPodcastRemoveDownload:
			{
				TInt index = iListbox->CurrentItemIndex();
				if(index >= 0 && index < iPodcastModel.ActiveShowList().Count())
				{
					if (iPodcastModel.ShowEngine().RemoveDownload(iPodcastModel.ActiveShowList()[index]->Uid())) {
						if (index > 0) {
							MQikListBoxModel& model(iListbox->Model());
							model.ModelBeginUpdateLC();
							model.RemoveDataL(index);
							model.ModelEndUpdateL();
						}

						UpdateListboxItemsL();												
					}
				}
			}break;
		case EPodcastStopDownloads:
			{
				iPodcastModel.ShowEngine().StopDownloads();
				UpdateListboxItemsL();	
			}break;
		case EPodcastResumeDownloads:
			{
				iPodcastModel.ShowEngine().ResumeDownloads();
				UpdateListboxItemsL();	
			}break;
		default:
			CPodcastClientView::HandleCommandL(aCommand);
			break;
		}
	}

	UpdateCommandsL();
	
}

// Engine callback when new shows are available
void CPodcastClientShowsView::ShowListUpdated()
{
	TRAP_IGNORE(UpdateListboxItemsL());
}

void CPodcastClientShowsView::FeedInfoUpdated(CFeedInfo* aFeedInfo)
{
	if(iPodcastModel.ActiveFeedInfo() != NULL && aFeedInfo->Uid() == iPodcastModel.ActiveFeedInfo()->Uid())
	{
		iPodcastModel.SetActiveFeedUid(aFeedInfo->Uid());
		TRAP_IGNORE(UpdateFeedUpdateStateL());
		// Title might have changed
		TRAP_IGNORE(UpdateListboxItemsL());
	}
}

void CPodcastClientShowsView::FeedUpdateCompleteL(TUint aFeedUid)
{
	if(iPodcastModel.ActiveFeedInfo() != NULL && iPodcastModel.ActiveFeedInfo()->Uid() == aFeedUid)
	{
		UpdateFeedUpdateStateL();		
	}
}

void CPodcastClientShowsView::FeedUpdateAllCompleteL()
{
}

void CPodcastClientShowsView::FeedDownloadUpdatedL(TUint aFeedUid, TInt /*aPercentOfCurrentDownload*/)
	{
	if(iPodcastModel.ActiveFeedInfo() != NULL && iPodcastModel.ActiveFeedInfo()->Uid() == aFeedUid)
		{
			UpdateFeedUpdateStateL();
		}
	}

void CPodcastClientShowsView::UpdateFeedUpdateStateL()
	{
	TBool listboxDimmed = EFalse;

	if(iPodcastModel.FeedEngine().ClientState() != ENotUpdating && iPodcastModel.ActiveFeedInfo() != NULL && iPodcastModel.FeedEngine().ActiveClientUid() == iPodcastModel.ActiveFeedInfo()->Uid())
		{
		listboxDimmed = ETrue;
		}
	
	if((iListbox->IsDimmed() && !listboxDimmed) || (!iListbox->IsDimmed() && listboxDimmed))
		{
		iListbox->SetDimmed(listboxDimmed);	
		UpdateCommandsL();
		}
	}

void CPodcastClientShowsView::ShowDownloadUpdatedL(TInt aPercentOfCurrentDownload, TInt aBytesOfCurrentDownload, TInt /*aBytesTotal*/)
{
	//DP("CPodcastClientShowsView::ShowDownloadUpdatedL");
	if(ViewContext() != NULL)
	{
		CShowInfo *showInfo = NULL;
		TInt index = iListbox->CurrentItemIndex();
		if(index >= 0 && index < iPodcastModel.ActiveShowList().Count())
		{
					showInfo = iPodcastModel.ActiveShowList()[index];
		}

		// now we only show when the active downloa has focus in a list
		//if(!iPodcastModel.ShowEngine().DownloadsStopped() && showInfo != NULL && showInfo->Uid() == iPodcastModel.ShowEngine().ShowDownloading()->Uid() &&				

		// we show progress bar only for pending shows and inside the feed to which
	    // the active download belongs
		if ((iCurrentCategory == EShowPendingShows && aBytesOfCurrentDownload != -1) ||
				(iPodcastModel.ShowEngine().ShowDownloading()!= NULL && iPodcastModel.ActiveFeedInfo() != NULL && 
				iPodcastModel.ShowEngine().ShowDownloading()->FeedUid() == iPodcastModel.ActiveFeedInfo()->Uid() &&
				aPercentOfCurrentDownload>=0 && aPercentOfCurrentDownload < KOneHundredPercent))
		{
			if(!iProgressAdded)
			{
				ViewContext()->AddProgressInfoL(EEikProgressTextPercentage, KOneHundredPercent);

				iProgressAdded = ETrue;
			}
			
			ViewContext()->SetAndDrawProgressInfo(aPercentOfCurrentDownload);
		}
		else
		{
			ViewContext()->RemoveAndDestroyProgressInfo();
			ViewContext()->DrawNow();
			iProgressAdded = EFalse;
		}

		if(aPercentOfCurrentDownload == KOneHundredPercent)
		{
			ViewContext()->RemoveAndDestroyProgressInfo();
			ViewContext()->DrawNow();
			iProgressAdded = EFalse;

			// To update icon list status and commands
			UpdateCommandsL();
			if(iCurrentCategory == EShowPendingShows && iPodcastModel.ShowEngine().ShowDownloading() != NULL)
			{
				iListbox->RemoveItemL(0);
				iPodcastModel.GetShowsDownloading();
				UpdateListboxItemsL();
			}
		}

		if(iPodcastModel.ShowEngine().ShowDownloading() != NULL)
		{
			UpdateShowItemL(iPodcastModel.ShowEngine().ShowDownloading(), aBytesOfCurrentDownload);
		}
		
		if (aPercentOfCurrentDownload == -1) {
			UpdateCommandsL();
		}
	}
}

CQikCommand* CPodcastClientShowsView::DynInitOrDeleteCommandL(CQikCommand* aCommand, const CCoeControl& /*aControlAddingCommands*/)
{
	if(aCommand->Type() == EQikCommandTypeCategory && CategoryHandleForCommandId(aCommand->Id()) == EShowFeedShows)
		{
		delete aCommand;
		 return NULL;
		}
		

	switch(aCommand->Id())
	{		
	case EQikListBoxCmdSelect:
		{
			if(iPodcastModel.ActiveShowList().Count() == 0)
			{			
				aCommand->SetInvisible(ETrue);
			}
			else
			{
				TInt index = iListbox->CurrentItemIndex();
				
				if(index>= 0 && index < iPodcastModel.ActiveShowList().Count())
				{					
					aCommand->SetInvisible(EFalse);	
					aCommand->SetTextL(R_PODCAST_VIEW_CMD);
					aCommand->SetShortTextL(R_PODCAST_VIEW_CMD);
				}
				else
				{
					aCommand->SetInvisible(ETrue);
				}
			}						
		}
		break;
	default:
		break;
	}
	return aCommand;
}

void CPodcastClientShowsView::GetShowIcons(CShowInfo* aShowInfo, TInt& aImageId, TInt& aMaskId)
{	
	TBool dlStop = iPodcastModel.ShowEngine().DownloadsStopped();
	if (aShowInfo->ShowType() == EAudioBook) {
		aImageId = EMbmPodcastclientAudiobookchapter_40x40;
		aMaskId = EMbmPodcastclientAudiobookchapter_40x40m;
	} else {
		if (iPodcastModel.PlayingShowUid() == aShowInfo->Uid()) { // && iPodcastModel.SoundEngine().State() == ESoundEnginePlaying) {
					aImageId = EMbmPodcastclientShow_playing_40x40;
					aMaskId = EMbmPodcastclientShow_playing_40x40m;
		} else {
			switch(aShowInfo->DownloadState())
			{
			case EDownloaded:
				aImageId = EMbmPodcastclientShow_40x40;
				aMaskId = EMbmPodcastclientShow_40x40m;
				break;	
			case ENotDownloaded:
				aImageId = EMbmPodcastclientNew_40x40;
				aMaskId = EMbmPodcastclientNew_40x40m;
				break;
			case EQueued:
				aImageId = dlStop ? EMbmPodcastclientSuspended_40x40 : EMbmPodcastclientQueued_40x40;
				aMaskId = EMbmPodcastclientQueued_40x40m;
				break;
			case EDownloading:
				aImageId = dlStop ? EMbmPodcastclientSuspended_40x40 : EMbmPodcastclientDownloading_40x40;
				aMaskId = EMbmPodcastclientDownloading_40x40m;
				break;
			}
		}
	}
}

void CPodcastClientShowsView::UpdateShowItemDataL(CShowInfo* aShowInfo, MQikListBoxData* aListboxData, TInt aSizeDownloaded)
{
	TBuf<32> infoSize;
	TInt bitmap = EMbmPodcastclientShow_40x40;
	TInt mask = EMbmPodcastclientShow_40x40m;
	
	GetShowIcons(aShowInfo, bitmap, mask);
	CQikContent* content = CQikContent::NewL(this, _L("*"), bitmap, mask);
	CleanupStack::PushL(content);
	aListboxData->SetIconL(content, EQikListBoxSlotLeftMediumIcon1);
	CleanupStack::Pop(content);
	
	if(aShowInfo->ShowType() == EAudioBook)
	{
		TBuf<KMaxShortDateFormatSpec*2> showDate;

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

		aListboxData->SetTextL(showDate, EQikListBoxSlotText3);
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
		else
		{
			infoSize.Format(KShowsSizeFormat(), (float)aShowInfo->ShowSize() / (float)KSizeMb);
			infoSize.Append(KShowsSizeUnit());

		}
	}
	aListboxData->SetTextL(aShowInfo->Title(), EQikListBoxSlotText1);
	aListboxData->SetTextL(infoSize, EQikListBoxSlotText4);
	aListboxData->SetEmphasis(aShowInfo->PlayState() == ENeverPlayed);					
}

void CPodcastClientShowsView::UpdateShowItemL(CShowInfo* aShowInfo, TInt aSizeDownloaded)
{
	// First find the item

	RShowInfoArray &array = iPodcastModel.ActiveShowList();
	TInt index = KErrNotFound;

	for (int i=0;i<array.Count();i++) {
		if (array[i]->Uid() == aShowInfo->Uid()) {
			index = i;
		}
	}
	MQikListBoxModel& model(iListbox->Model());

	if(index != KErrNotFound && index < model.Count())
	{
		model.ModelBeginUpdateLC();
		MQikListBoxData* data = model.RetrieveDataL(index);	
		CleanupClosePushL(*data);
		UpdateShowItemDataL(aShowInfo, data, aSizeDownloaded);
		CleanupStack::PopAndDestroy();// close data
		model.DataUpdatedL(index);
		model.ModelEndUpdateL();
		
	}
}

void CPodcastClientShowsView::UpdateListboxItemsL()
{
	if(IsVisible())
	{
		TBuf<KSizeBufLen> showSize;
		TBuf<KMaxShortDateFormatSpec*2> showDate;

		TInt len = 0;
	
		SetAppTitleNameL(KNullDesC());

		switch(iCurrentCategory)
		{
		case EShowAllShows:
			SelectCategoryL(EShowAllShows);
			iPodcastModel.GetAllShows();
			break;
		case EShowNewShows:
			SelectCategoryL(EShowNewShows);
			iPodcastModel.GetNewShows();
			break;		
		case EShowDownloadedShows:
			SelectCategoryL(EShowDownloadedShows);
			iPodcastModel.GetShowsDownloaded();
			break;
		case EShowPendingShows:
			SelectCategoryL(EShowPendingShows);
			iPodcastModel.GetShowsDownloading();
			break;
		default:
			
			iCategories->RenameCategoryL(EShowFeedShows, iPodcastModel.ActiveFeedInfo()->Title());
			
			iPodcastModel.GetShowsByFeed(iPodcastModel.ActiveFeedInfo()->Uid());
			SelectCategoryL(EShowFeedShows);
			break;
		}
		
		RShowInfoArray &fItems = iPodcastModel.ActiveShowList();
		len = fItems.Count();
		
		if (iListbox != NULL)
		{
			MQikListBoxModel& model(iListbox->Model());
			TBool allUidsMatch = EFalse;

			if(len == iListbox->Model().Count())
			{		
				allUidsMatch = ETrue;
				TUint itemId = 0;
				for(TInt loop = 0;loop< len ;loop++)
				{
					MQikListBoxData* data = model.RetrieveDataL(loop);	
					itemId = data->ItemId();
					data->Close();
					if(fItems[loop]->Uid() != itemId)
					{			
						allUidsMatch = EFalse;
						break;
					}					
				}
			}

			if(allUidsMatch && len > 0)
			{
				model.ModelBeginUpdateLC();
				for(TInt loop = 0;loop< len ;loop++)
				{											
					MQikListBoxData* data = model.RetrieveDataL(loop);	
					CleanupClosePushL(*data);
					UpdateShowItemDataL(fItems[loop], data);									
					CleanupStack::PopAndDestroy();//close data
					model.DataUpdatedL(loop);
				}

				model.ModelEndUpdateL();

			}
			else
			{
				iListbox->RemoveAllItemsL();			
				
				// Informs the list box model that there will be an update of the data.
				// Notify the list box model that changes will be made after this point.
				// Observe that a cleanupitem has been put on the cleanupstack and 
				// will be removed by ModelEndUpdateL. This means that you have to 
				// balance the cleanupstack.
				// When you act directly on the model you always need to encapsulate 
				// the calls between ModelBeginUpdateLC and ModelEndUpdateL.
				model.ModelBeginUpdateLC();
				
				if (len > 0) {
					for (TInt i=0;i<len;i++) {
						MQikListBoxData* listBoxData = model.NewDataL(MQikListBoxModel::EDataNormal);
						CleanupClosePushL(*listBoxData);
						CShowInfo *si = fItems[i];
						listBoxData->SetItemId(si->Uid());
						listBoxData->AddTextL(si->Title(), EQikListBoxSlotText1);						
						if(si->ShowType() == EAudioBook)
							{
							showSize.Format(KChapterFormatting(), si->TrackNo());
							if(si->PlayTime() != 0)
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
									HBufC* unknown =  iEikonEnv->AllocReadResourceLC(R_PODCAST_ONPHONE_STATUS_UNKNOWN);
									showDate = *unknown;
									CleanupStack::PopAndDestroy(unknown);
								}
							}
						else
							{
							if(si->ShowSize() == 0) 
								{
								showSize = KNullDesC();
								} 
							else
								{
								showSize.Format(KShowsSizeFormat(), (float)si->ShowSize()/ (float)KSizeMb);
								showSize.Append(KShowsSizeUnit());
								}

							if(si->PubDate().Int64() == 0) 
								{
								showDate = KNullDesC();
								} 
								else 
								{
								si->PubDate().FormatL(showDate, KDateFormat());
								}
							}					

						listBoxData->AddTextL(showSize, EQikListBoxSlotText4);												
						listBoxData->AddTextL(showDate, EQikListBoxSlotText3);
						
						listBoxData->SetEmphasis(si->PlayState() == ENeverPlayed);										
						CleanupStack::PopAndDestroy();	
						
						TInt bitmap = EMbmPodcastclientShow_40x40;
						TInt mask = EMbmPodcastclientShow_40x40m;
						
						GetShowIcons(si, bitmap, mask);
						
						CQikContent* content = CQikContent::NewL(this, _L("*"), bitmap, mask);
						CleanupStack::PushL(content);
						listBoxData->AddIconL(content,EQikListBoxSlotLeftMediumIcon1);
						CleanupStack::Pop(content);
					}
				} else {		
					MQikListBoxData* listBoxData = model.NewDataL(MQikListBoxModel::EDataNormal);
					CleanupClosePushL(*listBoxData);
					HBufC* noItems=  iEikonEnv->AllocReadResourceLC(R_PODCAST_SHOWS_NO_ITEMS);

					listBoxData->AddTextL(*noItems, EQikListBoxSlotText1);
					listBoxData->SetDimmed(ETrue);
					CleanupStack::PopAndDestroy(noItems);
					CleanupStack::PopAndDestroy();
				}
				
				// Informs that the update of the list box model has ended
				model.ModelEndUpdateL();
			}
			
			
		}	
		
		UpdateCommandsL();

	}
}

void CPodcastClientShowsView::UpdateCommandsL()
{
	CQikCommandManager& comMan = CQikCommandManager::Static();
	RShowInfoArray &fItems = iPodcastModel.ActiveShowList();
	TInt itemCnt = fItems.Count();
	TBool removeDeleteShowCmd = ETrue;
	TBool removeDownloadShowCmd = ETrue;
	TBool removeRemoveSuspendCmd = EFalse;
	TBool updatingState = (iCurrentCategory != EShowDownloadedShows && iPodcastModel.FeedEngine().ClientState() != ENotUpdating && iPodcastModel.FeedEngine().ActiveClientUid() == iPodcastModel.ActiveFeedInfo()->Uid());
	TBool playingPodcast = (iPodcastModel.PlayingPodcast() != NULL && (iPodcastModel.SoundEngine().State() == ESoundEnginePlaying || iPodcastModel.SoundEngine().State() == ESoundEnginePaused));
	TBool removeSetPlayed = EFalse;
	comMan.SetInvisible(*this, EPodcastViewPlayer, !playingPodcast);

	if(iListbox != NULL)
	{
		TInt index = iListbox->CurrentItemIndex();

		if(index>= 0 && index < itemCnt)
		{		
			comMan.SetInvisible(*this, EQikListBoxCmdSelect, EFalse);
			comMan.SetTextL(*this, EQikListBoxCmdSelect, R_PODCAST_VIEW_CMD);
			comMan.SetShortTextL(*this, EQikListBoxCmdSelect, R_PODCAST_VIEW_CMD);
			if(fItems[index]->DownloadState() != EQueued &&  fItems[index]->DownloadState() != EDownloading)
			{			
			}
			else
			{
				if(fItems[index]->DownloadState() == EQueued)
				{
					comMan.SetTextL(*this, EPodcastRemoveDownload, R_PODCAST_PLAYER_REMOVE_DL_CMD);
					comMan.SetShortTextL(*this, EPodcastRemoveDownload, R_PODCAST_PLAYER_REMOVE_DL_CMD);
				}
				else
				{					
					comMan.SetTextL(*this, EPodcastRemoveDownload, R_PODCAST_PLAYER_SUSPEND_DL_CMD);
					comMan.SetShortTextL(*this, EPodcastRemoveDownload, R_PODCAST_PLAYER_SUSPEND_DL_CMD);
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
		else
		{
			comMan.SetInvisible(*this, EQikListBoxCmdSelect, ETrue);		
		}
	}
	else
	{
		comMan.SetInvisible(*this, EQikListBoxCmdSelect, ETrue);
	}

	TBool iUpdateRunning = iPodcastModel.FeedEngine().ClientState() != ENotUpdating && iPodcastModel.ActiveFeedInfo() != NULL && iPodcastModel.FeedEngine().ActiveClientUid() == iPodcastModel.ActiveFeedInfo()->Uid();

	comMan.SetInvisible(*this, EPodcastUpdateFeed, iUpdateRunning || (iCurrentCategory != EShowFeedShows || (iPodcastModel.ActiveFeedInfo()->FeedType() == EBookFeed)));
	comMan.SetInvisible(*this, EPodcastCancelUpdateAllFeeds, !iUpdateRunning);

	comMan.SetInvisible(*this, EPodcastDownloadShow, removeDownloadShowCmd);

	comMan.SetChecked(*this, EPodcastShowUnplayedOnly, iPodcastModel.SettingsEngine().SelectUnplayedOnly());
	
	comMan.SetAvailable(*this, EPodcastDownloadShow, !updatingState);
	comMan.SetAvailable(*this, EPodcastShowUnplayedOnly, !updatingState);
	comMan.SetAvailable(*this, EPodcastMarkAllPlayed, !updatingState);

	//comMan.SetInvisible(*this, EPodcastDeleteShow, removeDeleteShowCmd);
	comMan.SetAvailable(*this, EPodcastDeleteShow, !removeDeleteShowCmd);
	comMan.SetAvailable(*this, EPodcastDeleteShowHardware, !removeDeleteShowCmd);
	comMan.SetAvailable(*this, EPodcastDeleteAllPlayed, !updatingState && !iPodcastModel.SettingsEngine().SelectUnplayedOnly());
	TBool isOrdinaryList = iCurrentCategory == EShowDownloadedShows || (iCurrentCategory == EShowFeedShows && !(iPodcastModel.ActiveFeedInfo()?(iPodcastModel.ActiveFeedInfo()->FeedType() == EBookFeed):ETrue));
	comMan.SetInvisible(*this, EPodcastDeleteAllPlayed, !isOrdinaryList);

	comMan.SetInvisible(*this, EPodcastViewPendingShows, EFalse);
	comMan.SetInvisible(*this, EPodcastViewDownloadedShows, EFalse);

	comMan.SetInvisible(*this, EPodcastSetOrderAudioBook, !(EShowFeedShows == iCurrentCategory && (iPodcastModel.ActiveFeedInfo()?(iPodcastModel.ActiveFeedInfo()->FeedType() == EBookFeed): ETrue)));
	comMan.SetInvisible(*this, EPodcastAddAudioBookChapter, !(EShowFeedShows == iCurrentCategory && (iPodcastModel.ActiveFeedInfo()?(iPodcastModel.ActiveFeedInfo()->FeedType() == EBookFeed): ETrue)));
	
	comMan.SetInvisible(*this, EPodcastRemoveAllDownloads, ETrue);
	comMan.SetInvisible(*this, EPodcastMarkAsPlayed, removeSetPlayed || !itemCnt);
	comMan.SetInvisible(*this, EPodcastMarkAsUnplayed, !removeSetPlayed || !itemCnt);


	switch(iCurrentCategory)
	{	
	case EShowPendingShows:
		{
			comMan.SetInvisible(*this, EPodcastMarkAllPlayed, ETrue);
			comMan.SetInvisible(*this, EPodcastUpdateLibrary, ETrue);
			comMan.SetInvisible(*this, EPodcastRemoveDownload, !itemCnt);
			comMan.SetAvailable(*this, EPodcastRemoveDownload, !removeRemoveSuspendCmd);
			comMan.SetInvisible(*this, EPodcastRemoveDownloadHardware, (removeRemoveSuspendCmd || !itemCnt));
			comMan.SetInvisible(*this, EPodcastRemoveAllDownloads, !itemCnt);
			comMan.SetAvailable(*this, EPodcastRemoveAllDownloads, iPodcastModel.ShowEngine().DownloadsStopped());
			comMan.SetInvisible(*this, EPodcastStopDownloads, (iPodcastModel.ShowEngine().DownloadsStopped()));
			comMan.SetInvisible(*this, EPodcastResumeDownloads, (!iPodcastModel.ShowEngine().DownloadsStopped()));			
			comMan.SetInvisible(*this, EPodcastShowUnplayedOnly, ETrue);
			comMan.SetInvisible(*this, EPodcastViewPendingShows, ETrue);
			comMan.SetInvisible(*this, EPodcastDownloadShow, ETrue);
			comMan.SetInvisible(*this, EPodcastMarkAsPlayed, ETrue);
			comMan.SetInvisible(*this, EPodcastMarkAsUnplayed, ETrue);
			comMan.SetInvisible(*this, EPodcastDeleteShow, ETrue);
		}break;
	case EShowDownloadedShows:
		{
			comMan.SetInvisible(*this, EPodcastMarkAllPlayed, EFalse);
			comMan.SetInvisible(*this, EPodcastUpdateLibrary, EFalse);
			comMan.SetInvisible(*this, EPodcastRemoveDownload, ETrue);
			comMan.SetInvisible(*this, EPodcastRemoveDownloadHardware, ETrue);
			comMan.SetInvisible(*this, EPodcastStopDownloads, ETrue);
			comMan.SetInvisible(*this, EPodcastResumeDownloads, ETrue);
			comMan.SetInvisible(*this, EPodcastShowUnplayedOnly, EFalse);
			comMan.SetInvisible(*this, EPodcastViewDownloadedShows, ETrue);
		}break;
	default:
		{
			comMan.SetInvisible(*this, EPodcastRemoveDownload, ETrue);
			comMan.SetInvisible(*this, EPodcastRemoveDownloadHardware, ETrue);
			comMan.SetInvisible(*this, EPodcastStopDownloads, ETrue);
			comMan.SetInvisible(*this, EPodcastResumeDownloads, ETrue);
			comMan.SetInvisible(*this, EPodcastMarkAllPlayed, EFalse);
			comMan.SetInvisible(*this, EPodcastUpdateLibrary, ETrue);
			comMan.SetInvisible(*this, EPodcastShowUnplayedOnly, EFalse);
		}break;
	}	

	if (iCurrentCategory == EShowPendingShows) {
		HBufC* titleBuffer = NULL;
		
		if (iPodcastModel.ShowEngine().DownloadsStopped()) {
			HBufC* titleFormat=  iEikonEnv->AllocReadResourceLC(R_PODCAST_SHOWS_DOWNLOADS_SUSPENDED);
			titleBuffer = HBufC::NewL(titleFormat->Length()+8);
			titleBuffer->Des().Format(*titleFormat, itemCnt);			
			CleanupStack::PopAndDestroy(titleFormat);
			CleanupStack::PushL(titleBuffer);
		} else {
			HBufC* titleFormat=  iEikonEnv->AllocReadResourceLC(R_PODCAST_SHOWS_TITLE_DOWNLOAD);
			titleBuffer = HBufC::NewL(titleFormat->Length()+8);
			titleBuffer->Des().Format(*titleFormat, itemCnt);			
			CleanupStack::PopAndDestroy(titleFormat);
			CleanupStack::PushL(titleBuffer);
		}
		ViewContext()->ChangeTextL(EPodcastListViewContextLabel, *titleBuffer);
		CleanupStack::PopAndDestroy(titleBuffer);
		
	} else {
		TUint numUnplayed = 0;
		TUint numShows = 0;
				
		if(itemCnt == 0)
		{
			ViewContext()->ChangeTextL(EPodcastListViewContextLabel, KNullDesC());
		}
		HBufC* titleFormat=  iEikonEnv->AllocReadResourceLC(R_PODCAST_SHOWS_TITLE_FORMAT);
		HBufC* titleBuffer = HBufC::NewL(titleFormat->Length()+8);
		if (iPodcastModel.ActiveFeedInfo()) {
			iPodcastModel.FeedEngine().GetStatsByFeed(iPodcastModel.ActiveFeedInfo()->Uid(), numShows, numUnplayed);
		} else {
			iPodcastModel.FeedEngine().GetDownloadedStats(numShows, numUnplayed);		
		}
		titleBuffer->Des().Format(*titleFormat, numUnplayed, numShows);
		CleanupStack::PopAndDestroy(titleFormat);
		CleanupStack::PushL(titleBuffer);
		ViewContext()->ChangeTextL(EPodcastListViewContextLabel, *titleBuffer);
		CleanupStack::PopAndDestroy(titleBuffer);	
	}
}


void CPodcastClientShowsView::HandleListBoxEventL(CQikListBox * /*aListBox*/, TQikListBoxEvent aEventType, TInt aItemIndex, TInt aSlotId)
{
	//RDebug::Print(_L("HandleListBoxEvent, itemIndex=%d, slotId=%d, aEventType=%d"), aItemIndex, aSlotId, aEventType);
	
	switch (aEventType)
	{
	case EEventHighlightMoved:
		{
			UpdateCommandsL();
		}
		break;
	case EEventItemHighlighted:
	case EEventItemConfirmed:
	case EEventItemTapped:
		{
			RShowInfoArray &fItems = iPodcastModel.ActiveShowList();

			if(aItemIndex>=0 && aItemIndex< fItems.Count())
			{
				DP2("Handle event for podcast %S, downloadState is %d", &(fItems[aItemIndex]->Title()), fItems[aItemIndex]->DownloadState());
				
				TPckgBuf<TInt> showUid;
				showUid() = fItems[aItemIndex]->Uid();
				TVwsViewId viewId = TVwsViewId(KUidPodcastClientID, KUidPodcastPlayViewID);
				iQikAppUi.ActivateViewL(viewId, TUid::Uid(KActiveShowUIDCmd), showUid);		

				UpdateCommandsL();
			}
		}
		break;
	default:
		break;
	}
}

TKeyResponse CPodcastClientShowsView::OfferKeyEventL(const TKeyEvent& aKeyEvent,TEventCode aType)
	{
	//RDebug::Print(_L("CPodcastClientShowsView::OfferKeyEventL"));
	CQikViewBase::OfferKeyEventL(aKeyEvent, aType);
	
	if(aType == EEventKey)
		{
			switch(aKeyEvent.iCode)
			{
			case '*':
			case EKeyLeftArrow:
				RDebug::Print(_L("CPodcastClientShowsView::OfferKeyEventL switching play state"));
				TInt index = iListbox->CurrentItemIndex();
				if(index >= 0 && index < iPodcastModel.ActiveShowList().Count())
				{
					if (iPodcastModel.ActiveShowList()[index]->PlayState() == EPlayed) {
						iPodcastModel.ActiveShowList()[index]->SetPlayState(ENeverPlayed);
					} else {
						iPodcastModel.ActiveShowList()[index]->SetPlayState(EPlayed);
					}
					
					if (iPodcastModel.SettingsEngine().SelectUnplayedOnly()) {
						MQikListBoxModel& model(iListbox->Model());
						model.ModelBeginUpdateLC();
						model.RemoveDataL(index);
						model.ModelEndUpdateL();
					}
					

				UpdateListboxItemsL();
				return EKeyWasConsumed;
				}
			}
		}
	
	return EKeyWasNotConsumed;
	}
