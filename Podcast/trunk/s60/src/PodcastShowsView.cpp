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
#include "Constants.h"
#include <aknnavide.h> 
#include <podcast.rsg>
#include <podcast.mbg>
const TInt KSizeBufLen = 64;
const TInt KDefaultGran = 5;
_LIT(KSizeDownloadingOf, "%S/%S");
_LIT(KChapterFormatting, "%03d");

class CPodcastShowsContainer : public CCoeControl
    {
    public: 
		CPodcastShowsContainer();
		~CPodcastShowsContainer();
		void ConstructL( const TRect& aRect );
	protected:
		CAknNavigationDecorator* iNaviDecorator;
        CAknNavigationControlContainer* iNaviPane;
	};

CPodcastShowsContainer::CPodcastShowsContainer()
{
}

void CPodcastShowsContainer::ConstructL( const TRect& aRect )
{
	CreateWindowL();

	 // Set the windows size
    SetRect( aRect );    
    
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
    CleanupStack::Pop( self );
    return self;
    }

CPodcastShowsView* CPodcastShowsView::NewLC(CPodcastModel& aPodcastModel)
    {
    CPodcastShowsView* self = new ( ELeave ) CPodcastShowsView(aPodcastModel);
    CleanupStack::PushL( self );
    self->ConstructL();
    return self;
    }

CPodcastShowsView::CPodcastShowsView(CPodcastModel& aPodcastModel):iPodcastModel(aPodcastModel)
{
}

void CPodcastShowsView::ConstructL()
{
	BaseConstructL(R_PODCAST_SHOWSVIEW);	
	CPodcastListView::ConstructL();
	iListContainer->Listbox()->SetListBoxObserver(this);
}
    
CPodcastShowsView::~CPodcastShowsView()
    {
    }

TUid CPodcastShowsView::Id() const
{
	return KUidPodcastShowsViewID;
}
		
void CPodcastShowsView::DoActivateL(const TVwsViewId& aPrevViewId,
	                                  TUid aCustomMessageId,
	                                  const TDesC8& aCustomMessage)
{
	CPodcastListView::DoActivateL(aPrevViewId, aCustomMessageId, aCustomMessage);
}

void CPodcastShowsView::DoDeactivate()
{
	CPodcastListView::DoDeactivate();
}

void CPodcastShowsView::HandleListBoxEventL(CEikListBox* aListBox, TListBoxEvent aEventType)
{
	switch(aEventType)
	{
	case EEventEnterKeyPressed:
	case EEventItemClicked:
	case EEventItemActioned:
		{
			RShowInfoArray &fItems = iPodcastModel.ActiveShowList();
			TInt index = iListContainer->Listbox()->CurrentItemIndex();
			if(index>=0 && index< fItems.Count())
			{
				DP2("Handle event for podcast %S, downloadState is %d", &(fItems[index]->Title()), fItems[index]->DownloadState());
				
				TPckgBuf<TInt> showUid;
				showUid() = fItems[index]->Uid();			
				AppUi()->ActivateLocalViewL(KUidPodcastPlayViewID, TUid::Uid(KActiveShowUIDCmd), showUid);		
			}
		}
		break;
	default:
		break;
	}
}

void CPodcastShowsView::UpdateListboxItemsL()
{
	if(iListContainer->IsVisible())
	{
		TBuf<KSizeBufLen> showSize;
		TBuf<KMaxShortDateFormatSpec*2> showDate;

		TInt len = 0;
	
		//SetAppTitleNameL(KNullDesC());

		switch(iCurrentCategory)
		{
		case EShowAllShows:
		
			iPodcastModel.ShowEngine().SelectAllShows();
			break;
		case EShowNewShows:
		
			iPodcastModel.ShowEngine().SelectNewShows();
			break;		
		case EShowDownloadedShows:
		
			iPodcastModel.ShowEngine().SelectShowsDownloaded();
			break;
		case EShowPendingShows:
	
			iPodcastModel.ShowEngine().SelectShowsDownloading();
			break;
		default:
			iPodcastModel.ShowEngine().SelectShowsByFeed(iPodcastModel.ActiveFeedInfo()->Uid());
			break;
		}
		
		iPodcastModel.SetActiveShowList(iPodcastModel.ShowEngine().GetSelectedShows());
		RShowInfoArray &fItems = iPodcastModel.ActiveShowList();
		len = fItems.Count();
		
		if (iListContainer->Listbox() != NULL)
		{			
			TBool allUidsMatch = EFalse;

			if(len == iListContainer->Listbox()->Model()->NumberOfItems())
			{		
				allUidsMatch = ETrue;
				TUint itemId = 0;
				for(TInt loop = 0;loop< len ;loop++)
				{
/*					MQikListBoxData* data = model.RetrieveDataL(loop);	
					itemId = data->ItemId();
					data->Close();
					if(fItems[loop]->Uid() != itemId)
					{			
						allUidsMatch = EFalse;
						break;
					}		*/			
				}
			}

			if(allUidsMatch && len > 0)
			{
				//model.ModelBeginUpdateLC();
				for(TInt loop = 0;loop< len ;loop++)
				{											
					//MQikListBoxData* data = model.RetrieveDataL(loop);	
					//CleanupClosePushL(*data);
					//UpdateShowItemDataL(fItems[loop], data);									
					//CleanupStack::PopAndDestroy();//close data
					//model.DataUpdatedL(loop);
				}

				//model.ModelEndUpdateL();

			}
			else
			{
				iListContainer->Listbox()->Reset();
				
				// Informs the list box model that there will be an update of the data.
				// Notify the list box model that changes will be made after this point.
				// Observe that a cleanupitem has been put on the cleanupstack and 
				// will be removed by ModelEndUpdateL. This means that you have to 
				// balance the cleanupstack.
				// When you act directly on the model you always need to encapsulate 
				// the calls between ModelBeginUpdateLC and ModelEndUpdateL.
			//	model.ModelBeginUpdateLC();
				
				if (len > 0) {
					for (TInt i=0;i<len;i++) {
					//	MQikListBoxData* listBoxData = model.NewDataL(MQikListBoxModel::EDataNormal);
					//	CleanupClosePushL(*listBoxData);
						CShowInfo *si = fItems[i];
						//listBoxData->SetItemId(si->Uid());
						//listBoxData->AddTextL(si->Title(), EQikListBoxSlotText1);						
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

						//listBoxData->AddTextL(showSize, EQikListBoxSlotText4);												
						//listBoxData->AddTextL(showDate, EQikListBoxSlotText3);
						
						//listBoxData->SetEmphasis(si->PlayState() == ENeverPlayed);										
						CleanupStack::PopAndDestroy();	
						
						TInt bitmap = EMbmPodcastShow_40x40;
						TInt mask = EMbmPodcastShow_40x40m;
						
						//GetShowIcons(si, bitmap, mask);
						
					/*	CQikContent* content = CQikContent::NewL(this, _L("*"), bitmap, mask);
						CleanupStack::PushL(content);
						listBoxData->AddIconL(content,EQikListBoxSlotLeftMediumIcon1);
						CleanupStack::Pop(content);*/
					}
				} else {		
//					MQikListBoxData* listBoxData = model.NewDataL(MQikListBoxModel::EDataNormal);
//					CleanupClosePushL(*listBoxData);
//					HBufC* noItems=  iEikonEnv->AllocReadResourceLC(R_PODCAST_SHOWS_NO_ITEMS);

					//listBoxData->AddTextL(*noItems, EQikListBoxSlotText1);
					//listBoxData->SetDimmed(ETrue);
	//				CleanupStack::PopAndDestroy(noItems);
					//CleanupStack::PopAndDestroy();
				}
				
				// Informs that the update of the list box model has ended
			//	model.ModelEndUpdateL();
			}
			
			
		}			
	}
}