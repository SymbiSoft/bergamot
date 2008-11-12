/**
 * This file is a part of Escarpod Podcast project
 * (c) 2008 The Bergamot Project
 * (c) 2008 Teknolog (Sebastian Brännström)
 * (c) 2008 Anotherguest (Lars Persson)
 */

#include "PodcastFeedView.h"
#include "PodcastAppUi.h"
#include "FeedEngine.h"
#include "ShowEngine.h"
#include "SettingsEngine.h"
#include "SoundEngine.h"
#include "PodcastPlayView.h"

#include <aknnavide.h> 
#include <podcast.rsg>
#include <podcast.mbg>
#include <gulicon.h>
#include <eikenv.h>
#include <e32const.h>

const TInt KMaxFeedNameLength = 100;
const TInt KMaxUnplayedFeedsLength =64;
const TInt KADayInHours = 24;
const TInt KDefaultGran = 5;
_LIT(KUnknownUpdateDateString, "?/?");

class CPodcastFeedContainer : public CCoeControl
    {
    public: 
		CPodcastFeedContainer();
		~CPodcastFeedContainer();
		void ConstructL( const TRect& aRect );
	protected:
		CAknNavigationDecorator* iNaviDecorator;
        CAknNavigationControlContainer* iNaviPane;
	};

CPodcastFeedContainer::CPodcastFeedContainer()
{
}

void CPodcastFeedContainer::ConstructL( const TRect& aRect )
{
	CreateWindowL();

	 // Set the windows size
    SetRect( aRect );    
    
    // Activate the window, which makes it ready to be drawn
    ActivateL();   
}

CPodcastFeedContainer::~CPodcastFeedContainer()
{
	delete iNaviDecorator;
}


CPodcastFeedView* CPodcastFeedView::NewL(CPodcastModel& aPodcastModel)
    {
    CPodcastFeedView* self = CPodcastFeedView::NewLC(aPodcastModel);
    CleanupStack::Pop( self );
    return self;
    }

CPodcastFeedView* CPodcastFeedView::NewLC(CPodcastModel& aPodcastModel)
    {
    CPodcastFeedView* self = new ( ELeave ) CPodcastFeedView(aPodcastModel);
    CleanupStack::PushL( self );
    self->ConstructL();
    return self;
    }

CPodcastFeedView::CPodcastFeedView(CPodcastModel& aPodcastModel):iPodcastModel(aPodcastModel)
{
}

void CPodcastFeedView::ConstructL()
{
	BaseConstructL(R_PODCAST_FEEDVIEW);	
	CPodcastListView::ConstructL();
	iPodcastModel.FeedEngine().AddObserver(this);
	CArrayPtr< CGulIcon >* icons = new(ELeave) CArrayPtrFlat< CGulIcon >(1);
	CleanupStack::PushL( icons );

	// Load the bitmap for feed icon	
	CFbsBitmap* bitmap = iEikonEnv->CreateBitmapL( _L("*"),EMbmPodcastFeed_40x40);
	CleanupStack::PushL( bitmap );		
	// Load the mask for feed icon	
	CFbsBitmap* mask = iEikonEnv->CreateBitmapL( _L("*"),EMbmPodcastFeed_40x40m );	
	CleanupStack::PushL( mask );
	// Append the feed icon to icon array
	icons->AppendL( CGulIcon::NewL( bitmap, mask ) );
	CleanupStack::Pop(2); // bitmap, mask

	bitmap = iEikonEnv->CreateBitmapL( _L("*"),EMbmPodcastAudiobookindividual_40x40);
	CleanupStack::PushL( bitmap );		
	// Load the mask for feed icon	
	mask = iEikonEnv->CreateBitmapL( _L("*"),EMbmPodcastAudiobookindividual_40x40m );	
	CleanupStack::PushL( mask );
	// Append the feed icon to icon array
	icons->AppendL( CGulIcon::NewL( bitmap, mask ) );
	CleanupStack::Pop(2); // bitmap, mask
	
	bitmap = iEikonEnv->CreateBitmapL( _L("*"),EMbmPodcastEmptyimage);
	CleanupStack::PushL( bitmap );		
	// Load the mask for feed icon	
	mask = iEikonEnv->CreateBitmapL( _L("*"),EMbmPodcastEmptyimage );	
	CleanupStack::PushL( mask );
	// Append the feed icon to icon array
	icons->AppendL( CGulIcon::NewL( bitmap, mask ) );
	CleanupStack::Pop(2); // bitmap, mask

	iListContainer->Listbox()->ItemDrawer()->FormattedCellData()->SetIconArrayL( icons );
	CleanupStack::Pop(icons); // icons

	iListContainer->Listbox()->SetListBoxObserver(this);
}
    
CPodcastFeedView::~CPodcastFeedView()
    {
	iPodcastModel.FeedEngine().RemoveObserver(this);
	delete iBooksFormat;
	delete iFeedsFormat;
	delete iNeverUpdated;
    }

TUid CPodcastFeedView::Id() const
{
	return KUidPodcastFeedViewID;
}
		
void CPodcastFeedView::DoActivateL(const TVwsViewId& aPrevViewId,
	                                  TUid aCustomMessageId,
	                                  const TDesC8& aCustomMessage)
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

	CPodcastListView::DoActivateL(aPrevViewId, aCustomMessageId, aCustomMessage);
	
}

void CPodcastFeedView::DoDeactivate()
{
	CPodcastListView::DoDeactivate();
}


void CPodcastFeedView::HandleListBoxEventL(CEikListBox* aListBox, TListBoxEvent aEventType)
{
	switch(aEventType)
	{
	case EEventEnterKeyPressed:
	case EEventItemClicked:
	case EEventItemActioned:
		{
			const RFeedInfoArray* sortedItems = NULL;
			TInt index = iListContainer->Listbox()->CurrentItemIndex();
			if(iCurrentViewMode == EFeedsAudioBooksMode)
				{
				sortedItems = &iPodcastModel.FeedEngine().GetSortedBooks();
				}
			else
				{
				sortedItems = &iPodcastModel.FeedEngine().GetSortedFeeds();
				}

			if(index >= 0 && index < sortedItems->Count())
				{
				iPodcastModel.ActiveShowList().Reset();
				iPodcastModel.SetActiveFeedInfo((*sortedItems)[index]);			
				AppUi()->ActivateLocalViewL(KUidPodcastShowsViewID,  TUid::Uid(EShowFeedShows), KNullDesC8());
				}
		}
		break;
	default:
		break;
	}
}

void CPodcastFeedView::FeedInfoUpdated(CFeedInfo* aFeedInfo)
	{
	//TRAP_IGNORE(FeedInfoUpdatedL(aFeedInfo))
	}

void CPodcastFeedView::FeedUpdateCompleteL(TUint aFeedUid)
	{
//	UpdateFeedInfoStatusL(aFeedUid, EFalse);
	}

void CPodcastFeedView::FeedUpdateAllCompleteL()
{
	iPodcastModel.FeedEngine().SetCatchupMode(EFalse);
	iUpdatingAllRunning = EFalse;
}

void CPodcastFeedView::FeedDownloadUpdatedL(TUint aFeedUid, TInt aPercentOfCurrentDownload)
	{

	if(!iUpdatingAllRunning)
	{
		iUpdatingAllRunning = ETrue;		
	}

	// UPdate status text
	//UpdateFeedInfoStatusL(aFeedUid, ETrue);

	if(aPercentOfCurrentDownload>=0 && aPercentOfCurrentDownload<100)
		{
		if(!iProgressAdded)
			{
//			ViewContext()->AddProgressInfoL(EEikProgressTextPercentage, 100);
			iProgressAdded = ETrue;
			}
	//	ViewContext()->SetAndDrawProgressInfo(aPercentOfCurrentDownload);
		}
		
	else if(iProgressAdded)
		{
	//	ViewContext()->RemoveAndDestroyProgressInfo();
	//	ViewContext()->DrawNow();
		iProgressAdded = EFalse;
		}
	}

void CPodcastFeedView::UpdateListboxItemsL()
	{
	if(iListContainer->IsVisible())
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
//		MQikListBoxModel& model(iListbox->Model());

		if(len == iListContainer->Listbox()->Model()->NumberOfItems())
			{
			TUint itemId = 0;
			for(TInt loop = 0;loop< len ;loop++)
				{
				//MQikListBoxData* data = model.RetrieveDataL(loop);	
				//itemId = data->ItemId();
				//data->Close();
				if((*sortedItems)[loop]->Uid() != itemId)
					{						
					break;
					}
				allUidsMatch = ETrue;
				}
			}

		if(allUidsMatch)
			{
		//	model.ModelBeginUpdateLC();

			for(TInt loop = 0;loop< len ;loop++)
				{
			//	MQikListBoxData* data = model.RetrieveDataL(loop);	
			//	CleanupClosePushL(*data);
		//	//	UpdateFeedInfoDataL((*sortedItems)[loop], data);
		//		CleanupStack::PopAndDestroy(data);
		//		model.DataUpdatedL(loop);
				}

			// Informs that the update of the list box model has ended
		//	model.ModelEndUpdateL();
			}
		else
			{
			iListContainer->Listbox()->Reset();
			
	//		model.ModelBeginUpdateLC();
									
		//	MQikListBoxData* listBoxData;
			TBuf<KMaxShortDateFormatSpec*2> updatedDate;
			TBuf<KMaxUnplayedFeedsLength> unplayedShows;
			
			if (len > 0) 
				{
				for (int i=0;i<len;i++) 
					{
				//	listBoxData = model.NewDataL(MQikListBoxModel::EDataNormal);
				//	CleanupClosePushL(*listBoxData);
					
					CFeedInfo *fi = (*sortedItems)[i];
				//	listBoxData->SetItemId(fi->Uid());
				//	listBoxData->AddTextL(fi->Title(), EQikListBoxSlotText1);					

					TUint unplayedCount = 0;
					TUint showCount = 0;
					iPodcastModel.ShowEngine().GetStatsByFeed(fi->Uid(), showCount, unplayedCount, fi->IsBookFeed());
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
						
				//	listBoxData->SetEmphasis(unplayedCount > 0);					
				//	listBoxData->AddTextL(unplayedShows, EQikListBoxSlotText2);
				//	listBoxData->AddTextL(updatedDate, EQikListBoxSlotText3);
					
					
				/*	CQikContent* content;
					
					if (fi->IsBookFeed()) {
						content = CQikContent::NewL(this, _L("*"), EMbmPodcastclientAudiobookindividual_40x40, EMbmPodcastclientAudiobookindividual_40x40m);
					} else {
						content = CQikContent::NewL(this, _L("*"), EMbmPodcastclientFeed_40x40, EMbmPodcastclientFeed_40x40m);
					}
					
					CleanupStack::PushL(content);
					listBoxData->AddIconL(content,EQikListBoxSlotLeftMediumIcon1);
					CleanupStack::Pop(content);
					
					CleanupStack::PopAndDestroy(listBoxData);*/
					}
				} 
			else 
				{
//				listBoxData = model.NewDataL(MQikListBoxModel::EDataNormal);
//				CleanupClosePushL(*listBoxData);
					
				TBuf<KMaxFeedNameLength> itemName;
				if(iCurrentViewMode == EFeedsAudioBooksMode)
					{
					iEikonEnv->ReadResourceL(itemName, R_PODCAST_BOOKS_NO_BOOKS);
					}
				else
					{
					iEikonEnv->ReadResourceL(itemName, R_PODCAST_FEEDS_NO_FEEDS);
					}
				iItemArray->AppendL(itemName);
				iListContainer->Listbox()->HandleItemAdditionL();			
			//	listBoxData->SetDimmed(ETrue);
					
				}

			// Informs that the update of the list box model has ended
			//model.ModelEndUpdateL();
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
		if(iNaviPane != NULL)
				{
				iNaviPane->Pop(iNaviDecorator);
				delete iNaviDecorator;
				iNaviDecorator = NULL;
				iNaviDecorator  = iNaviPane->CreateNavigationLabelL(*titleBuffer);
				}		
		CleanupStack::PopAndDestroy(titleBuffer);
		CleanupStack::PopAndDestroy(templateStr);

		}
	}

/** 
 * Command handling function intended for overriding by sub classes. 
 * Default implementation is empty.  
 * @param aCommand ID of the command to respond to. 
 */
void CPodcastFeedView::HandleCommandL(TInt aCommand)
	{
	switch(aCommand)
		{
		case EPodcastAddFeed:
			{
		/*	CPodcastClientAddFeedDlg* dlg = new (ELeave) CPodcastClientAddFeedDlg(iPodcastModel);
			if(dlg->ExecuteLD(R_PODCAST_ADD_FEED_DLG))
				{
				UpdateListboxItemsL();
				}*/
			break;
			}
		case EPodcastImportFeeds:
			{		
			CDesCArrayFlat* mimeTypes = new (ELeave) CDesCArrayFlat(KDefaultGran);
			CleanupStack::PushL(mimeTypes);
			CDesCArrayFlat* fileNames = new (ELeave) CDesCArrayFlat(KDefaultGran);
			CleanupStack::PushL(fileNames);
			HBufC* import_title = iEikonEnv->AllocReadResourceLC(R_PODCAST_IMPORT_FEEDS_TITLE);
			//if(CQikSelectFileDlg::RunDlgLD(*mimeTypes, *fileNames, import_title))
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
			//CQikSaveFileDlg::RunDlgLD(fileName, newName, NULL,ESaveFileUsingMove);
			}break;
		case EPodcastEditFeed:
			{
			if(iListContainer->Listbox() != NULL)
				{
				TInt index = iListContainer->Listbox()->CurrentItemIndex();

				const RFeedInfoArray& feeds = iPodcastModel.FeedEngine().GetSortedFeeds();

				TInt cnt = feeds.Count();

				if(index< cnt)
					{
					/*CPodcastClientAddFeedDlg* dlg = new (ELeave) CPodcastClientAddFeedDlg(iPodcastModel, feeds[index]);
					if(dlg->ExecuteLD(R_PODCAST_EDIT_FEED_DLG))
						{
						UpdateListboxItemsL();
						}*/
					}			
				}
			break;
			}

		case EPodcastDeleteFeedHardware:
		case EPodcastDeleteFeed:
			{
			if(iListContainer->Listbox() != NULL)
				{
				TInt index = iListContainer->Listbox()->CurrentItemIndex();
			/*	MQikListBoxModel& model(iListbox->Model());
				MQikListBoxData* data = model.RetrieveDataL(index);	
				if(data != NULL)*/
					{
					if(iEikonEnv->QueryWinL(R_PODCAST_REMOVE_FEED_TITLE, R_PODCAST_REMOVE_FEED_PROMPT))
						{
					//	iPodcastModel.FeedEngine().RemoveFeed(data->ItemId());
						//iListContainer->Listbox()-(index); remove item TODO
						}					
					}
				UpdateListboxItemsL();
				}
			break;	
			}
		case EPodcastUpdateAllFeeds:
			{
			const RFeedInfoArray& array = iPodcastModel.FeedEngine().GetSortedFeeds();
			TBool hasNewFeed = EFalse;
			for (int i=0;i<array.Count();i++) {
			if (array[i]->LastUpdated().Int64() == 0) {
			hasNewFeed = ETrue;
			break;
			}
			}

			if (hasNewFeed) {
			TBuf<200> message;
			TBuf<100> title;
			iEikonEnv->ReadResourceL(message, R_CATCHUP_FEED);
			iEikonEnv->ReadResourceL(title, R_CATCHUP_FEED_TITLE);
			if (iEikonEnv->QueryWinL(title, message)) {
			iPodcastModel.FeedEngine().SetCatchupMode(ETrue);
			}
			}

			iUpdatingAllRunning = ETrue;			
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
				//	ViewContext()->RemoveAndDestroyProgressInfo();
					//ViewContext()->DrawNow();
					iProgressAdded = EFalse;
					}				
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
			if(iListContainer->Listbox() != NULL)
				{
				TInt index = iListContainer->Listbox()->CurrentItemIndex();
			//	MQikListBoxModel& model(iListbox->Model());
//				MQikListBoxData* data = model.RetrieveDataL(index);	
	//			if(data != NULL)
					{
					CFeedInfo *feedInfo = NULL;//TODO iPodcastModel.FeedEngine().GetFeedInfoByUid(data->ItemId());

					if (feedInfo != NULL) {
					iPodcastModel.SetActiveFeedInfo(feedInfo);
					TBool aUnplayedOnlyState = iPodcastModel.SettingsEngine().SelectUnplayedOnly();
					// we only select unplayed chapters
					iPodcastModel.SettingsEngine().SetSelectUnplayedOnly(ETrue);
					iPodcastModel.ShowEngine().SelectShowsByFeed(feedInfo->Uid());
					iPodcastModel.SettingsEngine().SetSelectUnplayedOnly(aUnplayedOnlyState);

					RShowInfoArray& showArray = iPodcastModel.ShowEngine().GetSelectedShows();

					if(showArray.Count() == 0 || showArray[0] == NULL) {
					// can't play empty books...
					return;
					}

					// this should be the first unplayed chapter in this book
					CShowInfo *startShow = showArray[0];

					TPckgBuf<TInt> showUid;
					showUid() = startShow->Uid();
					AppUi()->ActivateLocalViewL(KUidPodcastPlayViewID, TUid::Uid(KActiveShowUIDCmd), showUid);											
					iPodcastModel.PlayPausePodcastL(startShow, ETrue);
					}

					}
				}			
			}
			break;
		case EPodcastPauseAudioBook:
			{
			iPodcastModel.SoundEngine().Pause();			
			}
			break;
		case EPodcastAddNewAudioBook:
			{
			//TODO: HandleAddNewAudioBookL();
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
		//	if(CQikSelectFileDlg::RunDlgLD(*mimeTypes, *fileNames, import_title))
				{
				if(fileNames->MdcaCount()>0)
					{
					//TODO
				//	CPodcastClientAudioBookDlg* titleDialog = new (ELeave) CPodcastClientAudioBookDlg(iPodcastModel, *fileNames, ETrue);
					//titleDialog->ExecuteLD(R_PODCAST_NEW_AUDIOBOOK_DLG);	
					}

				}
			UpdateListboxItemsL();
			CleanupStack::PopAndDestroy(3,mimeTypes); // title, fileNames, mimeTypes								
			}break;
		case EPodcastRemoveAudioBookHardware:
		case EPodcastRemoveAudioBook:
			{
			if(iListContainer->Listbox() != NULL)
				{
				TInt index = iListContainer->Listbox()->CurrentItemIndex();
				//MQikListBoxModel& model(iListbox->Model());
				//MQikListBoxData* data = model.RetrieveDataL(index);	
				//if(data != NULL)
					{
					if(iEikonEnv->QueryWinL(R_PODCAST_REMOVE_BOOK_TITLE, R_PODCAST_REMOVE_BOOK_PROMPT))
						{
						//iPodcastModel.FeedEngine().RemoveBookL(data->ItemId());
					//	iListContainer->Listbox()->RemoveItemL(index);
						}					
					}
				UpdateListboxItemsL();
				}
			}break;		
		default:
			CPodcastListView::HandleCommandL(aCommand);
			break;
		}
	}
	
