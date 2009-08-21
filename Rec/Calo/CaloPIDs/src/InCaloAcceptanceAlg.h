// $Id: InCaloAcceptanceAlg.h,v 1.3 2009-08-21 16:49:45 odescham Exp $
// ============================================================================
#ifndef CALOPIDS_INCALOACCEPTANCEALG_H 
#define CALOPIDS_INCALOACCEPTANCEALG_H 1
// ============================================================================
// Include files
// ============================================================================
// GaudiKernel
// ============================================================================
#include "GaudiKernel/AlgFactory.h" 
// ============================================================================
// Relations 
// ============================================================================
#include "Relations/IRelation.h"
#include "Relations/IsConvertible.h"
// ============================================================================
// LHCbKernel
// ============================================================================
#include "Kernel/IInAcceptance.h"
// ============================================================================
// Event
// ============================================================================
#include "Event/Track.h"
// ============================================================================
// CaloUtils 
// ============================================================================
#include "CaloUtils/Calo2Track.h"
#include "CaloUtils/CaloTrackAlg.h"
#include "CaloUtils/CaloAlgUtils.h"
// ============================================================================
// Local 
// ============================================================================
#include "ToVector.h"
// ============================================================================
/** @class InCaloAcceptanceAlg InCaloAcceptanceAlg.h
 *
 *  the trivial algorithm to fill "InCaloAcceptance" table 
 *
 *  @author Vanya BELYAEV ibelyaev@physics.syr.edu
 *  @date   2006-06-17
 */
// ============================================================================
class InCaloAcceptanceAlg : public CaloTrackAlg 
{
  /// friend factory for instantiation
  friend class AlgFactory<InCaloAcceptanceAlg>;
public:
  /// algorithm initialization 
  virtual StatusCode initialize () ;
  /// algorithm execution  
  virtual StatusCode execute    () ;  
protected:
  /// Standard protected constructor
  InCaloAcceptanceAlg 
  ( const std::string& name , 
    ISvcLocator*       pSvc ) ;
  /// virtual proected destructor 
  virtual ~InCaloAcceptanceAlg() {} ; 
private:
  InCaloAcceptanceAlg() ;
  InCaloAcceptanceAlg           ( const InCaloAcceptanceAlg& );
  InCaloAcceptanceAlg& operator=( const InCaloAcceptanceAlg& );
protected:
  typedef std::vector<std::string> Inputs ;
protected:
  // input containers of the tracks ;
  Inputs               m_inputs   ;
  std::string          m_output   ;
  std::string          m_toolName ;
  const IInAcceptance* m_tool     ;
} ;
// ============================================================================
#define INHERITS(T1,T2) \
     (Relations::IsConvertible<const T1*,const T2*>::value && \
     !Relations::IsConvertible<const T1*,const void*>::same)
// ============================================================================

// ============================================================================
/// The END  
// ============================================================================
#endif // CALOPIDS_INCALOACCEPTANCEALG_H
// ============================================================================
