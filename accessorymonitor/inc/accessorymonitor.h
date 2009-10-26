#include <accmonitor.h>
#include <aknvolumecontrol.h> 


class MAccessoryMonitorCallbacks {
public:
	virtual void AccessoryConnected() = 0;
	virtual void AccessoryDisconnected() = 0;
};

class CAccessoryMonitor: public MAccMonitorObserver
   {
   public:
	CAccessoryMonitor(MAccessoryMonitorCallbacks& aCallbacks);
   void ConstructL();
 
   private:
   void ConnectedL(CAccMonitorInfo* aAccessoryInfo);
   void DisconnectedL(CAccMonitorInfo* aAccessoryInfo);
   void AccMonitorObserverError( TInt aError );

   private:
   // Instance of the connected accessory. The content of the pointer passed to  ConnectedL()/DisconnectedL() must be copied to this value (with CopyL()),
   // because the original pointer is distroyed after mentioned methods have been called.
   CAccMonitorInfo* iAccessoryInfo;
   CAccMonitor* iAccMonitor;
   MAccessoryMonitorCallbacks& iCallbacks;
   CAknVolumeControl iVolume;
   };
