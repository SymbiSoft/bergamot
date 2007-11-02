#ifndef __PODCASTCLIENTDOCUMENT_H__
#define __PODCASTCLIENTDOCUMENT_H__

#include <QikDocument.h>
#include <QikApplication.h>
#include <QikAppUi.h>
#include "PodcastClientAppUi.h"


class CPodcastClientDocument : public CQikDocument
{
public:
  static CPodcastClientDocument* NewL(CQikApplication& aAppUi);
  ~CPodcastClientDocument();
private:
  CQikAppUi* CreateAppUiL();
  CPodcastClientDocument (CQikApplication& aApp);
  void ConstructL();
};

#endif

