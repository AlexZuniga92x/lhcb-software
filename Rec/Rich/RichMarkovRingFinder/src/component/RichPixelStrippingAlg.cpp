// $Id: RichPixelStrippingAlg.cpp,v 1.1 2004-06-25 11:53:06 buckley Exp $

// local
#include "RichPixelStrippingAlg.h"

//-----------------------------------------------------------------------------
// Implementation file for class : RichPixelStrippingAlg
//
// 2004-06-20 : Andy Buckley Andrew.Buckley@cern.ch
// 2004-06-20 : Chris Jones  Christopher.Rob.Jones@cern.ch
//-----------------------------------------------------------------------------

// Declaration of the Algorithm Factory
static const  AlgFactory<RichPixelStrippingAlg>          s_factory ;
const        IAlgFactory& RichPixelStrippingAlgFactory = s_factory ;

//=============================================================================
// Standard constructor, initializes variables
//=============================================================================
RichPixelStrippingAlg::RichPixelStrippingAlg( const std::string& name,
                                              ISvcLocator* pSvcLocator)
  : RichRecAlgBase ( name , pSvcLocator )
{

}
//=============================================================================
// Destructor
//=============================================================================
RichPixelStrippingAlg::~RichPixelStrippingAlg() {};

//=============================================================================
// Initialisation. Check parameters
//=============================================================================
StatusCode RichPixelStrippingAlg::initialize()
{
  debug() << "Initialize" << endreq;

  // Sets up various tools and services
  const StatusCode sc = RichRecAlgBase::initialize();
  if ( sc.isFailure() ) { return sc; }

  return StatusCode::SUCCESS;
};

//=============================================================================
// Main execution
//=============================================================================
StatusCode RichPixelStrippingAlg::execute()
{
  debug() << "Execute" << endreq;

  // Locate RichRecRings from TES
  SmartDataPtr<RichRecRings> rings( eventSvc(), RichRecRingLocation::MarkovRings );
  if ( !rings ) {
    warning() << "Failed to find RichRecRings at " << RichRecRingLocation::MarkovRings << endreq;
    return StatusCode::SUCCESS;
  } else {

    if ( msgLevel(MSG::DEBUG) ) {
      debug() << "Successfully located " << rings->size() << " RichRecRings at " 
              << RichRecRingLocation::MarkovRings << endreq;
    }

    // Create and initialise decision map for stripping out Markov-unassociated pixels
    std::map<RichRecPixel*,bool> decisionMap;
    for ( RichRecPixels::const_iterator iPix = richPixels()->begin(); iPix != richPixels()->end(); ++iPix ) {
      // Set true by default if in RICH1, otherwise false
      decisionMap[*iPix] = ( Rich::Rich1 == (*iPix)->smartID().rich() );
    }


    // Loop over all Markov rings
    for ( RichRecRings::const_iterator iRing = rings->begin(); iRing != rings->end(); ++iRing ) {
      
      // Ring centre point and radius on the PD plane
      //const HepPoint3D & ringPoint = (*iRing)->centrePointLocal();
      //const double ringRadius = (*iRing)->radius();

   
        
      // Set "Markov-seen pixels" to be selected by stripping
      SmartRefVector<RichRecPixel> & pixels = (*iRing)->richRecPixels();
      for ( SmartRefVector<RichRecPixel>::iterator iPix = pixels.begin(); iPix != pixels.end(); ++iPix ) {    
        if ( (*iRing)->richRecSegment() )
          decisionMap[*iPix] = true;
      }
    }


    // Remove as indicated by decisionMap
    for ( std::map<RichRecPixel*, bool>::const_iterator iP = decisionMap.begin(); iP != decisionMap.end(); ++iP) {
      if ( ! (*iP).second  ) richPixels()->remove( (*iP).first );
    }

  }
    

  return StatusCode::SUCCESS;
};

//=============================================================================
//  Finalize
//=============================================================================
StatusCode RichPixelStrippingAlg::finalize()
{
  debug() << "Finalize" << endreq;

  // Execute base class method
  return RichRecAlgBase::finalize();
}

//=============================================================================
