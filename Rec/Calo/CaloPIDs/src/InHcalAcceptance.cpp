// $Id: InHcalAcceptance.cpp,v 1.4 2008-01-24 10:22:42 vegorych Exp $
// ============================================================================
// Include files 
// ============================================================================
// GaudiKernel
// ============================================================================
#include "GaudiKernel/ToolFactory.h"
// ============================================================================
// Local
// ============================================================================
#include "InCaloAcceptance.h"
// ============================================================================
/** @class InHcalAcceptance 
 *  The precofigured instance of InCaloAcceptance Tool
 *  @author Victor EGORYCHEV Victor.Egorychev@cern.ch
 *  @author Vanya  BELYAEV    ibelyaev@phsycis.syr.edu
 *  @date   2006-05-28
 */
// ============================================================================
class InHcalAcceptance : public InCaloAcceptance 
{  
  /// friend factory for instantiation 
  friend class ToolFactory<InHcalAcceptance> ; 
protected:
  /// standard protected constructor 
  InHcalAcceptance
  ( const std::string& type   ,
    const std::string& name   , 
    const IInterface*  parent )
    : InCaloAcceptance( type, name , parent )
  {
    setProperty ( "Calorimeter" , DeCalorimeterLocation::Hcal ) ;
    setProperty ( "UseFiducial" , true  ) ;
    setProperty ( "Tolerance"   , 10 ) ;  /// 10 * Gaudi::Units::mm 
  } ;
  /// virtual and protected destructor 
  virtual ~InHcalAcceptance(){} ;
private:
  InHcalAcceptance() ;
  InHcalAcceptance            ( const InHcalAcceptance& ) ;
  InHcalAcceptance& operator= ( const InHcalAcceptance& ) ;  
} ;
// ============================================================================
/// Declaration of the Tool Factory
// ============================================================================
DECLARE_TOOL_FACTORY( InHcalAcceptance ) ;
// ============================================================================

// ============================================================================
// The END 
// ============================================================================
