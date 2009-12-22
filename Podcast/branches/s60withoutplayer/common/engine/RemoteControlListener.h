#ifndef __RemoteControlListener_h__
#define __RemoteControlListener_h__

#include <e32base.h>
#include <remconinterfaceselector.h> 
#include <remconcoreapitarget.h> 
#include <remconcoreapitargetobserver.h>

class CPodcastModel;

class CRemoteControlListener : public CBase, MRemConCoreApiTargetObserver 
	{
public:
	// construction and destruction
	static CRemoteControlListener* NewL(CPodcastModel &aModel);
	CRemoteControlListener(CPodcastModel &aModel);
	~CRemoteControlListener();
	
	//From MRemConCoreApiControllerObserver
public:
	void MrccatoPlay(TRemConCoreApiPlaybackSpeed aSpeed, TRemConCoreApiButtonAction aButtonAct);
	void MrccatoTuneFunction();
	void MrccatoSelectDiskFunction();
	void MrccatoSelectAvInputFunction(TUint8 aAvInputSignalNumber, TRemConCoreApiButtonAction aButtonAct);    
	void MrccatoSelectAudioInputFunction(TUint8 aAudioInputSignalNumber, TRemConCoreApiButtonAction aButtonAct);
	void MrccatoCommand(TRemConCoreApiOperationId aOperationId, TRemConCoreApiButtonAction aButtonAct);

// for construction
private:
	void ConstructL();

private:
	CPodcastModel &iModel;
	CRemConInterfaceSelector* iSelector;
	CRemConCoreApiTarget* iTarget;
	};

#endif
