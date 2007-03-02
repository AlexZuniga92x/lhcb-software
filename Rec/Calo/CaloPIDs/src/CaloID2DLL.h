// $Id: CaloID2DLL.h,v 1.3 2007-03-02 15:22:07 cattanem Exp $
// ============================================================================
#ifndef CALOPIDS_CALOID2DLL_H 
#define CALOPIDS_CALOID2DLL_H 1
// ============================================================================
// Include files
// ============================================================================
/// STD& STL
// ============================================================================
#include <cmath>
// ============================================================================
// GaudiKernel
// ============================================================================
#include "GaudiKernel/AlgFactory.h"
#include "GaudiKernel/VectorMap.h" 
// ============================================================================
// AIDA
// ============================================================================
#include "AIDA/IHistogram2D.h"
// ============================================================================
// Relations
// ============================================================================
#include "Relations/IRelation.h"
// ============================================================================
// Event 
// ============================================================================
#include "Event/Track.h"
// ============================================================================
// CaloUtils 
// ============================================================================
#include "CaloUtils/Calo2Track.h"
#include "CaloUtils/CaloTrackAlg.h"
// ============================================================================
#include "ToString.h"
#include "ToVector.h"
// ============================================================================
/** @class CaloID2DLL CaloID2DLL.h
 *  
 *
 *  @author Vanya BELYAEV ibelyaev@physics.syr.edu
 *  @date   2006-06-18
 */
// ============================================================================
class CaloID2DLL : public CaloTrackAlg 
{
  /// friend factory for instantiation
  friend class AlgFactory<CaloID2DLL>;
public:
  /// Algorithm initialization
  virtual StatusCode initialize () ;    
  /// Algorithm execution
  virtual StatusCode execute    () ;    
protected: 
  /// Standard protected constructor
  CaloID2DLL 
  ( const std::string& name , 
    ISvcLocator*       pSvc ) ;
  /// virtual protected destructor 
  virtual ~CaloID2DLL(){}; 
private:
  CaloID2DLL () ;
  CaloID2DLL            ( const CaloID2DLL& ) ;
  CaloID2DLL& operator= ( const CaloID2DLL& ) ;
protected:
  //
  inline double dLL 
  ( const double             p , 
    const double             v , 
    const LHCb::Track::Types t ) const ;
protected:
  // input relation table 
  std::string             m_input    ; ///< input relation table 
  // output relation table 
  std::string             m_output   ; ///< output relation table 
  // histogram title 
  std::string             m_title_lt    ; ///< histogram title long 
  std::string             m_title_dt    ; ///< histogram title down 
  std::string             m_title_tt    ; ///< histogram title TTrack
  std::string             m_title_ut    ; ///< histogram title upstr
  std::string             m_title_vt    ; ///< histogram title velo
  double                  m_pScale_lt   ; ///< scale for mom long
  double                  m_pScale_dt   ; ///< scale for mom down
  double                  m_pScale_tt   ; ///< scale for mom TT
  double                  m_pScale_ut   ; ///< scale for val upstr
  double                  m_pScale_vt   ; ///< scale for val velo
  double                  m_vScale_lt   ; ///< scale for val long
  double                  m_vScale_dt   ; ///< scale for val down
  double                  m_vScale_tt   ; ///< scale for val TT
  double                  m_vScale_ut   ; ///< scale for val upstr
  double                  m_vScale_vt   ; ///< scale for val velo
private:
  //  double                    m_pScale ;
  //  double                    m_vScale ;  
  //  const AIDA::IHistogram2D* m_histo  ;
  const AIDA::IHistogram2D* m_histo_lt  ;
  const AIDA::IHistogram2D* m_histo_dt  ;
  const AIDA::IHistogram2D* m_histo_tt  ;
  const AIDA::IHistogram2D* m_histo_ut  ;
  const AIDA::IHistogram2D* m_histo_vt  ;
} ;
// ============================================================================
inline double CaloID2DLL::dLL
( const double             p , 
  const double             v , 
  const LHCb::Track::Types t ) const 
{
  
  const AIDA::IHistogram2D* histo = 0 ;
  double pScale = -1.;
  double vScale = -1.;
  
  switch ( t )
  {
  case LHCb::Track::Long :
    histo  = m_histo_lt ;
    pScale = m_pScale_lt;
    vScale = m_vScale_lt; 
    break ;
  case LHCb::Track::Downstream :
    histo  = m_histo_dt ;
    pScale = m_pScale_dt;
    vScale = m_vScale_dt;
    break ;
  case LHCb::Track::Upstream :
    histo  = m_histo_ut ;
    pScale = m_pScale_ut;
    vScale = m_vScale_ut;
    break ;
  case LHCb::Track::Velo :
    histo  = m_histo_vt ;
    pScale = m_pScale_vt;
    vScale = m_vScale_vt;
    break ;
  case LHCb::Track::Ttrack :
    histo  = m_histo_tt ;
    pScale = m_pScale_tt;
    vScale = m_vScale_tt;
    break ;
  default :
    Error ("Injvald track type, return 0");
    return 0 ;
  }
  
  if ( 0 == histo ) 
  {
    Error ("Histogram is not specified return 0");
    return 0 ;    
  }
  
  const double _x = ::tanh ( p / pScale ) ;
  const double _y = ::tanh ( v / vScale ) ;
  const int    ix = histo->coordToIndexX ( _x ) ;
  const int    iy = histo->coordToIndexY ( _y ) ;
  return histo->binHeight(  ix , iy ) ;

} ;
// ============================================================================


// ============================================================================
// The END 
// ============================================================================
#endif // CALOID2DLL_H
// ============================================================================

