#include "RemoteControlListener.h"
#include <e32debug.h>
#include "PodcastModel.h"
#include "SoundEngine.h"

CRemoteControlListener* CRemoteControlListener::NewL(CPodcastModel &aModel)
	{
	CRemoteControlListener* mylistener = new (ELeave) CRemoteControlListener(aModel);
	CleanupStack::PushL(mylistener);
    mylistener->ConstructL();
    CleanupStack::Pop(mylistener);
    return mylistener;
	}

CRemoteControlListener::CRemoteControlListener(CPodcastModel &aModel) : iModel(aModel)
	{
	delete iSelector;
	}

CRemoteControlListener::~CRemoteControlListener()
	{
	delete iSelector;
	}
	
void CRemoteControlListener::MrccatoPlay(TRemConCoreApiPlaybackSpeed /*aSpeed*/, TRemConCoreApiButtonAction /*aButtonAct*/)
	{
	DP("Play Command received");
	iModel.SoundEngine().Play();
	}

void CRemoteControlListener::MrccatoTuneFunction()
	{
	DP("Tuner Command received");
	}

void CRemoteControlListener::MrccatoSelectDiskFunction()
	{
	DP("Select disc function received");
	}
	
void CRemoteControlListener::MrccatoSelectAvInputFunction(TUint8 /*aAvInputSignalNumber*/, TRemConCoreApiButtonAction /*aButtonAct*/)
	{
	DP("Select AV input function received");
	}
	
void CRemoteControlListener::MrccatoSelectAudioInputFunction(TUint8 /*aAudioInputSignalNumber*/, TRemConCoreApiButtonAction /*aButtonAct*/)
	{
	DP("Select audio input function received");
	}
	
void CRemoteControlListener::MrccatoCommand(TRemConCoreApiOperationId aOperationId, TRemConCoreApiButtonAction /*aButtonAct*/)
	{
	switch(aOperationId)
		{
		case ERemConCoreApiSelect:
			DP("ERemConCoreApiSelect");
			break;
	/** Up. */
		case ERemConCoreApiUp:
			DP("ERemConCoreApiUp");
			break;
	/** Down. */
		case ERemConCoreApiDown:
			DP("ERemConCoreApiDown");
			break;
	/** Left. */
		case ERemConCoreApiLeft:
			DP("ERemConCoreApiLeft");
			break;
	/** Right. */
		case ERemConCoreApiRight:
			DP("ERemConCoreApiRight");
			break;
	/** RightUp. */
		case ERemConCoreApiRightUp:
			DP("ERemConCoreApiRightUp");
			break;
	/** RightDown. */
		case ERemConCoreApiRightDown:
			DP("ERemConCoreApiRightDown");
			break; 				
	/** LeftUp. */
		case ERemConCoreApiLeftUp:					
			DP("ERemConCoreApiLeftUp");
			break;
	/** LeftDown. */
		case ERemConCoreApiLeftDown:					
			DP("ERemConCoreApiLeftDown");
			break;
	/** RootMenu. */
		case ERemConCoreApiRootMenu:				
			DP("ERemConCoreApiRootMenu");
			break;
	/** SetupMenu. */
		case ERemConCoreApiSetupMenu: 				
			DP("ERemConCoreApiSetupMenu");
			break;
	/** ContentsMenu. */
		case ERemConCoreApiContentsMenu:				
			DP("ERemConCoreApiContentsMenu");
			break;
	/** FavoriteMenu. */
		case ERemConCoreApiFavoriteMenu:				
			DP("ERemConCoreApiFavoriteMenu");
			break;
	/** Exit. */
		case ERemConCoreApiExit:						
			DP("ERemConCoreApiExit");
			break;
	/** 0. */
		case ERemConCoreApi0:
			DP("ERemConCoreApi0");
			break; 						
	/** 1. */
		case ERemConCoreApi1:
			DP("ERemConCoreApi1");
			break; 					
	/** 2. */
		case ERemConCoreApi2: 						
			DP("ERemConCoreApi2");
			break;
	/** 3. */
		case ERemConCoreApi3: 						
			DP("ERemConCoreApi3");
			break;
	/** 4. */
		case ERemConCoreApi4: 						
			DP("ERemConCoreApi4");
			break;
	/** 5. */
		case ERemConCoreApi5:
			DP("ERemConCoreApi5");
			break; 					
	/** 6. */
		case ERemConCoreApi6:
			DP("ERemConCoreApi6");
			break; 						
	/** 7. */
		case ERemConCoreApi7: 
			DP("ERemConCoreApi7");
			break;					
	/** 8. */
		case ERemConCoreApi8:
			DP("ERemConCoreApi8");
			break; 					
	/** 9. */
		case ERemConCoreApi9: 
			DP("ERemConCoreApi9");
			break;					
	/** Dot. */
		case ERemConCoreApiDot:
			DP("ERemConCoreApiDot");
			break;					
	/** Enter. */
		case ERemConCoreApiEnter:
			DP("ERemConCoreApiEnter");
			break; 					
	/** Clear. */
		case ERemConCoreApiClear: 
			DP("ERemConCoreApiClear");
			break;				
	/** ChannelUp. */
		case ERemConCoreApiChannelUp:
			DP("ERemConCoreApiChannelUp");
			break; 				
	/** ChannelDown. */
		case ERemConCoreApiChannelDown:
			DP("ERemConCoreApiChannelDown");
			break;				
	/** PreviousChannel. */
		case ERemConCoreApiPreviousChannel:
			DP("ERemConCoreApiPreviousChannel");
			break;		
	/** SoundSelect. */
		case ERemConCoreApiSoundSelect:
			DP("ERemConCoreApiSoundSelect");
			break;				
	/** InputSelect. */
		case ERemConCoreApiInputSelect:	
			DP("ERemConCoreApiInputSelect");
			break;		
	/** DisplayInformation. */
		case ERemConCoreApiDisplayInformation:	
			DP("ERemConCoreApiDisplayInformation");
			break;
	/** Help. */
		case ERemConCoreApiHelp:
			DP("ERemConCoreApiHelp");
			break;					
	/** PageUp. */
		case ERemConCoreApiPageUp:
			DP("ERemConCoreApiPageUp");
			break;				
	/** PageDown. */
		case ERemConCoreApiPageDown:				
			DP("ERemConCoreApiPageDown");
			break;
	/** Power. */
		case ERemConCoreApiPower: 
			DP("ERemConCoreApiPower");
			break;				
	/** VolumeUp. */
		case ERemConCoreApiVolumeUp:					
			DP("ERemConCoreApiVolumeUp");
			iModel.SoundEngine().VolumeUp();
			break;
	/** VolumeDown. */
		case ERemConCoreApiVolumeDown:
			DP("ERemConCoreApiVolumeDown");
			iModel.SoundEngine().VolumeDown();
			break;				
	/** Mute. */
		case ERemConCoreApiMute:
			DP("ERemConCoreApiMute");
			break;						
	/** Play. */
		case ERemConCoreApiPlay:
			DP("ERemConCoreApiPlay");
			iModel.SoundEngine().Play();
			break;						
	/** Stop. */
		case ERemConCoreApiStop:
			DP("ERemConCoreApiStop");
			iModel.SoundEngine().Stop();
			break;						
	/** Pause. */
		case ERemConCoreApiPause: 
			DP("ERemConCoreApiPause");
			iModel.SoundEngine().Pause();
			break;					
	/** Record. */
		case ERemConCoreApiRecord:
			DP("ERemConCoreApiRecord");
			break;					
	/** Rewind. */
		case ERemConCoreApiRewind:
			DP("ERemConCoreApiRewind");
			break;					
	/** FastForward. */
		case ERemConCoreApiFastForward:				
			DP("ERemConCoreApiFastForward");
			break;
	/** Eject. */
		case ERemConCoreApiEject: 					
			DP("ERemConCoreApiEject");
			break;
	/** Forward. */
		case ERemConCoreApiForward:					
			DP("ERemConCoreApiForward");
			break;
	/** Backward. */
		case ERemConCoreApiBackward:					
			DP("ERemConCoreApiBackward");
			break;
	/** Angle. */
		case ERemConCoreApiAngle: 					
			DP("ERemConCoreApiAngle");
			break;
	/** Subpicture. */
		case ERemConCoreApiSubpicture:				
			DP("ERemConCoreApiSubpicture");
			break;
	/** PausePlayFunction. */
		case ERemConCoreApiPausePlayFunction: 		
			DP("ERemConCoreApiPausePlayFunction");
			break;
	/** RestoreVolumeFunction. */
		case ERemConCoreApiRestoreVolumeFunction: 	
			DP("ERemConCoreApiRestoreVolumeFunction");
			break;
	/** TuneFunction. */
		case ERemConCoreApiTuneFunction:				
			DP("ERemConCoreApiTuneFunctio");
			break;
	/** SelectDiskFunction. */
		case ERemConCoreApiSelectDiskFunction:
			DP("ERemConCoreApiSelectDiskFunction");
			break;		
	/** SelectAvInputFunction. */
		case ERemConCoreApiSelectAvInputFunction:
			DP("ERemConCoreApiSelectAvInputFunction");
			break; 	
	/** SelectAudioInputFunction. */
		case ERemConCoreApiSelectAudioInputFunction:
			DP("ERemConCoreApiSelectAudioInputFunction");
			break;	
	/** F1. */
		case ERemConCoreApiF1:
			if (iModel.SoundEngine().State() == ESoundEnginePlaying) {
				iModel.SoundEngine().Pause();
			} else if (iModel.SoundEngine().State() == ESoundEnginePaused) {
				iModel.SoundEngine().Play();
			}
			DP("ERemConCoreApiF1");
			break;							
	/** F2. */
		case ERemConCoreApiF2:
			DP("ERemConCoreApiF2");
			break;							
	/** F3. */
		case ERemConCoreApiF3:
			DP("ERemConCoreApiF3");
			break;							
	/** F4. */
		case ERemConCoreApiF4:
			DP("ERemConCoreApiF4");
			break;							
	/** F5. */
		case ERemConCoreApiF5:
			DP("ERemConCoreApiF5");
			break;
		
		default:
			DP("Unknown");
			break;						
		}
	
	}

void CRemoteControlListener::ConstructL()
	{
	// RemConInterface Selector
	iSelector = CRemConInterfaceSelector::NewL();
	
	// to be able to receive remote commands
	iTarget = CRemConCoreApiTarget::NewL(*iSelector,*this);
	
	//Opens a target session to RemCon.
	iSelector->OpenTargetL();
	}


