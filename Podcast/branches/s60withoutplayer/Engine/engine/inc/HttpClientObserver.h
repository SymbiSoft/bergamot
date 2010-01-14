#ifndef _MHTTPCLIENTOBSERVER_H_
#define _MHTTPCLIENTOBSERVER_H_

#include <e32cmn.h>

class CHttpClient;

class MHttpClientObserver
{
public:
  virtual void Connected(CHttpClient* aHttpClient) = 0;
  virtual void Disconnected(CHttpClient* aHttpClient) = 0;
  virtual void CompleteL(CHttpClient* aHttpClient, TBool aSuccessful) = 0;
  virtual void Progress(CHttpClient* aHttpClient, TInt aBytes, TInt aTotalBytes) = 0;
  virtual void DownloadInfo(CHttpClient* aHttpClient, TInt aTotalBytes) = 0;
  virtual void FileError(TUint aError) = 0;
};

#endif
