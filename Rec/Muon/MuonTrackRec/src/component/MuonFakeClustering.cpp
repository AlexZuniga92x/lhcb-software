// $Id: MuonFakeClustering.cpp,v 1.1 2009-10-16 17:05:58 ggiacomo Exp $
// Include files 

// from Gaudi
#include "GaudiKernel/DeclareFactoryEntries.h" 
#include "GaudiKernel/IIncidentSvc.h" 
// local
#include "MuonFakeClustering.h"
#include "MuonTrackRec/MuonLogPad.h"
#include "MuonTrackRec/MuonHit.h"
#include "MuonTrackRec/OfflineTimeAlig.h"
#include "MuonDet/IMuonFastPosTool.h"
#include "MuonDet/DeMuonDetector.h"
using namespace LHCb;
using namespace std;
using namespace OfflineTimeAlig;

//-----------------------------------------------------------------------------
// Implementation file for class : MuonFakeClustering
//
// produce fake clusters by just copying the list of MuonLogPads to MuonHits
// 2009-10-15 : Giacomo GRAZIANI
//-----------------------------------------------------------------------------

// Declaration of the Tool Factory
DECLARE_TOOL_FACTORY( MuonFakeClustering );



MuonFakeClustering::MuonFakeClustering( const std::string& type,
                                        const std::string& name,
                                        const IInterface* parent )
  : GaudiTool ( type, name , parent ) 
{
  declareInterface<IMuonClusterRec>(this);
  declareProperty( "PosTool"          , m_posToolName = "MuonDetPosTool");
  declareProperty( "OfflineTimeAlignment", m_offlineTimeAlignment = false );
  declareProperty( "TimeResidualFile" , m_timeResidualFile = "none");
  m_clustersDone = false;
}

MuonFakeClustering::~MuonFakeClustering() {
  clearResMap();
  clear();
} 



StatusCode MuonFakeClustering::initialize () {
  StatusCode sc = GaudiTool::initialize() ;
  
  if (!sc) return sc;
  m_muonDetector = getDet<DeMuonDetector>(DeMuonLocation::Default);
  if(!m_muonDetector){
    err()<<"error retrieving the Muon detector element "<<endreq;
    return StatusCode::FAILURE;
  }
  m_posTool = tool<IMuonFastPosTool>(m_posToolName);
  if(!m_posTool) {
    error()<<"error retrieving the muon position tool "<<endreq;
    return StatusCode::FAILURE;
  }
  
  if(!loadTimeRes()){
    err()<<"Time corrections file "<<m_timeResidualFile<<" not found"<<endmsg;
    return StatusCode::FAILURE;
  }

  incSvc()->addListener( this, IncidentType::EndEvent );
  return sc;
}

void MuonFakeClustering::handle ( const Incident& incident )
{ 
  if ( IncidentType::EndEvent == incident.type() ) {
    verbose() << "End Event: clear everything"<<endmsg;
    clear() ;
  }
}

void MuonFakeClustering::clear() {
  std::vector<MuonHit*>::iterator ih;
  for(ih=m_clusters.begin() ; ih !=m_clusters.end() ; ih++)
    delete (*ih); // delete all the allocated MuonHit's
  m_clusters.clear();
  m_clustersDone=false;
}


StatusCode MuonFakeClustering::finalize ()

{
  debug()<<" MuonFakeClustering::finalize"<<endmsg;
  return   GaudiTool::finalize() ;
}


const std::vector<MuonHit*>* MuonFakeClustering::clusters(const std::vector<MuonLogPad*>* pads, 
                                                          bool force) {

  std::vector<MuonLogPad*>::const_iterator ipad;

  if( (m_clustersDone == false || force==true) && pads) {
    int nhits=0;
    clear();
    
    if (pads) {
      for (ipad = pads->begin(); ipad != pads->end(); ipad++ ){
        if(! (*ipad)->truepad() ) continue;
        // correct for time misalig. if requested
        if (m_offlineTimeAlignment) correctMisAlignment(*ipad);

        // fill the hit array
        // create a MuonHit object
        MuonHit* muon_Hit = new MuonHit(m_muonDetector, *ipad, 
                                        m_posTool );
        nhits++;
        // store a progressive hit number for debugging purposes
        StatusCode sc = muon_Hit->setHitID(nhits);
        
        m_clusters.push_back(muon_Hit);
      }
    }
  }
  if (pads) m_clustersDone = true;
  return &m_clusters;
}
