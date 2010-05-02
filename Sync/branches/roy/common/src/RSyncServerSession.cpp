#include "RSyncServerSession.h"
#include "debug.h"

// debug printouts cause freezing in Connect() for some reason

TInt RSyncServerSession::Connect()
{
	DP("RSyncServerSession::Connect");
	int error = CreateSession(KSyncServerName, TVersion(1,0,0), KDefaultMessageSlots);
	
	if (error == KErrNone) {
		DP("Successfully connected to sync server");
	} else {
		DP("Failed to connect to sync server, attempting to start");
		RProcess server;

		RSemaphore s;
		s.CreateGlobal(KSyncServerSemaphore, 0);
		
		error = server.Create(KSyncServerExe, TPtr(NULL,0));
		if (error == KErrNone) {
			DP("Successfully started server");
			server.Resume();
			DP("Waiting for semaphore");
	    	s.Wait();
			DP("Got semaphore");

			error = CreateSession(KSyncServerName, TVersion(1,0,0), KDefaultMessageSlots);
				
			if (error == KErrNone) {
				DP("Successfully connected to sync server");
			} else {
				return error;
			}
	    	server.Close();
		} else {
			DP("Failed to start server");
		}
		s.Close();	
	}
	DP1("RSyncServerSession::Connect returning %d", error);
	return error;
}

// Sends a packaged message to the server
void RSyncServerSession::SetTimer(TSmlProfileId profileId, TSyncServerPeriod period, TDay day, TInt hour, TInt minute)
	{
	//DP("RSyncServerSession::SetTimer BEGIN");
	minute += hour*100;
	TIpcArgs args(profileId, period, day, minute);
	SendReceive(ESetTimer, args);
	//DP("RSyncServerSession::SetTimer END");
	}

TSyncProfileDetails RSyncServerSession::GetTimer(TSmlProfileId profileId)
	{
	//DP("RSyncServerSession::GetTimer BEGIN");
    TPckgBuf<TSyncProfileDetails> pckg;
        
    TIpcArgs args((TInt)profileId, &pckg);
	
	SendReceive(EGetTimer, args);
	
	TSyncProfileDetails res = pckg();
	//DP1("RSyncServerSession::GetTimer END, res=%d", res);
	return res;
	}
