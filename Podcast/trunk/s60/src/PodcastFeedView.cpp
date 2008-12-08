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
#include "PodcastApp.h"
#include <caknfileselectiondialog.h>
#include <aknnavide.h> 
#include <podcast.rsg>
#include <podcast.mbg>
#include <gulicon.h>
#include <eikenv.h>
#include <e32const.h>
#include <eikdialg.h>
#include <aknquerydialog.h>
const TInt KMaxFeedNameLength = 100;
const TInt KMaxUnplayedFeedsLength =64;
const TInt KADayInHours = 24;
const TInt KDefaultGran = 5;
const TInt KNumberOfFilesMaxLength = 4;
_LIT(KUnknownUpdateDateString, "?/?");
_LIT(KFeedFormat, "%d\t%S\t%S %S");
_LIT(KDefaultDataFolder, "C:\\DATA\\");
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
	iNeverUpdated = iEikonEnv->AllocReadResourceL(R_PODCAST_FEEDS_NEVER_UPDATED);
	iBooksFormat = iEikonEnv->AllocReadResourceL(R_PODCAST_BOOKS_STATUS_FORMAT);
	iFeedsFormat = iEikonEnv->AllocReadResourceL(R_PODCAST_FEEDS_STATUS_FORMAT);
	CPodcastListView::ConstructL();
	iPodcastModel.FeedEngine().AddObserver(this);
	CArrayPtr< CGulIcon >* icons = new(ELeave) CArrayPtrFlat< CGulIcon >(1);
	CleanupStack::PushL( icons );
	
	// Load the bitmap for empty icon	
	CFbsBitmap* bitmap = iEikonEnv->CreateBitmapL( _L("*"),EMbmPodcastEmptyimage);
	CleanupStack::PushL( bitmap );		
	// Load the mask for feed icon	
	CFbsBitmap* mask = iEikonEnv->CreateBitmapL( _L("*"),EMbmPodcastEmptyimage );	
	CleanupStack::PushL( mask );
	// Append the feed icon to icon array
	icons->AppendL( CGulIcon::NewL( bitmap, mask ) );
	CleanupStack::Pop(2); // bitmap, mask
	
	// Load the bitmap for feed icon	
	bitmap = iEikonEnv->CreateBitmapL( _L("*"),EMbmPodcastFeed_40x40);
	CleanupStack::PushL( bitmap );		
	// Load the mask for feed icon	
	mask = iEikonEnv->CreateBitmapL( _L("*"),EMbmPodcastFeed_40x40m );	
	CleanupStack::PushL( mask );
	// Append the feed icon to icon array
	icons->AppendL( CGulIcon::NewL( bitmap, mask ) );
	CleanupStack::Pop(2); // bitmap, mask

	bitmap = iEikonEnv->CreateBitmapL( _L("*"),EMbmPodcastAudiobookindividual_40x40);
	CleanupStack::PushL( bitmap );		
	// Load the mask for audiobook icon	
	mask = iEikonEnv->CreateBitmapL( _L("*"),EMbmPodcastAudiobookindividual_40x40m );	
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
	iPreviousView = TVwsViewId(KUidPodcast, KUidPodcastBaseViewID);		
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

void CPodcastFeedView::FeedInfoUpdatedL(CFeedInfo* aFeedInfo)
	{
	// Since a title might have been updated
	if (iProgressAdded)
		{
		//ViewContext()->RemoveAndDestroyProgressInfo();
		//ViewContext()->DrawNow();
		iProgressAdded = EFalse;
		}

	if (iCurrentViewMode == EFeedsNormalMode)
		{
		const RFeedInfoArray& feeds = iPodcastModel.FeedEngine().GetSortedFeeds();

		TInt index = feeds.Find(aFeedInfo);

		if (index != KErrNotFound && index<iItemArray->Count())
			{
			UpdateFeedInfoDataL(aFeedInfo, index);
			iListContainer->Listbox()->DrawItem(index);
			}
		}
	}

void CPodcastFeedView::FeedInfoUpdated(CFeedInfo* aFeedInfo)
	{
	TRAP_IGNORE(FeedInfoUpdatedL(aFeedInfo))
	}

void CPodcastFeedView::FeedUpdateCompleteL(TUint aFeedUid)
	{
	UpdateFeedInfoStatusL(aFeedUid, EFalse);
	}

void CPodcastFeedView::FeedUpdateAllCompleteL()
{
	iUpdatingAllRunning = EFalse;
}

void CPodcastFeedView::FeedDownloadUpdatedL(TUint aFeedUid, TInt aPercentOfCurrentDownload)
	{

	if(!iUpdatingAllRunning)
	{
		iUpdatingAllRunning = ETrue;		
	}

	// Update status text
	UpdateFeedInfoStatusL(aFeedUid, ETrue);

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


void CPodcastFeedView::UpdateFeedInfoStatusL(TUint aFeedUid, TBool aIsUpdating)
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
		
		if (index != KErrNotFound && index < iItemArray->MdcaCount())
			{
			UpdateFeedInfoDataL(feeds[index], index, aIsUpdating);
			iListContainer->Listbox()->DrawItem(index);
			}
		}
	}

void CPodcastFeedView::UpdateFeedInfoDataL(CFeedInfo* aFeedInfo, TInt aIndex, TBool aIsUpdating )
	{
	TBuf<KMaxShortDateFormatSpec*2> updatedDate;
	//aListboxData->SetTextL(aFeedInfo->Title(), EQikListBoxSlotText1);
	//aListboxData->SetTextL(aFeedInfo->Description(), EQikListBoxSlotText2);
	
	TUint unplayedCount = 0;
	TUint showCount = 0;
	TBuf<KMaxUnplayedFeedsLength> unplayedShows;
	TListItemProperties itemProps;	
	TInt iconIndex = 0;
	if(aIsUpdating)
		{
			iEikonEnv->ReadResourceL(updatedDate, R_PODCAST_FEEDS_IS_UPDATING);
			unplayedShows = KNullDesC();			
		}
	else
	{
		iPodcastModel.FeedEngine().GetStatsByFeed(aFeedInfo->Uid(), showCount, unplayedCount, aFeedInfo->IsBookFeed());
		
		if (aFeedInfo->IsBookFeed()) {
		    iconIndex = 2;
			unplayedShows.Format(*iBooksFormat, unplayedCount, showCount);
		} else {
			iconIndex = 1;
			unplayedShows.Format(*iFeedsFormat, unplayedCount, showCount);
		}	
		
		//aListboxData->SetEmphasis(unplayedCount > 0);					
	
		itemProps.SetBold(unplayedCount > 0);
											
		if (aFeedInfo->LastUpdated().Int64() == 0) 
		{
			updatedDate.Copy(*iNeverUpdated);					
			unplayedShows.Copy(KUnknownUpdateDateString());
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
//	aListboxData->SetDisabled(aIsUpdating);
	itemProps.SetDimmed(aIsUpdating);
	
	
	iListboxFormatbuffer.Format(KFeedFormat(), iconIndex, &aFeedInfo->Title(), &unplayedShows, &updatedDate);
	iItemArray->Delete(aIndex);
	if(aIndex>= iItemArray->MdcaCount())
			{
			iItemArray->AppendL(iListboxFormatbuffer);
			}
		else
			{
			iItemArray->InsertL(aIndex, iListboxFormatbuffer);
			}
	iListContainer->Listbox()->ItemDrawer()->SetPropertiesL(aIndex, itemProps);
//	aListboxData->SetTextL(unplayedShows, EQikListBoxSlotText2);
//	aListboxData->SetTextL(updatedDate, EQikListBoxSlotText3);
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

		if(len == iListContainer->Listbox()->Model()->NumberOfItems())
			{
			TUint itemId = 0;
			for(TInt loop = 0;loop< len ;loop++)
				{				
				itemId = iItemIdArray[loop];
				if((*sortedItems)[loop]->Uid() != itemId)
					{						
					break;
					}
				allUidsMatch = ETrue;
				}
			}

		if(allUidsMatch)
			{
			for(TInt loop = 0;loop< len ;loop++)
				{	
				UpdateFeedInfoDataL((*sortedItems)[loop], loop);
				iListContainer->Listbox()->DrawItem(loop);		
				}
			}
		else
			{
			TListItemProperties itemProps;
			iListContainer->Listbox()->Reset();
			iListContainer->Listbox()->ItemDrawer()->ClearAllPropertiesL();
			iItemIdArray.Reset();
			iItemArray->Reset();
			TBuf<KMaxShortDateFormatSpec*2> updatedDate;
			TBuf<KMaxUnplayedFeedsLength> unplayedShows;
			
			if (len > 0) 
				{
				for (int i=0;i<len;i++) 
					{				
					
					CFeedInfo *fi = (*sortedItems)[i];
					iItemIdArray.Append(fi->Uid());
				//	listBoxData->SetItemId(fi->Uid());
				//	listBoxData->AddTextL(fi->Title(), EQikListBoxSlotText1);					
					TInt iconIndex = 0;
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
					
					if (fi->IsBookFeed())
						{
						iconIndex = 2;						
						}
					else
						{
						iconIndex = 1;						
						}
					iListboxFormatbuffer.Format(KFeedFormat(), iconIndex, &fi->Title(), &unplayedShows, &updatedDate);
					iItemArray->AppendL(iListboxFormatbuffer);
					itemProps.SetBold(unplayedCount > 0);
					iListContainer->Listbox()->ItemDrawer()->SetPropertiesL(i, itemProps);
				//	listBoxData->SetEmphasis(unplayedCount > 0);					
				//	listBoxData->AddTextL(unplayedShows, EQikListBoxSlotText2);
				//	listBoxData->AddTextL(updatedDate, EQikListBoxSlotText3);												
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
				iItemArray->Reset();
				iItemIdArray.Reset();
				itemName.Insert(0, _L("0\t"));
				//iItemArray->AppendL(itemName);
				//iItemIdArray.Append(0);
				TListItemProperties itemProps;
				itemProps.SetDimmed(ETrue);
				itemProps.SetHiddenSelection(ETrue);								
				iListContainer->Listbox()->ItemDrawer()->SetPropertiesL(0, itemProps);
					
			//	listBoxData->SetDimmed(ETrue);
					
				}
			iListContainer->Listbox()->HandleItemAdditionL();		
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
			_LIT(KURLPrefix, "http://");
			_LIT(KItpcPrefix, "itpc://");
			_LIT(KPcastPrefix, "pcast://");			
			TBuf<256> url;
			CAknTextQueryDialog * dlg =CAknTextQueryDialog::NewL(url) ;//CPodcastClientAddFeedDlg(iPodcastModel);
			dlg->PrepareLC(R_PODCAST_ADD_FEED_DLG);
			HBufC* prompt = iEikonEnv->AllocReadResourceLC(R_PODCAST_ADDFEED_PROMPT);
			dlg->SetPromptL(*prompt);
			CleanupStack::PopAndDestroy(prompt);
			if(dlg->RunLD())
				{
				// url is always present so access that								
				// Some pod links are written in format itpc://mylink.net/podcast.xml
				// Symbian HTTP stack does not like itpc:// 
				// Try to use a HTTP instead.
				TInt p = url.Find(KItpcPrefix);
				if (p >= 0)
					{
					url.Delete(p, KItpcPrefix().Length());
					}

				// Some pod links are written in format pcast://mylink.net/podcast.xml
				// Symbian HTTP stack does not like itpc:// 
				// Try to use a HTTP instead.
				p = url.Find(KPcastPrefix);
				if (p >= 0)
					{
					url.Delete(p, KPcastPrefix().Length());
					}

				// The URL must start with http://, otherwise the HTTP stack fails.
				TInt pos = url.Find(KURLPrefix);
				if (pos == KErrNotFound)
					{
					HBufC* newUrl = HBufC::NewL(url.Length() + KURLPrefix().Length());
					TPtr ptr = newUrl->Des();
					ptr.Append(KURLPrefix());
					ptr.Append(url);

					// replace the url buffer
					url.Copy(*newUrl);
					delete newUrl;					
					}

				// check which mode we are in.
				// we are creating a new feed
				CFeedInfo* newFeedInfo = CFeedInfo::NewL();
				CleanupStack::PushL(newFeedInfo);
				newFeedInfo->SetUrlL(url);
				newFeedInfo->SetTitleL(newFeedInfo->Url());
				CleanupStack::Pop(newFeedInfo);

				TBool added = iPodcastModel.FeedEngine().AddFeed(newFeedInfo); // takes ownership
				if (!added)
					{
					TBuf<200> message;
					TBuf<100> title;
					iEikonEnv->ReadResourceL(message, R_ADD_FEED_EXISTS);
					iEikonEnv->ReadResourceL(title, R_ADD_FEED_EXISTS_TITLE);
					iEikonEnv->InfoWinL(title, message);				
					}								
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
			CAknFileSelectionDialog* dlg = CAknFileSelectionDialog::NewL(ECFDDialogTypeNormal, R_PODCAST_IMPORT_PODCAST);
			TFileName importName;
			dlg->SetTitleL(*import_title);
									
			//if(CQikSelectFileDlg::RunDlgLD(*mimeTypes, *fileNames, import_title))i
			if(dlg->ExecuteL(importName))
				{
				if(importName.Length()>0)
					{
					iPodcastModel.FeedEngine().ImportFeedsL(importName);
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
			
				if(index < iItemArray->MdcaCount() && index >= 0)
					{
					if(iEikonEnv->QueryWinL(R_PODCAST_REMOVE_FEED_TITLE, R_PODCAST_REMOVE_FEED_PROMPT))
						{
						iPodcastModel.FeedEngine().RemoveFeed(iItemIdArray[index]);
						iItemArray->Delete(index);
						
						iListContainer->Listbox()->HandleItemRemovalL();
						}					
					}
				UpdateListboxItemsL();
				}
			break;	
			}
		case EPodcastUpdateAllFeeds:
			{
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

				if(index < iItemArray->MdcaCount() && index >= 0)
					{
					CFeedInfo *feedInfo = iPodcastModel.FeedEngine().GetFeedInfoByUid(iItemIdArray[index]);

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
			CAknFileSelectionDialog* dlg = CAknFileSelectionDialog::NewL(ECFDDialogTypeNormal, R_PODCAST_IMPORT_AUDIOBOOK);
			TFileName importName;
			
			dlg->SetTitleL(*import_title);
			
			if(dlg->ExecuteL(importName))
				{
				if( importName.Length()>0 )
					{
					TBuf<128> title;
					CAknTextQueryDialog * dlg =CAknTextQueryDialog::NewL(title) ;//CPodcastClientAddFeedDlg(iPodcastModel);

					HBufC* prompt= iEikonEnv->AllocReadResourceLC(R_PODCAST_ADDBOOK_PROMPT);
					
					if (dlg->ExecuteLD(R_PODCAST_NEW_AUDIOBOOK_DLG, *prompt))
						{
						// Add book // See CPodcastClientAudioBookDlg
						iPodcastModel.FeedEngine().ImportBookL(title, importName);
						UpdateListboxItemsL();
						}
					CleanupStack::PopAndDestroy(prompt);
					}
				}
												
			CleanupStack::PopAndDestroy(3,mimeTypes); // title, fileNames, mimeTypes								
			}break;
		case EPodcastRemoveAudioBookHardware:
		case EPodcastRemoveAudioBook:
			{
			if(iListContainer->Listbox() != NULL)
				{
				TInt index = iListContainer->Listbox()->CurrentItemIndex();			
				if(index < iItemArray->MdcaCount() && index >= 0)
					{
					if(iEikonEnv->QueryWinL(R_PODCAST_REMOVE_BOOK_TITLE, R_PODCAST_REMOVE_BOOK_PROMPT))
						{
						iPodcastModel.FeedEngine().RemoveBookL(iItemIdArray[index]);
						iItemArray->Delete(index);
						iListContainer->Listbox()->HandleItemRemovalL();
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
	

void CPodcastFeedView::DynInitMenuPaneL(TInt aResourceId,CEikMenuPane* aMenuPane)
{
	if(aResourceId == R_PODCAST_FEEDVIEW_MENU)
	{
		TBool playingPodcast = (iPodcastModel.PlayingPodcast() != NULL && (iPodcastModel.SoundEngine().State() == ESoundEnginePlaying || iPodcastModel.SoundEngine().State() == ESoundEnginePaused));
		if (iListContainer->Listbox() == NULL)
			return;
		TInt index = iListContainer->Listbox()->CurrentItemIndex();
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

		TUint cnt = sortedItems->Count();

		if (cnt == 0)
		{		
			aMenuPane->SetItemDimmed(EPodcastDeleteFeed, ETrue);
			aMenuPane->SetItemDimmed(EPodcastUpdateAllFeeds, ETrue);
			aMenuPane->SetItemDimmed(EPodcastEditFeed, ETrue);
			aMenuPane->SetItemDimmed(EPodcastRemoveAudioBook, ETrue);
		}
		else
		{			
			aMenuPane->SetItemDimmed(EPodcastDeleteFeed, (isBookMode||iUpdatingAllRunning));
			aMenuPane->SetItemDimmed(EPodcastUpdateAllFeeds, (isBookMode || iUpdatingAllRunning));
			aMenuPane->SetItemDimmed(EPodcastEditFeed, (isBookMode||iUpdatingAllRunning));	
			aMenuPane->SetItemDimmed(EPodcastRemoveAudioBook, !isBookMode);
		}
		
		aMenuPane->SetItemDimmed(EPodcastAddNewAudioBook, !isBookMode);
		aMenuPane->SetItemDimmed(EPodcastImportAudioBook, !isBookMode);
		aMenuPane->SetItemDimmed(EPodcastAddFeed, (isBookMode||iUpdatingAllRunning));
		aMenuPane->SetItemDimmed(EPodcastImportFeeds, (isBookMode||iUpdatingAllRunning));
		aMenuPane->SetItemDimmed(EPodcastExportFeeds, (isBookMode||iUpdatingAllRunning));
		
		TBool playingThisBook = (iPodcastModel.PlayingPodcast() != NULL) && (sortedItems != NULL && sortedItems->Count() > 0) && (iPodcastModel.PlayingPodcast()->FeedUid() == (*sortedItems)[index]->Uid()) && iPodcastModel.SoundEngine().State() == ESoundEnginePlaying;
		aMenuPane->SetItemDimmed(EPodcastPlayAudioBook, !(isBookMode && cnt && !playingThisBook));
		aMenuPane->SetItemDimmed(EPodcastPauseAudioBook, !(isBookMode && cnt && playingThisBook));
		
		aMenuPane->SetItemDimmed(EPodcastCancelUpdateAllFeeds, (isBookMode||!iUpdatingAllRunning));	
	}
}

void CPodcastFeedView::HandleAddNewAudioBookL()
	{
	CDesCArrayFlat* mimeArray = iEikonEnv->ReadDesCArrayResourceL(R_PODCAST_NEW_AUDIOBOOK_MIMEARRAY);
	CleanupStack::PushL(mimeArray);
	CDesCArrayFlat* fileNameArray = new (ELeave) CDesCArrayFlat(KDefaultGran);
	CleanupStack::PushL(fileNameArray);
	HBufC* dialogTitle = iEikonEnv->AllocReadResourceLC(R_PODCAST_NEW_AUDIOBOOK_SELECT_FILES);
	//TQikDefaultFolderDescription defaultFolder;
	//defaultFolder.SetDefaultFolder(EQikFileHandlingDefaultFolderAudio);
	fileNameArray->AppendL(_L("C:\\testfile.mp3"));
//	if(CQikSelectFileDlg::RunDlgLD(*mimeArray, *fileNameArray, defaultFolder, dialogTitle, EQikSelectFileDialogEnableMultipleSelect|EQikSelectFileDialogSortByName))
		{
		if(fileNameArray->Count() > 0)
			{
			TBuf<256> title;
			CAknTextQueryDialog * dlg =CAknTextQueryDialog::NewL(title) ;//CPodcastClientAddFeedDlg(iPodcastModel);
		
			HBufC* inputprompt= iEikonEnv->AllocReadResourceLC(R_PODCAST_ADDBOOK_PROMPT);			
			HBufC* promptformat = iEikonEnv->AllocReadResourceLC(R_PODCAST_ADDBOOK_PROMPTFORMAT);						
			HBufC* prompt = HBufC::NewLC(inputprompt->Length()+promptformat->Length()+ KNumberOfFilesMaxLength);
			prompt->Des().Format(*promptformat, fileNameArray->Count());
			prompt->Des().Append('\n');
			prompt->Des().Append(*inputprompt);
			if (dlg->ExecuteLD(R_PODCAST_NEW_AUDIOBOOK_DLG, *prompt))
				{
				// Add book // See CPodcastClientAudioBookDlg
				iPodcastModel.FeedEngine().AddBookL(title, fileNameArray);
				UpdateListboxItemsL();
				}		
			CleanupStack::PopAndDestroy(3, inputprompt);
			}
		}

	CleanupStack::PopAndDestroy(dialogTitle);
	CleanupStack::PopAndDestroy(fileNameArray);
	CleanupStack::PopAndDestroy(mimeArray);
	}

