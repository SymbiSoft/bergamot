#ifndef __SYNCCLIENTAPPUI_H__
#define __SYNCCLIENTAPPUI_H__

#include <QikAppUi.h>
#include "SyncClientView.h"


class CSyncClientAppUi : public CQikAppUi
{
public:
  void ConstructL();
  ~CSyncClientAppUi();

private:
  void HandleCommandL(CQikCommand &aCommand);

  CSyncClientView* iBaseView;
};


#endif


