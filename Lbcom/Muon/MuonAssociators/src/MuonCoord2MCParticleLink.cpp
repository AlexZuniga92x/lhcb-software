// $Id: MuonCoord2MCParticleLink.cpp,v 1.2 2007-01-15 16:42:28 cattanem Exp $
// Include files 

// from Gaudi
#include "GaudiKernel/AlgFactory.h" 

// local
#include "MuonCoord2MCParticleLink.h"
#include "Linker/LinkerWithKey.h"

//-----------------------------------------------------------------------------
// Implementation file for class : MuonCoord2MCParticleLink
//
// 2005-12-29 : Alessia Satta
//-----------------------------------------------------------------------------

// Declaration of the Algorithm Factory
DECLARE_ALGORITHM_FACTORY( MuonCoord2MCParticleLink );

using namespace LHCb;


//=============================================================================
// Standard constructor, initializes variables
//=============================================================================
MuonCoord2MCParticleLink::MuonCoord2MCParticleLink( const std::string& name,
                                                  ISvcLocator* pSvcLocator)
  : GaudiAlgorithm ( name , pSvcLocator )
{
  // declareProperty("AssociateAll" , m_associateAll=true) ;
}
//=============================================================================
// Destructor
//=============================================================================
MuonCoord2MCParticleLink::~MuonCoord2MCParticleLink() {}; 

//=============================================================================
// Initialization
//=============================================================================
StatusCode MuonCoord2MCParticleLink::initialize() {
  StatusCode sc = GaudiAlgorithm::initialize(); // must be executed first
  if ( sc.isFailure() ) return sc;  // error printed already by GaudiAlgorithm

  debug() << "==> Initialize" << endmsg;
  m_muonBuffer=tool<IMuonRawBuffer>("MuonRawBuffer");

  return StatusCode::SUCCESS;
};

//=============================================================================
// Main execution
//=============================================================================
StatusCode MuonCoord2MCParticleLink::execute() {

  debug() << "==> Execute" << endmsg;

  LinkerWithKey<LHCb::MCParticle,LHCb::MuonCoord> myLink(eventSvc(),
                                             msgSvc(),
                                             rootOnTES()+LHCb::MuonCoordLocation::MuonCoords);
  
  //get the digit link
  LinkedTo<LHCb::MCParticle,LHCb::MuonDigit>
    digitLink( evtSvc(), msgSvc(),LHCb::MuonDigitLocation::MuonDigit);
  if(digitLink.notFound())info()<<" my link not found "<<endmsg;
  m_digitLink=&(digitLink);
  list_digit=m_muonBuffer->getTile();
  //get the list of coords

  LHCb::MuonCoords* coords =
    get<LHCb::MuonCoords>(rootOnTES()+LHCb::MuonCoordLocation::MuonCoords);
  
  // loop and link MuonDigits to MC truth
  LHCb::MuonCoords::const_iterator iCoord;
  //debug()<<" qui "<<endreq;
  for(iCoord = coords->begin(); iCoord != coords->end(); iCoord++){
    //info()<<" qui 1 "<<endreq;
    //const LHCb::MCParticle* mcpart=NULL;
    //debug()<<" qui 2 "<<endreq;
    ///double w=1.0;
    std::map<const LHCb::MCParticle*,double> partMap;

    StatusCode sc=associateToTruth(*iCoord,partMap);
    if( sc.isFailure() ) {
      debug() << "Invalid layer/region" << endmsg;
      continue;
    }
    
    std::map<const LHCb::MCParticle*,double>::const_iterator iterMap;
    for( iterMap = partMap.begin(); iterMap!= partMap.end(); iterMap++ ) {
      myLink.link(*iCoord,iterMap->first,iterMap->second);
    // info()<<" set link "<<iterMap->first<<" "<<iterMap->second<<" "<<iterMap->first->particleID().pid()<<endreq;
    }
    
  }
  return StatusCode::SUCCESS;
};

//=============================================================================
//  Finalize
//=============================================================================
StatusCode MuonCoord2MCParticleLink::finalize() {

  debug() << "==> Finalize" << endmsg;

  return GaudiAlgorithm::finalize();  // must be called after all other actions
}

//=============================================================================
StatusCode MuonCoord2MCParticleLink:: associateToTruth(MuonCoord *coord,std::map<const LHCb::MCParticle*,double>& partMap)
{


  MuonTileID tile=coord->key();
  int station=tile.station();
  int region=tile.region();
  int typeLay=-1;
  if(station==0)typeLay=0;
  if(station==1)typeLay=1;
  if(station==2)typeLay=1;
  if(station==3)typeLay=1;
  if(station==4)typeLay=1;
  if(station==3&&region==0)typeLay=0;
  if(station==4&&region==0)typeLay=0;
//info()<<typeLay<<endreq;
  if(typeLay<0)return StatusCode::FAILURE;
  std::vector<LHCb::MuonTileID>::iterator iDigit;
  if(typeLay==0){
    //correspondance one to one digit coords
    for(iDigit = list_digit.begin(); iDigit != list_digit.end(); iDigit++){
      MuonTileID digitile=(*iDigit);
      if((digitile.intercept(tile)).isValid()){
        MCParticle* pplink = m_digitLink->first( digitile );
        if(pplink)partMap[pplink]=1.0;
        break;
        
      }
      
    }
    
  }else{
    if(coord->uncrossed()){
      for(iDigit = list_digit.begin(); iDigit != list_digit.end(); iDigit++){
        MuonTileID digitile=(*iDigit);
        if((digitile.intercept(tile)).isValid()){
          MCParticle* pplink = m_digitLink->first( digitile );
          if(pplink)partMap[pplink]=1.0;
          break;
          
        }
        
      }
      
    }else{

      //check if one or two
      for(iDigit = list_digit.begin(); iDigit != list_digit.end(); iDigit++){
        MuonTileID digitile=(*iDigit);
        if((digitile.intercept(tile)).isValid()){
          MCParticle* pplink = m_digitLink->first( digitile );
          //if gia esiste metti a 1 il peso altrimenti aggiungi e lascia a 0.5 ilpeso
          if(pplink)partMap[pplink]+=0.5;          
        }        
      }  
    }    
  }
  return StatusCode::SUCCESS;
}

