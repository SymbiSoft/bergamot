/**
 * This file is a part of Escarpod Podcast project
 * (c) 2008 The Bergamot Project
 * (c) 2008 Teknolog (Sebastian Brännström)
 * (c) 2008 Anotherguest (Lars Persson)
 */

#ifndef PODCASTLISTVIEWH
#define PODCASTLISTVIEWH 

#include <aknview.h>
#include <aknlists.h> 
#include <eiklbo.h>
#include "PodcastModel.h"

class CAknDoubleLargeStyleListBox;
class CEikFormattedCellListBox;
class CAknNavigationDecorator;
class CAknNavigationControlContainer;

class MKeyEventListener {
public:
virtual TKeyResponse OfferKeyEventL(const TKeyEvent& aKeyEvent,TEventCode aType) = 0;
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
		CEikFormattedCellListBox* Listbox();
		void ScrollToVisible();
	protected:
		CEikFormattedCellListBox * iListbox;
		MKeyEventListener* iKeyEventListener;
	};


class CPodcastListView : public CAknView
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

		virtual void UpdateListboxItemsL() = 0;
		
        void RunAboutDialogL();
		void SetNaviTextL(TDesC &aText);
	protected:
		 CAknNavigationDecorator* iNaviDecorator;
		 CAknNavigationControlContainer* iNaviPane;
		 CPodcastListContainer* iListContainer;
		 /** Previous activated view */
		 TVwsViewId iPreviousView;

		 /** Listbox flags which the listbox is created with*/
		 TInt iListboxFlags;
		 
		 CDesCArray* iItemArray;
		 RArray<TUint> iItemIdArray;
		TBuf<256> iListboxFormatbuffer;
    };
#endif // PODCASTBASEVIEWH

