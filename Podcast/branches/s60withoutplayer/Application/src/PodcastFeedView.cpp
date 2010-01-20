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

#include "PodcastFeedView.h"
#include "PodcastAppUi.h"
#include "FeedEngine.h"
#include "ShowEngine.h"
#include "SettingsEngine.h"
#include "PodcastApp.h"
#include "PodcastUtils.h"
#include <caknfileselectiondialog.h> 
#include <podcast.rsg>
#include <podcast.mbg>
#include <gulicon.h>
#include <eikenv.h>
#include <e32const.h>
#include <eikdialg.h>
#include <aknquerydialog.h>
#include <caknmemoryselectiondialog.h> 
#include <caknfilenamepromptdialog.h> 
#include <BAUTILS.H> 
#include <pathinfo.h> 
#include <f32file.h>

const TInt KMaxFeedNameLength = 100;
const TInt KMaxUnplayedFeedsLength =64;
const TInt KADayInHours = 24;
const TInt KDefaultGran = 5;
const TInt KNumberOfFilesMaxLength = 4;
#define KMaxMessageLength 200
#define KMaxTitleLength 100
const TInt KMimeBufLength = 100;

_LIT(KUnknownUpdateDateString, "?/?");
_LIT(KFeedFormat, "%d\t%S\t%S %S");


class CPodcastFeedContainer : public CCoeControl
    {
    public: 
		CPodcastFeedContainer();
		~CPodcastFeedContainer();
		void ConstructL( const TRect& aRect );
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

	bitmap = iEikonEnv->CreateBitmapL( _L("*"),EMbmPodcastFeed_new_40x40);
	CleanupStack::PushL( bitmap );		
	// Load the mask for audiobook icon	
	mask = iEikonEnv->CreateBitmapL( _L("*"),EMbmPodcastFeed_new_40x40m );	
	CleanupStack::PushL( mask );
	// Append the feed icon to icon array
	icons->AppendL( CGulIcon::NewL( bitmap, mask ) );
	CleanupStack::Pop(2); // bitmap, mask

	iListContainer->Listbox()->ItemDrawer()->FormattedCellData()->SetIconArrayL( icons );
	CleanupStack::Pop(icons); // icons

	iListContainer->Listbox()->SetListBoxObserver(this);
	iListContainer->SetKeyEventListener(this);
	
    CAknToolbar *toolbar = Toolbar();
	if (toolbar)
		{
		toolbar->SetToolbarObserver(this);
		}
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


void CPodcastFeedView::HandleListBoxEventL(CEikListBox* /* aListBox */, TListBoxEvent aEventType)
{
	switch(aEventType)
	{
	case EEventEnterKeyPressed:
	case EEventItemDoubleClicked:
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

void CPodcastFeedView::FeedInfoUpdatedL(TUint aFeedUid)
	{

	if (iCurrentViewMode == EFeedsNormalMode)
		{
		const RFeedInfoArray& feeds = iPodcastModel.FeedEngine().GetSortedFeeds();

		TInt index = KErrNotFound;
		for (int i = 0; i < feeds.Count(); i++) {
			if (feeds[i]->Uid() == aFeedUid) {
				index = i;
			}
		}

		if (index != KErrNotFound && index<iItemArray->Count())
			{
			UpdateFeedInfoDataL(feeds[index], index);
			if (iListContainer->Listbox()->TopItemIndex() <= index &&
				iListContainer->Listbox()->BottomItemIndex() >= index) {
					iListContainer->Listbox()->DrawItem(index);
			}
			}
		}
	}

void CPodcastFeedView::FeedInfoUpdated(TUint aFeedUid)
	{
	TRAP_IGNORE(FeedInfoUpdatedL(aFeedUid))
	}

void CPodcastFeedView::FeedUpdateCompleteL(TUint aFeedUid)
	{
	UpdateFeedInfoStatusL(aFeedUid, EFalse);
	}

void CPodcastFeedView::FeedUpdateAllCompleteL()
{
	iUpdatingAllRunning = EFalse;
	UpdateToolbar();
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
		
		if (index != KErrNotFound && index < iItemArray->MdcaCount())
			{
			UpdateFeedInfoDataL(feeds[index], index, aIsUpdating);
			if (iListContainer->Listbox()->TopItemIndex() <= index &&
				iListContainer->Listbox()->BottomItemIndex() >= index) {
					iListContainer->Listbox()->DrawItem(index);
			}
			}
		}
		UpdateToolbar();
	}

void CPodcastFeedView::UpdateFeedInfoDataL(CFeedInfo* aFeedInfo, TInt aIndex, TBool aIsUpdating )
	{
	TBuf<KMaxShortDateFormatSpec*2> updatedDate;
	
	TUint unplayedCount = 0;
	TUint showCount = 0;
	TBuf<KMaxUnplayedFeedsLength> unplayedShows;
	TListItemProperties itemProps;	
	TInt iconIndex = 0;
	if(aIsUpdating)
		{
			iconIndex = 1;
			iEikonEnv->ReadResourceL(updatedDate, R_PODCAST_FEEDS_IS_UPDATING);
			unplayedShows = KNullDesC();			
		}
	else
	{
		iPodcastModel.FeedEngine().GetStatsByFeed(aFeedInfo->Uid(), showCount, unplayedCount, aFeedInfo->IsBookFeed());
		
		if (unplayedCount > 0) {
		    iconIndex = 2;
		} else {
			iconIndex = 1;
		}			

		unplayedShows.Format(*iFeedsFormat, unplayedCount, showCount);
		
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
				aFeedInfo->LastUpdated().FormatL(updatedDate, KDateFormatShort());
			}
		}
	}
	itemProps.SetDimmed(aIsUpdating);
	
	iListboxFormatbuffer.Format(KFeedFormat(), iconIndex, &aFeedInfo->Title(), &updatedDate,  &unplayedShows);
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
							fi->LastUpdated().FormatL(updatedDate, KDateFormatShort());
							}
						}
					
					if (unplayedCount > 0)
						{
						iconIndex = 2;						
						}
					else
						{
						iconIndex = 1;						
						}
					iListboxFormatbuffer.Format(KFeedFormat(), iconIndex, &fi->Title(), &updatedDate, &unplayedShows);
					iItemArray->AppendL(iListboxFormatbuffer);
					
					iListContainer->Listbox()->ItemDrawer()->SetPropertiesL(i, itemProps);
					}
				} 
			else 
				{					
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

				TListItemProperties itemProps;
				itemProps.SetDimmed(ETrue);
				itemProps.SetHiddenSelection(ETrue);								
				iListContainer->Listbox()->ItemDrawer()->SetPropertiesL(0, itemProps);
				}
			iListContainer->Listbox()->HandleItemAdditionL();		
			}
		}
	
		//iListContainer->ScrollToVisible();
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
        case EPodcastHide:
			AppUi()->HandleCommandL(EEikCmdExit);
			break;
		case EPodcastAddFeed:
			{
			TBuf<KFeedUrlLength> url;
			CAknTextQueryDialog * dlg =CAknTextQueryDialog::NewL(url) ;//CPodcastClientAddFeedDlg(iPodcastModel);
			dlg->PrepareLC(R_PODCAST_ADD_FEED_DLG);
			HBufC* prompt = iEikonEnv->AllocReadResourceLC(R_PODCAST_ADDFEED_PROMPT);
			dlg->SetPromptL(*prompt);
			CleanupStack::PopAndDestroy(prompt);
			if(dlg->RunLD())
				{
				PodcastUtils::FixProtocolsL(url);

				CFeedInfo* newFeedInfo = CFeedInfo::NewL();
				CleanupStack::PushL(newFeedInfo);
				newFeedInfo->SetUrlL(url);
				newFeedInfo->SetTitleL(newFeedInfo->Url());
				CleanupStack::Pop(newFeedInfo);

				TBool added = iPodcastModel.FeedEngine().AddFeedL(newFeedInfo); // takes ownership
				if (!added)
					{
					TBuf<KMaxMessageLength> message;
					TBuf<KMaxTitleLength> title;
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
			CAknMemorySelectionDialog* memDlg = 
				CAknMemorySelectionDialog::NewL(ECFDDialogTypeNormal, ETrue);
			CleanupStack::PushL(memDlg);
			CAknMemorySelectionDialog::TMemory memory = 
				CAknMemorySelectionDialog::EPhoneMemory;

			if (memDlg->ExecuteL(memory))
				{
				TFileName importName;
			
				if (memory==CAknMemorySelectionDialog::EMemoryCard)
				{
					importName = PathInfo:: MemoryCardRootPath();
				}
				else
				{
					importName = PathInfo:: PhoneMemoryRootPath();
				}

				CAknFileSelectionDialog* dlg = CAknFileSelectionDialog::NewL(ECFDDialogTypeNormal, R_PODCAST_IMPORT_PODCAST);
				CleanupStack::PushL(dlg);

				dlg->SetDefaultFolderL(importName);
				
				if(dlg->ExecuteL(importName))
					{
					if(importName.Length()>0)
						{
						iPodcastModel.FeedEngine().ImportFeedsL(importName);
						UpdateListboxItemsL();
						}
					}
				CleanupStack::PopAndDestroy(dlg);
				}
			CleanupStack::PopAndDestroy(memDlg);								
			}break;
		case EPodcastExportFeeds:
			{
			CAknMemorySelectionDialog* memDlg = 
				CAknMemorySelectionDialog::NewL(ECFDDialogTypeSave, ETrue);
			CleanupStack::PushL(memDlg);
			CAknMemorySelectionDialog::TMemory memory = 
				CAknMemorySelectionDialog::EPhoneMemory;

			if (memDlg->ExecuteL(memory))
				{
				TFileName pathName;
				
				if (memory==CAknMemorySelectionDialog::EMemoryCard)
				{
					pathName = PathInfo::MemoryCardRootPath();
				}
				else
				{
					pathName = PathInfo::PhoneMemoryRootPath();
				}

				CAknFileSelectionDialog* dlg = CAknFileSelectionDialog::NewL(ECFDDialogTypeSave, R_PODCAST_EXPORT_FEEDS);
				CleanupStack::PushL(dlg);
										
				if(dlg->ExecuteL(pathName))
					{
					CAknFileNamePromptDialog *fileDlg = CAknFileNamePromptDialog::NewL(R_PODCAST_FILENAME_PROMPT_DIALOG);
					CleanupStack::PushL(fileDlg);
					fileDlg->SetPathL(pathName);
					TFileName fileName;
					if (fileDlg->ExecuteL(fileName) && fileName.Length() > 0)
						{
						pathName.Append(fileName);
						TFileName temp;
						iPodcastModel.FeedEngine().ExportFeedsL(temp);						
						BaflUtils::CopyFile(iEikonEnv->FsSession(), temp, pathName);
						BaflUtils::DeleteFile(iEikonEnv->FsSession(),temp);						
						}
					CleanupStack::PopAndDestroy(fileDlg);
					}
				CleanupStack::PopAndDestroy(dlg);
			}
			CleanupStack::PopAndDestroy(memDlg);									
			} break;
		case EPodcastEditFeed:
			{			
			TInt index = iListContainer->Listbox()->CurrentItemIndex();
		
			if(index < iItemArray->MdcaCount() && index >= 0)
				{
				CFeedInfo *info = iPodcastModel.FeedEngine().GetSortedFeeds()[index];

				TBuf<KFeedTitleLength> title;
				title.Copy(info->Title());
				TBuf<KFeedUrlLength> url;
				url.Copy(info->Url());
				CAknMultiLineDataQueryDialog  *dlg = CAknMultiLineDataQueryDialog ::NewL(title, url);
			
				if (dlg->ExecuteLD(R_PODCAST_EDIT_FEED_DLG)) 
					{
					
					if(info->Url().Compare(url) != 0)
						{
						TBuf<KMaxMessageLength> dlgMessage;
						TBuf<KMaxTitleLength> dlgTitle;
						iEikonEnv->ReadResourceL(dlgMessage, R_ADD_FEED_REPLACE);
						iEikonEnv->ReadResourceL(dlgTitle, R_ADD_FEED_REPLACE_TITLE);

						// Ask the user if it is OK to remove all shows
						if ( iEikonEnv->QueryWinL(dlgTitle, dlgMessage))
							{
							PodcastUtils::FixProtocolsL(url);
							
							//----- HACK ---- //
							CFeedInfo* temp = CFeedInfo::NewL();
							temp->SetUrlL(url);
							TBool added = iPodcastModel.FeedEngine().AddFeedL(temp);
							if (added) {
								// The Feed URL did not exist
								// Remove the temp entry so that the correct entry could be changed
								iPodcastModel.FeedEngine().RemoveFeedL(temp->Uid());	
								
								// user has accepted that shows will be deleted
								iPodcastModel.ShowEngine().DeleteAllShowsByFeedL(info->Uid());

								// update URL
								info->SetUrlL(url);	
							
								if (info->Title().Compare(title) != 0)
									{
									info->SetTitleL(title);
									info->SetCustomTitle();	
									}
								iPodcastModel.FeedEngine().UpdateFeed(info);
								UpdateListboxItemsL();
							} else {
								// the feed existed. Object deleted in AddFeed.	
								TBuf<KMaxMessageLength> dlgMessage;
								TBuf<KMaxTitleLength> dlgTitle;
								iEikonEnv->ReadResourceL(dlgMessage, R_ADD_FEED_EXISTS);
								iEikonEnv->ReadResourceL(dlgTitle, R_ADD_FEED_EXISTS_TITLE);
								iEikonEnv->InfoWinL(dlgTitle, dlgMessage);		
							}
						}
					} else { // no url change, maybe title?
						// Update the title
						if (info->Title().Compare(title) != 0)
						{
							info->SetTitleL(title);
							info->SetCustomTitle();	
							iPodcastModel.FeedEngine().UpdateFeed(info);
							UpdateListboxItemsL();
						}
					}
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
						CFeedInfo *info = iPodcastModel.FeedEngine().GetFeedInfoByUid(iItemIdArray[index]);
						
						DP1("Removing feed '%S'", &info->Title());
						iPodcastModel.FeedEngine().RemoveFeedL(iItemIdArray[index]);
						iItemArray->Delete(index);
						iItemIdArray.Remove(index);
						iListContainer->Listbox()->HandleItemRemovalL();
						iListContainer->Listbox()->DrawNow();
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
			}
			break;

		case EPodcastCancelUpdateAllFeeds:
			{
			if(iUpdatingAllRunning)
				{
				iUpdatingAllRunning = EFalse;

				iPodcastModel.FeedEngine().CancelUpdateAllFeedsL();
				}
			}break;
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
					/*
					AppUi()->ActivateLocalViewL(KUidPodcastPlayViewID, TUid::Uid(KActiveShowUIDCmd), showUid);
					*/
#pragma message("LAPER Handle MPX activate here. Or in themodel perhaps??")
					iPodcastModel.PlayPausePodcastL(startShow, ETrue);
					}

					}
				}			
			}
			break;
		case EPodcastPauseAudioBook:
			{
#pragma message("LAPER Handle MPX activate here. Or in themodel perhaps??")
			//iPodcastModel.SoundEngine().Pause();			
			}
			break;
		case EPodcastAddNewAudioBook:
			{
			HandleAddNewAudioBookL();
			UpdateListboxItemsL();
			}break;
		case EPodcastImportAudioBook:
			{
			CAknMemorySelectionDialog* memDlg = 
				CAknMemorySelectionDialog::NewL(ECFDDialogTypeNormal, ETrue);
			CleanupStack::PushL(memDlg);
			CAknMemorySelectionDialog::TMemory memory = 
				CAknMemorySelectionDialog::EPhoneMemory;

			if (memDlg->ExecuteL(memory))
				{
				TFileName importName;
			
				if (memory==CAknMemorySelectionDialog::EMemoryCard)
				{
					importName = PathInfo:: MemoryCardRootPath();
				}
				else
				{
					importName = PathInfo:: PhoneMemoryRootPath();
				}

				CAknFileSelectionDialog* dlg = CAknFileSelectionDialog::NewL(ECFDDialogTypeNormal, R_PODCAST_IMPORT_AUDIOBOOK);
				CleanupStack::PushL(dlg);

				dlg->SetDefaultFolderL(importName);
				
				if(dlg->ExecuteL(importName))
					{
					if(importName.Length()>0)
						{
						TBuf<KMaxTitleLength> title;
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
				CleanupStack::PopAndDestroy(dlg);
				}
			CleanupStack::PopAndDestroy(memDlg);								
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
						iItemIdArray.Remove(index);
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
		UpdateToolbar();
		((CPodcastAppUi*)AppUi())->UpdateAppStatus();
	}

void CPodcastFeedView::DynInitMenuPaneL(TInt /*aResourceId*/,CEikMenuPane* /*aMenuPane*/)
{
//	if(aResourceId == R_PODCAST_FEEDVIEW_MENU)
//	{
//		if (iListContainer->Listbox() == NULL)
//			return;
//		TInt index = iListContainer->Listbox()->CurrentItemIndex();
//		TBool isBookMode = (iCurrentViewMode == EFeedsAudioBooksMode);
//		const RFeedInfoArray* sortedItems = NULL;
//		if(isBookMode)
//		{
//			sortedItems = &iPodcastModel.FeedEngine().GetSortedBooks();
//		}
//		else
//		{
//			sortedItems = &iPodcastModel.FeedEngine().GetSortedFeeds();
//		}
//
//		TUint cnt = sortedItems->Count();
//
//		if (cnt == 0)
//		{		
//			aMenuPane->SetItemDimmed(EPodcastDeleteFeed, ETrue);
//			aMenuPane->SetItemDimmed(EPodcastUpdateAllFeeds, ETrue);
//			aMenuPane->SetItemDimmed(EPodcastEditFeed, ETrue);
//			aMenuPane->SetItemDimmed(EPodcastRemoveAudioBook, ETrue);
//		}
//		else
//		{			
//			aMenuPane->SetItemDimmed(EPodcastDeleteFeed, (isBookMode||iUpdatingAllRunning));
//			aMenuPane->SetItemDimmed(EPodcastUpdateAllFeeds, (isBookMode || iUpdatingAllRunning));
//			aMenuPane->SetItemDimmed(EPodcastEditFeed, (isBookMode||iUpdatingAllRunning));	
//			aMenuPane->SetItemDimmed(EPodcastRemoveAudioBook, !isBookMode);
//		}
//		
//		aMenuPane->SetItemDimmed(EPodcastAddNewAudioBook, !isBookMode);
//		aMenuPane->SetItemDimmed(EPodcastImportAudioBook, !isBookMode);
//		aMenuPane->SetItemDimmed(EPodcastAddFeed, (isBookMode||iUpdatingAllRunning));
//		aMenuPane->SetItemDimmed(EPodcastImportFeeds, (isBookMode||iUpdatingAllRunning));
//		aMenuPane->SetItemDimmed(EPodcastExportFeeds, (isBookMode||iUpdatingAllRunning));
//#pragma message("LAPER Handle MPX activate here. Or in themodel perhaps??")
//		TBool playingThisBook = EFalse;//(iPodcastModel.PlayingPodcast() != NULL) && (sortedItems != NULL && sortedItems->Count() > 0) && (iPodcastModel.PlayingPodcast()->FeedUid() == (*sortedItems)[index]->Uid()) && iPodcastModel.SoundEngine().State() == ESoundEnginePlaying;
//		aMenuPane->SetItemDimmed(EPodcastPlayAudioBook, !(isBookMode && cnt && !playingThisBook));
//		aMenuPane->SetItemDimmed(EPodcastPauseAudioBook, !(isBookMode && cnt && playingThisBook));
//		
//		aMenuPane->SetItemDimmed(EPodcastCancelUpdateAllFeeds, (isBookMode||!iUpdatingAllRunning));	
//	}
}

void CPodcastFeedView::HandleAddNewAudioBookL()
	{
	CAknMemorySelectionDialog* memDlg = 
		CAknMemorySelectionDialog::NewL(ECFDDialogTypeNormal, ETrue);
	CleanupStack::PushL(memDlg);
	CAknMemorySelectionDialog::TMemory memory = 
		CAknMemorySelectionDialog::EPhoneMemory;

	if (memDlg->ExecuteL(memory))
		{
		TFileName importName;
	
		if (memory==CAknMemorySelectionDialog::EMemoryCard)
		{
			importName = PathInfo:: MemoryCardRootPath();
		}
		else
		{
			importName = PathInfo:: PhoneMemoryRootPath();
		}

		CAknFileSelectionDialog* dlg = CAknFileSelectionDialog::NewL(ECFDDialogTypeSave, R_PODCAST_SHOWDIR_SELECTOR);
		HBufC* select = iEikonEnv->AllocReadResourceLC(R_PODCAST_SOFTKEY_SELECT);
		dlg->SetLeftSoftkeyFileL(*select);
		CleanupStack::PopAndDestroy(select);
		CleanupStack::PushL(dlg);

		dlg->SetDefaultFolderL(importName);

		CDesCArrayFlat* fileNameArray = new (ELeave) CDesCArrayFlat(KDefaultGran);

		if(dlg->ExecuteL(importName))
			{			
			CDirScan *dirScan = CDirScan::NewLC(iEikonEnv->FsSession());
			//DP1("Listing dir: %S", &folder);
			dirScan ->SetScanDataL(importName, KEntryAttDir, ESortByName);

			CDir *dirPtr;
			dirScan->NextL(dirPtr);
			for (TInt i = 0; i < dirPtr->Count(); i++)
				{
				const TEntry &entry = (TEntry) (*dirPtr)[i];
				if (!entry.IsDir()) 
					{
					TFileName pathName;
					pathName.Copy(importName);
					pathName.Append(entry.iName);

					TBuf<KMimeBufLength> mimeType;
					iPodcastModel.ShowEngine().GetMimeType(pathName, mimeType);
					DP2("'%S' has mime: '%S'", &pathName, &mimeType);
					if (mimeType.Left(5) == _L("audio"))
						{
						fileNameArray->AppendL(pathName);
						}
					}					
				}
			delete dirPtr;
			CleanupStack::PopAndDestroy(dirScan);
			
			TBuf<KMaxMessageLength> title;
			CAknTextQueryDialog * dlg =CAknTextQueryDialog::NewL(title);
		
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
		
			delete fileNameArray;
		CleanupStack::PopAndDestroy(dlg);
		}
	CleanupStack::PopAndDestroy(memDlg);		
	}

TKeyResponse CPodcastFeedView::OfferKeyEventL(const TKeyEvent& aKeyEvent, TEventCode aType)
	{
	if (aType == EEventKey)
		{
			switch (aKeyEvent.iCode) {
			case EKeyBackspace:
			case EKeyDelete:
				HandleCommandL(EPodcastDeleteFeedHardware);
				break;
			}
		}
	return EKeyWasNotConsumed;
	}

void CPodcastFeedView::OfferToolbarEventL(TInt aCommand)
	{
	HandleCommandL(aCommand);
	}

void CPodcastFeedView::DynInitToolbarL (TInt /*aResourceId*/, CAknToolbar * /*aToolbar*/)
	{

	}

void CPodcastFeedView::UpdateToolbar()
{
	CAknToolbar* toolbar = Toolbar();
	
	if (toolbar)
		{
		toolbar->SetItemDimmed(EPodcastUpdateAllFeeds, iUpdatingAllRunning, ETrue);
		}
}
