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

#include <commdb.h>
#include "PodcastModel.h"
#include "FeedEngine.h"
#include "SoundEngine.h"
#include "SettingsEngine.h"
#include "ShowEngine.h"
#include "connectionengine.h"

#include <cmdestination.h>
#include <cmmanager.h>
#include <bautils.h>

const TInt KDefaultGranu = 5;
_LIT(KDBFileName, "podcatcher.sqlite");
_LIT(KDBTemplateFileName, "podcatcher.sqlite.template");

EXPORT_C CPodcastModel* CPodcastModel::NewL()
{
	CPodcastModel* self = new (ELeave) CPodcastModel;
	CleanupStack::PushL(self);
	self->ConstructL();
	CleanupStack::Pop(self);
	return self;
}

CPodcastModel::~CPodcastModel()
{
	iActiveShowList.ResetAndDestroy();
	delete iFeedEngine;
	delete iSoundEngine;
	delete iSettingsEngine;
	iActiveShowList.Close();
	delete iShowEngine;

	delete iIapNameArray;
	iIapIdArray.Close();
	delete iCommDB;	
	sqlite3_close(iDB);
	iFsSession.Close();
	delete iConnectionEngine;
}

CPodcastModel::CPodcastModel()
{	
}

void CPodcastModel::ConstructL()
{
	User::LeaveIfError(iFsSession.Connect());
	
	iCommDB = CCommsDatabase::NewL (EDatabaseTypeUnspecified);
	//iCommDB ->ShowHiddenRecords(); // magic
	iIapNameArray = new (ELeave) CDesCArrayFlat(KDefaultGranu);

	UpdateIAPListL();
	
	iSettingsEngine = CSettingsEngine::NewL(*this);
	iConnectionEngine = CConnectionEngine::NewL(*this);	
	
	iFeedEngine = CFeedEngine::NewL(*this);
	iShowEngine = CShowEngine::NewL(*this);

	iSoundEngine = CSoundEngine::NewL(*this);	
}

void CPodcastModel::UpdateIAPListL()
{
	iIapNameArray->Reset();
	iIapIdArray.Reset();
	RCmManager cmManager;
	RCmDestination destination;
	TPodcastIAPItem IAPItem;
	cmManager.OpenL();
	RArray<TUint32> destArray;
	CleanupClosePushL(destArray);
	cmManager.AllDestinationsL(destArray);
	
	TInt cnt = destArray.Count();
	for(TInt loop = 0;loop<cnt;loop++)
		{
		destination = cmManager.DestinationL (destArray[loop]);
		if(!destination.IsHidden())
			{
			IAPItem.iIapId = destArray[loop];
			HBufC* name = destination.NameLC();
			iIapNameArray->AppendL(*name);
			CleanupStack::PopAndDestroy(name);
			iIapIdArray.Append(IAPItem);
			}
		}
	CleanupStack::PopAndDestroy();// close destArray
#ifndef __WINS__
	cmManager.Close();
#endif
	//CleanupStack::PopAndDestroy();// close destArray, cmMangaer
   /*
	CCommsDbTableView* table = iCommDB->OpenTableLC (TPtrC (NETWORK)); 
	TInt ret = table->GotoFirstRecord ();
	
	TBuf <KCommsDbSvrMaxFieldLength> bufName;
	while (ret == KErrNone) // There was a first record
	{
		table->ReadUintL(TPtrC(COMMDB_ID), IAPItem.iIapId);
		table->ReadTextL (TPtrC(COMMDB_NAME), bufName);		

		iIapIdArray.Append(IAPItem);
		iIapNameArray->AppendL(bufName); 		
		ret = table->GotoNextRecord();
	}
	CleanupStack::PopAndDestroy(); // Close table*/
}

EXPORT_C CDesCArrayFlat* CPodcastModel::IAPNames()
{
	return iIapNameArray;
}

EXPORT_C RArray<TPodcastIAPItem>& CPodcastModel::IAPIds()
{
	return iIapIdArray;
}


RFs& CPodcastModel::FsSession()
	{
	return iFsSession;
	}

EXPORT_C void CPodcastModel::SetPlayingPodcast(CShowInfo* aPodcast)
{
	iPlayingPodcast = aPodcast;
}

EXPORT_C CShowInfo* CPodcastModel::PlayingPodcast()
{
	return iPlayingPodcast;
}

EXPORT_C CFeedEngine& CPodcastModel::FeedEngine()
{
	return *iFeedEngine;
}
	
EXPORT_C CShowEngine& CPodcastModel::ShowEngine()
{
	return *iShowEngine;
}

EXPORT_C CSoundEngine& CPodcastModel::SoundEngine()
{
	return *iSoundEngine;
}

EXPORT_C CSettingsEngine& CPodcastModel::SettingsEngine()
{
	return *iSettingsEngine;
}

EXPORT_C CConnectionEngine& CPodcastModel::ConnectionEngine()
{
	return *iConnectionEngine;
}

EXPORT_C void CPodcastModel::PlayPausePodcastL(CShowInfo* aPodcast, TBool aPlayOnInit) 
	{
	
	// special treatment if this podcast is already active
	if (iPlayingPodcast->Uid() == aPodcast->Uid() && SoundEngine().State() > ESoundEngineOpening ) {
		if (aPodcast->PlayState() == EPlaying) {
			SoundEngine().Pause();
			aPodcast->SetPosition(iSoundEngine->Position());
			aPodcast->SetPlayState(EPlayed);
			aPodcast->SetPlayState(EPlayed);
		} else {
			iSoundEngine->Play();
		}
	} else {
		// switching file, so save position
		iSoundEngine->Pause();
		if (iPlayingPodcast != NULL) {
			iPlayingPodcast->SetPosition(iSoundEngine->Position());
		}
		
		iSoundEngine->Stop(EFalse);

		// we play video podcasts through the external player
		if(aPodcast != NULL && aPodcast->ShowType() != EVideoPodcast) {
			DP1("Starting: %S", &(aPodcast->FileName()));
			TRAPD( error, iSoundEngine->OpenFileL(aPodcast->FileName(), aPlayOnInit) );
			if (error != KErrNone) {
				DP1("Error: %d", error);
			} else {
				iSoundEngine->SetPosition(aPodcast->Position().Int64() / 1000000);
			}
		}

		iPlayingPodcast = aPodcast;		
	}
}

EXPORT_C CFeedInfo* CPodcastModel::ActiveFeedInfo()
{
	return iActiveFeed;
}

EXPORT_C void CPodcastModel::SetActiveFeedInfo(CFeedInfo* aFeedInfo)
{
	iActiveFeed = aFeedInfo;
}

EXPORT_C RShowInfoArray& CPodcastModel::ActiveShowList()
{
	return iActiveShowList;
}

void CPodcastModel::SetActiveShowList(RShowInfoArray& aShowArray)
{
	iActiveShowList.Reset();
	TInt cnt = aShowArray.Count();

	for(TInt loop = 0;loop < cnt; loop++)
	{
		iActiveShowList.Append(aShowArray[loop]);
	}
}

sqlite3* CPodcastModel::DB()
{
	if (iDB == NULL) {		

		TFileName dbFileName;
		iFsSession.PrivatePath(dbFileName);
		dbFileName.Append(KDBFileName);
		DP1("DB is at %S", &dbFileName);
		if (!BaflUtils::FileExists(iFsSession, dbFileName)) {
			TFileName dbTemplate;
			iFsSession.PrivatePath(dbTemplate);
			dbTemplate.Append(KDBTemplateFileName);
			DP1("No DB found, copying template from %S", &dbTemplate);
			BaflUtils::CopyFile(iFsSession, dbTemplate,dbFileName);
		}
		
		TBuf8<KMaxFileName> filename8;
		filename8.Copy(dbFileName);
		int rc = rc = sqlite3_open((const char*) filename8.PtrZ(), &iDB);
		if( rc != SQLITE_OK){
			DP("Error loading DB");
			User::Panic(_L("Podcatcher"), 10);
		}		

	}
	return iDB;
}

void CPodcastModel::SetProxyUsageIfNeededL(RHTTPSession& aSession)
	{
	TBool useProxy = EFalse;
	HBufC* serverName = NULL;
	TUint32 port = 0;
	
	TRAPD(err,GetProxyInformationForConnectionL(useProxy, serverName, port));
	if (err == KErrNone && useProxy)
		{
		CleanupStack::PushL(serverName);
		TBuf8<128> proxyAddr;
		proxyAddr.Append(*serverName);
		proxyAddr.Append(':');
		proxyAddr.AppendNum(port);
				
		RStringF prxAddr = aSession.StringPool().OpenFStringL(proxyAddr);
		CleanupClosePushL(prxAddr);
		THTTPHdrVal prxUsage(aSession.StringPool().StringF(HTTP::EUseProxy,RHTTPSession::GetTable()));

		aSession.ConnectionInfo().SetPropertyL(
						aSession.StringPool().StringF(HTTP::EProxyUsage,RHTTPSession::GetTable()), 
						aSession.StringPool().StringF(HTTP::EUseProxy,RHTTPSession::GetTable()));

		aSession.ConnectionInfo().SetPropertyL(aSession.StringPool().StringF(HTTP::EProxyAddress,RHTTPSession::GetTable()), prxAddr); 
		
		CleanupStack::PopAndDestroy(&prxAddr);
		CleanupStack::PopAndDestroy(serverName);
		}
	}


void CPodcastModel::GetProxyInformationForConnectionL(TBool& aIsUsed, HBufC*& aProxyServerName, TUint32& aPort)
	{
	TInt iapId = GetIapId();
	CCommsDbTableView* table = iCommDB->OpenViewMatchingUintLC(TPtrC(IAP), TPtrC(COMMDB_ID), iapId);
	
	TUint32 iapService;
	HBufC* iapServiceType;
	table->ReadUintL(TPtrC(IAP_SERVICE), iapService);
	iapServiceType = table->ReadLongTextLC(TPtrC(IAP_SERVICE_TYPE));
	
	CCommsDbTableView* proxyTableView = iCommDB->OpenViewOnProxyRecordLC(iapService, *iapServiceType);
	TInt err = proxyTableView->GotoFirstRecord();
	if( err != KErrNone)
		{
		User::Leave(KErrNotFound);	
		}

	proxyTableView->ReadBoolL(TPtrC(PROXY_USE_PROXY_SERVER), aIsUsed);
	if(aIsUsed)
		{
		HBufC* serverName = proxyTableView->ReadLongTextLC(TPtrC(PROXY_SERVER_NAME));
		proxyTableView->ReadUintL(TPtrC(PROXY_PORT_NUMBER), aPort);
		aProxyServerName = serverName->AllocL();
		CleanupStack::PopAndDestroy(serverName);
		}
		
	CleanupStack::PopAndDestroy(proxyTableView);
	CleanupStack::PopAndDestroy(iapServiceType);
	CleanupStack::PopAndDestroy(table);
	}
	
	
TInt CPodcastModel::GetIapId()
	{
	_LIT(KSetting, "IAP\\Id");
	TUint32 iapId = 0;
	iConnectionEngine->Connection().GetIntSetting(KSetting, iapId);
	return iapId;
	}

EXPORT_C void CPodcastModel::GetAllShows()
	{
	iActiveShowList.ResetAndDestroy();
	TRAP_IGNORE(iShowEngine->GetAllShowsL(iActiveShowList));
	}

EXPORT_C void CPodcastModel::GetNewShows()
	{
	iActiveShowList.ResetAndDestroy();
	TRAP_IGNORE(iShowEngine->GetNewShowsL(iActiveShowList));	
	}

EXPORT_C void CPodcastModel::GetShowsDownloaded()
	{
	iActiveShowList.ResetAndDestroy();
	TRAP_IGNORE(iShowEngine->GetShowsDownloadedL(iActiveShowList));
	}

EXPORT_C void CPodcastModel::GetShowsDownloading()
	{
	iActiveShowList.ResetAndDestroy();
	TRAP_IGNORE(iShowEngine->GetShowsDownloadingL(iActiveShowList));
	}

EXPORT_C void CPodcastModel::GetShowsByFeed(TUint aFeedUid)
	{
	iActiveShowList.ResetAndDestroy();
	TRAP_IGNORE(iShowEngine->GetShowsByFeedL(iActiveShowList, aFeedUid));
	}

EXPORT_C void CPodcastModel::MarkSelectionPlayed()
	{
	for (int i=0;i<iActiveShowList.Count();i++) {
		if(iActiveShowList[i]->PlayState() != EPlayed) {
			iActiveShowList[i]->SetPlayState(EPlayed);
			iShowEngine->UpdateShow(*iActiveShowList[i]);
		}
	}
	}

TInt CPodcastModel::FindActiveShowByUid(TUint aUid)
	{
	for (int i=0;i<iActiveShowList.Count();i++) {
		if (iActiveShowList[i]->Uid() == aUid) {
			return i;
		}
	}
	
	return KErrNotFound;
	}

