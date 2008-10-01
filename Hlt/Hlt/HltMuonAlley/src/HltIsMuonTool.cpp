// $Id: HltIsMuonTool.cpp,v 1.9 2008-10-01 09:32:53 leandrop Exp $
// Include files 

// from Gaudi
#include "GaudiKernel/ToolFactory.h" 

//boost
#include <boost/assign/list_of.hpp>

// local
#include "HltIsMuonTool.h"
#include "Event/MuonCoord.h"

//-----------------------------------------------------------------------------
// Implementation file for class :HltIsMuonTool
//
// 2008-01-25 : Leandro de Paula
//-----------------------------------------------------------------------------

// Declaration of the Tool Factory
DECLARE_TOOL_FACTORY( HltIsMuonTool );

using namespace LHCb;

//=============================================================================
// Standard constructor, initializes variables
//=============================================================================
HltIsMuonTool::HltIsMuonTool( const std::string& type,
                              const std::string& name,
                              const IInterface* parent )
  : GaudiTool ( type, name , parent )
{
// FOI parameters from off line   28/1/8
  declareProperty( "par1x", m_p1x = boost::assign::list_of (12.)(17.)(6.0)(6.7));
  declareProperty( "par2x", m_p2x = boost::assign::list_of (13.)(21.)(10.)(15.));
  declareProperty( "par3x", m_p3x = boost::assign::list_of (.11)(.09)(.09)(.10));
  declareProperty( "par1y", m_p1y = boost::assign::list_of (6.7)(9.0)(11.)(12.));
  declareProperty( "par2y", m_p2y = boost::assign::list_of (3.4)(3.4)(3.5)(3.4));
  declareProperty( "par3y", m_p3y = boost::assign::list_of (.02)(.02)(.02)(.02));

  declareProperty("UseFastDecoding", m_useFastDecoding = true );
  declareInterface<ITrackFunctionTool>(this);
}
//=============================================================================
// Destructor
//=============================================================================
HltIsMuonTool::~HltIsMuonTool() {} 

//=============================================================================
//=============================================================================
// Initialization
//=============================================================================
StatusCode HltIsMuonTool::initialize() {
  StatusCode sc = GaudiTool::initialize(); // must be executed first
  if ( sc.isFailure() ) return sc;  // error printed already by GaudiAlgorithm

  if (msgLevel (MSG::DEBUG)) debug() << "==> Initialize" << endmsg;
  m_iPosTool  = tool<IMuonPosTool>( "MuonPosTool" );
  if(!m_iPosTool)if (msgLevel (MSG::DEBUG)) debug()<<"error retrieving the pos tool "<<endreq;
  m_muonBuffer=tool<IMuonRawBuffer>("MuonRawBuffer");
  if(!m_muonBuffer)info()<<"error retrieving the decoding tool "<<endreq;

  return StatusCode::SUCCESS;
}

//=============================================================================

double  HltIsMuonTool::function(const Track& ctrack)
{
  
  // A dirty trick!
  Track* track = (Track*) &ctrack;
  
  std::vector< LHCb::LHCbID >  lista=   track->lhcbIDs ();
  MuonTileID tileM5;
  MuonTileID tileM4;
  MuonTileID tileM3;
  MuonTileID tileM2;
  std::vector<LHCbID>::iterator it;
  bool inFOIM5=false;
  bool inFOIM2=false;
  bool muonSeg=false;
  
  // looping over lhcbIDs 
  for(it=lista.end()-1;it>=lista.begin();it--){ 
    if(it->isMuon()){
      MuonTileID tile=it->muonID();
      if(tile.station()==4){
        muonSeg=true;
        tileM5=tile;
        // M5 hit out of FOI or invalid
        if(!(tileM5.isValid())||!(isInFOI(track,tileM5))) return 0;
      }else if(tile.station()==3){
        tileM4=tile;
        // M4 hit out of FOI or invalid
        if(!(tileM4.isValid())||!(isInFOI(track,tileM4))) return 0;
      }else if(tile.station()==2){
        tileM3=tile;
        // M3 hit out of FOI or invalid
        if(!(tileM3.isValid())||!(isInFOI(track,tileM3))) return 0;
      }else if(tile.station()==1){
        tileM2=tile;
        if(!(tileM2.isValid()))continue;
        // M2 hits inside FOI
         if(isInFOI(track,tileM2)) {inFOIM2=true; break;}
      }else{
        // All M2 hits outside FOI
        return 0;
      }
    }
  }
  if(!inFOIM2) return 0;
  if(muonSeg) return 1;
  
  
  if(!m_useFastDecoding){
    //M4 and M5 for L0 candidates
    LHCb::MuonCoords* coords =  get<LHCb::MuonCoords>(LHCb::MuonCoordLocation::MuonCoords);
    if ( coords==0 ) {
      err() << " Cannot retrieve MuonCoords " << endreq;
      return StatusCode::FAILURE;
    }
    
    // loop over the coords starting from M5
    LHCb::MuonCoords::const_iterator iCoord;
    for ( iCoord = coords->end() -1 ; iCoord >= coords->begin() ; iCoord-- ){
      int region = (*iCoord)->key().region();
      int station = (*iCoord)->key().station();
      if (msgLevel (MSG::DEBUG)) debug()<<"Station "<<station<<" - Region "<<region<<endreq;
      //if(region!=0 && (*iCoord)->uncrossed())continue;
      if(station==4){
        if(inFOIM5) continue;
        // M5 hit found inside FOI
        if(isInFOI(track,(*iCoord)->key())) inFOIM5=true;
      }else if(station==3){
        // All M5 hits outside FOI
        if(!inFOIM5) return 0;
        // M2, M3, M4 and M5 hits inside FOI
        if(isInFOI(track,(*iCoord)->key())) return 1;
      }else{
        // All M4 hits outside FOI
        return 0;
      }
    }
  }else{
    std::vector<std::vector<LHCb::MuonTileID>* > pads;
    std::vector<LHCb::MuonTileID>::iterator iPad;
    std::vector<std::vector<LHCb::MuonTileID>* >::iterator iList;
    StatusCode sc=m_muonBuffer->getPadsInStation(4,pads);
    if(sc.isFailure())return sc;
    for(iList=pads.begin();iList!=pads.end();iList++){
      for(iPad=(*iList)->begin();iPad!=(*iList)->end();iPad++){
        if(inFOIM5) continue;
        // M5 hit found inside FOI
        if(isInFOI(track,(*iPad))) inFOIM5=true;
      }
    }
    sc=m_muonBuffer->getPadsInStation(3,pads);
      if(sc.isFailure())return sc;
      for(iList=pads.begin();iList!=pads.end();iPad++){
        for(iPad=(*iList)->begin();iPad!=(*iList)->end();iPad++){
          // All M5 hits outside FOI
          if(!inFOIM5) return 0;
          // M2, M3, M4 and M5 hits inside FOI
          if(isInFOI(track,(*iPad))) return 1;
        }
        // All M4 hits outside FOI
        return 0;
      }  
  }
  // no muon hits
  return 0;
} 
//=====================================
bool  HltIsMuonTool::isInFOI(Track* track, MuonTileID tileMX){
 
  
  int region = tileMX.region();

  double x,y,z,dx,dy,dz;
  StatusCode sc = m_iPosTool-> calcTilePos(tileMX,x, dx,y, dy,z, dz);  
  if (!sc) {
    Warning("Failure from Pos Tool");
    return false ;
  }
  const State& stato=track->closestState(z);
  double xslope=stato.slopes().x()/stato.slopes().z();
  double yslope=stato.slopes().y()/stato.slopes().z();
  double zpoint=stato.z();
  double x_MX=stato.x()+xslope*(z-zpoint);
  double y_MX=stato.y()+yslope*(z-zpoint); 
  double ppoint=stato.p()/Gaudi::Units::GeV;
  double x_window = (m_p1x[region]+m_p2x[region]*exp(-m_p3x[region]*ppoint))*dx;
  double y_window = (m_p1y[region]+m_p2y[region]*exp(-m_p3y[region]*ppoint))*dy;

  bool result = ((fabs(x_MX-x)<x_window)&&(fabs(y_MX-y)<y_window));

  if (msgLevel (MSG::DEBUG)) {
    debug()<<" station M"<< tileMX.station()+1 <<" "<< x_window <<" "<< x <<" "<< x_MX <<" "<< dx << endreq ;
    debug()<<"   region " <<          region+1 <<" "<< y_window <<" "<< y <<" "<< y_MX <<" "<< dy <<endreq;
    debug() << (result?"accepted":"bad") << endreq;
  }
  
  return result;
}
