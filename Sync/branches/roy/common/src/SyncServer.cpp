#include "SyncServer.h"
#include "CSyncServerSession.h"
#include "RSyncServerSession.h"
#include "SyncServerData.h"
#include <f32file.h>
#include <BAUTILS.H>
#include <S32FILE.H>
#include <e32base.h>
#include "debug.h"

CSyncServer::CSyncServer(CActive::TPriority aActiveObjectPriority) 
	: CServer2(aActiveObjectPriority) { }

CSession2* CSyncServer::NewSessionL(const TVersion& /*aVersion*/, const RMessage2& /*aMessage*/) const
{
	return new (ELeave) CSyncServerSession;
}

void CSyncServer::PanicServer(TSyncServerPanic aPanic)
{
    _LIT(KTxtServerPanic,"Sync server panic");
    User::Panic(KTxtServerPanic,aPanic);
}

void CSyncServer::RunL() {
	CServer2::RunL();
}
TInt CSyncServer::ThreadFunction(TAny * /*aStarted */)
{
	DP("ThreadFunction BEGIN");
	CTrapCleanup* cleanup=CTrapCleanup::New();
	if (cleanup == NULL)
	 {
	    CSyncServer::PanicServer(ECreateTrapCleanup);
	 }

	CActiveScheduler *pA=new CActiveScheduler;
	__ASSERT_ALWAYS(pA!=NULL,CSyncServer::PanicServer(EMainSchedulerError));
	CActiveScheduler::Install(pA);
	
	CSyncServer *pS = new CSyncServer(EPriorityHigh);
	
    TRAPD(error, pS->LoadSettingsL());	
    if (error != KErrNone)
    {
        	DP1("LoadSettingsL error %d", error);
            CSyncServer::PanicServer(EStartServer);
    }

    error = pS->Start(KSyncServerName);
    if (error != KErrNone)
    {
    	DP("pS->Start error");
        CSyncServer::PanicServer(EStartServer);
    }
    
    
    RSemaphore s;
	error = s.OpenGlobal(KSyncServerSemaphore);
	if (error == KErrNone) {
		DP("Signalling semaphore");
		s.Signal();
	}
	s.Close();
	
	DP("CActiveScheduler::Start");
	CActiveScheduler::Start();
	DP("CActiveScheduler::Start done");
    delete pS;
    delete pA;
    delete cleanup;
	DP("ThreadFunction END");
    return KErrNone;
}

void CSyncServer::LoadSettingsL() {
	DP("LoadSettings");
	RFs fsSession;
	int error = fsSession.Connect(); 
	DP1("RFs error: %d", error);
	TFileName path;
	TParse	filestorename;
	
	TBuf<100> privatePath;
	fsSession.PrivatePath(privatePath);
	DP1("PrivatePath: %S", &privatePath);
	BaflUtils::EnsurePathExistsL(fsSession, privatePath);
	privatePath.Append(KConfigFileName);
	DP1("File: %S", &privatePath);
	fsSession.Parse(privatePath, filestorename);

	if (!BaflUtils::FileExists(fsSession, privatePath)) {
		DP("No config file");	
		return;
	}
	
	CFileStore* store;
	TRAP(error, store = CDirectFileStore::OpenL(fsSession,filestorename.FullName(),EFileRead));
	CleanupStack::PushL(store);
	
	if (error != KErrNone) {
		DP1("error=%d", error);
		fsSession.Close();
		CleanupStack::Pop(store);
		return;
	}
	
	RStoreReadStream instream;
	instream.OpenLC(*store, store->Root());

	int count = instream.ReadInt32L();
	DP1("Read count: %d", count);
	for (int i=0;i<count;i++) {
		CSyncServerData readData;
		instream  >> readData;	
		timerArray.Append(readData);
		DP3("Read settings %d: profile %d with period %d", i, readData.profileId, readData.period);
		SetTimer(readData.profileId, readData.period, readData.day, readData.hour, readData.minute);
	}
	DP("Read all accounts");
	CleanupStack::PopAndDestroy(); // instream
	fsSession.Close();
	CleanupStack::Pop(store);
}

void CSyncServer::SaveSettings() 
{
	DP("SaveSettings");
	RFs fsSession;
	fsSession.Connect(); 
	TFileName path;
	TParse	filestorename;

	TBuf<100> privatePath;
	fsSession.PrivatePath(privatePath);
	BaflUtils::EnsurePathExistsL(fsSession, privatePath);
	privatePath.Append(KConfigFileName);
	DP1("File: %S", &privatePath);
	fsSession.Parse(privatePath, filestorename);
	CFileStore* store = CDirectFileStore::ReplaceLC(fsSession, filestorename.FullName(), EFileWrite);
	store->SetTypeL(KDirectFileStoreLayoutUid);
	
	RStoreWriteStream outstream;
	TStreamId id = outstream.CreateLC(*store);
	
	outstream.WriteInt32L(timerArray.Count());
	DP1("Storing %i accounts", timerArray.Count());
	for (int i=0;i<timerArray.Count();i++) {
		DP1("Storing account %i", i);
		outstream  << timerArray[i];
		DP("Account stored");
	}
	
	outstream.CommitL();
	store->SetRootL(id);
	store->CommitL();
	CleanupStack::PopAndDestroy(); // outstream
	fsSession.Close();
	
	CleanupStack::Pop(store);
}

// Finds the profile from the timerArray
void CSyncServer::SetTimer(TSmlProfileId profileId, TSyncServerPeriod period, TDay day, TInt hour, TInt minute)
	{
	DP1("SetTimer for profile %d", profileId);

/*
 * 	CSyncServerData holds:
 * 	profileId
	period = (TSyncServerPeriod)
	timer
 */
	CSyncServerData* sData = NULL;

	for (int i=0;i<timerArray.Count();i++) {
		if (timerArray[i].profileId == profileId) {
			DP1("Found profile %d", profileId);
			sData = &timerArray[i];
		}
	}
	
	// sData will be NULL if this is the first time we've set
	// the timing information for this Sync Profile
	if (sData == NULL) {
		DP1("Creating new sync data for profile %d", profileId);
		sData = new CSyncServerData();
		sData->timer = new CSyncServerTimer(profileId);
		sData->timer->ConstructL();
		sData->profileId = profileId;
		sData->period = period;
		sData->day = day;
		sData->hour = hour;
		sData->minute = minute;
		
		if (period != ENever) {
			DP2("Starting timer for profile %d, period=%d", profileId, period);
			
			// thePeriod = aPeriod;
			// according to period, which is an enum
			// it calculates the next run-time for the timer/sync
			sData->timer->SetPeriod(period);
			sData->timer->SetDay(day);	
			sData->timer->SetHour(hour);
			sData->timer->SetMinute(minute);
			sData->timer->RunPeriodically();
		}
		timerArray.Append(*sData);
	} else 
		{
		// In case sData is NULL we create a new timer for it
		// set the period and run it, just as we would do if sData were not NULL
		if (sData->timer == NULL) {
			DP1("Creating new timer for profile %d", profileId);
			sData->timer = new CSyncServerTimer(profileId);
			sData->timer->ConstructL();
		} else if (sData->timer != NULL) {
			DP1("Stopping existing timer for profile %d", profileId);
			sData->timer->Cancel();
		}

		sData->profileId = profileId;
		sData->period = period;
		sData->day = day;
		sData->hour = hour;
		sData->minute = minute;
		
		if (period != ENever) {
			DP2("Starting timer for profile %d, period=%d", profileId, period);
			sData->timer->SetPeriod(period);
			sData->timer->SetDay(day);	
			sData->timer->SetHour(hour);
			sData->timer->SetMinute(minute);
			sData->timer->RunPeriodically();
		}

	}
}

TSyncProfileDetails CSyncServer::GetTimer(TSmlProfileId profileId)
{
	TSyncProfileDetails profDetalis;
	profDetalis.period = ENever;
	
	for (int i=0;i<timerArray.Count();i++)
		{
			if (timerArray[i].profileId == profileId) {
				DP2("Found profile %d with period %d", profileId, timerArray[i].period);
				
				profDetalis.period = timerArray[i].period;
				profDetalis.day = timerArray[i].day;
				profDetalis.hour = timerArray[i].hour;
				profDetalis.minute = timerArray[i].minute;
				
				return profDetalis;
			}
		}
	
	return profDetalis;
}

GLDEF_C TInt E32Main()
{
	DP("SyncServer: Starting");

	CTrapCleanup* cleanup=CTrapCleanup::New();
	TInt r=KErrNoMemory;
	if (cleanup)
		{
		TRAP(r,CSyncServer::ThreadFunction(NULL););
		delete cleanup;
		}
	__UHEAP_MARKEND;
	DP1("SyncServer: Exiting, error=%d", r);	
	return r;
}

