#ifndef RSYNCSERVERSESSION_H_
#define RSYNCSERVERSESSION_H_

#include <e32std.h>
#include "CSyncServerSession.h"

_LIT(KSyncServerExe, "\\sys\\bin\\SyncServer_0x2002C25C.exe");
_LIT(KSyncServerSemaphore, "KSyncServerSemaphore");

class RSyncServerSession : public RSessionBase
{
public:
	TInt Connect();
	void SetTimer(TSmlProfileId profileId, TSyncServerPeriod period, TDay day = ESunday, TInt hour = 0, TInt minute = 0);
	TSyncProfileDetails GetTimer(TSmlProfileId profileId);
	
private:
	RThread iServerThread;
};

#endif
