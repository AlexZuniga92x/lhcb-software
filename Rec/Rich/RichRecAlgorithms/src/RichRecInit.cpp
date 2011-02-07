
//--------------------------------------------------------------------------
/** @file RichRecInit.cpp
 *
 *  Implementation file for algorithm class : RichRecInit
 *
 *  @author Chris Jones   Christopher.Rob.Jones@cern.ch
 *  @date   17/04/2002
 */
//--------------------------------------------------------------------------

// local
#include "RichRecInit.h"

// All code is in general Rich reconstruction namespace
using namespace Rich::Rec;

//--------------------------------------------------------------------------

// Declaration of the Algorithm Factory
DECLARE_ALGORITHM_FACTORY( Initialise );

// Standard constructor, initializes variables
Initialise::Initialise( const std::string& name,
                        ISvcLocator* pSvcLocator )
  : Rich::Rec::AlgBase ( name, pSvcLocator )
{
  declareProperty( "CheckProcStatus", m_checkProcStatus = true  );
  declareProperty( "CreatePixels",    m_makePixels      = false );
  declareProperty( "CreateTracks",    m_makeTracks      = false );
  declareProperty( "CreatePhotons",   m_makePhotons     = false );
  declareProperty( "LoadRawTracks",   m_loadRawTracks   = false );
}

// Destructor
Initialise::~Initialise() {}

StatusCode Initialise::initialize()
{
  const StatusCode sc = Rich::Rec::AlgBase::initialize();
  if ( sc.isFailure() ) return sc;

  // preload tools
  if ( m_loadRawTracks || m_makeTracks ) trackCreator();
  if ( m_makePixels                    ) pixelCreator();
  if ( m_makePhotons                   ) photonCreator();

  return sc;
}

StatusCode Initialise::execute()
{
  // Event Status
  if ( !richStatus()->eventOK() ) return StatusCode::SUCCESS;

  // check for a ProcStat object
  if ( m_checkProcStatus )
  {
    // needed when running special modes where it is not available
    if ( !exist<LHCb::ProcStatus>(LHCb::ProcStatusLocation::Default) )
    {
      Warning( "No ProcStatus at " + LHCb::ProcStatusLocation::Default + " : Creating one",
               StatusCode::SUCCESS,0 ).ignore();
      LHCb::ProcStatus * procS = new LHCb::ProcStatus();
      put( procS, LHCb::ProcStatusLocation::Default );
    }
  }

  // raw tracks
  if ( m_loadRawTracks )
  {
    trackCreator()->nInputTracks();
  }

  // Pixels
  if ( m_makePixels )
  {
    if ( !pixelCreator()->newPixels() ) return StatusCode::FAILURE;
  }

  // Tracks
  if ( m_makeTracks )
  {
    if ( !trackCreator()->newTracks() ) return StatusCode::FAILURE;
  }

  // Photons
  if ( m_makePhotons )
  {
    if ( !photonCreator()->reconstructPhotons() ) return StatusCode::FAILURE;
  }

  return StatusCode::SUCCESS;
}
