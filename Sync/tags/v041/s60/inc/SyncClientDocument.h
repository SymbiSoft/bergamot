#ifndef __SYNCCLIENTDOCUMENT_H__
#define __SYNCCLIENTDOCUMENT_H__

// INCLUDES
#include <akndoc.h>

// FORWARD DECLARATIONS
class CSyncClientAppUi;
class CEikApplication;


// CLASS DECLARATION

/**
* CSyncClientDocument application class.
* An instance of class CSyncClientDocument is the Document part of the
* AVKON application framework for the SyncClient example application.
*/
class CSyncClientDocument : public CAknDocument
    {
    public: // Constructors and destructor

        /**
        * NewL.
        * Two-phased constructor.
        * Construct a CSyncClientDocument for the AVKON application aApp
        * using two phase construction, and return a pointer
        * to the created object.
        * @param aApp Application creating this document.
        * @return A pointer to the created instance of CSyncClientDocument.
        */
        static CSyncClientDocument* NewL( CEikApplication& aApp );

        /**
        * NewLC.
        * Two-phased constructor.
        * Construct a CSyncClientDocument for the AVKON application aApp
        * using two phase construction, and return a pointer
        * to the created object.
        * @param aApp Application creating this document.
        * @return A pointer to the created instance of CSyncClientDocument.
        */
        static CSyncClientDocument* NewLC( CEikApplication& aApp );

        /**
        * ~CSyncClientDocument
        * Virtual Destructor.
        */
        virtual ~CSyncClientDocument();

    public: // Functions from base classes

        /**
        * CreateAppUiL
        * From CEikDocument, CreateAppUiL.
        * Create a CSyncClientAppUi object and return a pointer to it.
        * The object returned is owned by the Uikon framework.
        * @return Pointer to created instance of AppUi.
        */
        CEikAppUi* CreateAppUiL();

    private: // Constructors

        /**
        * ConstructL
        * 2nd phase constructor.
        */
        void ConstructL();

        /**
        * CSyncClientDocument.
        * C++ default constructor.
        * @param aApp Application creating this document.
        */
        CSyncClientDocument( CEikApplication& aApp );

    };

#endif

// End of File

