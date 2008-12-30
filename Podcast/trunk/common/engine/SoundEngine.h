#ifndef SOUND_ENGINE_H
#define SOUND_ENGINE_H
#include <e32base.h>
#include <mdaaudiosampleplayer.h>
#include "TelephonyListener.h"

class CPodcastModel;

const TInt KVolumeSteps = 10;

enum TSoundEngineState
{
	ESoundEngineNotInitialized,
	ESoundEngineOpening,
	ESoundEnginePlaying,
	ESoundEnginePaused,
	ESoundEngineStopped
};

class MSoundEngineObserver
{
public:
	virtual void PlaybackInitializedL() = 0;

	virtual void PlaybackStartedL() = 0;
	virtual void PlaybackStoppedL() = 0;
};


/**
 * This class handles all playback and audio associated resources that the application/client needs
 */
class CSoundEngine : public CBase, public MMdaAudioPlayerCallback
{
public:
	static CSoundEngine* NewL(CPodcastModel& aPodcastModel);
	~CSoundEngine();
	void OpenFileL(const TDesC& aFileName, TBool aPlayOnInit);
	void Play();
	void Stop(TBool aMarkPlayed=ETrue);
	TTimeIntervalMicroSeconds Position();
	void SetPosition(TUint aPos);

	/**
	 * Returns the playtime/duration in seconds 
	 * @return Value in a TUint
	 */
	TUint PlayTime();
	void Pause(TBool aOverrideState = EFalse);

	TSoundEngineState State();
	void AddObserver(MSoundEngineObserver* aObserver);
	void RemoveObserver(MSoundEngineObserver* aObserver);
	
	void VolumeUp();
	void VolumeDown();
	void SetVolume(TUint aVolume);
	const TFileName& LastFileName();
	
private:
	void NotifyPlaybackStarted();
	void NotifyPlaybackStopped();
	void NotifyPlaybackInitialized();
	
protected:
	CSoundEngine(CPodcastModel& aPodcastModel);
	void ConstructL();
	void MapcPlayComplete(TInt aError);
	void MapcInitComplete(TInt aError, const TTimeIntervalMicroSeconds &aDuration);
private:
    CMdaAudioPlayerUtility *iPlayer;
	CPodcastModel& iPodcastModel;
	TSoundEngineState iState;
	RArray<MSoundEngineObserver*> iObservers;
	TFileName	iLastOpenedFileName;
	TBool iPlayOnInit;
	TTimeIntervalMicroSeconds iMaxPos;
};

#endif // SOUND_ENGINE_H

