/**
 * This file is a part of Escarpod Podcast project
 * (c) 2008 The Bergamot Project
 * (c) 2008 Teknolog (Sebastian Brännström)
 * (c) 2008 Anotherguest (Lars Persson)
 */

#include "PodcastPlayView.h"
#include "PodcastAppUi.h"
#include "SoundEngine.h"
#include "ShowEngine.h"
#include "FeedEngine.h"
#include "Podcast.hrh"
#include <aknnavide.h> 
#include <podcast.rsg>
#include <podcast.mbg>
#include <coecobs.h>
#include <badesca.h>
#include <eikedwin.h>
#include <eikapp.h>
#include <eiklabel.h>
#include <eikprogi.h>
#include <BARSREAD.H>
#include <akntabgrp.h>
#include <gulalign.h>
#include <aknsbasicbackgroundcontrolcontext.h>

const TInt KAudioTickerPeriod = 1000000;
const TInt KMaxCoverImageWidth = 200;


class CPodcastPlayContainer : public CCoeControl, public MSoundEngineObserver,public MShowEngineObserver
    {
    public: 
		CPodcastPlayContainer(CPodcastModel& aPodcastModel, CAknNavigationControlContainer* aNaviPane);
		~CPodcastPlayContainer();
		void ConstructL( const TRect& aRect );
		CAknNavigationDecorator* NaviDeco()
		{
			return iNaviDecorator;
		}

		void ViewActivatedL(TInt aCurrentShowUid);
    	void Draw( const TRect& aRect ) const;
 	    void UpdateViewL();
		CShowInfo* ShowInfo()
		{
			return iShowInfo;
		}
	protected:
		void ShowListUpdated(){};  
		void ShowDownloadUpdatedL(TInt aPercentOfCurrentDownload, TInt aBytesOfCurrentDownload, TInt aBytesTotal);
		void DownloadQueueUpdated(TInt /*aDownloadingShows*/, TInt /*aQueuedShows*/) {}

		static TInt PlayingUpdateStaticCallbackL(TAny* aPlayView);
		void UpdatePlayStatusL();

		void PlaybackInitializedL();
		void PlaybackStartedL();
		void PlaybackStoppedL(); 
		void UpdateMaxProgressValueL(TInt aDuration);
		
		TKeyResponse OfferKeyEventL(const TKeyEvent& aKeyEvent,TEventCode aType);
		void HandleResourceChange(TInt aType);
		void SizeChanged();
		TInt CountComponentControls() const;
		CCoeControl* ComponentControl(TInt aIndex) const;

		
        /**
        * From CCoeControl, MopSupplyObject.
        */
        TTypeUid::Ptr MopSupplyObject( TTypeUid aId );
	protected:
		CAknNavigationDecorator* iNaviDecorator;
		CPeriodic* iPlaybackTicker;
		CEikImage* iCoverImageCtrl;
		CEikProgressInfo* iPlayProgressbar;
		CEikProgressInfo* iDownloadProgressInfo;
		CEikLabel* iShowInfoTitle;
		CEikLabel* iShowInfoLabel;
		CShowInfo* iShowInfo;
		CPodcastModel& iPodcastModel;
		TUint iMaxProgressValue;
		TUint iBytesDownloaded;
		CAknNavigationControlContainer* iNaviPane;
		CAknTabGroup* iTabGroup;
		TFileName iLastImageFileName;
        CAknsBasicBackgroundControlContext* iBgContext;
	};

// -----------------------------------------------------------------------------
// CSIPExLogContainer::MopSupplyObject
// From CCoeControl.
// -----------------------------------------------------------------------------
// 	
TTypeUid::Ptr CPodcastPlayContainer::MopSupplyObject( TTypeUid aId )
    {
    if ( aId.iUid == MAknsControlContext::ETypeId ) 
        { 
        return MAknsControlContext::SupplyMopObject( aId, iBgContext );
        }
    return CCoeControl::MopSupplyObject( aId );
    }

void CPodcastPlayContainer::Draw( const TRect& aRect ) const
{
	CWindowGc& gc = SystemGc();

    gc.SetBrushStyle( CGraphicsContext::ESolidBrush );
    
    MAknsSkinInstance* skin = AknsUtils::SkinInstance();
    MAknsControlContext* cc = AknsDrawUtils::ControlContext( this );
    
    AknsDrawUtils::Background( skin, cc, this, gc, aRect );
}

TInt CPodcastPlayContainer::CountComponentControls() const
    {
    return 4; // return number of controls inside this container
    }

CCoeControl* CPodcastPlayContainer::ComponentControl(TInt aIndex) const
    {
    switch ( aIndex )
        {
        case 0:
            return iDownloadProgressInfo;
		case 1:
			return iPlayProgressbar;

		case 2:
			if(iTabGroup && iTabGroup->ActiveTabIndex() == 0)
			{
				return iCoverImageCtrl;
			}
			else
			{
				return iShowInfoLabel;
			}
			break;
		case 3:
			return iShowInfoTitle;
        default:
            return NULL;
        }
    }

void CPodcastPlayContainer::HandleResourceChange(TInt aType)
{
		switch( aType )
    	{
	    case KEikDynamicLayoutVariantSwitch:
		    SetRect(iEikonEnv->EikAppUi()->ClientRect());
			 if ( iBgContext )
            {
            iBgContext->SetRect( iEikonEnv->EikAppUi()->ClientRect() );
            }
		    break;
	    }
		if(iTabGroup != NULL)
		{
			iTabGroup->HandleResourceChange(aType);
		}
		
        
}

void CPodcastPlayContainer::ViewActivatedL(TInt aCurrentShowUid)
		{
			iShowInfo = iPodcastModel.ShowEngine().GetShowByUidL(aCurrentShowUid);
			
			UpdateViewL();
			
			TInt sndState = iPodcastModel.SoundEngine().State();
			if (iShowInfo != NULL && sndState == ESoundEngineNotInitialized
				&& (iPodcastModel.PlayingPodcast() == NULL
				|| (iPodcastModel.PlayingPodcast() != NULL
				&& iPodcastModel.PlayingPodcast()->Uid() == iShowInfo->Uid())))
			{
				iPodcastModel.PlayPausePodcastL(iShowInfo);
			}
		}

void CPodcastPlayContainer::SizeChanged()
{
	TInt playprogressHeight = 0;
	TInt downloadprogressHeight = 0;
	TInt titleHeight = 0;
	if(iDownloadProgressInfo != NULL)
	{
		iDownloadProgressInfo->SetSize(TSize(Size().iWidth, iDownloadProgressInfo->MinimumSize().iHeight));
		if(iDownloadProgressInfo->IsVisible())
		{
		downloadprogressHeight = iDownloadProgressInfo->Size().iHeight;
		}

	}


	if(iPlayProgressbar != NULL)
	{
		iPlayProgressbar->SetSize(TSize(Size().iWidth, iDownloadProgressInfo->MinimumSize().iHeight));
		iPlayProgressbar->SetPosition(TPoint(0,Size().iHeight-iPlayProgressbar->Size().iHeight));
		playprogressHeight = iPlayProgressbar->Size().iHeight;
	}

	if(iShowInfoTitle != NULL)
	{
		iShowInfoTitle->SetSize(TSize(Size().iWidth, iShowInfoTitle->MinimumSize().iHeight));
		iShowInfoTitle->SetPosition(TPoint(0, downloadprogressHeight));
		titleHeight = iShowInfoTitle->Size().iHeight;
	}

	if(iCoverImageCtrl)
	{
		iCoverImageCtrl->SetSize(TSize(Size().iWidth, iCoverImageCtrl->MinimumSize().iHeight));
		iCoverImageCtrl->SetPosition(TPoint(0, titleHeight+downloadprogressHeight));
	}

	if(iShowInfoLabel != NULL)
	{
		iShowInfoLabel->SetSize(TSize(Size().iWidth, Size().iHeight - playprogressHeight-titleHeight));
		iShowInfoLabel->SetPosition(TPoint(0, titleHeight+downloadprogressHeight));
	}
	
	if ( iBgContext )
	{
		iBgContext->SetRect( iEikonEnv->EikAppUi()->ClientRect() );
	}
	
	if(iTabGroup != NULL)
	{
		iTabGroup->SetSize(iTabGroup->MinimumSize());
	}
}

CPodcastPlayContainer::CPodcastPlayContainer(CPodcastModel& aPodcastModel,CAknNavigationControlContainer* aNaviPane ):iPodcastModel(aPodcastModel), iNaviPane(aNaviPane)
{
}

TKeyResponse CPodcastPlayContainer::OfferKeyEventL(const TKeyEvent& aKeyEvent,TEventCode aType)
{
	TKeyResponse Ret = EKeyWasNotConsumed;	
	
	switch (aKeyEvent.iCode)
	{
	case EKeyRightArrow:
		if(iTabGroup)
		{
			TInt IndexNum = iTabGroup->ActiveTabIndex();
			IndexNum = IndexNum + 1;
			
			if(IndexNum > 1) 
				IndexNum = 1;
			
			iTabGroup->SetActiveTabByIndex(IndexNum);
						
			DrawNow();
		}
		break;
 
	case EKeyLeftArrow:
		if(iTabGroup)
		{    
			TInt IndexNum = iTabGroup->ActiveTabIndex();
			IndexNum = IndexNum - 1;
			
			if(IndexNum < 0)
				IndexNum = 0;
			
			iTabGroup->SetActiveTabByIndex(IndexNum);
			
			DrawNow();
		}
		break;
 
	default:
		if(iTabGroup)
		{
			switch(iTabGroup->ActiveTabIndex())
			{
			case 1:
/*				if(iSecondControl)
				{
					Ret = iSecondControl->OfferKeyEventL(aKeyEvent,aEventCode);
				}*/
				break;
			default:
				/*if(iFirstControl)
				{
					Ret = iFirstControl->OfferKeyEventL(aKeyEvent,aEventCode);
				}*/
				break;
			}	
		}
		break;
	}
	
	return Ret;
} 

void CPodcastPlayContainer::ConstructL( const TRect& aRect)
{
	CreateWindowL();

	 // Set the windows size
      
    MakeVisible(EFalse);
	iBgContext = 
	    CAknsBasicBackgroundControlContext::NewL( KAknsIIDQsnBgAreaMain, 
	                                              aRect, 
	                                              ETrue );
	iCoverImageCtrl = new (ELeave) CEikImage;
	iCoverImageCtrl->SetContainerWindowL(*this);
	
	iCoverImageCtrl->CreatePictureFromFileL(iEikonEnv->EikAppUi()->Application()->BitmapStoreName(), EMbmPodcastEmptyimage);
	iCoverImageCtrl->SetAlignment(TGulAlignment(CGraphicsContext::ECenter, EVCenter));
	iCoverImageCtrl->SetSize(TSize(100,100));
	iPlayProgressbar = new (ELeave) CEikProgressInfo;
	iPlayProgressbar->SetContainerWindowL(*this);
	iPlayProgressbar->ConstructL();

	iPlayProgressbar->SetSize(iPlayProgressbar->MinimumSize());
	iPlayProgressbar->SetFinalValue(100);


	iShowInfoTitle = new (ELeave) CEikLabel;
	iShowInfoTitle->SetContainerWindowL(*this);
	iShowInfoTitle->SetTextL(_L("Title"));
	iShowInfoTitle->SetSize(iShowInfoTitle->MinimumSize());

	iShowInfoLabel = new (ELeave) CEikLabel;
	iShowInfoLabel->SetContainerWindowL(*this);
	iShowInfoLabel->SetTextL(_L("HELLO THIS IS A TEST"));
	iShowInfoLabel->SetSize(iShowInfoLabel->MinimumSize());

	iShowInfoLabel->ActivateL();
	iDownloadProgressInfo = new (ELeave) CEikProgressInfo;
	iDownloadProgressInfo->SetContainerWindowL(*this);
	iDownloadProgressInfo->ConstructL();

	iDownloadProgressInfo->SetSize(iDownloadProgressInfo->MinimumSize());
	iDownloadProgressInfo->SetFinalValue(100);
	iPlaybackTicker = CPeriodic::NewL(CActive::EPriorityStandard);
	iPodcastModel.SoundEngine().AddObserver(this);
	iPodcastModel.ShowEngine().AddObserver(this);

	iNaviDecorator = iNaviPane->CreateTabGroupL();

	iTabGroup = STATIC_CAST(CAknTabGroup*, iNaviDecorator->DecoratedControl());
	iTabGroup->SetTabFixedWidthL(KTabWidthWithTwoTabs);

	iTabGroup->AddTabL(0,_L("1"));
	iTabGroup->AddTabL(1,_L("2"));
 
	iTabGroup->SetActiveTabByIndex(0);

	SetRect( aRect );  
    // Activate the window, which makes it ready to be drawn
    ActivateL();   
}

CPodcastPlayContainer::~CPodcastPlayContainer()
{
	delete iNaviDecorator;
	delete iBgContext;
}

TInt CPodcastPlayContainer::PlayingUpdateStaticCallbackL(TAny* aPlayView)
	{
	static_cast<CPodcastPlayContainer*>(aPlayView)->UpdatePlayStatusL();
	return KErrNone;
	}

void CPodcastPlayContainer::PlaybackInitializedL()
	{
	if (iShowInfo == NULL) {
		return;
	}
	
	if (iPodcastModel.PlayingPodcast() != NULL && iPodcastModel.PlayingPodcast()->Uid() == iShowInfo->Uid())
		{
		UpdateViewL();
		UpdateMaxProgressValueL(iPodcastModel.SoundEngine().PlayTime());
		}
	}

void CPodcastPlayContainer::PlaybackStartedL()
	{
	if (iShowInfo == NULL) {
		return;
	}
	
	iPlaybackTicker->Cancel();

	if (iPodcastModel.PlayingPodcast() != NULL
			&& ( (iPodcastModel.PlayingPodcast()->Uid() == iShowInfo->Uid() )
					|| (iPodcastModel.PlayingPodcast()->ShowType() == EAudioBook
							&& iPodcastModel.PlayingPodcast()->FeedUid() == iShowInfo->FeedUid()) ))
		{
		if (iPodcastModel.PlayingPodcast()->ShowType() == EAudioBook && iPodcastModel.PlayingPodcast()->FeedUid() == iShowInfo->FeedUid())
			{
			iShowInfo = iPodcastModel.PlayingPodcast();
			}

		iPlaybackTicker->Start(KAudioTickerPeriod, KAudioTickerPeriod,
				TCallBack(PlayingUpdateStaticCallbackL, this));
		UpdatePlayStatusL();
		}
	}

void CPodcastPlayContainer::PlaybackStoppedL()
	{
	
	if (iShowInfo == NULL) {
		return;
	}
	
	iPlaybackTicker->Cancel();

	if (iPodcastModel.PlayingPodcast() != NULL && iPodcastModel.PlayingPodcast()->Uid() == iShowInfo->Uid())
		{
		UpdatePlayStatusL();
		}
	}

void CPodcastPlayContainer::ShowDownloadUpdatedL(
		TInt aPercentOfCurrentDownload, TInt aBytesOfCurrentDownload,
		TInt aBytesTotal)

{
	if (iShowInfo != iPodcastModel.ShowEngine().ShowDownloading()) {
		iBytesDownloaded = 0;
		UpdateViewL();
		return;
	}
	
	if (iDownloadProgressInfo != NULL)
	{
		if (iDownloadProgressInfo->Info().iFinalValue != aBytesTotal)
		{
			iDownloadProgressInfo->SetFinalValue(aBytesTotal);
		}
		
		iDownloadProgressInfo->SetAndDraw(aBytesOfCurrentDownload);
		//iDownloadProgressInfo->DrawDeferred();
		iBytesDownloaded = aBytesOfCurrentDownload;
		UpdatePlayStatusL();
	}
}

void CPodcastPlayContainer::UpdateMaxProgressValueL(TInt aDuration)
{
	iMaxProgressValue = aDuration;
}
void CPodcastPlayContainer::UpdatePlayStatusL()
{
}


/**

  	comMan.SetInvisible(*this, EPodcastMarkAsPlayed, iShowInfo->PlayState()
			!= ENeverPlayed);
		comMan.SetInvisible(*this, EPodcastMarkAsUnplayed,
			iShowInfo->PlayState() == ENeverPlayed);
		comMan.SetInvisible(*this, EPodcastResumeDownloads, ETrue);

  	if (iShowInfo->DownloadState() == ENotDownloaded)
		{
			comMan.SetInvisible(*this, EPodcastDownloadShow, EFalse);
			comMan.SetInvisible(*this, EPodcastPlay, ETrue);
			comMan.SetInvisible(*this, EPodcastRemoveDownload, ETrue);
			comMan.SetInvisible(*this, EPodcastSetVolume, ETrue);
			iDownloadProgressInfo->MakeVisible(EFalse);
			iPlayProgressbar->MakeVisible(EFalse);
		}
*/
void CPodcastPlayContainer::UpdateViewL()
{
	if (iShowInfo != NULL)
	{
		iShowInfoLabel->SetTextL(iShowInfo->Description());
		
		if (iShowInfoTitle != NULL)
		{
			iShowInfoTitle->SetTextL(iShowInfo->Title());
		}
		
	
		
		if (iShowInfo->DownloadState() == ENotDownloaded)
		{
			/*comMan.SetInvisible(*this, EPodcastDownloadShow, EFalse);
			comMan.SetInvisible(*this, EPodcastPlay, ETrue);
			comMan.SetInvisible(*this, EPodcastRemoveDownload, ETrue);
			comMan.SetInvisible(*this, EPodcastSetVolume, ETrue);*/
			iDownloadProgressInfo->MakeVisible(EFalse);
			iPlayProgressbar->MakeVisible(EFalse);
		}
		else
			if (iShowInfo->DownloadState() != EDownloaded) // Qued or downloading.
			{
			/*	comMan.SetInvisible(*this, EPodcastPlay, ETrue);
				comMan.SetInvisible(*this, EPodcastDownloadShow, ETrue);
				comMan.SetInvisible(*this, EPodcastRemoveDownload, EFalse);*/
				if (iShowInfo->DownloadState() == EDownloading)
				{
					/*comMan.SetTextL(*this, EPodcastRemoveDownload,
						R_PODCAST_PLAYER_SUSPEND_DL_CMD);
					comMan.SetShortTextL(*this, EPodcastRemoveDownload,
						R_PODCAST_PLAYER_SUSPEND_DL_CMD);*/
				}
				else
				{
					/*comMan.SetTextL(*this, EPodcastRemoveDownload,
						R_PODCAST_PLAYER_REMOVE_DL_CMD);
					comMan.SetShortTextL(*this, EPodcastRemoveDownload,
						R_PODCAST_PLAYER_REMOVE_DL_CMD);
					if (iPodcastModel.ShowEngine().DownloadsStopped())
					{
						comMan.SetInvisible(*this, EPodcastResumeDownloads,
							EFalse);
					}*/
				}
				
			//	comMan.SetInvisible(*this, EPodcastSetVolume, ETrue);
				iDownloadProgressInfo->MakeVisible(iShowInfo->DownloadState()
					== EDownloading);
				iPlayProgressbar->MakeVisible(EFalse);
			}
			else // Downloaded
			{
			/*	comMan.SetInvisible(*this, EPodcastPlay, EFalse);
				comMan.SetInvisible(*this, EPodcastSetVolume, EFalse);
				comMan.SetInvisible(*this, EPodcastDownloadShow, ETrue);
				comMan.SetInvisible(*this, EPodcastRemoveDownload, ETrue);*/
				iDownloadProgressInfo->MakeVisible(EFalse);
				iPlayProgressbar->MakeVisible(ETrue);
			}
			
			CFeedInfo* feedInfo = iPodcastModel.FeedEngine().GetFeedInfoByUid(iShowInfo->FeedUid());
			if (feedInfo != NULL)
			{
				TParsePtrC parser(feedInfo->ImageFileName());
				
				if (parser.NameAndExt().Length() > 0)
				{
					if (feedInfo->ImageFileName() != iLastImageFileName)
					{
						iLastImageFileName = feedInfo->ImageFileName();
						iCoverImageCtrl->CreatePictureFromFileL(iEikonEnv->EikAppUi()->Application()->BitmapStoreName(), EMbmPodcastEmptyimage, EMbmPodcastEmptyimage);
						iCoverImageCtrl->DrawDeferred();
					/*	if (!iBitmapConverter->IsActive())
						{
							TRAPD(err, iBitmapConverter->LoadImageDataL(feedInfo->ImageFileName()))
								;
							if (err == KErrNone)
							{
								iBitmapConverter->ConvertToBitmapL(
									iCurrentCoverImage, 0);
								iCoverImageCtrl->MakeVisible(ETrue);
							}
							else
							{
								iLastImageFileName = KNullDesC();
							}
						}*/
					}
				}
				else // no cover art file
				{
					iLastImageFileName = KNullDesC();
					if (iShowInfo->ShowType() == EAudioBook)
					{
						iCoverImageCtrl->CreatePictureFromFileL(iEikonEnv->EikAppUi()->Application()->BitmapStoreName(), EMbmPodcastAudiobook_120, EMbmPodcastAudiobook_120m);
					}
					else
					{
						iCoverImageCtrl->CreatePictureFromFileL(iEikonEnv->EikAppUi()->Application()->BitmapStoreName(), EMbmPodcastShow_120, EMbmPodcastShow_120m);
					}
				}
				
			}
			else // no feed info => manually added file
			{
				iLastImageFileName = KNullDesC();
				if (iShowInfo->ShowType() == EAudioBook)
				{
					iCoverImageCtrl->CreatePictureFromFileL(iEikonEnv->EikAppUi()->Application()->BitmapStoreName(), EMbmPodcastAudiobook_120, EMbmPodcastAudiobook_120m);
				}
				else
				{
					iCoverImageCtrl->CreatePictureFromFileL(iEikonEnv->EikAppUi()->Application()->BitmapStoreName(), EMbmPodcastShow_120, EMbmPodcastShow_120m);
				}
				//comMan.SetInvisible(*this, EPodcastDownloadShow, ETrue);
			}
		}
		else // no show info, we should never get here
		{
			iLastImageFileName = KNullDesC();
			if (iCoverImageCtrl != NULL)
			{
				iCoverImageCtrl->CreatePictureFromFileL(iEikonEnv->EikAppUi()->Application()->BitmapStoreName(), EMbmPodcastEmptyimage, EMbmPodcastEmptyimage);
			}
//			comMan.SetInvisible(*this, EPodcastDownloadShow, ETrue);
		}
		
		UpdatePlayStatusL();
		
	/*	if (iProgressBB != NULL)
		{
			iProgressBB->RequestRelayout(iProgressBB);
		}*/
		
		
		if (iPodcastModel.PlayingPodcast() != NULL && iPodcastModel.PlayingPodcast()->Uid() == iShowInfo->Uid() && iPodcastModel.SoundEngine().State() == ESoundEnginePlaying)
		{
			iPlaybackTicker->Cancel();
			iPlaybackTicker->Start(KAudioTickerPeriod, KAudioTickerPeriod,
				TCallBack(PlayingUpdateStaticCallbackL, this));
		}
		iCoverImageCtrl->SetSize(TSize(Size().iWidth, iCoverImageCtrl->MinimumSize().iHeight));

		SetSize(Size());
}

CPodcastPlayView* CPodcastPlayView::NewL(CPodcastModel& aPodcastModel)
    {
    CPodcastPlayView* self = CPodcastPlayView::NewLC(aPodcastModel);
    CleanupStack::Pop( self );
    return self;
    }

CPodcastPlayView* CPodcastPlayView::NewLC(CPodcastModel& aPodcastModel)
    {
    CPodcastPlayView* self = new ( ELeave ) CPodcastPlayView(aPodcastModel);
    CleanupStack::PushL( self );
    self->ConstructL();
    return self;
    }

CPodcastPlayView::CPodcastPlayView(CPodcastModel& aPodcastModel):iPodcastModel(aPodcastModel)
{
}

void CPodcastPlayView::ConstructL()
{
	BaseConstructL(R_PODCAST_PLAYVIEW);	
	iNaviPane =( CAknNavigationControlContainer * ) StatusPane()->ControlL( TUid::Uid( EEikStatusPaneUidNavi ) );

	iPlayContainer = new (ELeave) CPodcastPlayContainer(iPodcastModel, iNaviPane);
	iPlayContainer->SetMopParent(this);
	iPlayContainer->ConstructL(ClientRect());

}
    
CPodcastPlayView::~CPodcastPlayView()
    {
    delete iPlayContainer;    
    }

TUid CPodcastPlayView::Id() const
{
	return KUidPodcastPlayViewID;
}
		
void CPodcastPlayView::DoActivateL(const TVwsViewId& aPrevViewId,
	                                  TUid aCustomMessageId,
	                                  const TDesC8& aCustomMessage)
{
	iPreviousView = aPrevViewId;
	switch (aCustomMessageId.iUid)
	{
	case KActiveShowUIDCmd:
		{
			TPckg<TInt> showUidPkg(iCurrentViewShowUid);
			showUidPkg.Copy(aCustomMessage);
		}
		break;
	default:
		{
			if (iPodcastModel.PlayingPodcast() != NULL)
			{
				iCurrentViewShowUid = iPodcastModel.PlayingPodcast()->Uid();
			}
			
		}
		break;
	}


	if(iPlayContainer)
	{
		iPlayContainer->SetRect(ClientRect());
		AppUi()->AddToViewStackL( *this, iPlayContainer );	
		iPlayContainer->MakeVisible(ETrue);

		if(iPlayContainer->NaviDeco() != NULL && iNaviPane)
		{
		iNaviPane->PushL(*iPlayContainer->NaviDeco());
		}

		iPlayContainer->ViewActivatedL(iCurrentViewShowUid);
	}

}

void CPodcastPlayView::DoDeactivate()
{
	if ( iPlayContainer )
	{
        AppUi()->RemoveFromViewStack( *this, iPlayContainer );
		iPlayContainer->MakeVisible(EFalse);

		if(iNaviPane && iPlayContainer->NaviDeco() != NULL)
		{
		iNaviPane->Pop(iPlayContainer->NaviDeco());
		}
	}
}

/** 
* Command handling function intended for overriding by sub classes. 
* Default implementation is empty.  
* @param aCommand ID of the command to respond to. 
*/
void CPodcastPlayView::HandleCommandL(TInt aCommand)
{
	RDebug::Printf("CPodcastListView::HandleCommandL=%d", aCommand);
	switch(aCommand)
	{
	case EPodcastMarkAsPlayed:
		{
			iPlayContainer->ShowInfo()->SetPlayState(EPlayed);
			iPlayContainer->UpdateViewL();
		}
		break;
	case EPodcastMarkAsUnplayed:
		{
			iPlayContainer->ShowInfo()->SetPlayState(ENeverPlayed);
			iPlayContainer->UpdateViewL();
		}
		break;
		
	case EPodcastSetVolume:
		{
		//	CPodcastClientVolumeDlg* dlg = new (ELeave) CPodcastClientVolumeDlg(iPodcastModel);
		//	dlg->ExecuteLD(R_PODCAST_VOLUME_DLG);
		}
		break;
		
	case EPodcastZoomSetting:
		{
			// Launch the zoom dialog
		/*	const TInt zoomFactor =
				CQikZoomDialog::RunDlgLD(iPodcastModel.ZoomState());
			
			// If zoom state have changed it will be stored to persistent
			// storage and a relayout will be performed
			if (iPodcastModel.SetZoomState(zoomFactor))
			{
				// Sets the zoom factor for the view
				iLastZoomLevel = zoomFactor;
				SetZoomFactorL(CQikAppUi::ZoomFactorL(zoomFactor, *iEikonEnv));
			}*/
		}
		break;
		
	case EPodcastPlay:
		{
			if (iPodcastModel.PlayingPodcast() != NULL
				&& iPodcastModel.PlayingPodcast()->Uid() == iPlayContainer->ShowInfo()->Uid())
			{
				if (iPodcastModel.PlayingPodcast()->ShowType() == EVideoPodcast
					|| iPodcastModel.SoundEngine().State() == ESoundEngineNotInitialized)
				{
				//	QikFileUtils::StartAppL(iPodcastModel.PlayingPodcast()->FileName());//, lsSession);
				}
				else
				{
					if (iPodcastModel.SoundEngine().State() == ESoundEnginePlaying)
					{
						iPodcastModel.SoundEngine().Pause();
				//		iPlayProgressbar->SetFocusing(EFalse);
/*						comMan.SetTextL(*this, EPodcastPlay,
							R_PODCAST_PLAYER_PLAY_CMD);*/
//						RequestFocusL(iScrollableContainer);
					}
					else
					{
						iPodcastModel.SoundEngine().Play();
						//iPlayProgressbar->SetFocusing(EFalse);
						/*comMan.SetTextL(*this, EPodcastPlay,
							R_PODCAST_PLAYER_PAUSE_CMD);*/
					}
				}
			}
			else
			{
				iPodcastModel.PlayPausePodcastL(iPlayContainer->ShowInfo(), ETrue);
			}
			iPlayContainer->UpdateViewL();
		}
		break;
	case EPodcastStop:
		{
//			comMan.SetTextL(*this, EPodcastPlay, R_PODCAST_PLAYER_PLAY_CMD);
		//	iPlayProgressbar->SetFocusing(EFalse);
			iPodcastModel.SoundEngine().Stop();
			iPodcastModel.PlayingPodcast()->SetPosition(0);
			iPodcastModel.SetPlayingPodcast(NULL);
		}
		break;
	case EPodcastResumeDownloads:
		{
			iPodcastModel.ShowEngine().ResumeDownloads();
			iPlayContainer->UpdateViewL();
		}
		break;
	case EPodcastRemoveDownload:
		{
			iPodcastModel.ShowEngine().RemoveDownload(iPlayContainer->ShowInfo()->Uid());
			iPlayContainer->UpdateViewL();
		}
		break;
	case EPodcastDownloadShow:
		{
			iPodcastModel.ShowEngine().AddDownload(iPlayContainer->ShowInfo());
			iPlayContainer->UpdateViewL();
		}
		break;
		

	case EAknSoftkeyExit:
	case EEikCmdExit:
		{
            AppUi()->Exit();
            break;
		}
	case EAknSoftkeyBack:
		{
		AppUi()->ActivateViewL(iPreviousView);
		}break;
	case EPodcastSettings:
		AppUi()->ActivateLocalViewL(KUidPodcastSettingsViewID);
		break;
	case EPodcastAbout:
		break;
	default:
		break;
	}
}
