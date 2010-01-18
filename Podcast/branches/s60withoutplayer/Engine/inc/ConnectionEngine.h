/*
 * Copyright (c) 2007-2010 Sebastian Brannstrom, Lars Persson, EmbedDev AB
 *
 * All rights reserved.
 * This component and the accompanying materials are made available
 * under the terms of the License "Eclipse Public License v1.0"
 * which accompanies this distribution, and is available
 * at the URL "http://www.eclipse.org/legal/epl-v10.html".
 *
 * Initial Contributors:
 * EmbedDev AB - initial contribution.
 *
 * Contributors:
 *
 * Description:
 *
 */

#ifndef PODCAST_CONNECTIONENGINE_H
#define PODCAST_CONNECTIONENGINE_H


// Connect using mobility extension
#include <in_sock.h>
#include <connpref.h>
#include <commdbconnpref.h>
#include <comms-infras/cs_mobility_apiext.h>

// Using connection manager settings UI
#include <cmapplicationsettingsui.h>

// Forward declarations
class MDataMobilityPrinter;

/**
 * This class helps to connect using three available methods
 * SNAP, Mobility extension and Connection manager UI
 */
class CConnectionEngine:public CActive, public MMobilityProtocolResp
	{
	enum TConnectionType
		{
		EUserSelectConnection,
		ESNAPConnection,
		EMobilityConnection
		};
public:
	static CConnectionEngine* NewL();
	~CConnectionEngine();
	void StartL(TConnectionType aConnectionType);
private: // Methods
	CConnectionEngine();
	void ConstructL();
protected:
	// From CActive
	void RunL();
	void DoCancel();
	TInt RunError( TInt aError );
protected:
	// From MMobilityProtocolResp
	void PreferredCarrierAvailable( TAccessPointInfo aOldAPInfo,
			TAccessPointInfo aNewAPInfo,
			TBool aIsUpgrade,
			TBool aIsSeamless );
	
	void NewCarrierActive( TAccessPointInfo aNewAPInfo, TBool aIsSeamless );
	void Error( TInt aError );
protected:
	// UI Setting
	TBool ConnectionSettingL();
private:
	RConnection                   iConnection;
	RSocketServ                   iSocketServer;
	// Connect using mobility extension
	CActiveCommsMobilityApiExt*   iMobility;

	// Using connection manager settings UI
	TCmSettingSelection   iUserSelection;
	
	// Connect using SNAP setting
	TConnSnapPref iSnapPreference;
	TCommDbConnPref iCommDBPreference;
	TConnectionType iConnectionType;
	};

#endif // PODCAST_CONNECTIONENGINE_H
