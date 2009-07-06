
//-----------------------------------------------------------------------------
/** @file TrackSelectorBase.cpp
 *
 *  Implementation file for reconstruction tool : TrackSelectorBase
 *
 *  CVS Log :-
 *  $Id: TrackSelectorBase.cpp,v 1.2 2009-07-06 18:29:28 jonrob Exp $
 *
 *  @author Chris Jones   Christopher.Rob.Jones@cern.ch
 *  @date   30/12/2005
 */
//-----------------------------------------------------------------------------

// Gaudi
#include "GaudiKernel/SystemOfUnits.h"

// local
#include "TrackSelectorBase.h"

//-----------------------------------------------------------------------------

TrackSelectorBase::TrackSelectorBase( const std::string& type,
                                      const std::string& name,
                                      const IInterface* parent )
  : GaudiTool ( type, name, parent )
{
  // interface
  declareInterface<ITrackSelector>(this);
  // JOs
  m_trTypes =
    boost::assign::list_of("Velo")("VeloR")("Long")("Upstream")("Downstream")("Ttrack");
  declareProperty( "TrackTypes", m_trTypes );
}

TrackSelectorBase::~TrackSelectorBase() { }

StatusCode TrackSelectorBase::initialize()
{
  const StatusCode sc = GaudiTool::initialize();
  if ( sc.isFailure() ) return sc;

  // initialise track type and alg selections
  m_selTypes.clear();
  if ( !m_trTypes.empty() )
  {
    for ( TrackTypes::const_iterator iT = m_trTypes.begin();
          iT != m_trTypes.end(); ++iT )
    {
      if      ( *iT == "Velo"       ) { m_selTypes[LHCb::Track::Velo]       = true; }
      else if ( *iT == "VeloR"      ) { m_selTypes[LHCb::Track::VeloR]      = true; }
      else if ( *iT == "Long"       ) { m_selTypes[LHCb::Track::Long]       = true; }
      else if ( *iT == "Upstream"   ) { m_selTypes[LHCb::Track::Upstream]   = true; }
      else if ( *iT == "Downstream" ) { m_selTypes[LHCb::Track::Downstream] = true; }
      else if ( *iT == "Ttrack"     ) { m_selTypes[LHCb::Track::Ttrack]     = true; }
      else
      {
        return Error( "Unknown track type '"+*iT+"'" );
      }
    }
    // Note, track types not selected above, will automatically NOT be selected
  }

  return sc;
}
