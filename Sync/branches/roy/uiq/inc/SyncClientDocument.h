#ifndef __SYNCCLIENTDOCUMENT_H__
#define __SYNCCLIENTDOCUMENT_H__

#include <QikDocument.h>
#include <QikApplication.h>
#include <QikAppUi.h>
#include "SyncClientAppUi.h"

class CSyncClientDocument : public CQikDocument
{
public:
  static CSyncClientDocument* NewL(CQikApplication& aAppUi);
  ~CSyncClientDocument();
private:
  CQikAppUi* CreateAppUiL();
  CSyncClientDocument (CQikApplication& aApp);
  void ConstructL();
};

#endif

