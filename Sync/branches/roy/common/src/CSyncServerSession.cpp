#include "CSyncServerSession.h"
#include "SyncServerData.h"
#include "debug.h"

// Whenever a change is made in the settings dialog of the Sync Profile
// in Swim, it sends a message to set the timer.

// This in turn, calls srv->SetTimer(profileId, period);
// and srv->SaveSettings() which saves the settings to a config file
void CSyncServerSession::ServiceL(const RMessage2& aMessage) 
{
	CSyncServer* srv = (CSyncServer*) Server();
	switch(aMessage.Function()) {
	case ESetTimer: {
		TSmlProfileId profileId = aMessage.Int0();
		
		/*
		 * enum TSyncServerPeriod
			{
				ENever = 0,
				EFifteenMinutes,
				EOneHour,
				EFourHours,
				ETwelveHours,
				EDaily,
				EWeekly
			};
		 */
		
		// Gets the correct period from the message
		TSyncServerPeriod period = (TSyncServerPeriod) aMessage.Int1();
		TDay day = (TDay) aMessage.Int2();
		
		// minute holds the hour details as well
		TInt minute = aMessage.Int3();
		
		// If minute was 59, and hour was 3.
		// Then it would be 359
		
		// 359 / 100 = 3
		// And minute = 359 - 3*100 = 59
		TInt hour = minute / 100;
		minute -= hour*100;
		
		DP2("SetTimer: profileId=%d, period=%d", profileId, period);
		
		// Sets the timer according to the period
		srv->SetTimer(profileId, period, day, hour, minute);
		srv->SaveSettings();
		break;
	}
	case EGetTimer: {
		TSmlProfileId profileId = aMessage.Int0();
		DP1("GetTimer: profileId=%d", profileId);
		TSyncProfileDetails profDetalis = srv->GetTimer(profileId);
				
	    TPckgBuf<TSyncProfileDetails> p(profDetalis);
	    
	    aMessage.WriteL(1,p);
		break;
	}
	};
	aMessage.Complete(KErrNone);
}

void CSyncServerSession::PanicClient(const RMessage2& aMessage,TInt aPanic) const
{
	_LIT(KTxtServer,"Sync server");
	DP("PanicClient");
	aMessage.Panic(KTxtServer,aPanic);
}
