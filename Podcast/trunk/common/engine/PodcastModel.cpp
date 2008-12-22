#include <commdb.h>
#include "PodcastModel.h"
#include "FeedEngine.h"
#include "SoundEngine.h"
#include "SettingsEngine.h"
#include "ShowEngine.h"

const TInt KDefaultGranu = 5;
_LIT(KDBFileName, "escarpod.sqlite");

#define EQikCmdZoomLevel1							0x0800
/** @publishedAll @released */
#define EQikCmdZoomLevel2							0x0801
/** @publishedAll @released */
#define EQikCmdZoomLevel3							0x0802
/** @publishedAll @released */

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
	iActiveShowList.ResetAndDestroy();
	iActiveFeedList.ResetAndDestroy();
	delete iFeedEngine;
	delete iSoundEngine;
	delete iSettingsEngine;
	iActiveShowList.Close();
	iActiveFeedList.Close();
	delete iShowEngine;

	delete iIapNameArray;
	iIapIdArray.Close();
	delete iCommDB;
	iConnection.Close();
	iSocketServ.Close();
	delete iRemConListener;
	sqlite3_close(iDB);
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

void CPodcastModel::SetPlayingShowUid(TUint aShowUid)
{
	iPlayingShowUid = aShowUid;
}

CShowInfo* CPodcastModel::PlayingPodcast()
{
	return iShowEngine->GetShowByUid(iPlayingShowUid);
}

TUint CPodcastModel::PlayingShowUid()
{
	return iPlayingShowUid;
}

CSoundEngine& CPodcastModel::SoundEngine()
{
	return *iSoundEngine;
}

CSettingsEngine& CPodcastModel::SettingsEngine()
{
	return *iSettingsEngine;
}

void CPodcastModel::PlayPausePodcastL(TUint aShowUid, TBool aPlayOnInit) 
	{
	
	// special treatment if this show is already active
	CShowInfo *playing = PlayingPodcast();
	if (iPlayingShowUid == aShowUid && SoundEngine().State() > ESoundEngineOpening ) {

		if (playing->PlayState() == EPlaying) {
			SoundEngine().Pause();
			playing->SetPosition(iSoundEngine->Position());
			playing->SetPlayState(EPlayed);
			iShowEngine->UpdateShow(playing);
		} else {
			iSoundEngine->Play();
		}
	} else {
		// switching file, so save position
		iSoundEngine->Pause();
		if (playing != NULL) {
			playing->SetPosition(iSoundEngine->Position());
		}
		
		iSoundEngine->Stop(EFalse);
		CShowInfo *toBePlayed = iShowEngine->GetShowByUid(aShowUid);
		// we play video podcasts through the external player
		if(toBePlayed != NULL && toBePlayed->ShowType() != EVideoPodcast) {
			DP1("Starting: %S", &(toBePlayed->FileName()));
			TRAPD( error, iSoundEngine->OpenFileL(toBePlayed->FileName(), aPlayOnInit) );
			if (error != KErrNone) {
				DP1("Error: %d", error);
			} else {
				iSoundEngine->SetPosition(toBePlayed->Position().Int64() / 1000000);
				iShowEngine->UpdateShow(toBePlayed);
			}
		}

		iPlayingShowUid = aShowUid;		
	}
}

CFeedInfo* CPodcastModel::ActiveFeedInfo()
{
	return iFeedEngine->GetFeedInfoByUid(iActiveFeedUid);
}

void CPodcastModel::SetActiveFeedUid(TUint aFeedUid)
{
	iActiveFeedUid = aFeedUid;
}

RShowInfoArray& CPodcastModel::ActiveShowList()
{
	return iActiveShowList;
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

sqlite3* CPodcastModel::DB()
{
	if (iDB == NULL) {
		RFs fs;
		fs.Connect();

		TFileName dbFileName;
		fs.PrivatePath(dbFileName);
		dbFileName.Append(KDBFileName);
		DP1("DB is at %S", &dbFileName);
		TBuf8<KMaxFileName> filename8;
		filename8.Copy(dbFileName);
		int rc = rc = sqlite3_open((const char*) filename8.PtrZ(), &iDB);
		if( rc != SQLITE_OK){
			DP("Error loading DB");
			User::Panic(_L("Escarpod"), 10);
		}
		fs.Close();

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
	
	
	SetProxyUsageIfNeeded(aSession);

	
	
	DP("ConnectHttpSessionL END");
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

void CPodcastModel::GetAllShows()
	{
	iActiveShowList.ResetAndDestroy();
	iShowEngine->GetAllShows(iActiveShowList);
	}

void CPodcastModel::GetNewShows()
	{
	iActiveShowList.ResetAndDestroy();
	iShowEngine->GetNewShows(iActiveShowList);	
	}

void CPodcastModel::GetShowsDownloaded()
	{
	iActiveShowList.ResetAndDestroy();
	iShowEngine->GetShowsDownloaded(iActiveShowList);
	}

void CPodcastModel::GetShowsDownloading()
	{
	iActiveShowList.ResetAndDestroy();
	iShowEngine->GetShowsDownloading(iActiveShowList);
	}

void CPodcastModel::GetShowsByFeed(TUint aFeedUid)
	{
	iActiveShowList.ResetAndDestroy();
	iShowEngine->GetShowsByFeed(iActiveShowList, aFeedUid);
	}

void CPodcastModel::MarkSelectionPlayed()
	{
	for (int i=0;i<iActiveShowList.Count();i++) {
		if(iActiveShowList[i]->PlayState() != EPlayed) {
			//iShowEngine->SetShowPlayState(iActiveShowList[i],EPlayed);
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

void CPodcastModel::GetFeedList()
	{
	iActiveFeedList.ResetAndDestroy();
	iFeedEngine->GetFeedsByType(iActiveFeedList, EShowFeed);
	}

void CPodcastModel::GetBookList()
	{
	iActiveFeedList.ResetAndDestroy();
	iFeedEngine->GetFeedsByType(iActiveFeedList, EBookFeed);
	}

RFeedInfoArray& CPodcastModel::ActiveFeedList()
	{
	return iActiveFeedList;
	}

void CPodcastModel::RemoveFeed(TUint aUid) 
	{
	/*CFeedInfo *feedToRemove = GetFeedInfoByUid(aUid);
	
	if (feedToRemove != NULL) {
		iShowEngine.DeleteAllShowsByFeed(aUid);
						
		//delete the image file if it exists
		if ( (feedToRemove->ImageFileName().Length() >0) && BaflUtils::FileExists(iFs, feedToRemove->ImageFileName() ))
			{
			iFs.Delete(feedToRemove->ImageFileName());
			}
			
		//delete the folder. It has the same name as the title.
		TFileName filePath;
		filePath.Copy(iSettingsEngine().BaseDir());
		filePath.Append(feedToRemove->Title());
		filePath.Append(_L("\\"));
		iFs.RmDir(filePath);

		delete feedToRemove;
		
		DP("Removed feed from array");
		
		// now remove it from DB
		iFeedEngine.RemoveFeed(aUid);

		return;
	}*/
}

TUint CPodcastModel::GetShowsDownloadingCount()
	{
	return iShowEngine->GetNumDownloadingShows();
	}

void CPodcastModel::ResumeDownloads()
	{
	iShowEngine->ResumeDownloads();
	}

void CPodcastModel::StopDownloads()
	{
	iShowEngine->StopDownloads();
	}
void CPodcastModel::AddShowEngineObserver(MShowEngineObserver *aObserver)
	{
	iShowEngine->AddObserver(aObserver);
	}
