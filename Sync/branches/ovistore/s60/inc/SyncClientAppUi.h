#ifndef __SYNCCLIENT_APPUI_H__
#define __SYNCCLIENT_APPUI_H__

// INCLUDE FILES
#include <aknappui.h>
#include <aknnavi.h>
#include <aknnavide.h> 
#include <eikspane.h> 
#include <aknnavilabel.h>

// FORWARD DECLARATIONS
class CSyncClientAppView;

// CLASS DECLARATION

/**
* Application UI class.
* Provides support for the following features:
* - EIKON control architecture
* 
*/
class CSyncClientAppUi : public CAknAppUi
    {
public: // Constructors and destructor

    /**
    * Symbian OS default constructor.
    */      
    void ConstructL();

    /**
    * Constructor
    */      
    CSyncClientAppUi();


    /**
    * Destructor.
    */      
    ~CSyncClientAppUi();


public: // Functions from base classes
 
    /**
    * From CEikAppUi, takes care of command handling.
    * @param aCommand command to be handled
    */
    void HandleCommandL(TInt aCommand);

protected:
    void HandleStatusPaneSizeChange();
    
private:
	
	void RunAboutDialogL();
	void ShowHelp();
private: // Data members

    CSyncClientAppView* iAppView;
    CAknNavigationDecorator* iNaviLabelDecorator;
    };


#endif
