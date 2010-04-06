// INCLUDE FILES
#include "SyncClientAppUi.h"
#include "SyncClientDocument.h"

// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CSyncClientDocument::NewL()
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
CSyncClientDocument* CSyncClientDocument::NewL( CEikApplication&
                                                          aApp )
    {
    CSyncClientDocument* self = NewLC( aApp );
    CleanupStack::Pop( self );
    return self;
    }

// -----------------------------------------------------------------------------
// CSyncClientDocument::NewLC()
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
CSyncClientDocument* CSyncClientDocument::NewLC( CEikApplication&
                                                           aApp )
    {
    CSyncClientDocument* self =
        new ( ELeave ) CSyncClientDocument( aApp );

    CleanupStack::PushL( self );
    self->ConstructL();
    return self;
    }

// -----------------------------------------------------------------------------
// CSyncClientDocument::ConstructL()
// Symbian 2nd phase constructor can leave.
// -----------------------------------------------------------------------------
//
void CSyncClientDocument::ConstructL()
    {
    // No implementation required
    }

// -----------------------------------------------------------------------------
// CSyncClientDocument::CSyncClientDocument()
// C++ default constructor can NOT contain any code, that might leave.
// -----------------------------------------------------------------------------
//
CSyncClientDocument::CSyncClientDocument( CEikApplication& aApp )
    : CAknDocument( aApp )
    {
    // No implementation required
    }

// ---------------------------------------------------------------------------
// CSyncClientDocument::~CSyncClientDocument()
// Destructor.
// ---------------------------------------------------------------------------
//
CSyncClientDocument::~CSyncClientDocument()
    {
    // No implementation required
    }

// ---------------------------------------------------------------------------
// CSyncClientDocument::CreateAppUiL()
// Constructs CreateAppUi.
// ---------------------------------------------------------------------------
//
CEikAppUi* CSyncClientDocument::CreateAppUiL()
    {
    // Create the application user interface, and return a pointer to it;
    // the framework takes ownership of this object
    return ( static_cast <CEikAppUi*> ( new ( ELeave )
                                        CSyncClientAppUi ) );
    }

// End of File

