#ifndef SETTINGSENGINE_H_
#define SETTINGSENGINE_H_

#include <e32base.h>
#include "PodcastModel.h"

_LIT(KInternalPodcastDir, "c:\\Media files\\Podcasts\\");
_LIT(KFlashPodcastDir, "\\Podcasts\\");

_LIT(KConfigImportFile, "config.cfg");
_LIT(KDefaultFeedsFile, "defaultfeeds.xml");
_LIT(KImportFeedsFile, "feeds.xml");

_LIT(KConfigFile, "config.db");
_LIT(KShowDB, "shows.db");
_LIT(KFeedDB, "feeds.db");
_LIT(KBookDB, "books.db");
_LIT(KEscarpodDB, "escarpod.sqlite");

// constants
const TInt KSettingsUid = 1000;
const TInt KMaxVolume = 100;
const TInt KVolumeDelta = 10;
const TInt KDefaultUpdateFeedInterval = 60;
const TInt KDefaultMaxSimultaneousDownloads = 1;
const TInt KDefaultMaxListItems = 100;
const TInt KDefaultSeekTime = 15;

enum TAutoUpdateSetting
	{
	EAutoUpdateOff,
	EAutoUpdatePeriodically,
	EAutoUpdateAtTime
	};

class CSettingsEngine : public CBase
	{
	public:
		static CSettingsEngine* NewL(CPodcastModel& aPodcastModel);
		virtual ~CSettingsEngine();
	

		TFileName DefaultFeedsFileName();
		TFileName ImportFeedsFileName();
		TFileName PrivatePath();

		TInt MaxListItems();

		TFileName& BaseDir();
		void SetBaseDir(TFileName& aFileName);

		TInt UpdateFeedInterval();
		void SetUpdateFeedInterval(TInt aInterval);

		TInt MaxSimultaneousDownloads();
		void SetMaxSimultaneousDownloads(TInt aMaxDownloads);
		
		TAutoUpdateSetting UpdateAutomatically();	
		void SetUpdateAutomatically(TAutoUpdateSetting aAutoOn);
		
		TBool DownloadAutomatically();
		void SetDownloadAutomatically(TBool aAutoDownloadOn);
		
		TTime UpdateFeedTime();
		void SetUpdateFeedTime(TTime aTime);
		
		TInt SpecificIAP();
		void SetSpecificIAP(TInt aIap);
		
		TInt Volume();
		void SetVolume(TInt aVolume);

		TInt SeekStepTime();
		void SetSeekStepTimek(TInt aSeekStepTime);

		void SetSelectUnplayedOnly(TBool aOnlyUnplayed);
		TBool SelectUnplayedOnly();

		void SaveSettingsL();


	private:
		CSettingsEngine(CPodcastModel& aPodcastModel);
		void ConstructL();
		void ImportSettings();
		void LoadSettingsL();
		void GetDefaultBaseDirL(TDes &aBaseDir);


	private:
		// the settings we serialize
		TFileName iBaseDir;
		TInt iUpdateFeedInterval;
		TAutoUpdateSetting iUpdateAutomatically;
		TBool iDownloadAutomatically;
		TInt iMaxSimultaneousDownloads;
		TInt iIap;
		TInt iVolume;
		TInt iMaxListItems;
		TTime iUpdateFeedTime;
		TInt iSeekStepTime;

		// Other member variables
		RFs iFs;						// File system
		CPodcastModel &iPodcastModel; 	// reference to the model

	    TBool iSelectOnlyUnplayed;
	};

#endif /*SETTINGSENGINE_H_*/
