// $Id: CompareCaloDigits.cpp,v 1.1.1.1 2005-01-11 07:51:47 ocallot Exp $
// Include files 

// CLHEP
#include "CLHEP/Units/SystemOfUnits.h"

// from Gaudi
#include "GaudiKernel/AlgFactory.h"
#include "GaudiKernel/IRegistry.h" 

// local
#include "CompareCaloDigits.h"

//-----------------------------------------------------------------------------
// Implementation file for class : CompareCaloDigits
//
// 2003-11-18 : Olivier Callot
//-----------------------------------------------------------------------------

// Declaration of the Algorithm Factory
static const  AlgFactory<CompareCaloDigits>          s_factory ;
const        IAlgFactory& CompareCaloDigitsFactory = s_factory ; 


//=============================================================================
// Standard constructor, initializes variables
//=============================================================================
CompareCaloDigits::CompareCaloDigits( const std::string& name,
                                      ISvcLocator* pSvcLocator)
  : GaudiAlgorithm ( name , pSvcLocator ) 
{
  declareProperty( "Extension", m_extension = "Test" );  
}

//=============================================================================
// Destructor
//=============================================================================
CompareCaloDigits::~CompareCaloDigits() {}; 

//=============================================================================
// Main execution
//=============================================================================
StatusCode CompareCaloDigits::execute() {

  debug() << "==> Execute" << endmsg;

  //== SPD. Digits are yes/no -> tolerance = .5
  CaloDigits* spd1 = get<CaloDigits>( CaloDigitLocation::Spd );
  CaloDigits* spd2 = get<CaloDigits>( CaloDigitLocation::Spd + m_extension );
  compareContainers( spd1, spd2, 0.5 );

  //== PreShower. 
  CaloDigits* prs1 = get<CaloDigits>( CaloDigitLocation::Prs );
  CaloDigits* prs2 = get<CaloDigits>( CaloDigitLocation::Prs + m_extension );
  compareContainers( prs1, prs2, 0.1 );

  //== Ecal.
  CaloDigits* ecal1 = get<CaloDigits>( CaloDigitLocation::Ecal );
  CaloDigits* ecal2 = get<CaloDigits>( CaloDigitLocation::Ecal + m_extension );
  compareContainers( ecal1, ecal2, 1. );

  //== Hcal. 
  CaloDigits* hcal1 = get<CaloDigits>( CaloDigitLocation::Hcal );
  CaloDigits* hcal2 = get<CaloDigits>( CaloDigitLocation::Hcal + m_extension );
  compareContainers( hcal1, hcal2, 1. );

  return StatusCode::SUCCESS;
};

//=========================================================================
//  Compare two CaloDigits containers
//=========================================================================
void CompareCaloDigits::compareContainers ( CaloDigits* dig1,
                                            CaloDigits* dig2,
                                            double tol) {
  if ( dig1->size() != dig2->size() ) {
    error() << "Incoherent size : " << dig1->registry()->identifier() 
            << " : " << dig1->size() << " and " 
            << dig2->registry()->identifier() << " : " << dig2->size() 
            << endreq;
  } else {
    debug() << "Comparing " << dig1->name()
            << " and " << dig2->name()
            << " both sizes = " << dig2->size() << endreq;
  }
  
  CaloDigits::const_iterator it1, it2;
  it1 = dig1->begin();
  it2 = dig2->begin();
  while ( dig1->end() != it1 && dig2->end() != it2 ) {
    if (  !((*it1)->cellID() ==  (*it2)->cellID()) ||
          fabs( (*it1)->e() - (*it2)->e() ) > tol ) {
      info() << "Error in sequence/energy (old-new): " 
             << (*it1)->cellID() << " " << (*it2)->cellID() 
             << format( "  E %7.1f %7.1f %7.2f ", 
                        (*it1)->e()/MeV, (*it2)->e()/MeV ,
                        (*it1)->e()/MeV - (*it2)->e()/MeV )
             << endreq;
    } else {
      verbose() << (*it1)->cellID() << " "
                << (*it2)->cellID() << "  E " 
                << (*it1)->e()/MeV  << " " 
                << (*it2)->e()/MeV << endreq;
    }
    if ( (*it1)->cellID() ==  (*it2)->cellID() ) {
      it1++;
      it2++;
    } else if ( (*it1)->cellID() <  (*it2)->cellID() ) {
      it1++;
    } else {
      it2++;
    }
  }
}

//=============================================================================
