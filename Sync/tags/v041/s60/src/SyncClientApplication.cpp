// INCLUDE FILES
#include "SyncClientDocument.h"
#include "SyncClientApplication.h"

// ============================ MEMBER FUNCTIONS ===============================

// UID for the application;
// this should correspond to the uid defined in the mmp file
const TUid KUidSyncClientApp = { 0xE0983804 };

// -----------------------------------------------------------------------------
// CSyncClientApplication::CreateDocumentL()
// Creates CApaDocument object
// -----------------------------------------------------------------------------
//
CApaDocument* CSyncClientApplication::CreateDocumentL()
    {
    // Create an SyncClient document, and return a pointer to it
    return (static_cast<CApaDocument*>
                    ( CSyncClientDocument::NewL( *this ) ) );
    }

// -----------------------------------------------------------------------------
// CSyncClientApplication::AppDllUid()
// Returns application UID
// -----------------------------------------------------------------------------
//
TUid CSyncClientApplication::AppDllUid() const
    {
    // Return the UID for the SyncClient application
    return KUidSyncClientApp;
    }

// End of File

