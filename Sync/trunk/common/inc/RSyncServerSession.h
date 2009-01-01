#ifndef RSYNCSERVERSESSION_H_
#define RSYNCSERVERSESSION_H_

#include <e32std.h>
#include "CSyncServerSession.h"

_LIT(KSyncServerExe, "\\sys\\bin\\SyncServer_0xE0983802.exe");
_LIT(KSyncServerSemaphore, "KSyncServerSemaphore");

class RSyncServerSession : public RSessionBase
{
public:
	TInt Connect();
	void SetTimer(TSmlProfileId profileId, TSyncServerPeriod period);
	TSyncServerPeriod GetTimer(TSmlProfileId profileId);
	
private:
	RThread iServerThread;
};

#endif
