// $Id: CaloTrack2IDAlg.cpp,v 1.1 2006-06-18 18:35:27 ibelyaev Exp $
// ============================================================================
// CVS tag $Name: not supported by cvs2svn $, version $Revision: 1.1 $
// ============================================================================
// $Log: not supported by cvs2svn $ 
// ============================================================================
// Include files 
// ============================================================================
// GaudiKernel
// ============================================================================
#include "GaudiKernel/DeclareFactoryEntries.h" 
// ============================================================================
// Relations 
// ============================================================================
#include "Relations/Relation1D.h"
// ============================================================================
// Event 
// ============================================================================
#include "Event/Track.h"
// ============================================================================
// Boost 
// ============================================================================
#include "boost/static_assert.hpp"
// ============================================================================
// local
// ============================================================================
#include "CaloTrack2IDAlg.h"
// ============================================================================

// ============================================================================
/// Declaration of the Algorithm Factory
// ============================================================================
DECLARE_ALGORITHM_FACTORY( CaloTrack2IDAlg );
// ============================================================================
/// Standard protected constructor
// ============================================================================
CaloTrack2IDAlg::CaloTrack2IDAlg 
( const std::string& name , 
  ISvcLocator*       pSvc ) 
  : CaloTrackAlg( name , pSvc ) 
  // input tracks 
  , m_inputs   ( 1 , LHCb::TrackLocation::Default ) ///< input tracks 
  // output data 
  , m_output   () ///< output data
  // filter data 
  , m_filter   () ///< filter data
  // too to use 
  , m_toolName ( "<NOT DEFINED>" ) ///< tool to use 
  // tool itself 
  , m_tool ( 0 ) ///< tool itself
{
  declareProperty ( "Inputs"      , m_inputs   ) ;
  declareProperty ( "Output"      , m_output   ) ;
  declareProperty ( "Filter"      , m_filter   ) ;
  declareProperty ( "Tool"        , m_toolName ) ;
  //
  setProperty     ( "StatPrint"   , "false"    ) ;
  // track types:
  setProperty     ( "AcceptedType" , Gaudi::Utils::toString<int>
                    ( LHCb::Track::Long       ,
                      LHCb::Track::Downstream ,
                      LHCb::Track::Ttrack     ) ) ;
} ;
// ============================================================================
/// standard algorithm initilization
// ============================================================================
StatusCode CaloTrack2IDAlg::initialize () 
{
  StatusCode sc = CaloTrackAlg::initialize () ;
  if ( sc.isFailure() ) { return sc ; }
  //
  m_tool = tool<ICaloTrackIdEval> ( m_toolName , this ) ;
  //
  if ( m_inputs.empty() ) { Warning ( "empty 'Inputs'-list"  ) ; }
  if ( m_output.empty() ) { Warning ( "empty 'Output'-value" ) ; }
  if ( m_filter.empty() ) { Warning ( "empty 'Filter'-value" ) ; }
  //
  return StatusCode::SUCCESS ;
} ;
// ============================================================================
#define INHERITS(T1,T2) \
     (Relations::IsConvertible<const T1*,const T2*>::value && \
     !Relations::IsConvertible<const T1*,const void*>::same)
// ============================================================================
/// standard algorithm execution
// ============================================================================
StatusCode CaloTrack2IDAlg::execute () 
{
  typedef LHCb::Track::Container                Tracks ;
  typedef LHCb::Relation1D<LHCb::Track,float>   Table  ;
  typedef LHCb::Calo2Track::ITrAccTable         Filter ;
  
  BOOST_STATIC_ASSERT(INHERITS(Table,LHCb::Calo2Track::ITrEvalTable));
  
  Table* table = new Table ( 100 ) ;
  put ( table , m_output ) ;
  
  Filter* filter  = 0 ;
  if ( !m_filter.empty() ) { filter = get<Filter> ( m_filter ) ; }
  
  Assert ( !m_inputs.empty() , "No input data specified!" ) ;

  size_t nTracks = 0 ;
  // loop over input containers 
  for ( Inputs::const_iterator input = m_inputs.begin() ;
        m_inputs.end() != input ; ++input ) 
  {
    const Tracks* tracks = get<Tracks>( *input ) ;
    // loop over all tracks 
    for ( Tracks::const_iterator itrack = tracks->begin() ; 
          tracks->end() != itrack ; ++itrack ) 
    {
      const LHCb::Track* track = *itrack ;
      ++nTracks ;
      // use the track ?
      if ( !use ( track ) ) { continue ; }                      // CONTINUE
      // use filter ?
      if ( 0 != filter  ) 
      {
        Filter::Range r = filter->relations( track ) ;
        // no positive information? skip! 
        if ( r.empty() || !r.front().to() ) { continue ; }      // CONTINUE
      }
      double value = 0 ;
      StatusCode sc = m_tool->process ( track , value ) ;
      if ( sc.isFailure() ) 
      {
        Warning ( " Failure from the tool, skip the track!", sc ) ;
        continue ;                                              // CONTINUE 
      }
      // make a relations (fast, efficient, call for i_sort is mandatory!)
      table->i_push( track, (float) value ) ;        // NB! i_push
    }
  }
  // MANDATORY after i_push!                         // NB! i_sort 
  table->i_sort() ;
  
  if ( statPrint() || msgLevel ( MSG::DEBUG ) )
  {
    counter ( "#tracks" ) += nTracks ;
    Table::Range links = table->i_relations() ;
    counter ( "#links"  ) += links .size() ;
    StatEntity& energy = counter("#energy") ;
    for ( Table::iterator link = links.begin() ; links.end() != link ; ++link ) 
    { energy += link->to() / Gaudi::Units::GeV ; }
  }
  
  return StatusCode::SUCCESS ;
} ;
// ============================================================================

// ============================================================================
// The END 
// ============================================================================
