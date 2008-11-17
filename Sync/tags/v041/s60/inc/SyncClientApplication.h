#ifndef __SYNCCLIENTAPPLICATION_H__
#define __SYNCCLIENTAPPLICATION_H__

// INCLUDES
#include <aknapp.h>

// CLASS DECLARATION

class CSyncClientApplication : public CAknApplication
    {
    public: // Functions from base classes

        /**
        * From CApaApplication, AppDllUid.
        * @return Application's UID (KUidHelloWorldBasicApp).
        */
        TUid AppDllUid() const;

    protected: // Functions from base classes

        /**
        * From CApaApplication, CreateDocumentL.
        * Creates CHelloWorldBasicDocument document object. The returned
        * pointer in not owned by the CHelloWorldBasicApplication object.
        * @return A pointer to the created document object.
        */
        CApaDocument* CreateDocumentL();
    };

#endif

// End of File

