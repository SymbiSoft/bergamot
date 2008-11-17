#include "CSyncServerSession.h"
#include "SyncServerData.h"
#include "debug.h"

void CSyncServerSession::ServiceL(const RMessage2& aMessage) 
{
	CSyncServer* srv = (CSyncServer*) Server();
	switch(aMessage.Function()) {
	case ESetTimer: {
		TSmlProfileId profileId = aMessage.Int0();
		TSyncServerPeriod period = (TSyncServerPeriod) aMessage.Int1();
		DP2("SetTimer: profileId=%d, period=%d", profileId, period);
		srv->SetTimer(profileId, period);
		srv->SaveSettings();
		break;
	}
	case EGetTimer: {
		TSmlProfileId profileId = aMessage.Int0();
		DP1("GetTimer: profileId=%d", profileId);
		TSyncServerPeriod ret = srv->GetTimer(profileId);
		DP1("GetTimer: timer=%d", ret);
	    TPckgBuf<TInt> p(ret);
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
