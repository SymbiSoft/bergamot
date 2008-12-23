#include <qikcommand.h>
#include <qikcontent.h>
#include <QikMediaFileFolderUtils.h>
#include <QikSelectFileDlg.h>
#include <QikSaveFileDlg.h>
#include <PodcastClient.mbg>
#include <PodcastClient.rsg>

#include "PodcastClientAudioBookDlg.h"
#include "PodcastClientAddFeedDlg.h"
#include "PodcastClientFeedView.h"
#include "PodcastClientPlayView.h"
#include "PodcastModel.h"
#include "ShowEngine.h"
#include "SoundEngine.h"
#include "SettingsEngine.h"
#include "FeedInfo.h"

const TInt KMaxFeedNameLength = 100;
const TInt KMaxUnplayedFeedsLength =64;
const TInt KADayInHours = 24;
const TInt KDefaultGran = 5;
_LIT(KUnknownUpdateDateString, "?/?");
/**
Creates and constructs the view.

@param aAppUi Reference to the AppUi
@return Pointer to a CPodcastClientView object
*/
CPodcastClientFeedView* CPodcastClientFeedView::NewLC(CQikAppUi& aAppUi, CPodcastModel& aPodcastModel)
	{
	CPodcastClientFeedView* self = new (ELeave) CPodcastClientFeedView(aAppUi, aPodcastModel);
	CleanupStack::PushL(self);
	self->ConstructL();
	return self;
	}

CPodcastClientFeedView::CPodcastClientFeedView(CQikAppUi& aAppUi, CPodcastModel& aPodcastModel):CPodcastClientView(aAppUi, aPodcastModel)
	{
	}

CPodcastClientFeedView::~CPodcastClientFeedView()
	{
	iPodcastModel.FeedEngine().RemoveObserver(this);
	delete iBooksFormat;
	delete iFeedsFormat;
	delete iNeverUpdated;
	}

void CPodcastClientFeedView::ConstructL()
	{
	CPodcastClientView::ConstructL();

	iPodcastModel.FeedEngine().AddObserver(this);
	}

/**
Returns the view Id
@return Returns the Uid of the view
*/
TVwsViewId CPodcastClientFeedView::ViewId()const
	{
	return TVwsViewId(KUidPodcastClientID, KUidPodcastFeedViewID);
	}

void CPodcastClientFeedView::ViewConstructL()
	{
	iNeverUpdated = iEikonEnv->AllocReadResourceL(R_PODCAST_FEEDS_NEVER_UPDATED);
	iBooksFormat = iEikonEnv->AllocReadResourceL(R_PODCAST_BOOKS_STATUS_FORMAT);
	iFeedsFormat = iEikonEnv->AllocReadResourceL(R_PODCAST_FEEDS_STATUS_FORMAT);
	iViewLabel = iEikonEnv->AllocReadResourceL(R_PODCAST_FEEDS_TITLE);
	iAudioBooksLabel = iEikonEnv->AllocReadResourceL(R_PODCAST_AUDIOBOOKS_TITLE);
    ViewConstructFromResourceL(R_PODCAST_FEEDVIEW_UI_CONFIGURATIONS);
	CPodcastClientView::ViewConstructL();
	}


void CPodcastClientFeedView::ViewActivatedL(const TVwsViewId &aPrevViewId, TUid aCustomMessageId, const TDesC8 &aCustomMessage)
	{
	switch(aCustomMessageId.iUid)
	{
	case EFeedsAudioBooksMode:
		{
		iCurrentViewMode = EFeedsAudioBooksMode;
		}
		break;
	case EFeedsNormalMode:
		{
		iCurrentViewMode = EFeedsNormalMode;
		}break;
	default:
		{
		// Fix for issue #72
		if (aPrevViewId.iAppUid != KUidPodcastClientID) {
			break;
		}
		
		if(/*(aPrevViewId == TVwsViewId(KUidPodcastClientID, KUidPodcastShowsViewID) ||
			aPrevViewId == TVwsViewId(KUidPodcastClientID, KUidPodcastFeedViewID)) &&*/	
			iPodcastModel.ActiveFeedInfo() != NULL && iPodcastModel.ActiveFeedInfo()->IsBookFeed())
			{
			iCurrentViewMode = EFeedsAudioBooksMode;
			}
		else
			{
			iCurrentViewMode = EFeedsNormalMode;
			}
		}
		break;
	}	

	CPodcastClientView::ViewActivatedL(aPrevViewId, aCustomMessageId, aCustomMessage);
	SetParentView( TVwsViewId(KUidPodcastClientID, KUidPodcastBaseViewID) );
	}


CQikCommand* CPodcastClientFeedView::DynInitOrDeleteCommandL(CQikCommand* aCommand, const CCoeControl& /*aControlAddingCommands*/)
	{
	switch(aCommand->Id())
		{
		case EQikListBoxCmdSelect:
			{
			aCommand->SetTextL(R_PODCAST_SHOW_CMD);
			break;
			}
			
		case EQikSoftkeyCmdSelectCategory:
			{	
			aCommand->SetTextL(R_PODCAST_FEEDS_CATEGORY);
			break;
			}
		
		default:
			break;
		}
	
	return aCommand;
	}

void CPodcastClientFeedView::UpdateFeedInfoDataL(CFeedInfo* aFeedInfo,  MQikListBoxData* aListboxData, TBool aIsUpdating )
	{
	TBuf<KMaxShortDateFormatSpec*2> updatedDate;
	aListboxData->SetTextL(aFeedInfo->Title(), EQikListBoxSlotText1);
	//aListboxData->SetTextL(aFeedInfo->Description(), EQikListBoxSlotText2);
	
	TUint unplayedCount = 0;
	TUint showCount = 0;
	TBuf<KMaxUnplayedFeedsLength> unplayedShows;

	if(aIsUpdating)
		{
			iEikonEnv->ReadResourceL(updatedDate, R_PODCAST_FEEDS_IS_UPDATING);
			unplayedShows = KNullDesC();			
		}
	else
	{
		iPodcastModel.FeedEngine().GetStatsByFeed(aFeedInfo->Uid(), showCount, unplayedCount, aFeedInfo->IsBookFeed());
		
		if (aFeedInfo->IsBookFeed()) {
			unplayedShows.Format(*iBooksFormat, unplayedCount, showCount);
		} else {
			unplayedShows.Format(*iFeedsFormat, unplayedCount, showCount);
		}	
		
		aListboxData->SetEmphasis(unplayedCount > 0);					
				
		if (aFeedInfo->LastUpdated().Int64() == 0) 
		{
			updatedDate.Zero();
		}
		else 
		{
			TTime now;
			TTimeIntervalHours interval;
			now.HomeTime();
			now.HoursFrom(aFeedInfo->LastUpdated(), interval);
			if (interval.Int() < KADayInHours) 
			{
				aFeedInfo->LastUpdated().FormatL(updatedDate, KTimeFormat());
			}
			else 
			{
				aFeedInfo->LastUpdated().FormatL(updatedDate, KDateFormat());
			}
		}
	}
	aListboxData->SetDisabled(aIsUpdating);
	aListboxData->SetDimmed(aIsUpdating);

	aListboxData->SetTextL(unplayedShows, EQikListBoxSlotText2);
	aListboxData->SetTextL(updatedDate, EQikListBoxSlotText3);
	}

void CPodcastClientFeedView::FeedInfoUpdated(CFeedInfo* aFeedInfo)
	{
	TRAP_IGNORE(FeedInfoUpdatedL(aFeedInfo))
	}

void CPodcastClientFeedView::FeedInfoUpdatedL(CFeedInfo* aFeedInfo)
	{
	if (ViewContext() == NULL) 
		{
		return;
		}
	
	// Since a title might have been updated
	if(iProgressAdded)
		{
		ViewContext()->RemoveAndDestroyProgressInfo();
		ViewContext()->DrawNow();
		iProgressAdded = EFalse;
		}


	if(iCurrentViewMode == EFeedsNormalMode)
	{				
		const RFeedInfoArray& feeds = iPodcastModel.FeedEngine().GetSortedFeeds();
		
		TInt index = feeds.Find(aFeedInfo);
		MQikListBoxModel& model(iListbox->Model());
		
		if(index != KErrNotFound && index<model.Count())
		{
			model.ModelBeginUpdateLC();
			MQikListBoxData* data = model.RetrieveDataL(index);
			
			if(data != NULL)
			{
				CleanupClosePushL(*data);
				UpdateFeedInfoDataL(aFeedInfo, data);
				CleanupStack::PopAndDestroy(data);
				model.DataUpdatedL(index);
			}
			model.ModelEndUpdateL();
		}
	}
	}

void CPodcastClientFeedView::UpdateFeedInfoStatusL(TUint aFeedUid, TBool aIsUpdating)
	{
	
	if(iCurrentViewMode == EFeedsNormalMode)
		{	
		const RFeedInfoArray& feeds = iPodcastModel.FeedEngine().GetSortedFeeds();

		TInt cnt = feeds.Count();
		TInt index = KErrNotFound;
		while(index == KErrNotFound && cnt>0)
		{
			cnt--;
			if(feeds[cnt]->Uid() == aFeedUid)
			{
				index = cnt;
				break;
			}
		}
	//	TInt index = feeds.Find(aFeedInfo);

		MQikListBoxModel& model(iListbox->Model());
		
		if(index != KErrNotFound && index < model.Count())
			{
			model.ModelBeginUpdateLC();
			MQikListBoxData* data = model.RetrieveDataL(index);	
			
			if(data != NULL)
				{
				CleanupClosePushL(*data);
				UpdateFeedInfoDataL(feeds[index], data, aIsUpdating);
				CleanupStack::PopAndDestroy(data);
				model.DataUpdatedL(index);
				}
			model.ModelEndUpdateL();
			}
		}
	}

void CPodcastClientFeedView::FeedUpdateCompleteL(TUint aFeedUid)
	{
	UpdateFeedInfoStatusL(aFeedUid, EFalse);
	}

void CPodcastClientFeedView::FeedUpdateAllCompleteL()
{
	iUpdatingAllRunning = EFalse;
	UpdateCommandsL();
}

void CPodcastClientFeedView::FeedDownloadUpdatedL(TUint aFeedUid, TInt aPercentOfCurrentDownload)
	{

	if(!iUpdatingAllRunning)
	{
		iUpdatingAllRunning = ETrue;
		UpdateCommandsL();
	}

	if (ViewContext() == NULL) 
		{
		return;
		}
	
	UpdateFeedInfoStatusL(aFeedUid, ETrue);

	if(aPercentOfCurrentDownload>=0 && aPercentOfCurrentDownload<100)
		{
		if(!iProgressAdded)
			{
			ViewContext()->AddProgressInfoL(EEikProgressTextPercentage, 100);
			iProgressAdded = ETrue;
			}
		ViewContext()->SetAndDrawProgressInfo(aPercentOfCurrentDownload);
		}
		
	else if(iProgressAdded)
		{
		ViewContext()->RemoveAndDestroyProgressInfo();
		ViewContext()->DrawNow();
		iProgressAdded = EFalse;
		}
	}

void CPodcastClientFeedView::UpdateListboxItemsL()
	{
	if(IsVisible())
		{		
		const RFeedInfoArray* sortedItems = NULL;
		if(iCurrentViewMode == EFeedsAudioBooksMode)
			{
			sortedItems = &iPodcastModel.FeedEngine().GetSortedBooks();
			}
		else
			{
			sortedItems = &iPodcastModel.FeedEngine().GetSortedFeeds();
			}
		
		TInt len = sortedItems->Count();
		TBool allUidsMatch = EFalse;
		MQikListBoxModel& model(iListbox->Model());

		if(len == iListbox->Model().Count())
			{
			TUint itemId = 0;
			for(TInt loop = 0;loop< len ;loop++)
				{
				MQikListBoxData* data = model.RetrieveDataL(loop);	
				itemId = data->ItemId();
				data->Close();
				if((*sortedItems)[loop]->Uid() != itemId)
					{						
					break;
					}
				allUidsMatch = ETrue;
				}
			}

		if(allUidsMatch)
			{
			model.ModelBeginUpdateLC();

			for(TInt loop = 0;loop< len ;loop++)
				{
				MQikListBoxData* data = model.RetrieveDataL(loop);	
				CleanupClosePushL(*data);
				UpdateFeedInfoDataL((*sortedItems)[loop], data);
				CleanupStack::PopAndDestroy(data);
				model.DataUpdatedL(loop);
				}

			// Informs that the update of the list box model has ended
			model.ModelEndUpdateL();
			}
		else
			{
			iListbox->RemoveAllItemsL();
			
			model.ModelBeginUpdateLC();
									
			MQikListBoxData* listBoxData;
			TBuf<KMaxShortDateFormatSpec*2> updatedDate;
			TBuf<KMaxUnplayedFeedsLength> unplayedShows;
			
			if (len > 0) 
				{
				for (int i=0;i<len;i++) 
					{
					listBoxData = model.NewDataL(MQikListBoxModel::EDataNormal);
					CleanupClosePushL(*listBoxData);
					
					CFeedInfo *fi = (*sortedItems)[i];
					listBoxData->SetItemId(fi->Uid());
					listBoxData->AddTextL(fi->Title(), EQikListBoxSlotText1);					

					TUint unplayedCount = 0;
					TUint showCount = 0;
					iPodcastModel.FeedEngine().GetStatsByFeed(fi->Uid(), showCount, unplayedCount, fi->IsBookFeed());
					if (fi->IsBookFeed()) {
						unplayedShows.Format(*iBooksFormat, unplayedCount, showCount);
					} else {
						unplayedShows.Format(*iFeedsFormat, unplayedCount, showCount);
					}

					if (fi->LastUpdated().Int64() == 0) 
						{
						updatedDate.Copy(*iNeverUpdated);					
						unplayedShows.Copy(KUnknownUpdateDateString());
						}
					else 
						{
						TTime now;
						TTimeIntervalHours interval;
						now.UniversalTime();
						now.HoursFrom(fi->LastUpdated(), interval);
						if (interval.Int() < 24) 
							{
							fi->LastUpdated().FormatL(updatedDate, KTimeFormat());
							}
						else 
							{
							fi->LastUpdated().FormatL(updatedDate, KDateFormat());
							}
						}
						
					listBoxData->SetEmphasis(unplayedCount > 0);					
					listBoxData->AddTextL(unplayedShows, EQikListBoxSlotText2);
					listBoxData->AddTextL(updatedDate, EQikListBoxSlotText3);
					
					
					CQikContent* content;
					
					if (fi->IsBookFeed()) {
						content = CQikContent::NewL(this, _L("*"), EMbmPodcastclientAudiobookindividual_40x40, EMbmPodcastclientAudiobookindividual_40x40m);
					} else {
						content = CQikContent::NewL(this, _L("*"), EMbmPodcastclientFeed_40x40, EMbmPodcastclientFeed_40x40m);
					}
					
					CleanupStack::PushL(content);
					listBoxData->AddIconL(content,EQikListBoxSlotLeftMediumIcon1);
					CleanupStack::Pop(content);
					
					CleanupStack::PopAndDestroy(listBoxData);
					}
				} 
			else 
				{
				listBoxData = model.NewDataL(MQikListBoxModel::EDataNormal);
				CleanupClosePushL(*listBoxData);
					
				TBuf<KMaxFeedNameLength> itemName;
				if(iCurrentViewMode == EFeedsAudioBooksMode)
					{
					iEikonEnv->ReadResourceL(itemName, R_PODCAST_BOOKS_NO_BOOKS);
					}
				else
					{
					iEikonEnv->ReadResourceL(itemName, R_PODCAST_FEEDS_NO_FEEDS);
					}
				listBoxData->AddTextL(itemName, EQikListBoxSlotText1);
				listBoxData->SetDimmed(ETrue);
				
				CleanupStack::PopAndDestroy(listBoxData);
				}

			// Informs that the update of the list box model has ended
			model.ModelEndUpdateL();
			}
		
		// Update the contextbar so information matches the listbox content
		HBufC* templateStr = NULL;
		switch(iCurrentViewMode)
			{
		case EFeedsAudioBooksMode:
			{
			templateStr = iEikonEnv->AllocReadResourceLC(R_PODCAST_BOOKS_TITLE_FORMAT);
			}break;
		default:
			{
			templateStr = iEikonEnv->AllocReadResourceLC(R_PODCAST_FEEDS_TITLE_FORMAT);
			}
			break;
			}
		HBufC* titleBuffer = HBufC::NewLC(templateStr->Length()+8);
		titleBuffer->Des().Format(*templateStr, len);
		ViewContext()->ChangeTextL(EPodcastListViewContextLabel, *titleBuffer);
		CleanupStack::PopAndDestroy(titleBuffer);
		CleanupStack::PopAndDestroy(templateStr);

		}
	
	UpdateCommandsL();
	}


void CPodcastClientFeedView::HandleListBoxEventL(CQikListBox* /*aListBox*/, TQikListBoxEvent aEventType, TInt aItemIndex, TInt aSlotId)
	{
	//RDebug::Print(_L("HandleListBoxEvent, itemIndex=%d, slotId=%d, aEventType=%d"), aItemIndex, aSlotId, aEventType);
	
	switch (aEventType)
		{
		case EEventHighlightMoved:
			UpdateCommandsL();
			break;
		case EEventItemHighlighted:
		case EEventItemConfirmed:
		case EEventItemTapped:
			{		
			const RFeedInfoArray* sortedItems = NULL;
			if(iCurrentViewMode == EFeedsAudioBooksMode)
				{
				sortedItems = &iPodcastModel.FeedEngine().GetSortedBooks();
				}
			else
				{
				sortedItems = &iPodcastModel.FeedEngine().GetSortedFeeds();
				}

			if(aItemIndex >= 0 && aItemIndex < sortedItems->Count())
				{
				iPodcastModel.ActiveShowList().Reset();
				iPodcastModel.SetActiveFeedInfo((*sortedItems)[aItemIndex]);
				TVwsViewId showsView = TVwsViewId(KUidPodcastClientID, KUidPodcastShowsViewID);
				iQikAppUi.ActivateViewL(showsView,  TUid::Uid(EShowFeedShows), KNullDesC8());
				}
			break;	
			}
		
		default:
			break;
		}
	}


void CPodcastClientFeedView::UpdateCommandsL()
	{
	CQikCommandManager& comMan = CQikCommandManager::Static();
	
	
	switch(iCurrentViewMode)
	{
	case EFeedsAudioBooksMode:
		{
		SetAppTitleNameL(*iAudioBooksLabel);
		}
		break;
	default:
		{
		SetAppTitleNameL(*iViewLabel);
		}
		break;
	}	

	TBool playingPodcast = (iPodcastModel.PlayingPodcast() != NULL && (iPodcastModel.SoundEngine().State() == ESoundEnginePlaying || iPodcastModel.SoundEngine().State() == ESoundEnginePaused));
	comMan.SetInvisible(*this, EPodcastViewPlayer, !playingPodcast);

	if (iListbox == NULL)
		return;
	
	TInt index = iListbox->CurrentItemIndex();
	

	TBool isBookMode = (iCurrentViewMode == EFeedsAudioBooksMode);
	const RFeedInfoArray* sortedItems = NULL;
	if(isBookMode)
			{
			sortedItems = &iPodcastModel.FeedEngine().GetSortedBooks();
			}
		else
			{
			sortedItems = &iPodcastModel.FeedEngine().GetSortedFeeds();
			}
	
	// hide commands that should not be visible in no feeds
	TUint cnt = sortedItems->Count();
	if (cnt == 0)
		{
		comMan.SetInvisible(*this, EQikListBoxCmdSelect, ETrue);
		comMan.SetInvisible(*this, EPodcastDeleteFeed, ETrue);
		comMan.SetInvisible(*this, EPodcastUpdateAllFeeds, ETrue);
		comMan.SetInvisible(*this, EPodcastEditFeed, ETrue);
		comMan.SetInvisible(*this, EPodcastRemoveAudioBook, ETrue);
		}
	else
		{
		comMan.SetInvisible(*this, EQikListBoxCmdSelect, EFalse);
		comMan.SetInvisible(*this, EPodcastDeleteFeed, isBookMode);
		comMan.SetInvisible(*this, EPodcastUpdateAllFeeds, isBookMode);
		comMan.SetInvisible(*this, EPodcastEditFeed, isBookMode);	
		comMan.SetInvisible(*this, EPodcastRemoveAudioBook, !isBookMode);
		}
	
	comMan.SetInvisible(*this, EPodcastAddNewAudioBook, !isBookMode);
	comMan.SetInvisible(*this, EPodcastImportAudioBook, !isBookMode);
	comMan.SetInvisible(*this, EPodcastAddFeed, isBookMode);
	comMan.SetInvisible(*this, EPodcastImportFeeds, isBookMode);
	comMan.SetInvisible(*this, EPodcastExportFeeds, isBookMode);

	comMan.SetInvisible(*this, EPodcastViewAudioBooks, isBookMode);
	comMan.SetInvisible(*this, EPodcastViewFeeds, !isBookMode);
		
	TBool playingThisBook = (iPodcastModel.PlayingPodcast() != NULL) && (sortedItems != NULL && sortedItems->Count() > 0) && (iPodcastModel.PlayingPodcast()->FeedUid() == (*sortedItems)[index]->Uid()) && iPodcastModel.SoundEngine().State() == ESoundEnginePlaying;
	comMan.SetAvailable(*this, EPodcastPlayAudioBook, isBookMode && cnt && !playingThisBook);
	comMan.SetAvailable(*this, EPodcastPauseAudioBook, isBookMode && cnt && playingThisBook);
		
	comMan.SetInvisible(*this, EPodcastCancelUpdateAllFeeds, isBookMode);	
	comMan.SetAvailable(*this, EPodcastUpdateAllFeeds, !iUpdatingAllRunning);
	comMan.SetAvailable(*this, EPodcastCancelUpdateAllFeeds, iUpdatingAllRunning);
	comMan.SetDimmed(*this, EPodcastAddFeed, iUpdatingAllRunning);
	comMan.SetDimmed(*this, EPodcastEditFeed, iUpdatingAllRunning);
	comMan.SetDimmed(*this, EPodcastDeleteFeed, iUpdatingAllRunning);

	comMan.SetDimmed(*this, EPodcastImportFeeds, iUpdatingAllRunning);
	comMan.SetDimmed(*this, EPodcastExportFeeds, iUpdatingAllRunning);

	comMan.SetAvailable(*this, EPodcastRemoveAudioBookHardware,iCurrentViewMode==EFeedsAudioBooksMode && !iUpdatingAllRunning);
	comMan.SetAvailable(*this, EPodcastDeleteFeedHardware,!(iCurrentViewMode==EFeedsAudioBooksMode) && !iUpdatingAllRunning);
	}



void CPodcastClientFeedView::HandleCommandL(CQikCommand& aCommand)
	{
	
	ViewContext()->RemoveAndDestroyProgressInfo();
	switch(aCommand.Id())
		{
		case EPodcastAddFeed:
			{
			CPodcastClientAddFeedDlg* dlg = new (ELeave) CPodcastClientAddFeedDlg(iPodcastModel);
			if(dlg->ExecuteLD(R_PODCAST_ADD_FEED_DLG))
				{
				UpdateListboxItemsL();
				}
			break;
			}
		case EPodcastImportFeeds:
			{		
			CDesCArrayFlat* mimeTypes = new (ELeave) CDesCArrayFlat(KDefaultGran);
			CleanupStack::PushL(mimeTypes);
			CDesCArrayFlat* fileNames = new (ELeave) CDesCArrayFlat(KDefaultGran);
			CleanupStack::PushL(fileNames);
			HBufC* import_title = iEikonEnv->AllocReadResourceLC(R_PODCAST_IMPORT_FEEDS_TITLE);
			if(CQikSelectFileDlg::RunDlgLD(*mimeTypes, *fileNames, import_title))
				{
				if(fileNames->MdcaCount()>0)
					{
					iPodcastModel.FeedEngine().ImportFeedsL(fileNames->MdcaPoint(0));
					UpdateListboxItemsL();
					}
				}
		
			CleanupStack::PopAndDestroy(3,mimeTypes); // title, fileNames, mimeTypes								
			}break;
		case EPodcastExportFeeds:
			{
			TFileName fileName;
			iPodcastModel.FeedEngine().ExportFeedsL(fileName);
			TBuf<1024> fName;
			TFileName newName;
			newName.Copy(_L("feeds.xml"));
			CQikSaveFileDlg::RunDlgLD(fileName, newName, NULL,ESaveFileUsingMove);
			}break;
		case EPodcastEditFeed:
			{
			if(iListbox != NULL)
				{
				TInt index = iListbox->CurrentItemIndex();

				const RFeedInfoArray& feeds = iPodcastModel.FeedEngine().GetSortedFeeds();

				TInt cnt = feeds.Count();

				if(index< cnt)
					{
					CPodcastClientAddFeedDlg* dlg = new (ELeave) CPodcastClientAddFeedDlg(iPodcastModel, feeds[index]);
					if(dlg->ExecuteLD(R_PODCAST_EDIT_FEED_DLG))
						{
						UpdateListboxItemsL();
						}
					}			
				}
			break;
			}
			
		case EPodcastDeleteFeedHardware:
		case EPodcastDeleteFeed:
			{
			if(iListbox != NULL)
				{
				TInt index = iListbox->CurrentItemIndex();
				MQikListBoxModel& model(iListbox->Model());
				MQikListBoxData* data = model.RetrieveDataL(index);	
				if(data != NULL)
					{
					if(iEikonEnv->QueryWinL(R_PODCAST_REMOVE_FEED_TITLE, R_PODCAST_REMOVE_FEED_PROMPT))
						{
						iPodcastModel.FeedEngine().RemoveFeed(data->ItemId());
						iListbox->RemoveItemL(index);
						}
					data->Close();
					}
				UpdateListboxItemsL();
				}
			break;	
			}
		case EPodcastUpdateAllFeeds:
		{
			iUpdatingAllRunning = ETrue;
			UpdateCommandsL();
			iPodcastModel.FeedEngine().UpdateAllFeedsL();
			HBufC* str = iEikonEnv->AllocReadResourceLC(R_PODCAST_FEEDS_UPDATE_MESSAGE);
			User::InfoPrint(*str);
			CleanupStack::PopAndDestroy(str);
		}
		break;

		case EPodcastCancelUpdateAllFeeds:
			{
				if(iUpdatingAllRunning)
				{
					iUpdatingAllRunning = EFalse;
					
					iPodcastModel.FeedEngine().CancelUpdateAllFeedsL();

					if(iProgressAdded)
					{
						ViewContext()->RemoveAndDestroyProgressInfo();
						ViewContext()->DrawNow();
						iProgressAdded = EFalse;
					}

					UpdateCommandsL();
				}
			}break;
			//
		case EPodcastDeleteAllPlayed:
			{
				//HandleAddNewAudioBookL();
			/*if(iListbox != NULL)
				{
				TInt index = iListbox->CurrentItemIndex();
				}*/
			break;
			}
	case EPodcastPlayAudioBook:
		{
		if(iListbox != NULL)
			{
			TInt index = iListbox->CurrentItemIndex();
			MQikListBoxModel& model(iListbox->Model());
			MQikListBoxData* data = model.RetrieveDataL(index);	
			if(data != NULL)
				{
				CFeedInfo *feedInfo = iPodcastModel.FeedEngine().GetFeedInfoByUid(data->ItemId());
				
				if (feedInfo != NULL) {
					iPodcastModel.SetActiveFeedInfo(feedInfo);
					TBool aUnplayedOnlyState = iPodcastModel.SettingsEngine().SelectUnplayedOnly();
					// we only select unplayed chapters
					iPodcastModel.SettingsEngine().SetSelectUnplayedOnly(ETrue);
					iPodcastModel.GetShowsByFeed(feedInfo->Uid());
					iPodcastModel.SettingsEngine().SetSelectUnplayedOnly(aUnplayedOnlyState);

					RShowInfoArray& showArray = iPodcastModel.ActiveShowList();

					if(showArray.Count() == 0 || showArray[0] == NULL) {
						// can't play empty books...
						return;
					}

					// this should be the first unplayed chapter in this book
					CShowInfo *startShow = showArray[0];

					TPckgBuf<TInt> showUid;
					showUid() = startShow->Uid();
					TVwsViewId viewId = TVwsViewId(KUidPodcastClientID, KUidPodcastPlayViewID);
					iQikAppUi.ActivateViewL(viewId, TUid::Uid(KActiveShowUIDCmd), showUid);											
					iPodcastModel.PlayPausePodcastL(startShow, ETrue);
				}
				data->Close();
				}
			}
			UpdateCommandsL();
		}
		break;
	case EPodcastPauseAudioBook:
		{
		iPodcastModel.SoundEngine().Pause();
		UpdateCommandsL();
		}
		break;
	case EPodcastAddNewAudioBook:
		{
		HandleAddNewAudioBookL();
		UpdateListboxItemsL();
		}break;
	case EPodcastImportAudioBook:
		{
		CDesCArrayFlat* mimeTypes = iEikonEnv->ReadDesCArrayResourceL(R_PODCAST_IMPORT_AUDIOBOOK_MIMEARRAY);
			
		//CDesCArrayFlat* mimeTypes = new (ELeave) CDesCArrayFlat(KDefaultGran);
		CleanupStack::PushL(mimeTypes);
		
		CDesCArrayFlat* fileNames = new (ELeave) CDesCArrayFlat(KDefaultGran);
		CleanupStack::PushL(fileNames);
		HBufC* import_title = iEikonEnv->AllocReadResourceLC(R_PODCAST_IMPORT_BOOK_TITLE);
		if(CQikSelectFileDlg::RunDlgLD(*mimeTypes, *fileNames, import_title))
			{
			if(fileNames->MdcaCount()>0)
				{
				CPodcastClientAudioBookDlg* titleDialog = new (ELeave) CPodcastClientAudioBookDlg(iPodcastModel, *fileNames, ETrue);
				titleDialog->ExecuteLD(R_PODCAST_NEW_AUDIOBOOK_DLG);	
				}
			
			}
		UpdateListboxItemsL();
		CleanupStack::PopAndDestroy(3,mimeTypes); // title, fileNames, mimeTypes								
		}break;
	case EPodcastRemoveAudioBookHardware:
	case EPodcastRemoveAudioBook:
		{
			if(iListbox != NULL)
				{
				TInt index = iListbox->CurrentItemIndex();
				MQikListBoxModel& model(iListbox->Model());
				MQikListBoxData* data = model.RetrieveDataL(index);	
				if(data != NULL)
					{
					if(iEikonEnv->QueryWinL(R_PODCAST_REMOVE_BOOK_TITLE, R_PODCAST_REMOVE_BOOK_PROMPT))
						{
						iPodcastModel.FeedEngine().RemoveBookL(data->ItemId());
						iListbox->RemoveItemL(index);
						}
					data->Close();
					}
				UpdateListboxItemsL();
				}
		}break;
		default:
			CPodcastClientView::HandleCommandL(aCommand);
			break;
		}
	
	}

void CPodcastClientFeedView::HandleAddNewAudioBookL()
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
			CPodcastClientAudioBookDlg* titleDialog = new (ELeave) CPodcastClientAudioBookDlg(iPodcastModel, *fileNameArray);
			titleDialog->ExecuteLD(R_PODCAST_NEW_AUDIOBOOK_DLG);			
			}
		}

	CleanupStack::PopAndDestroy(dialogTitle);
	CleanupStack::PopAndDestroy(fileNameArray);
	CleanupStack::PopAndDestroy(mimeArray);
	}
