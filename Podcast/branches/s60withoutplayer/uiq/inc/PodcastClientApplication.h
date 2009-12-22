#ifndef __PODCASTCLIENTAPPLICATION_H__
#define __PODCASTCLIENTAPPLICATION_H__

#include <qikapplication.h>
#include <eikstart.h>
#include "PodcastClientDocument.h"

class CPodcastClientApplication : public CQikApplication
{
public:
  CApaDocument* CreateDocumentL();
  TUid AppDllUid() const;
  IMPORT_C CApaApplication* NewApplication();
};


#endif

