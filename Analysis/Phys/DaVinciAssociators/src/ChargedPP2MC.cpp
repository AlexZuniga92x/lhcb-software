// $Id: ChargedPP2MC.cpp,v 1.7 2006-06-23 14:54:38 phicharp Exp $
// Include files 

// from Gaudi
#include "GaudiKernel/AlgFactory.h"
#include "GaudiKernel/IDataProviderSvc.h"
#include "GaudiKernel/SmartDataPtr.h"

// from Event
#include "Event/Track.h"
#include "Event/MCParticle.h"
#include "Event/ProtoParticle.h"

// Associators
#include "Relations/RelationWeighted1D.h"

// local
#include "DaVinciAssociators/Particle2MCLinker.h"
#include "AsctAlgorithm.h"

static const std::string& 
ChargedPP2MCAsctLocation = "Rec/Relations/ChargedPP2MC";

class ChargedPP2MC : public AsctAlgorithm {

  friend class AlgFactory<ChargedPP2MC>;
  
public:
  /// Standard constructor
  ChargedPP2MC( const std::string& name, ISvcLocator* pSvcLocator );

  virtual ~ChargedPP2MC( ); ///< Destructor

  virtual StatusCode initialize();    ///< Algorithm initialization
  virtual StatusCode execute   ();    ///< Algorithm execution
  virtual StatusCode finalize  ();    ///< Algorithm finalization

protected:

private:
  Object2MCLinker* m_track2MCLink;
};

using namespace LHCb;

// Define data types
typedef RelationWeighted1D<ProtoParticle,MCParticle,double>  Table;

//-----------------------------------------------------------------------------
// Implementation file for class : ChargedPP2MC
//
// 10/07/2002 : Philippe Charpentier
//-----------------------------------------------------------------------------

// Declaration of the Algorithm Factory
static const  AlgFactory<ChargedPP2MC>          s_factory ;
const        IAlgFactory& ChargedPP2MCFactory = s_factory ; 

#define _verbose if( msgLevel(MSG::VERBOSE) ) verbose()
#define _debug if( msgLevel(MSG::DEBUG) ) debug()
#define _info if( msgLevel(MSG::INFO) ) info()

//=============================================================================
// Standard constructor, initializes variables
//=============================================================================
ChargedPP2MC::ChargedPP2MC( const std::string& name,
                                        ISvcLocator* pSvcLocator)
  : AsctAlgorithm ( name , pSvcLocator )
{
  m_inputData.push_back( ProtoParticleLocation::Charged );
  m_inputData.push_back( ProtoParticleLocation::Upstream );

  m_outputTable = ChargedPP2MCAsctLocation;
}

//=============================================================================
// Destructor
//=============================================================================
ChargedPP2MC::~ChargedPP2MC() {}; 

//=============================================================================
// Initialisation. Check parameters
//=============================================================================
StatusCode ChargedPP2MC::initialize() {

  _debug << "==> Initialise" << endreq;

  StatusCode sc = GaudiAlgorithm::initialize();
  if( !sc.isSuccess() ) return sc;

  // Get a Linker class for Tr2MCP
  m_track2MCLink = new Object2MCLinker( this, 
                                      "", "", 
                                      TrackLocation::Default);
  return StatusCode::SUCCESS;
};

//=============================================================================
// Main execution
//=============================================================================
StatusCode ChargedPP2MC::execute() {

  _debug << "==> Execute" << endreq;

  // create a Relations table if needed
  Table* table = 
    "" == outputTable() ? NULL : new Table();

  ///////////////////////////////////////
  // Loop on ProtoParticles containers //
  ///////////////////////////////////////

  for( std::vector<std::string>::const_iterator inp = m_inputData.begin(); 
       m_inputData.end()!= inp; inp++) {
    // Get ProtoParticles
    SmartDataPtr<ProtoParticles> protos ( evtSvc(), *inp);
    if( 0 == protos ) continue;
    // Create a linker table
    const std::string linkContainer = 
      *inp + Particle2MCMethod::extension[Particle2MCMethod::ChargedPP];
    // Just a fake helper class
    Object2MCLinker p2MCLink(this);
    Object2MCLinker::Linker*
      linkerTable = p2MCLink.linkerTable( linkContainer );

    if( NULL == table && NULL == linkerTable ) continue;

    int npp = protos->size();
    _verbose << "    " << npp
        << " ProtoParticles retrieved from " 
        << *inp << endreq;
    
    // loop on Protos to match them from their tracks
    int nrel = 0 ;
    int nass = 0;
    for( ProtoParticles::const_iterator pIt=protos->begin() ;
         protos->end() != pIt; pIt++) {
      _verbose
        << "    ProtoParticle " << (*pIt)->key();
      // Follow links from this protoParticle via tracks
      const Track* track = (*pIt)->track() ;
      if( 0 != track ) {
        _verbose
          << " from track " << track->key();
        double weight = 0.;
        const MCParticle* mcPart = m_track2MCLink->first(track, weight);
        if( NULL == mcPart ) {
          _verbose << " not associated to an MCPart";
        } else {
          _verbose << " associated to MCParts";
          nass++;
          do {
            _verbose << " - " << mcPart->key();
            if( NULL != table ) 
              table->relate( *pIt, mcPart, weight);
            if( NULL != linkerTable ) 
              linkerTable->link( *pIt, mcPart, weight);
            nrel++;
            mcPart = m_track2MCLink->next();
          } while( NULL != mcPart );
        }
      } else {
        _verbose << " not originating from a track";
      }
      _verbose << endreq;
    }
    _debug
      << "Out of " << npp << " Charged ProtoParts in " << *inp << ", "
      << nass << " are associated, "
      << nrel << " relations found" << endreq;
  }
  
  // Register the relations table on the TES
  if( NULL != table ) {
    put( table, outputTable() );
    _debug
      << "     Registered table " << outputTable() << endreq;
  }
  return StatusCode::SUCCESS ;
};

//=============================================================================
//  Finalize
//=============================================================================
StatusCode ChargedPP2MC::finalize() {

  _debug << "==> Finalize" << endreq;
  if( NULL != m_track2MCLink ) delete m_track2MCLink;
  m_track2MCLink = NULL;
  return GaudiAlgorithm::finalize();
}

//=============================================================================
