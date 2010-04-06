#ifndef __SYNCCLIENTAPPLICATION_H__
#define __SYNCCLIENTAPPLICATION_H__

#include <QikApplication.h>
#include <EikStart.h>
#include "SyncClientDocument.h"

class CSyncClientApplication : public CQikApplication
{
public:
  CApaDocument* CreateDocumentL();
  TUid AppDllUid() const;
  IMPORT_C CApaApplication* NewApplication();
};


#endif

