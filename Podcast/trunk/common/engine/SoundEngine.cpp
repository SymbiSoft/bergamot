#include <e32std.h>
#include "SettingsEngine.h"
#include "SoundEngine.h"
#include "PodcastModel.h"
#include "ShowInfo.h"
#include "ShowEngine.h"

CSoundEngine* CSoundEngine::NewL(CPodcastModel& aPodcastModel)
{
	CSoundEngine* self = new (ELeave) CSoundEngine(aPodcastModel);
	CleanupStack::PushL(self);
	self->ConstructL();
	CleanupStack::Pop(self);
	return self;
}

CSoundEngine::~CSoundEngine()
{
	delete iPlayer;
	iObservers.Close();
}

CSoundEngine::CSoundEngine(CPodcastModel& aPodcastModel): iPodcastModel(aPodcastModel)
{
}

void CSoundEngine::ConstructL()
{
    iPlayer = CMdaAudioPlayerUtility::NewL(*this);
}

void CSoundEngine::AddObserver(MSoundEngineObserver* aObserver)
{
	iObservers.Append(aObserver);
}

void CSoundEngine::RemoveObserver(MSoundEngineObserver* observer)
	{
	TInt index = iObservers.Find(observer);
	
	if (index > KErrNotFound)
		{
		iObservers.Remove(index);
		}
	}

void CSoundEngine::MapcPlayComplete(TInt aError) {
	DP1("MapcPlayComplete: %d", aError);
		
	if (iPodcastModel.PlayingPodcast() != NULL) {
		if(aError == KErrNone) // normal ending
			{
			iMaxPos = 0;
			iPodcastModel.PlayingPodcast()->SetPlayState(EPlayed);
			iPodcastModel.PlayingPodcast()->SetPosition(0);
			}
	}

	iState = ESoundEngineStopped;
	//iPodcastModel.ShowEngine().NotifyShowListUpdated();
	NotifyPlaybackStopped();

	if(iPodcastModel.PlayingPodcast() != NULL && iPodcastModel.PlayingPodcast()->ShowType() == EAudioBook)
	{
		CShowInfo* nextShow = NULL;
		
		/*TRAP_IGNORE(nextShow = iPodcastModel.ShowEngine().GetNextShowByTrackL(iPodcastModel.PlayingPodcast()));
		if(nextShow != NULL)
		{
			TRAP_IGNORE(iPodcastModel.PlayPausePodcastL(nextShow->Uid(), ETrue));
		}*/
	}
}	

void CSoundEngine::MapcInitComplete(TInt aError, const TTimeIntervalMicroSeconds &/*aDuration */) {
	if (aError != KErrNone)
		{
		DP1("MapcInitComplete error=%d", aError);	
		
		iState = ESoundEngineNotInitialized;		
		}
	else
		{
		iState = ESoundEngineStopped;
		
		iPlayer->SetVolume((iPodcastModel.SettingsEngine().Volume() * iPlayer->MaxVolume()) / 100);
		
		if (iPodcastModel.PlayingPodcast() != NULL) 
			{
			DP1("Resuming from position: %ld", iPodcastModel.PlayingPodcast()->Position().Int64());
			TInt duration = (iPlayer->Duration().Int64()/1000000);
			if(duration == 0) // sounds should at least be marked as one second long if they are <1 second
				{
				duration++;
				}

			iPodcastModel.PlayingPodcast()->SetPlayTime(duration);
			iPlayer->SetPosition(iPodcastModel.PlayingPodcast()->Position());			
			}
		}

	if(aError == KErrNone)
		{
		NotifyPlaybackInitialized();
		if(iPlayOnInit)
			{
			
			TRAP_IGNORE(iPodcastModel.PlayPausePodcastL(iPodcastModel.PlayingShowUid()));
			}
		}

	iPlayOnInit = EFalse;
}

void CSoundEngine::OpenFileL(const TDesC& aFileName, TBool aPlayOnInit)
{
	iState = ESoundEngineNotInitialized;
	iMaxPos = 0;
	iLastOpenedFileName= aFileName;
	iPlayer->Stop();
	iPlayer->Close();
	iPlayer->OpenFileL(aFileName);
	iPlayOnInit = aPlayOnInit;
	iState = ESoundEngineOpening;
}

const TFileName& CSoundEngine::LastFileName()
{
	return iLastOpenedFileName;
}

TTimeIntervalMicroSeconds CSoundEngine::Position()
{
	TTimeIntervalMicroSeconds pos = 0;
	
	if(iState > ESoundEngineOpening)
	{
		iPlayer->GetPosition(pos);
	}

	// store maximum position, we need this if we get interrupted by a phone call
	if (pos > iMaxPos) {
		iMaxPos = pos;
	}
	return iMaxPos;
}

void CSoundEngine::SetPosition(TUint aPos)
{
	if(iState > ESoundEngineOpening)
	{
		TTimeIntervalMicroSeconds pos = ((TUint64)aPos)*1000000;
		if(iState == ESoundEnginePlaying)
		{
			iPlayer->Pause();
		}

		iMaxPos = pos;
		iPlayer->SetPosition(pos);
		
		if(iState == ESoundEnginePlaying)
		{
			iPlayer->Play();
		}
	}
}


TUint CSoundEngine::PlayTime()
{
	if(iState > ESoundEngineOpening)
	{
		return iPlayer->Duration().Int64()/1000000;
	}

	return 0;
}


void CSoundEngine::Play()
{
	if(iState > ESoundEngineOpening)
	{
		/*TUint skipBack = 5000000; // 5 seconds
		TTimeIntervalMicroSeconds newPos;
		
		newPos = (iMaxPos.Int64() - skipBack > 0 ? iMaxPos.Int64() - skipBack : 0) ;
		iMaxPos = newPos;*/
		iPlayer->SetPosition(iMaxPos);
		iPlayer->Play();
		iState = ESoundEnginePlaying;

		NotifyPlaybackStarted();
	}
}

void CSoundEngine::Stop(TBool aMarkPlayed)
{
	if(iState > ESoundEngineOpening)
	{
		if (aMarkPlayed) {
			// seem to need to do this here, even though we do it in MapcPlayComplete
			iPodcastModel.PlayingPodcast()->SetPlayState(EPlayed);
		}
		iState = ESoundEngineStopped;
		SetPosition(0);
		iPlayer->Stop();
		iMaxPos = 0;
	}
}

void CSoundEngine::Pause(TBool aOverrideState)
{
	DP("Pause");
	if(iState > ESoundEngineOpening || aOverrideState)
	{
		iState = ESoundEnginePaused;
		iPlayer->Pause();

		// had a crash here, so we check for NULL first
		if (iPodcastModel.PlayingPodcast() != NULL) {
			iPodcastModel.PlayingPodcast()->SetPosition(iMaxPos);
			// really wasteful saving EVERYTHING every time
			//TRAP_IGNORE(iPodcastModel.ShowEngine().SaveShowsL());
		}
	}
}

TSoundEngineState CSoundEngine::State()
{
	return iState;
}

void CSoundEngine::SetVolume(TUint aVolume)
{
	if(iState > ESoundEngineOpening)
		{
			iPlayer->SetVolume((aVolume*iPlayer->MaxVolume()) / 100);
		}
}

void CSoundEngine::NotifyPlaybackStopped()
	{
	for (int i=0;i<iObservers.Count();i++) {
		TRAPD(err, iObservers[i]->PlaybackStoppedL());
	}
	
	}


void CSoundEngine::NotifyPlaybackStarted()
	{
	for (int i=0;i<iObservers.Count();i++) {
		TRAPD(err, iObservers[i]->PlaybackStartedL());
	}
	
	}

void CSoundEngine::NotifyPlaybackInitialized()
	{
	for (int i=0;i<iObservers.Count();i++) {
		TRAPD(err, iObservers[i]->PlaybackInitializedL());
	}
	
	}
