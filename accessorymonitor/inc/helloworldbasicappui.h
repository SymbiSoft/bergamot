/*
* ==============================================================================
*  Name        : helloworldbasicappui.h
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

#ifndef __HELLOWORLDBASICAPPUI_H__
#define __HELLOWORLDBASICAPPUI_H__

// INCLUDES
#include <aknappui.h>
#include "accessorymonitor.h"

// FORWARD DECLARATIONS
class CHelloWorldBasicAppView;


// CLASS DECLARATION
/**
* CHelloWorldBasicAppUi application UI class.
* Interacts with the user through the UI and request message processing
* from the handler class
*/
class CHelloWorldBasicAppUi : public CAknAppUi, public MAccessoryMonitorCallbacks
    {
    public: // Constructors and destructor

        /**
        * ConstructL.
        * 2nd phase constructor.
        */
        void ConstructL();

        /**
        * CHelloWorldBasicAppUi.
        * C++ default constructor. This needs to be public due to
        * the way the framework constructs the AppUi
        */
        CHelloWorldBasicAppUi();

        /**
        * ~CHelloWorldBasicAppUi.
        * Virtual Destructor.
        */
        virtual ~CHelloWorldBasicAppUi();
protected:
	void AccessoryConnected();
	void AccessoryDisconnected();

    private:  // Functions from base classes

        /**
        * From CEikAppUi, HandleCommandL.
        * Takes care of command handling.
        * @param aCommand Command to be handled.
        */
        void HandleCommandL( TInt aCommand );

        /**
        * HandleResourceChangeL()
        * Called by framework when layout is changed.
        * @param aType the type of resources that have changed
        */
        void HandleResourceChangeL( TInt aType );

    private: // Data

        /**
        * The application view
        * Owned by CHelloWorldBasicAppUi
        */
        CHelloWorldBasicAppView* iAppView;
        CAccessoryMonitor *iMonitor;
    };

#endif // __HELLOWORLDBASICAPPUI_H__

// End of File

