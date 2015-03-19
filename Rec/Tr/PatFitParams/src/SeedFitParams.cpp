// Include files 

// from Gaudi
#include "GaudiKernel/AlgFactory.h" 
#include "Event/MCTrackInfo.h"

// from LHCbEvent
#include "Event/MCParticle.h"
#include "Event/MCVertex.h"
#include "Event/MCHit.h"

#include "Event/StateParameters.h"

#include <unordered_map>
#include <boost/assign/list_of.hpp>
#include <boost/array.hpp>

// local
#include "SeedFitParams.h"

//-----------------------------------------------------------------------------
// Implementation file for class : SeedFitParams
//
// 2006-12-08 : Olivier Callot
//-----------------------------------------------------------------------------

// Declaration of the Algorithm Factory
DECLARE_ALGORITHM_FACTORY( SeedFitParams )


//=============================================================================
// Standard constructor, initializes variables
//=============================================================================
SeedFitParams::SeedFitParams( const std::string& name,
                              ISvcLocator* pSvcLocator)
  : GaudiTupleAlg ( name , pSvcLocator ),
    m_fitTool(NULL)
{
  declareProperty( "TupleName",          m_tupleName  = "Track" );
  declareProperty( "ZRef",               m_zRef       = StateParameters::ZMidT );
  declareProperty( "ZSeed",              m_zSeed      = StateParameters::ZEndT );
  declareProperty( "ZTT",                m_zTT        = 2485.0 * Gaudi::Units::mm );

  using namespace boost::assign;
  // these are the "tunables" for PatSeeding
  // for many of these parameters, PatSeeding only takes the first one
  // while we fit for several to disentangle dependencies which would
  // move the values we'd obtain away from their "best" value
  {
    std::vector<double> tmp = list_of(4.21826e-09)(-8.93796e-08)(0.372981);
    declareProperty( "InitialArrowParams", m_initialArrowParams  = tmp);
  }
  {
    std::vector<double> tmp = list_of(40.3751)(1163.24)(-682850);
    declareProperty( "MomentumScaleParams",m_momentumScaleParams  = tmp);
  }
  {
    std::vector<double> tmp = list_of(5372.27)(-3111.41)(384.74);
    declareProperty( "zMagnetParams",      m_zMagParams = tmp);
  }

  {
    std::vector<double> tmp = list_of(-3.77e-4)(4.7);
    declareProperty( "dRatioParams",      m_dRatio = tmp);
  }
  {
    std::vector<double> tmp = list_of(1.25e-14);
    declareProperty( "yCorrectionParams", m_yCorrection = tmp);
  }
 
  m_nEvent = 0;
  m_nTrack = 0;
}
//=============================================================================
// Destructor
//=============================================================================
SeedFitParams::~SeedFitParams() {} 

//=============================================================================
// Initialization
//=============================================================================
StatusCode SeedFitParams::initialize() {
  StatusCode sc = GaudiTupleAlg::initialize(); // must be executed first
  if ( sc.isFailure() ) return sc;  // error printed already by GaudiAlgorithm

  debug() << "==> Initialize" << endmsg;

  m_initialArrowPar.init( "InitialArrow"   , m_initialArrowParams );
  m_momentumScalePar.init( "MomentumScale" , m_momentumScaleParams );
  m_zMagPar.init( "zMagnet"  , m_zMagParams );
  m_dRatioPar.init( "dRatio", m_dRatio );
  m_yCorrectionPar.init( "yCorrection", m_yCorrection );

  m_fitTool = tool<FitTool>( "FitTool" );

  return StatusCode::SUCCESS;
}

//=============================================================================
// Main execution
//=============================================================================
StatusCode SeedFitParams::execute() {

  debug() << "==> Execute" << endmsg;

  ++m_nEvent;

  const auto* partCtnr = get<LHCb::MCParticles>( LHCb::MCParticleLocation::Default );

  MCTrackInfo trackInfo( evtSvc(), msgSvc() );

  // Get the MC hits, and create our own "linkers"
  std::unordered_map<const LHCb::MCParticle*, std::vector<const LHCb::MCHit*> > tlinker;
  std::unordered_map<const LHCb::MCParticle*, std::vector<const LHCb::MCHit*> > ttlinker;
  {
    const auto* itHits = get<LHCb::MCHits>(LHCb::MCHitLocation::IT);
    const auto* otHits = get<LHCb::MCHits>(LHCb::MCHitLocation::OT);
    const auto* ttHits = get<LHCb::MCHits>(LHCb::MCHitLocation::TT);
    for (const auto* hit: *itHits) tlinker[hit->mcParticle()].push_back(hit);
    for (const auto* hit: *otHits) tlinker[hit->mcParticle()].push_back(hit);
    for (const auto* hit: *ttHits) ttlinker[hit->mcParticle()].push_back(hit);
    // sanitise linker table(s) - MCHits must be unique; free up unused memory
    for (auto& kvpair: tlinker) {
      std::sort(std::begin(kvpair.second), std::end(kvpair.second));
      kvpair.second.erase(std::unique(
	    std::begin(kvpair.second), std::end(kvpair.second)),
	  std::end(kvpair.second));
      std::vector<const LHCb::MCHit*> tmp(std::begin(kvpair.second), std::end(kvpair.second));
      kvpair.second.swap(tmp);
      // sort by increasing z
      std::sort(std::begin(kvpair.second), std::end(kvpair.second),
	      [] (const LHCb::MCHit* h1, const LHCb::MCHit* h2) {
	      return h1->midPoint().z() < h2->midPoint().z(); });
    }
    for (auto& kvpair: ttlinker) {
      std::sort(std::begin(kvpair.second), std::end(kvpair.second));
      kvpair.second.erase(std::unique(
	    std::begin(kvpair.second), std::end(kvpair.second)),
	  std::end(kvpair.second));
      std::vector<const LHCb::MCHit*> tmp(std::begin(kvpair.second), std::end(kvpair.second));
      kvpair.second.swap(tmp);
      // sort by increasing z
      std::sort(std::begin(kvpair.second), std::end(kvpair.second),
	      [] (const LHCb::MCHit* h1, const LHCb::MCHit* h2) {
	      return h1->midPoint().z() < h2->midPoint().z(); });
    }
  }

  for (const auto part: *partCtnr) {
    if (!trackInfo.fullInfo(part)) continue;
    if (!trackInfo.hasT(part)) continue;
    if (!trackInfo.hasTT(part) && !trackInfo.hasVelo(part)) continue;
    const auto* vOrigin = part->originVertex();
    if (!vOrigin) continue;
    //== No velo -> request that the mother comes from near the beam line
    if (!trackInfo.hasVelo(part)) {
      const auto* mother = vOrigin->mother();
      if (!mother) continue;
      if (!mother->originVertex()) continue;
      if (std::abs(mother->originVertex()->position().rho()) > 4.) continue;
    }
    const bool hasInteractionVertex = std::end(part->endVertices()) ==
      std::find_if(std::begin(part->endVertices()), std::end(part->endVertices()),
	  [] (const LHCb::MCVertex* vtx) {
	  return vtx->position().z() <= StateParameters::ZEndT; });
    if (hasInteractionVertex) continue;

    double momentum = part->momentum().R();
    if ( 0 > part->particleID().threeCharge() ) {
      momentum = -1 * momentum;
    }

    // A container for used hits
    std::vector<Gaudi::XYZPoint> trHits;
    std::vector<Gaudi::XYZPoint> ttHits;

    if (std::end(tlinker) != tlinker.find(part)) {
      trHits.reserve(tlinker[part].size());
      for (const auto* hit: tlinker[part]) trHits.push_back(hit->midPoint());
    }
    if (std::end(ttlinker) != ttlinker.find(part)) {
      ttHits.reserve(ttlinker[part].size());
      for (const auto* hit: ttlinker[part]) ttHits.push_back(hit->midPoint());
    }
    
    ++m_nTrack;
    if ( msgLevel( MSG::DEBUG ) ) {
      debug() << format( "Track MC %4d z0 %7.2f p%8.2f Velo%2d TT%2d T%2d",
                         part->key(), vOrigin->position().z(), momentum/1000,
                         trackInfo.hasVelo( part ),
                         trackInfo.hasTT( part ),
                         trackInfo.hasT( part )  ) << endmsg;
      for ( std::vector<Gaudi::XYZPoint>::iterator itP = trHits.begin(); trHits.end() != itP ; ++itP ) {
        debug() << format( "  x%8.2f y%8.2f z%8.2f ", (*itP).x(), (*itP).y(), (*itP).z() ) << endmsg;
      }
    }


    Tuple tTrack = nTuple( m_tupleName, m_tupleName );
    tTrack->column( "moment",  momentum );
    double pz = part->momentum().Z();
    tTrack->column( "slopeX",  part->momentum().X() / pz );
    tTrack->column( "slopeY",  part->momentum().Y() / pz );

    double ax, bx, cx, dx, ay, by;
    
    m_fitTool->fitCubic( trHits, 0, m_zRef, ax, bx, cx, dx );
    m_fitTool->fitLine ( trHits, 1, m_zRef, ay, by );

    m_momentumScalePar.setFun( 0, 1. );
    m_momentumScalePar.setFun( 1, by * by );
    m_momentumScalePar.setFun( 2, fabs(cx) );

    double dp = 1. / (cx * momentum) - m_momentumScalePar.sum();
    m_momentumScalePar.addEvent( dp );

    if ( msgLevel( MSG::DEBUG ) ) {
      debug() << format( "p %7.3f, N%4d, ax%8.2f bx%8.2f cx%8.2f dp%10.4f",
                         momentum/1000., trHits.size(),
                         ax, 1.e3*bx, 1.e6*cx, dp ) << endmsg;
    }    

    // work out intercept point of line joining T1 and T3 with z = 0
    const double dzT1 = StateParameters::ZBegT - m_zRef;
    const double dzT3 = StateParameters::ZEndT - m_zRef;
    const double xT1 = ((dx * dzT1 + cx) * dzT1 + bx) * dzT1 + ax;
    const double xT3 = ((dx * dzT3 + cx) * dzT3 + bx) * dzT3 + ax;
    const double tx13 = (xT3 - xT1) / (dzT3 - dzT1);
    const double xAtZero = ax - tx13 * m_zRef;
    // initial arrow depends in y and cx in T, so we determine it as
    // InitialArrow(by) = InitialArrow[0] + InitialArrow[1] * by * by
    // 		+ InitialArrow[2] * cx * cx
    m_initialArrowPar.setFun( 0, 1. );
    m_initialArrowPar.setFun( 1, by*by );
    m_initialArrowPar.setFun( 2, cx*cx );
    double dSag = 0.;
    if (1. < fabs(xAtZero)) {
      dSag = -cx / xAtZero - m_initialArrowPar.sum();
      m_initialArrowPar.addEvent( dSag );
    }


    double axt, bxt, cxt, ayt, byt, zMagnet, zEst;
    if ( 2 < ttHits.size() ) {
      //m_fitTool->fitLine( ttHits, 0, m_zTT, axt, bxt );
      m_fitTool->fitParabola( ttHits, 0, m_zTT, axt, bxt, cxt );
      m_fitTool->fitLine( ttHits, 1, m_zTT, ayt, byt );
      
      double dz = m_zSeed - m_zRef;
      double x0 = ax + dz * ( bx + dz * ( cx + dz * dx ) );
      double tx = bx + dz * (2*cx + dz * 3*dx );

      zMagnet = (axt-bxt*m_zTT - (x0-tx*m_zSeed) ) / (tx-bxt);
      
      m_zMagPar.setFun( 0, 1. );
      m_zMagPar.setFun( 1, by*by );
      m_zMagPar.setFun( 2, bx*bx );
      zEst = m_zMagPar.sum();
      if ( fabs( zMagnet-zEst) < 100. ) m_zMagPar.addEvent( zMagnet-zEst );

    } else {
      zMagnet = 0.;
      zEst = 0.;
      axt = 0.;
      bxt = 0.;
      cxt = 0.;
      ayt = 0.;
      byt = 0.;
    }

    m_dRatioPar.setFun(0, 1.0);
    m_dRatioPar.setFun(1, fabs(cx));
    if (fabs(cx) > 1e-10 && fabs(dx) > 1e-15)
      m_dRatioPar.addEvent(dx / cx - m_dRatioPar.sum());

    // check if we can improve pointing constraint in y
    // use only tracks which do not point to the origin too well, for those
    // with fabs(y(y=0)) < 40 cm, the correction is small anyway, so we avoid
    // to pollute our statistics with them (if the correction is slightly
    // off for the well pointing tracks, they will still point rather well if
    // we under- or overcorrect a little)
    const double y0 = ay - by * m_zRef;
    m_yCorrectionPar.setFun(0, 1.0);
    if (400. < fabs(y0))
      m_yCorrectionPar.addEvent(fabs((by * by * cx * cx) / y0) - m_yCorrectionPar.sum());
      
    tTrack->column( "ax", ax );
    tTrack->column( "bx", bx );
    tTrack->column( "cx", cx );
    tTrack->column( "dx", dx );
    tTrack->column( "ay", ay );
    tTrack->column( "by", by );

    tTrack->column( "axt", axt );
    tTrack->column( "bxt", bxt );
    tTrack->column( "cxt", cxt );

    tTrack->column( "ayt", ayt );
    tTrack->column( "byt", byt );

    tTrack->column( "x0", xAtZero );
    tTrack->column( "dp", dp );
    tTrack->column( "dSag", dSag );

    tTrack->column( "zMagnet", zMagnet );
    tTrack->column( "zEst", zEst );

    tTrack->write();
  }

  return StatusCode::SUCCESS;
}

//=============================================================================
//  Finalize
//=============================================================================
StatusCode SeedFitParams::finalize() {

  debug() << "==> Finalize" << endmsg;
  MsgStream& msg = info();
  msg << "============================================" << endmsg;
  msg << "  Processed " << m_nEvent << " events and " << m_nTrack << " tracks. " << endmsg;
  msg << "============================================" << endmsg;
  m_initialArrowPar.updateParameters( msg );
  m_momentumScalePar.updateParameters(  msg );
  m_zMagPar.updateParameters(  msg );
  m_dRatioPar.updateParameters(  msg );
  m_yCorrectionPar.updateParameters(  msg );

  std::cout << std::endl;
  m_initialArrowPar.printPythonParams( name() );
  m_momentumScalePar.printPythonParams( name() );
  m_zMagPar.printPythonParams( name() );
  m_dRatioPar.printPythonParams( name() );
  m_yCorrectionPar.printPythonParams( name() );
  std::cout << std::endl;
  
  std::string toolName = "ToolSvc.PatSeedingTool";
  
  m_initialArrowPar.printPythonParams( toolName );
  m_momentumScalePar.printPythonParams( toolName );
  m_zMagPar.printPythonParams( toolName );
  m_dRatioPar.printPythonParams( toolName );
  m_yCorrectionPar.printPythonParams( toolName );
  std::cout << std::endl;

  return GaudiTupleAlg::finalize();  // must be called after all other actions
}

//=============================================================================
