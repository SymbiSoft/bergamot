#include <badesca.h>
#include <qikcommand.h>
#include <PodcastClient.rsg>
#include <e32debug.h>
#include <eikedwin.h>
#include <eiklabel.h>
#include <qikslider.h>
#include <coecobs.h>
#include <qikappui.h>
#include <eikimage.h>
#include <QikZoomDlg.h>
#include <devicekeys.h>
#include <e32keys.h>
#include <podcastclient.mbg>
#include <qikgenericbuildingblock.h>
#include <qikutils.h>

#include "PodcastClientSettingsDlg.h"
#include "HttpClient.h"
#include "PodcastModel.h"
#include "PodcastClient.hrh"
#include "PodcastClientPlayView.h"
#include "PodcastClientGlobals.h"
#include "SoundEngine.h"
#include "ShowEngine.h"
#include "SettingsEngine.h"

const TInt KAudioTickerPeriod = 1000000;
const TInt KMaxCoverImageWidth = 200;
const TInt KTimeLabelSize = 64;

_LIT(KZeroTime,"0:00:00");
/**
 Creates and constructs the view.

 @param aAppUi Reference to the AppUi
 @return Pointer to a CPodcastClientPlayView object
 */
CPodcastClientPlayView* CPodcastClientPlayView::NewLC(CQikAppUi& aAppUi,
		CPodcastModel& aPodcastModel)
	{
	//RDebug::Print(_L("NewLC"));
	CPodcastClientPlayView* self = new (ELeave) CPodcastClientPlayView(aAppUi, aPodcastModel);
	CleanupStack::PushL(self);
	self->ConstructL();
	return self;
	}

/**
 Constructor for the view.
 Passes the application UI reference to the construction of the super class.

 KNullViewId should normally be passed as parent view for the applications 
 default view. The parent view is the logical view that is normally activated 
 when a go back command is issued. KNullViewId will activate the system 
 default view. 

 @param aAppUi Reference to the application UI
 */
CPodcastClientPlayView::CPodcastClientPlayView(CQikAppUi& aAppUi,
		CPodcastModel& aPodcastModel) :
	CQikViewBase(aAppUi, KNullViewId), iPodcastModel(aPodcastModel),
			iMaxProgressValue(100)
	{
	}

/**
 Destructor for the view
 */
CPodcastClientPlayView::~CPodcastClientPlayView()
	{
	delete iPlaybackTicker;
	delete iCurrentCoverImage;
	delete iBitmapConverter;

	iPodcastModel.ShowEngine().RemoveObserver(this);
	iPodcastModel.SoundEngine().RemoveObserver(this);
	}

/**
 2nd stage construction of the view.
 */
void CPodcastClientPlayView::ConstructL()
	{
	// Calls ConstructL that initialises the standard values. 
	// This should always be called in the concrete view implementations.
	BaseConstructL();
	iPlaybackTicker = CPeriodic::NewL(CActive::EPriorityStandard);
	iPodcastModel.SoundEngine().AddObserver(this);
	iBitmapConverter = CQikImageConverter::NewL(iEikonEnv->FsSession(), *this);
	iPodcastModel.ShowEngine().AddObserver(this);
	}

/**
 Returns the view Id

 @return Returns the Uid of the view
 */
TVwsViewId CPodcastClientPlayView::ViewId() const
	{
	return TVwsViewId(KUidPodcastClientID, KUidPodcastPlayViewID);
	}

void CPodcastClientPlayView::HandleControlEventL(CCoeControl* aControl,
		TCoeEvent aEventType)
	{
	CQikViewBase::HandleControlEventL(aControl, aEventType);

	if (aEventType == MCoeControlObserver::EEventStateChanged)
		{
		// Set position in current playback
		if (aControl == iPlayProgressbar)
			{
			TInt value = iPlayProgressbar->CurrentValue();
			TUint ptime = iPodcastModel.SoundEngine().PlayTime();
			if (ptime > 0)
				{
				iPodcastModel.SoundEngine().SetPosition((value*ptime)/iMaxProgressValue);
				}
			}
		}
	}

TKeyResponse CPodcastClientPlayView::OfferKeyEventL(const TKeyEvent& aKeyEvent,
		TEventCode aType)
	{
	CQikViewBase::OfferKeyEventL(aKeyEvent, aType);
	if (aType == EEventKey)
		{
		switch (aKeyEvent.iCode)
			{
			case EKeyLeftArrow:
			case EKeyRightArrow:
			case EDeviceKeyFourWayLeft:
			case EDeviceKeyFourWayRight:
			case '*':
			case '#':
				{
				TUint ptime = iPodcastModel.SoundEngine().PlayTime();
				if (ptime > 0)
					{
					TInt stepValue = iPodcastModel.SettingsEngine().SeekStepTime();
					TUint pos = iPodcastModel.SoundEngine().Position().Int64()/1000000;
					if (aKeyEvent.iCode == EKeyLeftArrow|| aKeyEvent.iCode
							== EDeviceKeyFourWayLeft || aKeyEvent.iCode == '*')
						{
						if (((TInt)pos) - stepValue > 0) {
							pos-=stepValue;
						} else {
							pos = 0;
						}
						}
					else
						{
						pos+=stepValue;
						}

					iPodcastModel.SoundEngine().SetPosition(pos);
					if (!iPlaybackTicker->IsActive())
						{
						UpdatePlayStatusL();
						}
					}
				}
				break;

			case '6':
				if (iPodcastModel.SettingsEngine().Volume() < KMaxVolume)
					{
					iPodcastModel.SettingsEngine().SetVolume(iPodcastModel.SettingsEngine().Volume()+KVolumeDelta);
					}
				break;
			case '4':

				if (iPodcastModel.SettingsEngine().Volume() > 0)
					{
					iPodcastModel.SettingsEngine().SetVolume(iPodcastModel.SettingsEngine().Volume()-KVolumeDelta);
					}
				break;
			}
		}
	return EKeyWasNotConsumed;
	}

TInt CPodcastClientPlayView::PlayingUpdateStaticCallbackL(TAny* aPlayView)
	{
	static_cast<CPodcastClientPlayView*>(aPlayView)->UpdatePlayStatusL();
	return KErrNone;
	}

void CPodcastClientPlayView::PlaybackInitializedL()
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

void CPodcastClientPlayView::UpdateMaxProgressValueL(TInt aDuration)
	{
	iMaxProgressValue = aDuration;

	if (iMaxProgressValue>120)
		{
		iPlayProgressbar->SetMaxValueL(iMaxProgressValue);
		iPlayProgressbar->SetNumberOfMarkersL(iMaxProgressValue/60);
		}
	else
		{
		if (iMaxProgressValue == 0)
			{
			iMaxProgressValue++;
			}

		iMaxProgressValue = iMaxProgressValue*10;

		iPlayProgressbar->SetMaxValueL(iMaxProgressValue);
		iPlayProgressbar->SetNumberOfMarkersL(10);
		}

	iPlayProgressbar->DrawDeferred();
	}

void CPodcastClientPlayView::PlaybackStartedL()
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

void CPodcastClientPlayView::PlaybackStoppedL()
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

/**
 Handles all commands in the view.
 Called by the UI framework when a command has been issued.
 The command Ids are defined in the .hrh file.

 @param aCommand The command to be executed
 @see CQikViewBase::HandleCommandL
 */
void CPodcastClientPlayView::HandleCommandL(CQikCommand& aCommand)
	{
	CQikCommandManager& comMan = CQikCommandManager::Static();

	switch (aCommand.Id())
		{
		case EPodcastMarkAsPlayed:
			{
			iPodcastModel.ShowEngine().SetShowPlayState(iShowInfo, EPlayed);
			UpdateViewL();
			}
			break;
		case EPodcastMarkAsUnplayed:
			{
			iPodcastModel.ShowEngine().SetShowPlayState(iShowInfo, ENeverPlayed);
			UpdateViewL();
			}
			break;

		case EPodcastSetVolume:
			{
			CPodcastClientVolumeDlg* dlg = new (ELeave) CPodcastClientVolumeDlg(iPodcastModel);
			dlg->ExecuteLD(R_PODCAST_VOLUME_DLG);
			}
			break;
		case EPodcastSettings:
			{
			CPodcastClientSettingsDlg* dlg = new (ELeave) CPodcastClientSettingsDlg(iPodcastModel);
			dlg->ExecuteLD(R_PODCAST_SETTINGS_DLG);
			}
			break;

		case EPodcastZoomSetting:
			{
			// Launch the zoom dialog
			const TInt zoomFactor =
					CQikZoomDialog::RunDlgLD(iPodcastModel.ZoomState());

			// If zoom state have changed it will be stored to persistent
			// storage and a relayout will be performed
			if (iPodcastModel.SetZoomState(zoomFactor))
				{
				// Sets the zoom factor for the view
				iLastZoomLevel = zoomFactor;
				SetZoomFactorL(CQikAppUi::ZoomFactorL(zoomFactor, *iEikonEnv));
				}
			}
			break;

		case EPodcastPlay:
			{
			if (iPodcastModel.PlayingPodcast() != NULL
					&& iPodcastModel.PlayingPodcast()->Uid() == iShowInfo->Uid())
				{
				if (iPodcastModel.PlayingPodcast()->ShowType() == EVideoPodcast
						|| iPodcastModel.SoundEngine().State() == ESoundEngineNotInitialized)
					{
					QikFileUtils::StartAppL(iPodcastModel.PlayingPodcast()->FileName());//, lsSession);
					}
				else
					{
					if (iPodcastModel.SoundEngine().State() == ESoundEnginePlaying)
						{
						iPodcastModel.SoundEngine().Pause();
						iPlayProgressbar->SetFocusing(EFalse);
						comMan.SetTextL(*this, EPodcastPlay,
								R_PODCAST_PLAYER_PLAY_CMD);
						RequestFocusL(iScrollableContainer);
						}
					else
						{
						iPodcastModel.SoundEngine().Play();
						iPlayProgressbar->SetFocusing(EFalse);
						comMan.SetTextL(*this, EPodcastPlay,
								R_PODCAST_PLAYER_PAUSE_CMD);
						}
					}
				}
			else
				{
				iPodcastModel.PlayPausePodcastL(iShowInfo, ETrue);
				}
			UpdateViewL();
			}
			break;
		case EPodcastStop:
			{
			comMan.SetTextL(*this, EPodcastPlay, R_PODCAST_PLAYER_PLAY_CMD);
			iPlayProgressbar->SetFocusing(EFalse);
			iPodcastModel.SoundEngine().Stop();
			iPodcastModel.PlayingPodcast()->SetPosition(0);
			iPodcastModel.SetPlayingPodcast(NULL);
			}
			break;
		case EPodcastResumeDownloads:
			{
			iPodcastModel.ShowEngine().ResumeDownloads();
			UpdateViewL();
			}
			break;
		case EPodcastRemoveDownload:
			{
			iPodcastModel.ShowEngine().RemoveDownload(iShowInfo->Uid());
			UpdateViewL();
			}
			break;
		case EPodcastDownloadShow:
			{
			iPodcastModel.ShowEngine().AddDownload(iShowInfo);
			UpdateViewL();
			}
			break;
		case EPodcastViewMain:
			{
			TVwsViewId playView = TVwsViewId(KUidPodcastClientID,
					KUidPodcastBaseViewID);
			iQikAppUi.ActivateViewL(playView);
			}
			break;
		case EPodcastViewFeeds:
			{
			TVwsViewId podcastsView = TVwsViewId(KUidPodcastClientID,
					KUidPodcastFeedViewID);
			iQikAppUi.ActivateViewL(podcastsView);
			}
			break;

		case EPodcastViewNewShows:
			{
			TVwsViewId podcastsView = TVwsViewId(KUidPodcastClientID,
					KUidPodcastShowsViewID);
			iQikAppUi.ActivateViewL(podcastsView, TUid::Uid(EShowNewShows),
					KNullDesC8());
			}
			break;
		case EPodcastViewDownloadedShows:
			{
			TVwsViewId podcastsView = TVwsViewId(KUidPodcastClientID,
					KUidPodcastShowsViewID);
			iQikAppUi.ActivateViewL(podcastsView,
					TUid::Uid(EShowDownloadedShows), KNullDesC8());
			}
			break;
		case EPodcastViewPendingShows:
			{
			TVwsViewId podcastsView = TVwsViewId(KUidPodcastClientID,
					KUidPodcastShowsViewID);
			iQikAppUi.ActivateViewL(podcastsView, TUid::Uid(EShowPendingShows),
					KNullDesC8());
			}
			break;

			// Just issue simple info messages to show that
			// the commands have been selected
		default:
			CQikViewBase::HandleCommandL(aCommand);
			break;
		}
	}

void CPodcastClientPlayView::ViewConstructL()
	{
	//RDebug::Print(_L("ViewConstructL"));
	ViewConstructFromResourceL(R_PODCAST_PLAYVIEW_UI_CONFIGURATIONS);

	iPlayProgressbar
			=LocateControlByUniqueHandle<CQikSlider>(EPodcastPlayViewProgressCtrl);
	iTimeLabel
			= LocateControlByUniqueHandle<CEikLabel>(EPodcastPlayViewProgressTime);
	iTotTimeLabel
			= LocateControlByUniqueHandle<CEikLabel>(EPodcastPlayViewProgressTotTime);

	iInformationEdwin
			= LocateControlByUniqueHandle<CEikLabel>(EPodcastPlayViewInformation);
	iScrollableContainer
			= LocateControlByUniqueHandle<CQikScrollableContainer>(EPodcastPlayViewScrollContainer);
	iTitleEdwin
			= LocateControlByUniqueHandle<CEikLabel>(EPodcastPlayViewTitleCtrl);
	iProgressBB
			= LocateControlByUniqueHandle<CQikGenericBuildingBlock>(EPodcastPlayViewBottomLineBB);

	iVolumeSlider
			= LocateControlByUniqueHandle<CQikSlider>(EPodcastPlayViewVolumeCtrl);

	iCoverImageCtrl
			= LocateControlByUniqueHandle<CEikImage>(EPodcastPlayViewCoverImage);

	iDownloadProgressInfo
			= LocateControlByUniqueHandle<CEikProgressInfo>(EPodcastPlayViewProgressInfo);

	ViewContext()->AddTextL(EPodcastPlayViewTitleCtrl, KNullDesC(), EHRightVCenter);
	iCategories = QikCategoryUtils::ConstructCategoriesLC(R_PODCAST_CATEGORY);
	SetCategoryModel(iCategories);
	CleanupStack::Pop(iCategories);
	}

void CPodcastClientPlayView::ViewActivatedL(const TVwsViewId &aPrevViewId,
		TUid aCustomMessageId, const TDesC8 &aCustomMessage)
	{
	CQikViewBase::ViewActivatedL(aPrevViewId, aCustomMessageId, aCustomMessage);
	SelectCategoryL(EShowAllShows);

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

	if (iLastZoomLevel != iPodcastModel.ZoomState())
		{
		iLastZoomLevel = iPodcastModel.ZoomState();
		SetZoomFactorL(CQikAppUi::ZoomFactorL(iLastZoomLevel, *iEikonEnv));
		}

	iShowInfo = iPodcastModel.ShowEngine().GetShowByUidL(iCurrentViewShowUid);

	UpdateViewL();

	if (aPrevViewId.iAppUid == KUidPodcastClientID)
		{
		SetParentView(aPrevViewId);
		}

	iScrollableContainer->ScrollToMakeVisible(iTitleEdwin);
	RequestFocusL(iScrollableContainer);

	TInt sndState = iPodcastModel.SoundEngine().State();
	if (iShowInfo != NULL && sndState == ESoundEngineNotInitialized
			&& (iPodcastModel.PlayingPodcast() == NULL
					|| (iPodcastModel.PlayingPodcast() != NULL
							&& iPodcastModel.PlayingPodcast()->Uid() == iShowInfo->Uid())))
		{
		iPodcastModel.PlayPausePodcastL(iShowInfo);
		}
	}

void CPodcastClientPlayView::ViewDeactivated()
	{
	CQikViewBase::ViewDeactivated();
	iPlaybackTicker->Cancel();
	}

void CPodcastClientPlayView::ShowDownloadUpdatedL(
		TInt aPercentOfCurrentDownload, TInt aBytesOfCurrentDownload,
		TInt aBytesTotal)

	{
	//RDebug::Print(_L("CPodcastClientPlayView::ShowDownloadUpdatedL"));
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

		iDownloadProgressInfo->SetValue(aBytesOfCurrentDownload);
		iDownloadProgressInfo->DrawDeferred();
		iBytesDownloaded = aBytesOfCurrentDownload;
		UpdatePlayStatusL();
		}

	if (aPercentOfCurrentDownload == KOneHundredPercent)
		{
		if (iPodcastModel.PlayingPodcast() == iPodcastModel.ShowEngine().ShowDownloading())
			{
			// To update icon list status and commands
			CShowInfo* playingPodcast = iPodcastModel.PlayingPodcast();
			// Reset current podcast statsus
			iPodcastModel.PlayPausePodcastL(NULL);
			iPodcastModel.PlayPausePodcastL(playingPodcast);
			}
		iBytesDownloaded = 0;
		UpdateViewL();
		}

	}

void CPodcastClientPlayView::ImageConverterEventL(
		TQikImageConverterEvent aMessage, TInt aErrCode)
	{
	if (aErrCode == KErrNone && aMessage
			== MQikImageConverterObserver::EImageConvertComplete)
		{
		if (iCurrentCoverImage->SizeInPixels().iWidth<=KMaxCoverImageWidth)
			{
			iCoverImageCtrl->SetNewBitmaps(iCurrentCoverImage, NULL);
			iCoverImageCtrl->SetPictureOwnedExternally(EFalse);

			delete iBitmapConverter; // make sure that we close the file handle
			iBitmapConverter = NULL;
			iBitmapConverter = CQikImageConverter::NewL(iEikonEnv->FsSession(), *this);

			iCurrentCoverImage = NULL;
			RequestRelayout(this);
			}
		else
			{
			iBitmapConverter->RescaleImage(*iCurrentCoverImage, TSize(
					KMaxCoverImageWidth, iCurrentCoverImage->SizeInPixels().iHeight));
			}
		}
	else
		if (aErrCode == KErrNone && aMessage
				== MQikImageConverterObserver::EBitmapRescaleComplete)
			{
			iCoverImageCtrl->SetNewBitmaps(iCurrentCoverImage, NULL);
			iCoverImageCtrl->SetPictureOwnedExternally(EFalse);

			delete iBitmapConverter; // make sure that we close the file handle
			iBitmapConverter = NULL;
			iBitmapConverter = CQikImageConverter::NewL(iEikonEnv->FsSession(), *this);

			iCurrentCoverImage = NULL;
			RequestRelayout(this);
			}
		else
			if (aErrCode != KErrNone)
				{
				iCoverImageCtrl->SetNewBitmaps(NULL, NULL);
				iCoverImageCtrl->SetPictureOwnedExternally(EFalse);

				delete iBitmapConverter; // make sure that we close the file handle
				iBitmapConverter = NULL;
				iBitmapConverter
						= CQikImageConverter::NewL(iEikonEnv->FsSession(), *this);

				RequestRelayout(this);

				delete iCurrentCoverImage;
				iCurrentCoverImage = NULL;
				}

	}

void CPodcastClientPlayView::UpdateViewL()
	{
	CQikCommandManager& comMan = CQikCommandManager::Static();

	if (iShowInfo != NULL)
		{
		iInformationEdwin->SetTextL(iShowInfo->Description());

		if (iTitleEdwin != NULL)
			{
			iTitleEdwin->SetTextL(iShowInfo->Title());
			}

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
		else
			if (iShowInfo->DownloadState() != EDownloaded) // Qued or downloading.
				{
				comMan.SetInvisible(*this, EPodcastPlay, ETrue);
				comMan.SetInvisible(*this, EPodcastDownloadShow, ETrue);
				comMan.SetInvisible(*this, EPodcastRemoveDownload, EFalse);
				if (iShowInfo->DownloadState() == EDownloading)
					{
					comMan.SetTextL(*this, EPodcastRemoveDownload,
							R_PODCAST_PLAYER_SUSPEND_DL_CMD);
					comMan.SetShortTextL(*this, EPodcastRemoveDownload,
							R_PODCAST_PLAYER_SUSPEND_DL_CMD);
					}
				else
					{
					comMan.SetTextL(*this, EPodcastRemoveDownload,
							R_PODCAST_PLAYER_REMOVE_DL_CMD);
					comMan.SetShortTextL(*this, EPodcastRemoveDownload,
							R_PODCAST_PLAYER_REMOVE_DL_CMD);
					if (iPodcastModel.ShowEngine().DownloadsStopped())
						{
						comMan.SetInvisible(*this, EPodcastResumeDownloads,
								EFalse);
						}
					}

				comMan.SetInvisible(*this, EPodcastSetVolume, ETrue);
				iDownloadProgressInfo->MakeVisible(iShowInfo->DownloadState()
						== EDownloading);
				iPlayProgressbar->MakeVisible(EFalse);
				}
			else // Downloaded
				{
				comMan.SetInvisible(*this, EPodcastPlay, EFalse);
				comMan.SetInvisible(*this, EPodcastSetVolume, EFalse);
				comMan.SetInvisible(*this, EPodcastDownloadShow, ETrue);
				comMan.SetInvisible(*this, EPodcastRemoveDownload, ETrue);
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
					iCoverImageCtrl->CreatePictureFromFileL(_L("*"), EMbmPodcastclientEmptyimage, EMbmPodcastclientEmptyimage);
					iCoverImageCtrl->DrawDeferred();
					if (!iBitmapConverter->IsActive())
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
						}
					}
				}
			else // no cover art file
				{
				iLastImageFileName = KNullDesC();
				if (iShowInfo->ShowType() == EAudioBook)
					{
					iCoverImageCtrl->CreatePictureFromFileL(_L("*"), EMbmPodcastclientAudiobook_120, EMbmPodcastclientAudiobook_120m);
					}
				else
					{
					iCoverImageCtrl->CreatePictureFromFileL(_L("*"), EMbmPodcastclientShow_120, EMbmPodcastclientShow_120m);
					}
				}

			}
		else // no feed info => manually added file
			{
			iLastImageFileName = KNullDesC();
			if (iShowInfo->ShowType() == EAudioBook)
				{
				iCoverImageCtrl->CreatePictureFromFileL(_L("*"), EMbmPodcastclientAudiobook_120, EMbmPodcastclientAudiobook_120m);
				}
			else
				{
				iCoverImageCtrl->CreatePictureFromFileL(_L("*"), EMbmPodcastclientShow_120, EMbmPodcastclientShow_120m);
				}
			comMan.SetInvisible(*this, EPodcastDownloadShow, ETrue);
			}
		}
	else // no show info, we should never get here
		{
		iLastImageFileName = KNullDesC();
		if (iCoverImageCtrl != NULL)
			{
			iCoverImageCtrl->CreatePictureFromFileL(_L("*"), EMbmPodcastclientEmptyimage, EMbmPodcastclientEmptyimage);
			}
		comMan.SetInvisible(*this, EPodcastDownloadShow, ETrue);
		}

	UpdatePlayStatusL();

	if (iProgressBB != NULL)
		{
		iProgressBB->RequestRelayout(iProgressBB);
		}

	RequestRelayout(this);

	if (iPodcastModel.PlayingPodcast() != NULL && iPodcastModel.PlayingPodcast()->Uid() == iShowInfo->Uid() && iPodcastModel.SoundEngine().State() == ESoundEnginePlaying)
		{
		iPlaybackTicker->Cancel();
		iPlaybackTicker->Start(KAudioTickerPeriod, KAudioTickerPeriod,
				TCallBack(PlayingUpdateStaticCallbackL, this));
		}

	}

void CPodcastClientPlayView::UpdatePlayStatusL()
	{
	CQikCommandManager& comMan = CQikCommandManager::Static();
	TBuf<KTimeLabelSize> time= _L("0:00:00/0:00:00");
	TUint pos = 0;

	if (iPodcastModel.PlayingPodcast() != NULL && iPodcastModel.PlayingPodcast()->Uid() == iShowInfo->Uid())
		{

		if (iPodcastModel.SoundEngine().State() == ESoundEnginePlaying)
			{
			comMan.SetTextL(*this, EPodcastPlay, R_PODCAST_PLAYER_PAUSE_CMD);
			iPlayProgressbar->SetFocusing(EFalse);
			RequestFocusL(iScrollableContainer);
			}
		else
			{
			comMan.SetTextL(*this, EPodcastPlay, R_PODCAST_PLAYER_PLAY_CMD);
			iPlayProgressbar->SetFocusing(EFalse);
			}

		//	comMan.SetDimmed(*this, EPodcastPlay, iPodcastModel.SoundEngine().State() == ESoundEngineNotInitialized);
		comMan.SetDimmed(*this, EPodcastStop, (iPodcastModel.SoundEngine().State() == ESoundEngineNotInitialized || 
												iPodcastModel.SoundEngine().State() == ESoundEngineStopped && iPodcastModel.PlayingPodcast()->Position() == 0));
		if (iPlayProgressbar != NULL)
			{
			iPlayProgressbar->SetDimmed(iPodcastModel.SoundEngine().State() <= ESoundEngineOpening);

			if (iPodcastModel.SoundEngine().PlayTime()>0)
				{
				if (iPodcastModel.SoundEngine().State() == ESoundEnginePlaying
						|| iPodcastModel.SoundEngine().State() == ESoundEnginePaused
						|| iPodcastModel.SoundEngine().State() == ESoundEngineStopped)
					{
					TUint duration = iPodcastModel.SoundEngine().PlayTime();
					pos = iPodcastModel.SoundEngine().Position().Int64()/1000000;
					iPlayProgressbar->SetValue((iMaxProgressValue*pos)/duration);
					iPlayProgressbar->DrawDeferred();
					}
				}
			else
				{
				if (iShowInfo->PlayTime()>0)
					{
					TUint duration = iShowInfo->PlayTime();
					pos = iShowInfo->Position().Int64()/1000000;
					iPlayProgressbar->SetValue((iMaxProgressValue*pos)/duration);
					}
				else
					{
					iPlayProgressbar->SetValue(0);
					}
				iPlayProgressbar->DrawDeferred();
				}
			}
		}
	else
		{
		comMan.SetTextL(*this, EPodcastPlay, R_PODCAST_PLAYER_PLAY_CMD);
		comMan.SetDimmed(*this, EPodcastPlay, EFalse);
		comMan.SetDimmed(*this, EPodcastStop, ETrue);

		if (iShowInfo)
			{
			comMan.SetInvisible(*this, EPodcastStop,
					!(iShowInfo->DownloadState() == EDownloaded));
			}

		// not sure why we end up here, but this prevents crashing (teknolog)
		if (iPlayProgressbar == NULL)
			{
			return;
			}

		iPlayProgressbar->SetDimmed(ETrue);

		if (iShowInfo && iShowInfo->PlayTime()>0)
			{
			TUint duration = iShowInfo->PlayTime();
			pos = iShowInfo->Position().Int64()/1000000;

			UpdateMaxProgressValueL(duration);
			iPlayProgressbar->SetValue((iMaxProgressValue*pos)/duration);
			}
		else
			{
			iPlayProgressbar->SetValue(0);
			UpdateMaxProgressValueL(0);
			}

		iPlayProgressbar->DrawDeferred();
		}

	if (iShowInfo != NULL)
		{
		if (iShowInfo->DownloadState() != EDownloaded)
			{
			if (iBytesDownloaded > 0)
				{
				const TInt KSizeBufLen = 64;
				TBuf<KSizeBufLen> dlSize;
				TBuf<KSizeBufLen> totSize;
				_LIT(KSizeDownloadingOf, "%S/%S");

				totSize.Format(KShowsSizeFormat(), (float)iShowInfo->ShowSize()	/ (float) KSizeMb);
				dlSize.Format(KShowsSizeFormat(), (float) iBytesDownloaded / (float) KSizeMb);
				time.Format(KSizeDownloadingOf(), &dlSize, &totSize);
				time.Append(KShowsSizeUnit());
				}
			else
				{
				time.Format(KShowsSizeFormat(), (float)iShowInfo->ShowSize() / (float)KSizeMb);
				time.Append(KShowsSizeUnit());
				}
			//}
			}
		else
			{
			CShowInfo* showInfo = iPodcastModel.PlayingPodcast();

			if (showInfo != NULL)
				{
				TUint playtime = 0;
				TBuf<KTimeLabelSize> totTime= _L("00:00");

				if (showInfo->Uid() == iShowInfo->Uid()
						&& iShowInfo->PlayTime() == 0)
					{
					playtime = iPodcastModel.SoundEngine().PlayTime();
					}
				else
					{
					playtime = iShowInfo->PlayTime();
					}

				if (playtime >= 0)
					{
					TInt hour = playtime/3600;
					playtime = playtime-(hour*3600);

					TInt sec = (playtime%60);
					TInt min = (playtime/60);
					totTime.Format(_L("%01d:%02d:%02d"),hour, min, sec);
					}
				else
					{
					totTime = KZeroTime();
					}

				if (pos >= 0)
					{
					TInt hour = pos/3600;
					pos = pos-(hour*3600);

					TInt sec = (pos%60);
					TInt min = (pos/60);
					time.Format(_L("%01d:%02d:%02d"),hour, min, sec);
					}
				time.Append(_L("/"));
				time.Append(totTime);

				if (iShowInfo->ShowType() == EAudioBook && iShowInfo->TrackNo()
						!= KMaxTUint)
					{
					totTime.Format(_L("%03d - "), iShowInfo->TrackNo());
					time.Insert(0, totTime);
					}
				}
			else
				if (showInfo == NULL) // No other show playing start to init this one
					{
					iPodcastModel.PlayPausePodcastL(iShowInfo);
					}
			}
		}

	ViewContext()->ChangeTextL(EPodcastPlayViewTitleCtrl, time);

	if (iTimeLabel != NULL)
		{
		iTimeLabel->SetText(time);
		iTimeLabel->SetSize(iTimeLabel->MinimumSize());
		iTimeLabel->DrawDeferred();
		}

	if (iTotTimeLabel != NULL)
		{
		iTotTimeLabel->SetText(time);
		iTotTimeLabel->SetSize(iTotTimeLabel->MinimumSize());
		}

	}

