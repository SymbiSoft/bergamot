#ifndef _MHTTPEVENTHANDLERCALLBACKS_H_
#define _MHTTPEVENTHANDLERCALLBACKS_H_

#include <e32cmn.h>

class MHttpEventHandlerCallbacks
{
public:
  virtual void ConnectedCallback() = 0;
  virtual void DisconnectedCallback() = 0;
  virtual void FileCompleteCallback(TFileName &fileName) = 0;
  virtual void ProgressCallback(int percent) = 0;
  virtual void DownloadInfoCallback(int size) = 0;
};

#endif
