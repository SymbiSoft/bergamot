/*
* ==============================================================================
*  Name        : helloworldbasicquerydialog.cpp
*  Part of     : helloworldbasicquery
*  Interface   :
*  Description :
*  Version     :
*
*  Copyright (c) 2006 Nokia Corporation.
*  This material, including documentation and any related
*  computer programs, is protected by copyright controlled by
*  Nokia Corporation.
* ==============================================================================
*/

// INCLUDE FILES

#include <avkon.hrh>
#include "HelloWorldBasicQueryDialog.h"

// ================= MEMBER FUNCTIONS =======================

// C++ default constructor can NOT contain any code, that
// might leave.
//
CHelloWorldQueryDialog::CHelloWorldQueryDialog( TDes& aBuf, HBufC *aDefInput )
    : CAknTextQueryDialog( aBuf )
    , iDefInput(*aDefInput)
    {
    }

// ---------------------------------------------------------
// CHelloWorldQueryDialog::PreLayoutDynInitL
// ---------------------------------------------------------
//
void  CHelloWorldQueryDialog::PreLayoutDynInitL()
    {
    // first we have to execute PreLayoutDynInitL() of the base-class
    CAknTextQueryDialog::PreLayoutDynInitL();

    // acquire pointer to editor-control and set the default input.
    CAknQueryControl* control = QueryControl();
    control->SetTextL(iDefInput);

    // enable OK-button, so that default text can be accepted as it is
    // without modifying the text
    MakeLeftSoftkeyVisible( ETrue );
    }

//  End of File

