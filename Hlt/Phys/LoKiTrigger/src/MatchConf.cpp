// $Id$
// ============================================================================
// Include files
// ============================================================================
// LoKi
// ============================================================================
#include "LoKi/MatchConf.h"
#include "LoKi/ToCpp.h"
// ============================================================================
/** @file 
 *  Implementation file for class LoKi::Hlt1::MatchConf
 *  @date 2008-11-14 
 *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
 */
// ============================================================================
/*  constructor 
 *  @param mTool ITrackMatch tool name 
 *  @param address  TES location of "matched" tracks 
 *  @param trType   track type
 *  @param moveIDs  transfer IDs ?
 *  @param moveAncs transfer ancestors ? 
 *  @param moveInfo transfer extra info ? 
 */
// ============================================================================
LoKi::Hlt1::MatchConf::MatchConf 
( const std::string&       mTool    ,   //                ITrackMatch tool name  
  const std::string&       address  ,   //       TES location of matched tracks
  const LHCb::Track::Types trType   ,   //                           Track type 
  const bool               moveIDs  ,   //                       transfer IDs ? 
  const bool               moveAncs ,   //                 transfer ancestors ? 
  const bool               moveInfo )   //                transfer Extra Info ? 
  : LoKi::Hlt1::TrackTool ()
  , m_mTool    ( mTool    )    
  , m_address  ( address  ) 
  , m_trType   ( trType   ) 
  , m_moveIDs  ( moveIDs  ) 
  , m_moveAncs ( moveAncs ) 
  , m_moveInfo ( moveInfo ) 
{} 
// ============================================================================
/*  constructor 
 *  @param mTool ITrackMatch tool name 
 *  @param address  TES location of "matched" tracks 
 *  @param trType   track type
 *  @param moveIDs  transfer IDs ?
 *  @param moveAncs transfer ancestors ? 
 *  @param moveInfo transfer extra info ? 
 */
// ============================================================================
LoKi::Hlt1::MatchConf::MatchConf 
( const std::string&       mTool    ,   //                ITrackMatch tool name  
  const std::string&       address  ,   //       TES location of matched tracks
  const LHCb::Track::Types trType   ,   //                           Track type 
  const bool               moveIDs  ,   //                       transfer IDs ? 
  const bool               moveAncs ,   //                 transfer ancestors ? 
  const bool               moveInfo ,   //                transfer Extra Info ? 
  const LoKi::Functor<const LHCb::Track*,bool>& cut ) //                 filter 
  : LoKi::Hlt1::TrackTool ( cut )
  , m_mTool    ( mTool    ) 
  , m_address  ( address  ) 
  , m_trType   ( trType   ) 
  , m_moveIDs  ( moveIDs  ) 
  , m_moveAncs ( moveAncs ) 
  , m_moveInfo ( moveInfo ) 
{}
// =============================================================================
// printout to ostream
// =============================================================================
std::ostream& LoKi::Hlt1::MatchConf::fillStream ( std::ostream& s ) const 
{
  s << "LoKi.Hlt1.MatchConf("
    << "'" << mTool  () << "',"
    << "'" << address() << "',"
    << "LHCb.Track." << trType() << ","
    << ( moveIDs  () ? "True," : "False," ) 
    << ( moveAncs () ? "True," : "False," ) 
    << ( moveInfo () ? "True," : "False," ) ;
  //
  if ( hasFilter() ) { s << " , " << filter() ; }
  //
  return s << " ) " ;
}
// ============================================================================
// conversion to the string 
// ============================================================================
std::string LoKi::Hlt1::MatchConf::toString () const 
{
  std::ostringstream s ;
  fillStream ( s ) ;
  return s.str() ;
}
// ============================================================================
std::string Gaudi::Utils::toCpp ( const LoKi::Hlt1::MatchConf& t ) 
{
  std::string s = " LoKi::Hlt1::MatchConf(" 
    + toCpp ( t.mTool     () ) + ","
    + toCpp ( t.address   () ) + ","
    + "LHCb::Track::" + LHCb::Track::TypesToString ( t.trType () ) + ","
    + toCpp ( t.moveIDs   () ) + ","
    + toCpp ( t.moveAncs  () ) + ","
    + toCpp ( t.moveInfo  () ) ;
  if ( t.hasFilter() ) { s += "," + toCpp( t.filter() ) ; }
  //
  return s +  ") " ;
}
// ============================================================================
// The END 
// ============================================================================
