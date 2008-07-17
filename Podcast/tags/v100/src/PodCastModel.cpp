#include <qikon.hrh>
#include <commdb.h>
#include <podcastclient.rsg>
#include "PodcastModel.h"
#include "FeedEngine.h"
#include "SoundEngine.h"
#include "SettingsEngine.h"
#include "ShowEngine.h"
#include "PodCastClientSettingsDlg.h"

const TInt KDefaultGranu = 5;

CPodcastModel* CPodcastModel::NewL()
{
	CPodcastModel* self = new (ELeave) CPodcastModel;
	CleanupStack::PushL(self);
	self->ConstructL();
	CleanupStack::Pop(self);
	return self;
}

CPodcastModel::~CPodcastModel()
{
#if defined (__WINS__)
#else
	delete iTelephonyListener;
#endif
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
	delete iRemConListener;
}

CPodcastModel::CPodcastModel()
{
	 iZoomState =  EQikCmdZoomLevel2;
}

void CPodcastModel::ConstructL()
{
	iEnv = CEikonEnv::Static();
	iCommDB = CCommsDatabase::NewL (EDatabaseTypeUnspecified);
	//iCommDB ->ShowHiddenRecords(); // magic
	iIapNameArray = new (ELeave) CDesCArrayFlat(KDefaultGranu);

	UpdateIAPListL();

	iSettingsEngine = CSettingsEngine::NewL(*this);
	iFeedEngine = CFeedEngine::NewL(*this);
	iShowEngine = CShowEngine::NewL(*this);

	iSoundEngine = CSoundEngine::NewL(*this);
#if defined (__WINS__)
#else
	iTelephonyListener = CTelephonyListener::NewL(*this);
	iTelephonyListener->StartL();
	// Crashing on WINS
    iRemConListener = CRemoteControlListener::NewL(*this);
#endif	
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

CDesCArrayFlat* CPodcastModel::IAPNames()
{
	return iIapNameArray;
}

RArray<TPodcastIAPItem>& CPodcastModel::IAPIds()
{
	return iIapIdArray;
}



CEikonEnv* CPodcastModel::EikonEnv()
{
	return iEnv;
}

void CPodcastModel::SetPlayingPodcast(CShowInfo* aPodcast)
{
	iPlayingPodcast = aPodcast;
}

CShowInfo* CPodcastModel::PlayingPodcast()
{
	return iPlayingPodcast;
}

CFeedEngine& CPodcastModel::FeedEngine()
{
	return *iFeedEngine;
}
	
CShowEngine& CPodcastModel::ShowEngine()
{
	return *iShowEngine;
}

CSoundEngine& CPodcastModel::SoundEngine()
{
	return *iSoundEngine;
}

CSettingsEngine& CPodcastModel::SettingsEngine()
{
	return *iSettingsEngine;
}

void CPodcastModel::PlayPausePodcastL(CShowInfo* aPodcast, TBool aPlayOnInit) 
	{
	
	// special treatment if this podcast is already active
	if (iPlayingPodcast == aPodcast && SoundEngine().State() > ESoundEngineOpening ) {
		if (aPodcast->PlayState() == EPlaying) {
			SoundEngine().Pause();
			aPodcast->SetPosition(iSoundEngine->Position());
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
			RDebug::Print(_L("Starting: %S"), &(aPodcast->FileName()));
			TRAPD( error, iSoundEngine->OpenFileL(aPodcast->FileName(), aPlayOnInit) );
			if (error != KErrNone) {
				RDebug::Print(_L("Error: %d"), error);
			} else {
				iSoundEngine->SetPosition(aPodcast->Position().Int64() / 1000000);
			}
		}

		iPlayingPodcast = aPodcast;		
	}
}

CFeedInfo* CPodcastModel::ActiveFeedInfo()
{
	return iActiveFeed;
}

void CPodcastModel::SetActiveFeedInfo(CFeedInfo* aFeedInfo)
{
	iActiveFeed = aFeedInfo;
}

RShowInfoArray& CPodcastModel::ActiveShowList()
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


TBool CPodcastModel::SetZoomState(TInt aZoomState)
{
	if(iZoomState != aZoomState)
	{
		iZoomState = aZoomState;
		return ETrue;
	}

	return EFalse;
}

TInt CPodcastModel::ZoomState()
{
	return iZoomState;
}

RConnection& CPodcastModel::Connection()
{
	return iConnection;
}

TConnPref& CPodcastModel::ConnPref()
{
	return iConnPref;
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
	RDebug::Print(_L("ConnectHttpSessionL START"));
	iConnection.Stop();
	if(iSettingsEngine->SpecificIAP() == -1)
	{
		TInt iapSelected = iConnPref.IapId();
		CPodcastClientIAPDlg* selectIAPDlg = new (ELeave) CPodcastClientIAPDlg(*this, iapSelected);
		if(selectIAPDlg->ExecuteLD(R_PODCAST_IAP_DLG))
		{
			iConnPref.SetIapId(iapSelected);
		}
		else
		{
			return EFalse;
		}
	}

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
	
	
	SetProxyUsageIfNeeded(aSession);

	
	
	RDebug::Print(_L("ConnectHttpSessionL END"));
	return ETrue;
}

void CPodcastModel::SetIap(TInt aIap)
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

void CPodcastModel::SetProxyUsageIfNeeded(RHTTPSession& aSession)
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