// INCLUDE FILES
#include <eikstart.h>
#include "PodcastApp.h"

LOCAL_C CApaApplication* NewApplication()
    {
    return new CPodcastApp;
    }

GLDEF_C TInt E32Main()
    {
    return EikStart::RunApplication( NewApplication );
    }
