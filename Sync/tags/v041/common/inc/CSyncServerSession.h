#ifndef CSYNCSERVERSESSION_H_
#define CSYNCSERVERSESSION_H_

#include "SyncServer.h"
#include <e32base.h>

class CSyncServerSession : public CSession2
{
public:
	void ServiceL(const RMessage2& aMessage);
	void PanicClient(const RMessage2& aMessage,TInt aPanic) const;
private:
//	const CSyncServer *iServer;
};

#endif
