
// INCLUDE FILES
#include <eikstart.h>
#include "SyncClientApplication.h"


LOCAL_C CApaApplication* NewApplication()
	{
	return new CSyncClientApplication;
	}

GLDEF_C TInt E32Main()
	{
	return EikStart::RunApplication( NewApplication );
	}
