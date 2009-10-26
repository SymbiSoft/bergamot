/*
* ==============================================================================
*  Name        : helloworldbasicappview.h
*  Part of     : Helloworldbasic
*  Interface   :
*  Description :
*  Version     :
*
*  Copyright (c) 2005-2006 Nokia Corporation.
*  This material, including documentation and any related
*  computer programs, is protected by copyright controlled by
*  Nokia Corporation.
* ==============================================================================
*/

#ifndef __HELLOWORLDBASICAPPVIEW_H__
#define __HELLOWORLDBASICAPPVIEW_H__

// INCLUDES
#include <coecntrl.h>

// CLASS DECLARATION
class CHelloWorldBasicAppView : public CCoeControl
    {
    public: // New methods

        /**
        * NewL.
        * Two-phased constructor.
        * Create a CHelloWorldBasicAppView object, which will draw itself to aRect.
        * @param aRect The rectangle this view will be drawn to.
        * @return a pointer to the created instance of CHelloWorldBasicAppView.
        */
        static CHelloWorldBasicAppView* NewL( const TRect& aRect );

        /**
        * NewLC.
        * Two-phased constructor.
        * Create a CHelloWorldBasicAppView object, which will draw itself
        * to aRect.
        * @param aRect Rectangle this view will be drawn to.
        * @return A pointer to the created instance of CHelloWorldBasicAppView.
        */
        static CHelloWorldBasicAppView* NewLC( const TRect& aRect );

        /**
        * ~CHelloWorldBasicAppView
        * Virtual Destructor.
        */
        virtual ~CHelloWorldBasicAppView();

       /**
        * GetText
        * @return modifiable reference to the descriptor, which stores
        *   user-given text
        */
        TDes& GetText();

    public:  // Functions from base classes

        /**
        * From CCoeControl, Draw
        * Draw this CHelloWorldBasicAppView to the screen.
        * If the user has given a text, it is also printed to the center of
        * the screen.
        * @param aRect the rectangle of this view that needs updating
        */
        void Draw( const TRect& aRect ) const;

        /**
        * From CoeControl, SizeChanged.
        * Called by framework when the view size is changed.
        */
        virtual void SizeChanged();

    private: // Constructors

        /**
        * ConstructL
        * 2nd phase constructor.
        * Perform the second phase construction of a
        * CHelloWorldBasicAppView object.
        * @param aRect The rectangle this view will be drawn to.
        */
        void ConstructL(const TRect& aRect);

        /**
        * CHelloWorldBasicAppView.
        * C++ default constructor.
        */
        CHelloWorldBasicAppView();

    private:

       /**
        * font used, when printing user-given text to the screen
        */
        const CFont* iFont;

       /**
        * descriptor storing the user-given text
        */
        TBuf<24> iText;
        
    public:
    	void AccessoryConnected();
    	void AccessoryDisconnected();

    };

#endif // __HELLOWORLDBASICAPPVIEW_H__

// End of File

