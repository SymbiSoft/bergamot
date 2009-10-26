/*
* ==============================================================================
*  Name        : helloworldbasicapplication.cpp
*  Part of     : Helloworldbasic
*  Interface   : 
*  Description : 
*  Version     : 
*
*  Copyright (c) 2005-2006 Nokia Corporation.
*  This material, including documentation and any related 
*  computer programs, is protected by copyright controlled by 
*  Nokia Corporation.
* ==============================================================================
*/

// INCLUDE FILES
#include "HelloWorldBasicDocument.h"
#include "HelloWorldBasicApplication.h"

// ============================ MEMBER FUNCTIONS ===============================

// UID for the application;
// this should correspond to the uid defined in the mmp file
const TUid KUidHelloWorldBasicApp = { 0xA000017F };

// -----------------------------------------------------------------------------
// CHelloWorldBasicApplication::CreateDocumentL()
// Creates CApaDocument object
// -----------------------------------------------------------------------------
//
CApaDocument* CHelloWorldBasicApplication::CreateDocumentL()
    {
    // Create an HelloWorldBasic document, and return a pointer to it
    return (static_cast<CApaDocument*>
                    ( CHelloWorldBasicDocument::NewL( *this ) ) );
    }

// -----------------------------------------------------------------------------
// CHelloWorldBasicApplication::AppDllUid()
// Returns application UID
// -----------------------------------------------------------------------------
//
TUid CHelloWorldBasicApplication::AppDllUid() const
    {
    // Return the UID for the HelloWorldBasic application
    return KUidHelloWorldBasicApp;
    }

// End of File

