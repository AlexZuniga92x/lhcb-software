// $Id: TTrackAndDistCut.cpp,v 1.7 2009-10-08 21:10:33 graven Exp $
// Include files 

// from Gaudi
#include "GaudiKernel/DeclareFactoryEntries.h" 
#include "Event/MuonCoord.h"


// local
#include "TTrackAndDistCut.h"
using namespace LHCb;

//-----------------------------------------------------------------------------
// Implementation file for class : TTrackAndDistCut
//
// 2007-02-21 : Alessia Satta
//-----------------------------------------------------------------------------

// Declaration of the Algorithm Factory
DECLARE_ALGORITHM_FACTORY( TTrackAndDistCut );


//=============================================================================
// Standard constructor, initializes variables
//=============================================================================
TTrackAndDistCut::TTrackAndDistCut( const std::string& name,
                                ISvcLocator* pSvcLocator)
  : HltAlgorithm ( name , pSvcLocator )
  , m_selections(*this)
{
  declareProperty( "maxDist",m_maxdist=100);
  m_selections.declareProperties();
  
}
//=============================================================================
// Destructor
//=============================================================================
TTrackAndDistCut::~TTrackAndDistCut() {} 

//=============================================================================
// Initialization
//=============================================================================
StatusCode TTrackAndDistCut::initialize() {
  StatusCode sc = HltAlgorithm::initialize(); // must be executed first
  if ( sc.isFailure() ) return sc;  // error printed already by GaudiAlgorithm
  debug() << "==> Initialize" << endmsg;

  m_selections.retrieveSelections();
  m_selections.registerSelection();

  m_iPosTool  = tool<IMuonFastPosTool>( "MuonFastPosTool" );
  if(!m_iPosTool)info()<<"error retrieving the pos tool "<<endreq;

  return StatusCode::SUCCESS;
}

//=============================================================================
// Main execution
//=============================================================================
StatusCode TTrackAndDistCut::execute() {

  debug() << "==> Execute" << endmsg;
  setFilterPassed(true);
  for ( std::vector<Track*>::const_iterator itT = m_selections.input<1>()->begin();
        m_selections.input<1>()->end() != itT; itT++ ) {   
    MuonTileID tileM4, tileM5;
    double dist = calcDLL(*itT, tileM4, tileM5);    
    if(dist<m_maxdist){ 
       (*itT)->addToAncestors (*itT);
       (*itT)->addToLhcbIDs(tileM4);
       (*itT)->addToLhcbIDs(tileM5);
       (*itT)->addInfo(hltInfoID("MuonTdist"),dist);
       m_selections.output()->push_back(*itT);
    }
  }
  
  
  return StatusCode::SUCCESS;
}

//=============================================================================



double  TTrackAndDistCut::calcDLL(Track* track, MuonTileID & tileM4, MuonTileID & tileM5)
{

  double distDLL=0;
  std::vector< LHCb::LHCbID >  lista=   track->lhcbIDs ();
  MuonTileID tileM2;
  MuonTileID tileM3;

      
  std::vector<LHCbID>::iterator it;
  for(it=lista.begin();it<lista.end();it++){ 
    if(it->isMuon())
    {
      MuonTileID tile=it->muonID();
      if(tile.station()==1)tileM2=tile;
      if(tile.station()==2)tileM3=tile;
       
    }
     
  }
  if(!(tileM2.isValid())||!(tileM3.isValid()))return 10000;
  
  //M2 and M3
  double x,y,z,dx,dy,dz;

  m_iPosTool-> calcTilePos(tileM2,x, dx,y, dy,z, dz);  

  State* stato=NULL;
  stato=&(track->firstState());
  double xpoint=stato->x();
  double ypoint=stato->y();
  double zpoint=stato->z();
  double xslope=stato->slopes().x()/stato->slopes().z();
  double yslope=stato->slopes().y()/stato->slopes().z();
  double x_M2=xpoint+xslope*(z-zpoint);
  double y_M2=ypoint+yslope*(z-zpoint); 
  distDLL=((x_M2-x)/dx)*((x_M2-x)/dx)+
    ((y_M2-y)/dy)*((y_M2-y)/dy);   
  debug()<<" dist M2 "<<distDLL<<" "<<x<<" "<<dx<<" "<<x_M2<<endreq;
  debug()<<" dist M2 "<<distDLL<<" "<<y<<" "<<dy<<" "<<y_M2<<endreq;
  //info()<<
  m_iPosTool-> calcTilePos(tileM3,x, dx,y, dy,z, dz);
  double x_M3=xpoint+xslope*(z-zpoint);
  double y_M3=ypoint+yslope*(z-zpoint);
  distDLL+=((x_M3-x)/dx)*((x_M3-x)/dx)+
    ((y_M3-y)/dy)*((y_M3-y)/dy);

  debug()<<" dist M3 "<<distDLL<<endreq;

  //M4 and M5




  LHCb::MuonCoords* coords =  get<LHCb::MuonCoords>(LHCb::MuonCoordLocation::MuonCoords);


  if ( coords==0 ) {
    err() << " Cannot retrieve MuonCoords " << endreq;
    return StatusCode::FAILURE;
  }
  // loop over the coords
  double dist_tempM4=100000;
  double dist_tempM5=100000;
  LHCb::MuonCoords::const_iterator iCoord;
  for ( iCoord = coords->begin() ; iCoord != coords->end() ; iCoord++ ){
    int region = (*iCoord)->key().region();
    int station = (*iCoord)->key().station();
    if(station==0||station==1||station==2)continue;
    if(region!=0&&(*iCoord)->uncrossed())continue;
    debug()<<station<<" "<<region<<endreq;

    m_iPosTool-> calcTilePos( (*iCoord)->key(),x, dx,y, dy,z, dz);
    double dist_temp=0;
    double x_Mi=xpoint+xslope*(z-zpoint);
    double y_Mi=ypoint+yslope*(z-zpoint);
    dist_temp=((x_Mi-x)/dx)*((x_Mi-x)/dx)+
      ((y_Mi-y)/dy)*((y_Mi-y)/dy);
   
    if(station==3){
      if(double(dist_temp)<double(dist_tempM4)){
         dist_tempM4=dist_temp;
         tileM4 = (*iCoord)->key();
      }
    }

    if(station==4){
      if(double(dist_temp)<double(dist_tempM5)){
         dist_tempM5=dist_temp;
         tileM5 = (*iCoord)->key();
      }
    }
  }

  distDLL+=dist_tempM4;
  debug()<<" dist M4 "<<distDLL<<endreq;
  distDLL+=dist_tempM5;
  debug()<<" dist M5 "<<distDLL<<endreq;
  return distDLL;
}  
