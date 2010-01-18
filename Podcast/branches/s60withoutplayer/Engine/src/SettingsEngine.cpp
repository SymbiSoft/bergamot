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

#include <bautils.h>
#include <s32file.h>

#include "SettingsEngine.h"
#include "SoundEngine.h"
#include "FeedEngine.h"
#include "ShowEngine.h"
const TUid KMainSettingsStoreUid = {0x1000};
const TUid KMainSettingsUid = {0x1002};
const TUid KExtraSettingsUid = {0x2001};
const TInt KMaxParseBuffer = 1024;

CSettingsEngine::CSettingsEngine(CPodcastModel& aPodcastModel) : iPodcastModel(aPodcastModel)
	{
		iSelectOnlyUnplayed = EFalse;
	}

CSettingsEngine::~CSettingsEngine()
	{
	TRAP_IGNORE(SaveSettingsL());	
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
	
	// Check that our basedir exist. Create it otherwise;
	GetDefaultBaseDirL(iBaseDir);
	DP1("Base dir: %S", &iBaseDir);
	
	// load settings
	TRAPD(loadErr, LoadSettingsL());
	if (loadErr != KErrNone)
		{
		DP1("CSettingsEngine::ConstructL\tLoadSettingsL returned error=%d", loadErr);
		DP("CSettingsEngine::ConstructL\tImporting default settings instead");
	
		ImportSettingsL();
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

	BaflUtils::GetDiskListL(iPodcastModel.FsSession(), *disks);
	
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
				
		TRAPD(err, BaflUtils::EnsurePathExistsL(iPodcastModel.FsSession(), iBaseDir));
		
		if (err != KErrNone)
			{
			DP("Leave in EnsurePathExistsL");
			iBaseDir.Copy(KPodcastDir2);
			DP1("Trying podcast dir '%S'", &iBaseDir);
			TRAPD(err, BaflUtils::EnsurePathExistsL(iPodcastModel.FsSession(), iBaseDir));
			if (err != KErrNone) 
				{
				DP("Leave in EnsurePathExistsL");
				iBaseDir.Copy(KPodcastDir3);
				DP1("Trying podcast dir '%S'", &iBaseDir);
				TRAPD(err, BaflUtils::EnsurePathExistsL(iPodcastModel.FsSession(), iBaseDir));
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
	
	CDictionaryFileStore* store = CDictionaryFileStore::OpenLC(iPodcastModel.FsSession(), configPath, KMainSettingsStoreUid);

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
				
		CleanupStack::PopAndDestroy(1); // readStream and iniFile
		DP("CSettingsEngine::LoadSettingsL\t Settings loaded OK");
	}
	CleanupStack::PopAndDestroy();// close store
}

EXPORT_C void CSettingsEngine::SaveSettingsL()
	{
	DP("CSettingsEngine::SaveSettingsL\tTrying to save settings");

	TFileName configPath;
	configPath.Copy(PrivatePath());
	configPath.Append(KConfigFile);

	CDictionaryFileStore* store = CDictionaryFileStore::OpenLC(iPodcastModel.FsSession(), configPath, KMainSettingsStoreUid);

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

	stream.CommitL();
	store->CommitL();
	CleanupStack::PopAndDestroy(2); // stream and store
	}

void CSettingsEngine::ImportSettingsL()
	{
	DP("CSettingsEngine::ImportSettings");

	TFileName configPath;
	configPath.Copy(PrivatePath());
	configPath.Append(KConfigImportFile);
	
	DP1("Importing settings from %S", &configPath);
	
	RFile rfile;
	TInt error = rfile.Open(iPodcastModel.FsSession(), configPath,  EFileRead);
	if (error != KErrNone) 
		{
		DP("CSettingsEngine::ImportSettings()\tFailed to read settings");
		return;
		}
	CleanupClosePushL(rfile);
	TFileText tft;
	tft.Set(rfile);
	
	HBufC* line = HBufC::NewLC(KMaxParseBuffer);
	TPtr linePtr(line->Des());
	error = tft.Read(linePtr);
	
	while (error == KErrNone) 
		{
		if (line->Locate('#') == 0) 
			{
			error = tft.Read(linePtr);
			continue;
			}
		
		TInt equalsPos = line->Locate('=');
		if (equalsPos != KErrNotFound) 
			{
			TPtrC tag = line->Left(equalsPos);
			TPtrC value = line->Mid(equalsPos+1);
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
		
		error = tft.Read(linePtr);
		}
	CleanupStack::PopAndDestroy(2);//rfile.Close(); & delete buffer
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
	iPodcastModel.FsSession().PrivatePath(privatePath);
	TRAP_IGNORE(BaflUtils::EnsurePathExistsL(iPodcastModel.FsSession(), privatePath));
	return privatePath;
	}

TInt CSettingsEngine::MaxListItems() 
	{
	return iMaxListItems;
	}

EXPORT_C TFileName& CSettingsEngine::BaseDir()
	{
	return iBaseDir;
	}

EXPORT_C void CSettingsEngine::SetBaseDir(TFileName& aFileName)
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

EXPORT_C TInt CSettingsEngine::UpdateFeedInterval() 
	{
	return iUpdateFeedInterval;
	}

EXPORT_C void CSettingsEngine::SetUpdateFeedInterval(TInt aInterval)
	{
	iUpdateFeedInterval = aInterval;
	iPodcastModel.FeedEngine().RunFeedTimer();
	}

EXPORT_C TInt CSettingsEngine::MaxSimultaneousDownloads() 
	{
	return iMaxSimultaneousDownloads;
	}

EXPORT_C void CSettingsEngine::SetMaxSimultaneousDownloads(TInt aMaxDownloads)
	{
	iMaxSimultaneousDownloads = aMaxDownloads;
	}

EXPORT_C TAutoUpdateSetting CSettingsEngine::UpdateAutomatically() 
	{
	return iUpdateAutomatically;
	}

EXPORT_C void CSettingsEngine::SetUpdateAutomatically(TAutoUpdateSetting aAutoSetting)
	{
	iUpdateAutomatically = aAutoSetting;
	}

EXPORT_C TBool CSettingsEngine::DownloadAutomatically() 
	{
	return iDownloadAutomatically;
	}

EXPORT_C void CSettingsEngine::SetDownloadAutomatically(TBool aDownloadAuto)
	{
	iDownloadAutomatically = aDownloadAuto;
	}

EXPORT_C TTime CSettingsEngine::UpdateFeedTime()
	{
	return iUpdateFeedTime;
	}

EXPORT_C void CSettingsEngine::SetUpdateFeedTime(TTime aUpdateTime)
	{
	iUpdateFeedTime = aUpdateTime;
	iPodcastModel.FeedEngine().RunFeedTimer();
	}

EXPORT_C TInt CSettingsEngine::SpecificIAP()
	{
	return iIap;
	}

EXPORT_C void CSettingsEngine::SetSpecificIAP(TInt aIap)
	{
	iIap = aIap;
	}

EXPORT_C void CSettingsEngine::SetSelectUnplayedOnly(TBool aOnlyUnplayed)
	{
	iSelectOnlyUnplayed = aOnlyUnplayed;
	}

EXPORT_C TBool CSettingsEngine::SelectUnplayedOnly()
	{
	return iSelectOnlyUnplayed;
	}

EXPORT_C TInt CSettingsEngine::SeekStepTime()
	{
	return iSeekStepTime;
	}

EXPORT_C void CSettingsEngine::SetSeekStepTimek(TInt aSeekStepTime)
	{
	iSeekStepTime = aSeekStepTime;
	}
