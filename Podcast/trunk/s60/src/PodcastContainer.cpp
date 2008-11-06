#include <AknLists.h> // for AknListBoxLayouts
#include <Avkon.hrh>
#include <AknIconArray.h> // for CArrayPtr
#include <AknIconUtils.h>
#include <AknNoteWrappers.h> // for CAknErrorNote
#include <AknUtils.h> 
#include <barsread.h> // for TResourceReader
#include <gulicon.h> // for CGulIcon
#include <f32file.h>
#include <aknnavide.h> //NaviPane Decorator
#include <GDI.H>

#include <Podcast.rsg>

#include "PodcastContainer.h"
#include "PodcastAppUi.h"
#include "Podcast.hrh"


CPodcastContainer* CPodcastContainer::NewL( const TRect& aRect )
    {
    CPodcastContainer* self = CPodcastContainer::NewLC( aRect );
    CleanupStack::Pop( self );
    return self;
    }

CPodcastContainer* CPodcastContainer::NewLC( const TRect& aRect )
    {
    CPodcastContainer* self = new ( ELeave ) CPodcastContainer();
    CleanupStack::PushL( self );
    self->ConstructL( aRect );
    return self;
    }
    
CPodcastContainer::~CPodcastContainer()
    {
    delete iGrid;    
    delete iNaviDecorator;
    }

void CPodcastContainer::SetGridInitialParametersL()
    {
    TSize mpSize;     // main pane size
    AknLayoutUtils::LayoutMetricsSize( AknLayoutUtils::EMainPane, mpSize );

    iNumOfItems = 0; 
    
    iNumOfColumns = 1;
    iNumOfRows = 1;
    
    // Determinate cell size
    iSizeOfCell.iWidth = mpSize.iWidth / iNumOfColumns;
    iSizeOfCell.iHeight = mpSize.iHeight / iNumOfRows;
    
    // Determinate scrolling type
    iVerticalScrollingType = CAknGridView::EScrollIncrementLineAndLoops;    
    iHorizontalScrollingType = CAknGridView::EScrollIncrementLineAndLoops;

      
    iVerticalOrientation = EFalse;
    iLeftToRight = ETrue;
    iTopToBottom = ETrue;
    
    // Set grid layout
    iGrid->SetLayoutL( iVerticalOrientation, 
        iLeftToRight, iTopToBottom, 
        iNumOfColumns, iNumOfRows,  
        iSizeOfCell );

    // Set scrolling type
    iGrid->SetPrimaryScrollingType( iVerticalScrollingType );
    iGrid->SetSecondaryScrollingType( iHorizontalScrollingType );
    
    // Set current index in grid
     iGrid->SetCurrentDataIndex( 0 );
    }

void CPodcastContainer::CreateGridResourceL()
    {
    // Load graphics
    LoadGraphicsL();

    // Create data
    AddDataL();
    }

void CPodcastContainer::LoadGraphicsL()
    {    
    // Create icon array with granularity of 1 icon
    CArrayPtr< CGulIcon >* icons = new(ELeave) CAknIconArray( 1 );
    CleanupStack::PushL( icons );

	// Creates bitmap and mask for an icon.
 /*   AknIconUtils::CreateIconL( iBitmap, 
            iMask, 
            KIconName, 
            EMbmPodcastPodcast, 
            EMbmPodcastPodcast_mask );
*/
    // Determinate icon size
    TSize iconSize;
    iconSize.iWidth = iSizeOfCell.iWidth / 2;
    iconSize.iHeight = iSizeOfCell.iHeight / 2;

	//Initializes the icon to the given size.
	//Note that this call sets the sizes of both bitmap and mask 
    AknIconUtils::SetSize( iBitmap, iconSize );
    
    // Append the icon to icon array
    icons->AppendL( CGulIcon::NewL( iBitmap, iMask ) );
    
    // Attach icon array to grid's item drawer
    iGrid->ItemDrawer()->FormattedCellData()->SetIconArrayL( icons );

    CleanupStack::Pop( icons );    
    }

void CPodcastContainer::AddDataL()
    {
    
    MDesCArray* array = iGrid->Model()->ItemTextArray();
    CDesCArray* cArray = ( CDesCArray* )array;
	
	// The number of items in the grid.
    TInt numberOfData = 0;
    
    // Delete data from array
    if ( iGrid->GridModel()->NumberOfData() > 0 )
    {
        cArray->Delete( 0, iGrid->GridModel()->NumberOfData() );
    }
    iGrid->HandleItemRemovalL();

    // Create text which is added to the grid.
    TBuf< KLengthOfItemResourceString > cellText;
    TBuf< KLengthOfItemResourceString > cellTextFormat;

    
    // Add the item to the last index
    for ( TInt loop = 0; loop < numberOfData; loop++ )
        {
        //cellText.Format( cellTextFormat, &(*iPodcast)[loop].iName );
        cArray->AppendL( cellText );
        }

    // Inform list box that data was added.
    iGrid->HandleItemAdditionL();
    }

void CPodcastContainer::ApplySelGridGraphicStyle()
    {
    AknListBoxLayouts::SetupStandardGrid( *iGrid );

    // Layout of the graphic
    AknListBoxLayouts::SetupFormGfxCell(
       *iGrid,              // Reference to grid control
       iGrid->ItemDrawer(),// Pointer to the item drawer
       0,                   // Column index
       0,                   // Left position
       0,                   // Top position
       0,                   // Right - unused
       0,                   // Bottom - unused
       iSizeOfCell.iWidth, // Width
       iSizeOfCell.iHeight/2,// Height
       TPoint(0,0),         // Start position
       TPoint(iSizeOfCell.iWidth, iSizeOfCell.iHeight/2));     // End position
     
    const CFont* fontText = AknLayoutUtils::FontFromId(
                    EAknLogicalFontPrimaryFont, NULL );
                    
    // Deretminate the baseline for the text
    TInt baseline = iSizeOfCell.iHeight - fontText->DescentInPixels() - 1;

    // Layout of text
    AknListBoxLayouts::SetupFormTextCell(
       *iGrid,                          // Reference to grid
       iGrid->ItemDrawer(),            // Pointer to the item drawer
       1,                           // Column index
       fontText,                       // Font
       215,                         // Color (215 = black)
       0,                           // Left margin
       0,                           // Right margin - unused
       baseline,                    // Baseline
       iSizeOfCell.iWidth,            // Text width
       CGraphicsContext::ECenter,    // Text alignment
       TPoint(0, iSizeOfCell.iHeight/2), // Start position
       TPoint(iSizeOfCell.iWidth, 
               iSizeOfCell.iHeight));    // End position
               
    //Update icon size
	SetIconSizes( iGrid->ItemDrawer()->FormattedCellData()->IconArray() );
    }

void CPodcastContainer::SetIconSizes( CArrayPtr<CGulIcon>* aIconArray )
    {
    TSize iconSize;
    iconSize.iWidth = iSizeOfCell.iWidth / 2;
    iconSize.iHeight = iSizeOfCell.iHeight / 2;

    // note  AknIconUtils::SetSize sets both mask and bitmap size.
    // regardless which is given.
    if (aIconArray && aIconArray->Count() > 0) {
    	AknIconUtils::SetSize( (*aIconArray)[0]->Bitmap(), iconSize );
    }
    }
    
void CPodcastContainer::HandleListBoxEventL( CEikListBox* /*aListBox*/,
		TListBoxEvent /*aListBoxEvent*/ )
    {
    // no implementation required 
    }
void CPodcastContainer::ConstructL( const TRect& aRect )
    {
    CreateWindowL();

    // Create CAknGrid  
    iGrid = new(ELeave) CAknGrid;
    iGrid->SetContainerWindowL( *this );
    iGridM = new(ELeave) CAknGridM;
    iGrid->SetModel( iGridM );    
    
    //Call ConstructL for the grid object: 
    TInt gridFlags = EAknListBoxSelectionGrid;
    iGrid->ConstructL( this, gridFlags);
   
    // Set initial parameters for the grid
    SetGridInitialParametersL();    
      
    // Create resouce for grid
    //CreateGridResourceL();
    
    // Set observer
    iGrid->SetListBoxObserver( this );
    
    //Set navi pane text from resource
    //SetNaviPaneTextL();

    // Set the windows size
    SetRect( aRect );    
    
    // Activate the window, which makes it ready to be drawn
    ActivateL();   
    }   
     
void CPodcastContainer::SizeChanged()
    {         
    if ( iGrid )
        {
        // Set initial parameters for the grid
        TRAP_IGNORE( SetGridInitialParametersL() );
    
	    //Apply the layout to cells in the grid
	 	ApplySelGridGraphicStyle();   

        iGrid->SetRect( Rect() );
        }
    }

void CPodcastContainer::Draw( const TRect& /*aRect*/ ) const
    {
    // no implementation required 
    }

TInt CPodcastContainer::CountComponentControls() const
    {
    return 1; // return number of controls inside this container
    }

CCoeControl* CPodcastContainer::ComponentControl(TInt aIndex) const
    {
    switch ( aIndex )
        {
        case 0:
            return iGrid;
        default:
            return NULL;
        }
    }

TKeyResponse CPodcastContainer::OfferKeyEventL( const TKeyEvent& aKeyEvent,
        TEventCode aType )
    {
    if ( iGrid )
        {
        return iGrid->OfferKeyEventL ( aKeyEvent, aType );
        }
    else
        {
        return EKeyWasNotConsumed;    
        }
    }

void CPodcastContainer::SetNaviPaneTextL()
    {
    TResourceReader spText; 
	
	// Create navipane pointer
	if ( !iNaviPane )
		{
		CEikStatusPane *sp = 
		            ( ( CAknAppUi* )iEikonEnv->EikAppUi() )->StatusPane();
		// Fetch pointer to the default navi pane control
		iNaviPane = ( CAknNavigationControlContainer * )
		            sp->ControlL( TUid::Uid( EEikStatusPaneUidNavi ) );
		}
    
    if ( iNaviDecorator )
        {
        delete iNaviDecorator;
        iNaviDecorator = NULL;        
        }
        
        
    // Set text to navi pane
    iNaviDecorator = iNaviPane->CreateNavigationLabelL( spText );
    CleanupStack::PopAndDestroy(); // Pushed by CreateResourceReaderLC
    iNaviPane->PushL( *iNaviDecorator );
    }

