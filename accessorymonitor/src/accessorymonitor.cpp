#include "accessorymonitor.h"
#include "debug.h"

CAccessoryMonitor::CAccessoryMonitor(MAccessoryMonitorCallbacks &aCallbacks) : iCallbacks(aCallbacks)
	{
	
	}


void CAccessoryMonitor::ConstructL()
   {
   DP("CAccessoryMonitor::ConstructL BEGIN");
   iAccMonitor = CAccMonitor::NewL();
   RConnectedAccessories array;
   CleanupClosePushL(array);
   iAccessoryInfo = CAccMonitorInfo::NewL();
	iAccMonitor->StartObservingL(this);

   iAccMonitor->GetConnectedAccessoriesL(array);
 
   DP1("accessories count=%d", array.Count());
  // Iterate through available accessories
	for (TInt i = 0; array.Count() != i; i++)
	{
		DP2("Accessory %d has type %d", i, array[i]->AccDeviceType());

	}

 
   CleanupStack::PopAndDestroy(&array);
   DP("CAccessoryMonitor::ConstructL END");
   }

void CAccessoryMonitor::ConnectedL(CAccMonitorInfo* aAccessoryInfo)
    {
    DP("CAccessoryMonitor::ConnectedL BEGIN");
    iAccessoryInfo->Reset();
 
    iAccessoryInfo->CopyL(aAccessoryInfo);
 
    iCallbacks.AccessoryConnected();
    DP("CAccessoryMonitor::ConnectedL END");
    
    }

void CAccessoryMonitor::DisconnectedL(CAccMonitorInfo*  aAccessoryInfo)
   {
   DP("CAccessoryMonitor::DisconnectedL BEGIN");
   iAccessoryInfo->Reset();
 
   iAccessoryInfo->CopyL(aAccessoryInfo);
   iCallbacks.AccessoryDisconnected();
   DP("CAccessoryMonitor::DisconnectedL END");
   }
void CAccessoryMonitor::AccMonitorObserverError( TInt aError )
	{
    DP("CAccessoryMonitor::AccMonitorObserverError");
	}
