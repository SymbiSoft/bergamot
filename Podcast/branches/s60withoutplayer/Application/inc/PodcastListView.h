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

#ifndef PODCASTLISTVIEWH
#define PODCASTLISTVIEWH 

#include <aknview.h>
#include <aknlists.h> 
#include <eiklbo.h>
#include <aknsbasicbackgroundcontrolcontext.h>
#include <AknToolbarObserver.h>
#include <AknToolbar.h>
#include <aknlongtapdetector.h>
#include <aknstyluspopupmenu.h>

#include "PodcastModel.h"

class CAknDoubleLargeStyleListBox;
class CEikFormattedCellListBox;


class MKeyEventListener {
public:
virtual TKeyResponse OfferKeyEventL(const TKeyEvent& aKeyEvent,TEventCode aType) = 0;
};

class MPointerListener {
public:
virtual void PointerEventL(const TPointerEvent& aPointerEvent) = 0;
};

class CPodcastListContainer : public CCoeControl
    {
    public: 
		CPodcastListContainer();
		~CPodcastListContainer();
		void ConstructL( const TRect& aRect, TInt aListboxFlags );
		void SizeChanged();
        TInt CountComponentControls() const;
        CCoeControl* ComponentControl( TInt aIndex ) const;
		void HandleResourceChange(TInt aType);
		virtual TKeyResponse OfferKeyEventL(const TKeyEvent& aKeyEvent,TEventCode aType);
		void SetKeyEventListener(MKeyEventListener *aKeyEventListener);
		void SetPointerListener(MPointerListener *aPointerListener);
		
		CEikFormattedCellListBox* Listbox();
		void ScrollToVisible();
    	void Draw(const TRect& aRect) const;
		CEikFormattedCellListBox * iListbox;		

	protected:
		TTypeUid::Ptr MopSupplyObject( TTypeUid aId );
		void HandlePointerEventL(const TPointerEvent& aPointerEvent);

	private:
		MKeyEventListener* iKeyEventListener;
		MPointerListener* iPointerListener;
        CAknsBasicBackgroundControlContext* iBgContext;
	};


class CPodcastListView : public CAknView, public MAknToolbarObserver,
public MPointerListener, public MAknLongTapDetectorCallBack 
    {
    public: 
        ~CPodcastListView();
	protected:
	    void ConstructL();
		CPodcastListView();	

		/** 
		 * Handles a view activation and passes the message of type 
		 * @c aCustomMessageId. This function is intended for overriding by 
		 * sub classes. This function is called by @c AknViewActivatedL().
		 * @param aPrevViewId Specifies the view previously active.
		 * @param aCustomMessageId Specifies the message type.
		 * @param aCustomMessage The activation message.
		 */
		void DoActivateL(const TVwsViewId& aPrevViewId,
	                                  TUid aCustomMessageId,
	                                  const TDesC8& aCustomMessage);

	    /** 
		 * View deactivation function intended for overriding by sub classes. 
		 * This function is called by @c AknViewDeactivated().
		 */
		void DoDeactivate();

		void HandleViewRectChange();
		
		/** 
		* Event handler for status pane size changes.
		* @c CAknView provides an empty implementation for sub classes that do 
		* not want to handle this event.
		*/
		void HandleStatusPaneSizeChange();
				
		/** 
		* Command handling function intended for overriding by sub classes. 
		* Default implementation is empty.  
		* @param aCommand ID of the command to respond to. 
		*/
		void HandleCommandL(TInt aCommand);

		void OfferToolbarEventL(TInt aCommand);
		void DynInitToolbarL (TInt aResourceId, CAknToolbar *aToolbar);

		virtual void UpdateListboxItemsL() = 0;
		
        void RunAboutDialogL();
        void SetEmptyTextL(TInt aResourceId);
        void ShowOkMessage(TDesC &aText);
        void ShowErrorMessage(TDesC &aText);
		void CloseToolbarExtension();

		// From MAknLongTapDetectorCallBack
		virtual void HandleLongTapEventL( const TPoint& aPenEventLocation, const TPoint& aPenEventScreenLocation );
		 
		void PointerEventL(const TPointerEvent& aPointerEvent);

		virtual void UpdateToolbar() = 0;
	protected:
		 CPodcastListContainer* iListContainer;
		 /** Previous activated view */
		 TVwsViewId iPreviousView;

		 /** Listbox flags which the listbox is created with*/
		 TInt iListboxFlags;
		 
		 CDesCArray* iItemArray;
		 RArray<TUint> iItemIdArray;
		 
		 TBuf<256> iListboxFormatbuffer;
		 
		 CAknToolbar *iToolbar;
		 CAknStylusPopUpMenu* iStylusPopupMenu;
		 CAknLongTapDetector* iLongTapDetector;
    };
#endif // PODCASTBASEVIEWH

