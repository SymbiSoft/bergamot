/*
* ==============================================================================
*  Name        : helloworldbasicquerydialog.h
*  Part of     : helloworldbasic
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

#ifndef HELLOWORLDQUERYDIALOG_H
#define HELLOWORLDQUERYDIALOG_H


//  INCLUDES
#include <aknquerydialog.h>

// CLASS DECLARATION

/**
*  CHelloWorldQueryDialog class.
*  This class is used to query the user for text information
*/
class CHelloWorldQueryDialog : public CAknTextQueryDialog
    {
    public:  // Constructors and destructor

        /**
        * C++ default constructor.
        * @param aBuf buffer to store the input
        * @param aDefInput default input for the dialog
        */
        CHelloWorldQueryDialog( TDes& aBuf, HBufC *aDefInput );

        /**
        * Destructor.
        */
        virtual ~CHelloWorldQueryDialog() {};

    private:    // from CEikDialog

        /**
        * sets the default value to the dialog.
        */
        void  PreLayoutDynInitL();

    private:    // Data

       /**
        * the default input for the dialog
        */
        HBufC&  iDefInput;
    };

#endif // HELLOWORLDQUERYDIALOG_H

// End of File

