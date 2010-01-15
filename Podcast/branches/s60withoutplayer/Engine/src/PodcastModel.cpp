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
#include <bautils.h>

const TInt KDefaultGranu = 5;
_LIT(KDBFileName, "escarpod.sqlite");
_LIT(KDBTemplateFileName, "escarpod.sqlite.template");

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
	iConnection.Close();
	iSocketServ.Close();	
	sqlite3_close(iDB);
	iFsSession.Close();
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
	iFeedEngine = CFeedEngine::NewL(*this);
	iShowEngine = CShowEngine::NewL(*this);

	iSoundEngine = CSoundEngine::NewL(*this);

	User::LeaveIfError(iSocketServ.Connect());
	User::LeaveIfError(iConnection.Open(iSocketServ));
}

void CPodcastModel::UpdateIAPListL()
{
	iIapNameArray->Reset();
	iIapIdArray.Reset();
    //TUint32 bearerset = KCommDbBearerWcdma | KCommDbBearerWLAN | KCommDbBearerLAN;
    //TCommDbConnectionDirection connDirection = ECommDbConnectionDirectionOutgoing;

    //CCommsDbTableView* table = iCommDB->OpenIAPTableViewMatchingBearerSetLC(bearerset, connDirection);

	CCommsDbTableView* table = iCommDB->OpenTableLC (TPtrC (IAP)); 
	TInt ret = table->GotoFirstRecord ();
	TPodcastIAPItem IAPItem;
	TBuf <KCommsDbSvrMaxFieldLength> bufName;
	while (ret == KErrNone) // There was a first record
	{
		table->ReadUintL(TPtrC(COMMDB_ID), IAPItem.iIapId);
		table->ReadTextL (TPtrC(COMMDB_NAME), bufName);
		table->ReadTextL (TPtrC(IAP_BEARER_TYPE), IAPItem.iBearerType);
		table->ReadTextL (TPtrC(IAP_SERVICE_TYPE), IAPItem.iServiceType);

		iIapIdArray.Append(IAPItem);
		iIapNameArray->AppendL(bufName); 
		//iIapNameArray->AppendL(IAPItem.iBearerType); 
		ret = table->GotoNextRecord();
	}
	CleanupStack::PopAndDestroy(); // Close table
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

RConnection& CPodcastModel::Connection()
{
	return iConnection;
}

TConnPref& CPodcastModel::ConnPref()
{
	return iConnPref;
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
			User::Panic(_L("Escarpod"), 10);
		}		

	}
	return iDB;
}


class CConnectionWaiter:public CActive
{
public:
	CConnectionWaiter():CActive(0)
	{
		CActiveScheduler::Add(this);
		iStatus = KRequestPending;
		SetActive();
	}

	~CConnectionWaiter()
	{
		Cancel();
	}

	void DoCancel()
	{
		TRequestStatus* status = &iStatus;
		User::RequestComplete(status, KErrCancel);
	}

	void RunL()
	{
		CActiveScheduler::Stop();
	}
};

TBool CPodcastModel::ConnectHttpSessionL(RHTTPSession &aSession)
{
	DP("ConnectHttpSessionL START");
	iConnection.Stop();
	/*if(iSettingsEngine->SpecificIAP() == -1)
	{
		TInt iapSelected = iConnPref.IapId();
		/*CPodcastClientIAPDlg* selectIAPDlg = new (ELeave) CPodcastClientIAPDlg(*this, iapSelected);
		if(selectIAPDlg->ExecuteLD(R_PODCAST_IAP_DLG))
		{
			iConnPref.SetIapId(iapSelected);
		}
		else
		{
			return EFalse;
		}
	}*/

	CConnectionWaiter* connectionWaiter = new (ELeave) CConnectionWaiter;
	
	iConnection.Start(iConnPref, connectionWaiter->iStatus);
	CActiveScheduler::Start();
	TInt result = connectionWaiter->iStatus.Int();
	delete connectionWaiter;
	User::LeaveIfError(result);

	RHTTPConnectionInfo connInfo = aSession.ConnectionInfo();
	RStringPool pool = aSession.StringPool();
	// Attach to socket server
	connInfo.SetPropertyL(pool.StringF(HTTP::EHttpSocketServ, RHTTPSession::GetTable()), THTTPHdrVal(iSocketServ.Handle()));
	// Attach to connection
	TInt connPtr = REINTERPRET_CAST(TInt, &iConnection);
	connInfo.SetPropertyL(pool.StringF(HTTP::EHttpSocketConnection, RHTTPSession::GetTable()), THTTPHdrVal(connPtr));
	
	
	SetProxyUsageIfNeededL(aSession);

	
	
	DP("ConnectHttpSessionL END");
	return ETrue;
}

EXPORT_C void CPodcastModel::SetIap(TInt aIap)
	{
	if (aIap == -1) {
		iConnPref.SetDialogPreference(ECommDbDialogPrefDoNotPrompt);
		iConnPref.SetDirection(ECommDbConnectionDirectionOutgoing);
		iConnPref.SetIapId(0);
	} else {
		iConnPref.SetDialogPreference(ECommDbDialogPrefDoNotPrompt);
		iConnPref.SetDirection(ECommDbConnectionDirectionOutgoing);
		iConnPref.SetIapId(aIap);
	}
	
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
	iConnection.GetIntSetting(KSetting, iapId);
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
			iShowEngine->UpdateShow(iActiveShowList[i]);
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

