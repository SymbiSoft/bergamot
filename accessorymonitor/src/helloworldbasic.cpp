/*
* ==============================================================================
*  Name        : helloworldbasic.cpp
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
#include <eikstart.h>
#include "HelloWorldBasicApplication.h"

/**
 * factory function to create the Hello World Basic application class
 */
LOCAL_C CApaApplication* NewApplication()
	{
	return new CHelloWorldBasicApplication;
	}

/**
 * A normal Symbian OS executable provides an E32Main() function which is
 * called by the operating system to start the program.
 */
GLDEF_C TInt E32Main()
	{
	return EikStart::RunApplication( NewApplication );
	}

