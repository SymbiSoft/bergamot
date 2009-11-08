#include <bautils.h>
#include <s32file.h>

#include "SettingsEngine.h"
#include "SoundEngine.h"
#include "FeedEngine.h"
#include "ShowEngine.h"
const TUid KMainSettingsStoreUid = {0x1000};
const TUid KMainSettingsUid = {0x1002};
const TUid KExtraSettingsUid = {0x2001};
CSettingsEngine::CSettingsEngine(CPodcastModel& aPodcastModel) : iPodcastModel(aPodcastModel)
	{
		iSelectOnlyUnplayed = ETrue;
	}

CSettingsEngine::~CSettingsEngine()
	{
	TRAP_IGNORE(SaveSettingsL());
	iFs.Close();
	}

CSettingsEngine* CSettingsEngine::NewL(CPodcastModel& aPodcastModel)
	{
	CSettingsEngine* self = new (ELeave) CSettingsEngine(aPodcastModel);
	CleanupStack::PushL(self);
	self->ConstructL();
	CleanupStack::Pop();
	return self;
	}

void CSettingsEngine::ConstructL()
	{
	// default values
	iVolume = KMaxVolume;
	iUpdateFeedInterval = KDefaultUpdateFeedInterval;
	iMaxSimultaneousDownloads = KDefaultMaxSimultaneousDownloads;
	iDownloadAutomatically = EFalse;
	iUpdateAutomatically = EAutoUpdateOff;
	iMaxListItems = KDefaultMaxListItems;
	iIap = 0;
	iSeekStepTime = KDefaultSeekTime;
	// Connect to file system		
	iFs.Connect();
	
	// Check that our basedir exist. Create it otherwise;
	GetDefaultBaseDirL(iBaseDir);
	DP1("Base dir: %S", &iBaseDir);
	
	// load settings
	TRAPD(loadErr, LoadSettingsL());
	if (loadErr != KErrNone)
		{
		DP1("CSettingsEngine::ConstructL\tLoadSettingsL returned error=%d", loadErr);
		DP("CSettingsEngine::ConstructL\tImporting default settings instead");
	
		ImportSettings();
		TRAPD(error,SaveSettingsL());
		if (error != KErrNone) 
			{
			DP1("error saving: %d", error);
			}
		}		
	}

void CSettingsEngine::GetDefaultBaseDirL(TDes & /*aBaseDir*/)
	{
	CDesCArray* disks = new(ELeave) CDesCArrayFlat(10);
	CleanupStack::PushL(disks);

	BaflUtils::GetDiskListL(iFs, *disks);
	
	#ifdef __WINS__
		iBaseDir.Copy(KPodcastDir3);
		CleanupStack::PopAndDestroy(disks);
		return;
	#endif
	
	DP1("Disks count: %u", disks->Count());
	
	for (int i=0;i<disks->Count();i++) {
		TPtrC ptr = disks->operator[](i);
		DP2("Disk %u: %S", i, &ptr);
	}

	if (disks->Count() == 1)  // if only one drive, use C:
		{
		iBaseDir.Copy(KPodcastDir3);
		} 
	else // else we use the flash drive
		{
		iBaseDir.Copy(KPodcastDir1);
		DP1("Trying podcast dir '%S'", &iBaseDir);
				
		TRAPD(err, BaflUtils::EnsurePathExistsL(iFs, iBaseDir));
		
		if (err != KErrNone)
			{
			DP("Leave in EnsurePathExistsL");
			iBaseDir.Copy(KPodcastDir2);
			DP1("Trying podcast dir '%S'", &iBaseDir);
			TRAPD(err, BaflUtils::EnsurePathExistsL(iFs, iBaseDir));
			if (err != KErrNone) 
				{
				DP("Leave in EnsurePathExistsL");
				iBaseDir.Copy(KPodcastDir3);
				DP1("Trying podcast dir '%S'", &iBaseDir);
				TRAPD(err, BaflUtils::EnsurePathExistsL(iFs, iBaseDir));
				if (err != KErrNone)
					{
					DP("Leave in EnsurePathExistsL");
					}
				}
			}
		}
	CleanupStack::PopAndDestroy(disks);
	}

void CSettingsEngine::LoadSettingsL()
	{
	DP("CSettingsEngine::LoadSettingsL\t Trying to load settings");
	
	// Create path for the config file
	TFileName configPath;
	configPath.Copy(PrivatePath());
	configPath.Append(KConfigFile);

	DP1("Checking settings file: %S", &configPath);
	
	CDictionaryFileStore* store = CDictionaryFileStore::OpenLC(iFs, configPath, KMainSettingsStoreUid);

	if( store->IsPresentL(KMainSettingsUid) )
	{
		RDictionaryReadStream stream;
		stream.OpenLC(*store, KMainSettingsUid);
		
		TInt len = stream.ReadInt32L();
		stream.ReadL(iBaseDir, len);
		iUpdateFeedInterval = stream.ReadInt32L();
		iUpdateAutomatically = static_cast<TAutoUpdateSetting>(stream.ReadInt32L());
		iDownloadAutomatically = stream.ReadInt32L();
		
		iMaxSimultaneousDownloads = stream.ReadInt32L();
		iIap = stream.ReadInt32L();
		iPodcastModel.SetIap(iIap);
		
		TInt low = stream.ReadInt32L();
		TInt high = stream.ReadInt32L();
		iUpdateFeedTime = MAKE_TINT64(high, low);
					
		iSelectOnlyUnplayed = stream.ReadInt32L();
		iSeekStepTime = stream.ReadInt32L();
		
		TInt zoomState;
		TRAPD(err, zoomState = stream.ReadInt32L());
		if (err == KErrNone) {
			iPodcastModel.SetZoomState(zoomState);
		}
		CleanupStack::PopAndDestroy(1); // readStream and iniFile
		DP("CSettingsEngine::LoadSettingsL\t Settings loaded OK");
	}
	CleanupStack::PopAndDestroy();// close store
}

void CSettingsEngine::SaveSettingsL()
	{
	DP("CSettingsEngine::SaveSettingsL\tTrying to save settings");

	TFileName configPath;
	configPath.Copy(PrivatePath());
	configPath.Append(KConfigFile);

	CDictionaryFileStore* store = CDictionaryFileStore::OpenLC(iFs, configPath, KMainSettingsStoreUid);

	RDictionaryWriteStream stream;
	stream.AssignLC(*store, KMainSettingsUid);
		
	stream.WriteInt32L(iBaseDir.Length());
	stream.WriteL(iBaseDir);
	stream.WriteInt32L(iUpdateFeedInterval);
	stream.WriteInt32L(iUpdateAutomatically);
	stream.WriteInt32L(iDownloadAutomatically);
	stream.WriteInt32L(iMaxSimultaneousDownloads);
	stream.WriteInt32L(iIap);
	
	stream.WriteInt32L(I64LOW(iUpdateFeedTime.Int64()));
	stream.WriteInt32L(I64HIGH(iUpdateFeedTime.Int64()));
	stream.WriteInt32L(iSelectOnlyUnplayed);
	stream.WriteInt32L(iSeekStepTime);

	stream.WriteInt32L(iPodcastModel.ZoomState());
	stream.CommitL();
	store->CommitL();
	CleanupStack::PopAndDestroy(2); // stream and store
	}

void CSettingsEngine::ImportSettings()
	{
	DP("CSettingsEngine::ImportSettings");

	TFileName configPath;
	configPath.Copy(PrivatePath());
	configPath.Append(KConfigImportFile);
	
	DP1("Importing settings from %S", &configPath);
	
	RFile rfile;
	TInt error = rfile.Open(iFs, configPath,  EFileRead);
	if (error != KErrNone) 
		{
		DP("CSettingsEngine::ImportSettings()\tFailed to read settings");
		return;
		}
	
	TFileText tft;
	tft.Set(rfile);
	
	TBuf<1024> line;
	error = tft.Read(line);
	
	while (error == KErrNone) 
		{
		if (line.Locate('#') == 0) 
			{
			error = tft.Read(line);
			continue;
			}
		
		TInt equalsPos = line.Locate('=');
		if (equalsPos != KErrNotFound) 
			{
			TPtrC tag = line.Left(equalsPos);
			TPtrC value = line.Mid(equalsPos+1);
			DP3("line: %S, tag: '%S', value: '%S'", &line, &tag, &value);
			if (tag.CompareF(_L("BaseDir")) == 0) 
				{
				iBaseDir.Copy(value);
				} 
			else if (tag.CompareF(_L("UpdateFeedIntervalMinutes")) == 0) 
				{
				TLex lex(value);
				lex.Val(iUpdateFeedInterval);
				DP1("Updating automatically every %d minutes", iUpdateFeedInterval);
				} 
			else if (tag.CompareF(_L("DownloadAutomatically")) == 0) 
				{
				TLex lex(value);
				lex.Val((TInt &) iDownloadAutomatically);
				DP1("Download automatically: %d", iDownloadAutomatically);
				} 
			else if (tag.CompareF(_L("MaxSimultaneousDownloads")) == 0) 
				{
				TLex lex(value);
				lex.Val(iMaxSimultaneousDownloads);
				DP1("Max simultaneous downloads: %d", iMaxSimultaneousDownloads);
				}
			else if (tag.CompareF(_L("MaxShowsPerFeed")) == 0) 
				{
				TLex lex(value);
				lex.Val(iMaxListItems);
				DP1("Max shows per feed: %d", iMaxListItems);
				}
			}
		
		error = tft.Read(line);
		}
	rfile.Close();
	}

TFileName CSettingsEngine::DefaultFeedsFileName()
	{
	TFileName defaultFeeds;
	defaultFeeds.Append(PrivatePath());
	defaultFeeds.Append(KDefaultFeedsFile);
	DP1("Default feeds file: %S", &defaultFeeds);
	return defaultFeeds;
	}

TFileName CSettingsEngine::ImportFeedsFileName()
	{
	TFileName importFeeds;
	importFeeds.Append(BaseDir());
	importFeeds.Append(KImportFeedsFile);
	return importFeeds;
	}


TFileName CSettingsEngine::PrivatePath()
	{
	TFileName privatePath;
	iFs.PrivatePath(privatePath);
	TRAP_IGNORE(BaflUtils::EnsurePathExistsL(iFs, privatePath));
	return privatePath;
	}

TInt CSettingsEngine::MaxListItems() 
	{
	return iMaxListItems;
	}

TFileName& CSettingsEngine::BaseDir()
	{
	return iBaseDir;
	}

void CSettingsEngine::SetBaseDir(TFileName& aFileName)
	{
	TInt length = aFileName.Length();
	if (length > 0) 
		{
		if (aFileName[length-1] != '\\') 
			{
			aFileName.Append(_L("\\"));
			}
		}
	iBaseDir = aFileName;
	}

TInt CSettingsEngine::UpdateFeedInterval() 
	{
	return iUpdateFeedInterval;
	}

void CSettingsEngine::SetUpdateFeedInterval(TInt aInterval)
	{
	iUpdateFeedInterval = aInterval;
	iPodcastModel.FeedEngine().RunFeedTimer();
	}

TInt CSettingsEngine::MaxSimultaneousDownloads() 
	{
	return iMaxSimultaneousDownloads;
	}

void CSettingsEngine::SetMaxSimultaneousDownloads(TInt aMaxDownloads)
	{
	iMaxSimultaneousDownloads = aMaxDownloads;
	}

TAutoUpdateSetting CSettingsEngine::UpdateAutomatically() 
	{
	return iUpdateAutomatically;
	}

void CSettingsEngine::SetUpdateAutomatically(TAutoUpdateSetting aAutoSetting)
	{
	iUpdateAutomatically = aAutoSetting;
	}

TBool CSettingsEngine::DownloadAutomatically() 
	{
	return iDownloadAutomatically;
	}

void CSettingsEngine::SetDownloadAutomatically(TBool aDownloadAuto)
	{
	iDownloadAutomatically = aDownloadAuto;
	}

TTime CSettingsEngine::UpdateFeedTime()
	{
	return iUpdateFeedTime;
	}

void CSettingsEngine::SetUpdateFeedTime(TTime aUpdateTime)
	{
	iUpdateFeedTime = aUpdateTime;
	iPodcastModel.FeedEngine().RunFeedTimer();
	}

TInt CSettingsEngine::SpecificIAP()
	{
	return iIap;
	}

void CSettingsEngine::SetSpecificIAP(TInt aIap)
	{
	iIap = aIap;
	}

TInt CSettingsEngine::Volume()
	{
	return iVolume;
	}

void CSettingsEngine::SetVolume(TInt aVolume)
	{
	iVolume = aVolume;
	if(&iPodcastModel.SoundEngine() != NULL) 
		{
		iPodcastModel.SoundEngine().SetVolume(iVolume);
		}
	}

void CSettingsEngine::SetSelectUnplayedOnly(TBool aOnlyUnplayed)
	{
	iSelectOnlyUnplayed = aOnlyUnplayed;
	}

TBool CSettingsEngine::SelectUnplayedOnly()
	{
	return iSelectOnlyUnplayed;
	}

TInt CSettingsEngine::SeekStepTime()
	{
	return iSeekStepTime;
	}

void CSettingsEngine::SetSeekStepTimek(TInt aSeekStepTime)
	{
	iSeekStepTime = aSeekStepTime;
	}
