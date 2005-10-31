// $Id: MuonChamberLayout.cpp,v 1.3 2005-10-31 15:27:28 asarti Exp $
// Include files 

//Muon
#include "MuonDet/MuonChamberLayout.h"
#include "MuonDet/MuonChamberGrid.h"
#include "MuonDet/DeMuonDetector.h"

//Detector description
#include "DetDesc/IGeometryInfo.h"
#include "DetDesc/DetectorElement.h"

//standard
#include <iostream>
#include <vector>

//-----------------------------------------------------------------------------
// Implementation file for class : MuonChamberLayout
//
// 2004-01-07 : Alessio Sarti
//-----------------------------------------------------------------------------

//=============================================================================
// Standard constructor, initializes variables
//=============================================================================
MuonChamberLayout::MuonChamberLayout() {

  MuonLayout R1(1,1);  
  MuonLayout R2(1,2);  
  MuonLayout R3(1,4);  
  MuonLayout R4(2,8); 

  //When initialized in this way needs un update for the Layout
  MuonChamberLayout(R1,R2,R3,R4,this->dataSvc());
}


MuonChamberLayout::MuonChamberLayout(MuonLayout R1,
				     MuonLayout R2,
				     MuonLayout R3,
				     MuonLayout R4,
				     IDataProviderSvc* detSvc) {


  //Setting up the layout for the various regions
  m_layout[0] = R1;
  m_layout[1] = R2;
  m_layout[2] = R3;
  m_layout[3] = R4;
  m_detSvc = detSvc;

  //Performing vectors and grid initialization
  initialize(); 
}

StatusCode MuonChamberLayout::initialize() {

  bool debug = false;
  StatusCode sc = DetectorElement::initialize();
  if( sc.isFailure() ) { 
    if(debug) std::cout << "Failure to initialize DetectorElement" << std::endl;
    return sc ; 
  }
  
  //Grid initialization
  int iDd;
  unsigned int cgX[4] = {1,1,1,2}; m_cgX.resize(4);
  for(iDd = 0; iDd<4; iDd++) { m_cgX.at(iDd) = cgX[iDd]; }
  unsigned int cgY[4] = {1,2,4,8}; m_cgY.resize(4);
  for(iDd = 0; iDd<4; iDd++) { m_cgY.at(iDd) = cgY[iDd]; }

  if(debug)  std::cout<<" Building a Grid inside MuonLayout"<<std::endl;

  //Offset: needs to change?
  int myoffset[4] = {0,16,48,112}; m_offSet.resize(4);
  for(iDd = 0; iDd<4; iDd++) { m_offSet.at(iDd) = myoffset[iDd]; }

  //Initializing the chamber grid with detector elements
  m_chamberGrid.resize(368);
  for(int iD = 0; iD<368; iD++) { m_chamberGrid.at(iD) = -1; }

  //Fills the chambers vector
  //  fillChambersVector();
  return sc;
}

//=============================================================================
// Destructor
//=============================================================================
MuonChamberLayout::~MuonChamberLayout() {}; 

void MuonChamberLayout::Copy(MuonChamberLayout &lay) {
  //Copy constructor
  bool debug = false;
  for(int iL = 0; iL<4; iL++) {
    if(debug) std::cout<<"My Copied layout: "<<iL<<std::endl;
    lay.setLayout(iL,this->layout(iL));
  }
  lay.setDataProvider(this->DataProvider());
}



std::vector<DeMuonChamber*> MuonChamberLayout::neighborChambers(DeMuonChamber *Chmb, int x_direction, int y_direction) {

  std::vector<int> myChambers; 
  int sC_idY(0),sC_idX(0),reg(0);
  bool debug = false;

  if(debug) std::cout<<"My Chamber under test. "<<Chmb->chamberNumber()<<" "<<Chmb->regionNumber()<<" "<<Chmb->stationNumber()<<std::endl;
  
  float myX = (Chmb->geometry())->toGlobal(HepPoint3D(0,0,0)).x();
  float myY = (Chmb->geometry())->toGlobal(HepPoint3D(0,0,0)).y();

  gridPosition(myX,myY,Chmb->stationNumber(),sC_idX,sC_idY,reg);

  if(debug) std::cout<<"Position in the grid "<<sC_idX<<" "<<sC_idY<<" "<<reg<<" "<<x_direction<<" "<<y_direction<<std::endl;

  //Deciding the corner-side
  int scX(0),scY(0);
  if(x_direction != 0) scX++;
  if(y_direction != 0) scY++;

  for(int ix = 0; ix <= scX; ix++) {
    for(int iy = 0; iy <= scY; iy++) {
      //Do the computation only for OTHER chambers
      if(ix || iy) {
	chamberXY(sC_idX,sC_idY,ix*x_direction,iy*y_direction,reg,myChambers);
      }
    }
  }

  if(debug) std::cout<<"Returning chambers: "<<myChambers<<std::endl;

  //Now get the chambers
  std::vector<DeMuonChamber*> myChams;
  myChams = createChambers(myChambers,Chmb->stationNumber());
  return myChams;

}

std::vector<DeMuonChamber*> MuonChamberLayout::neighborChambers(int chmbNum, int sta, int reg, int x_dir, int y_dir) {

  char pt[200]; bool debug = false;
  sprintf(pt,"/dd/Structure/LHCb/Muon/M%d/R%d/Cham%03d",sta+1,reg+1,chmbNum+1);
  SmartDataPtr<DeMuonChamber> deChmb(m_detSvc,pt);
  std::vector<DeMuonChamber*> myChams;
  //Create a chamber * Chmb.
  if(deChmb) {
    myChams = neighborChambers(deChmb,x_dir,y_dir);
  } else {
    if(debug)  std::cout<<"Not found chamber! "<<std::endl;
  }
  return myChams;
}

std::vector<DeMuonChamber*> MuonChamberLayout::createChambers(std::vector<int> mytiles, int station){

  std::vector<int>::iterator idTile;
  std::vector<DeMuonChamber*> myChambers;

  bool debug = false;
  for(idTile = mytiles.begin(); idTile<mytiles.end(); idTile++){
    const long int cTile = *idTile;
    //Added protection against non existing chamber
    if(cTile>0) {
      int region = findRegion(cTile);
      if(debug)  std::cout<<"Returned chambers:: "<<station+1<<" "<<region+1<<" "<<cTile-m_offSet.at(region)<<std::endl;
      DeMuonChamber * myChmb = new DeMuonChamber(station,region,cTile-m_offSet.at(region)-1);
      myChambers.push_back(myChmb);
    }
  }
  if(debug)  std::cout<<"Exiting from chamber creation "<<std::endl;

  return myChambers;
}

void MuonChamberLayout::chamberXY(int sx, int sy, int shx, int shy, int reg, std::vector<int> &chamberNumber){

  int vSize(0); int chN = -1;  bool debug = false;
  int fx = sx + shx;
  int fy = sy + shy;

  if(debug) std::cout<<" Chamber XY. Reg: "<<reg<<" "<<m_cgX.at(reg)<<" "<<m_cgY.at(reg)<<" "<<fx<<" "<<fy<<" "<<sx<<" "<<sy<<" "<<shx<<" "<<shy<<std::endl;

  //Protect against chambers outside the current region grid
  if(fx<0 || fy<0 || fx >= 4*m_cgX.at(reg) 
     || fy >= 4*m_cgY.at(reg)) {

    //Starting point in the new (larger) grid
    sy = sy + 2*m_cgY.at(reg);
    if(reg<2) {
      if(sx < 2*m_cgX.at(reg)) { sx = 1; }
      else { sx = 2;}
    } 

    //Goes to other region
    if(reg+1<4) {
      sx = (sx / (2/m_cgX.at(reg+1)))+m_cgX.at(reg+1);
      //Checks in larger grid (higher region)
      chamberXY(sx,sy,shx,shy,reg+1,chamberNumber);
    } else {
      //Exits [chamber outside the R4 grid]
      if(debug) std::cout<<" Chamber not Found (>3) "<<chN<<std::endl;
      chamberNumber.push_back(-1);
      return;
    }
    //Fix the chamber number for chambers in region != from starting one
    vSize = chamberNumber.size();
    if(vSize>0) {
      if(debug) std::cout<<" Encode and chamber in first call: "<<chN<<" :: "<<chamberNumber.at(vSize-1)<<" "<<chamberNumber.size()<<" "<<reg+1<<std::endl;
      if(chamberNumber.at(vSize-1)>-1) return;
    }
  } else {
    //Look inside the grid
    int enc = fx+4*m_cgX.at(reg)*fy+m_offSet.at(reg);
    chN = m_chamberGrid.at(enc);
    if(debug) std::cout<<" Encode and chamber in first call: "<<chN<<" "<<enc<<" "<<reg<<" "<<fx<<" "<<fy<<std::endl;
  }

  //If a chamber number is <0 this means that 
  //-> The chamber is in a different region
  //-> The chamber has NOT been turned on in the xml  
  if(chN<0 && shouldLowReg(fx,fy,reg)) {
    //Found a -1 chamber: need to low
    //the region number (if > 1) to look in inside inner region 
    if(reg-1>=0) {
      //When dropping down the region number 
      if(reg < 3){
	if(fx < 2*m_cgX.at(reg)) { shx = -1; }
	else { shx = 1; }
	} else {
	  shx = -2;
	}
      shy = m_cgY.at(reg);
      if(debug) std::cout<<" Subcall 2: "<<fx<<" "<<fy<<" "<<shx<<" "<<-shy<<" "<<reg-1<<std::endl;
      chamberXY(fx,fy,shx,-shy,reg-1,chamberNumber);
    } else {
      //lowest (inner) region. Exiting
      if(debug) std::cout<<" Chamber not Found (<0) "<<chN<<std::endl;
      chamberNumber.push_back(-1);
      return;
    }
    vSize = chamberNumber.size();
    if(vSize>0) {
      if(debug) std::cout<<" Encode and chamber in second call: "<<chN<<" :: "<<chamberNumber.at(vSize-1)<<" "<<chamberNumber.size()<<" "<<std::endl;
      if(chamberNumber.at(vSize-1)>-1) {
	chN = chamberNumber.at(vSize-1);
      }
    }
    
    //Second chamber needed for regions 0,1,2
    if(reg <3) {
      if((sy/m_cgY.at(reg) < 1) || (sy/m_cgY.at(reg) > 2)) {
	if(reg-1>0) { 
	  if(debug) std::cout<<" Third call "<<std::endl;
	  chamberXY(fx,fy,0,-m_cgY.at(reg),reg-1,chamberNumber);
	} else {
	  if(debug) std::cout<<" Chamber not Found "<<std::endl;
	  chamberNumber.push_back(-1);
	  return;
	}
	vSize = chamberNumber.size();
	if(vSize) {
	  if(debug) std::cout<<" Encode and chamber in third call: "<<chN<<" :: "<<chamberNumber.at(vSize-1)<<" "<<chamberNumber.size()<<" "<<std::endl;
	  if(chamberNumber.at(vSize-1)>-1) return;
	}
      }
    }
  } else if(chN != -1) {
    //Need a +1 to avoid skipping chamber n.1 (idx 0)
    chamberNumber.push_back(chN+m_offSet.at(reg)+1);
    if(debug) std::cout << "Closing Chamber " << chN 
			<< " in R" << reg
			<< " xIndex " << fx
			<< " yIndex " << fy << std::endl;
  } else {
    std::cout << "Chamber " << chN 
	      << " in R" << reg
	      << " xIndex " << fx
	      << " yIndex " << fy
	      << " is not in TES/xml. " << std::endl;
  }

  return;
}

int MuonChamberLayout::findRegion(int chamber){
  int reg(0);
  int offset[4] = {16,48,112,368};
  for(reg=0; reg<4; reg++) {
    if(chamber < offset[reg]) break;
  }
  if(reg>3) {
    std::cout<<"Region not found for chamber: "<<chamber<<". Go back and check the code!"<<std::endl;
    reg = -1;
  }
  return reg; 
}

void MuonChamberLayout::chamberMostLikely(float x,float y, int station, int& chmb, int& reg){
  
  int myReg(-1);
  int fx(-1),fy(-1);

  if(m_xS.at(station) && m_yS.at(station)) {    
    gridPosition(x,y,station,fx,fy,myReg);
  } else {
    std::cout << "Null chamber dimensions"<< std::endl;
  }

  //Here's the chamber!!!!
  if(fx > -1 && fy > -1) {
    //Look inside the grid
    int enc = fx+4*m_cgX.at(myReg)*fy+m_offSet.at(myReg);
    chmb = m_chamberGrid.at(enc);
    reg = myReg;
  } else {
    std::cout<<"Matrix Index problem!!!!"<<std::endl;
  }

  return;
}

//Returns the Tile for a given chamber
MuonTileID MuonChamberLayout::tileChamber(DeMuonChamber* chmb){

  bool debug = false;
  MuonTileID myTile;

  //Region and station got from chamber
  myTile.setStation(chmb->stationNumber());
  unsigned int reg = chmb->regionNumber();
  myTile.setRegion(reg);

  //Gap is not used here
  myTile.setLayer(0);

  myTile.setLayout(MuonLayout(m_cgX.at(reg),m_cgY.at(reg)));

  //Chamber number is needed to find x,y
  unsigned int chN = chmb->chamberNumber();
  unsigned int mychN;
  unsigned int fx(0),fy(0),enc(0);
  for(fx = 0; fx<4*m_cgX.at(reg); fx++){
    for(fy = 0; fy<4*m_cgY.at(reg); fy++){
      enc = fx+4*m_cgX.at(reg)*fy+m_offSet.at(reg);
      mychN = m_chamberGrid.at(enc);
      if(chN == mychN) {

	//set X and Y
	myTile.setX(fx);
	myTile.setY(fy);
	
	// quarter definition according to D.Hutchcroft
	//      +y          
	//    3 | 0
	//   ---+--- +x
	//    2 | 1
	
	//Why do I need to care about Quarter? Still I implement it
	int myQuarter = -1;
	if(fx >= m_cgX.at(reg)/2) {
	  myQuarter = 0;
	  if(fy >= m_cgY.at(reg)/2) {
	    myQuarter = 1;
	  }
	} else {
	  myQuarter = 3;
	  if(fy >= m_cgY.at(reg)/2) {
	    myQuarter = 2;
	  }
	}
	myTile.setQuarter(myQuarter);
	
	if(debug) std::cout<<"X, Y and Q " <<fx<<" "<<fy<<" "<<myQuarter<<" for chamber "<<chN<<" "<<reg<<std::endl;
	return myTile;
      }
    }
  }
  
  std::cout<<"X, Y and Q not found! Check for grid problems" <<std::endl;
  return myTile;
}


//Returns the Tile for a given chamber number
MuonTileID MuonChamberLayout::tileChamberNumber(int sta, int reg, int chmbNum){


  char pt[200];  MuonTileID myTile;
  sprintf(pt,"/dd/Structure/LHCb/Muon/M%d/R%d/Cham%03d",sta+1,reg+1,chmbNum+1);

  SmartDataPtr<DeMuonChamber> deChmb(m_detSvc,pt);
  if(deChmb) {
    myTile = tileChamber(deChmb);
  } else {
    std::cout<<" Could not find the chamber "<<pt<<" in the TES. Check the configuration!"<<std::endl;
  }
  return myTile;
}


void MuonChamberLayout::gridPosition(float x, float y, int iS, int &idx, int &idy, int &reg){
  
  int myIdx(0),myIdy(0),myReg(-1);
  float rX(0),rY(0);
  bool debug = false;
  float xs = m_xS.at(iS);   float ys = m_yS.at(iS); 
  //Extracting chamber indexes in the finest grid.
  myIdx = (int)x/xs;  myIdy = (int)y/ys;
  rX = x-xs*myIdx; rY = y-ys*myIdy;
  if(debug) std::cout << "Rest: "<<rX<<" "<<rY<<" "<<xs<<" "<<ys<<" "<<x<<" "<<y<< std::endl;
  for(int iRx = 0; iRx<4; iRx++) {
    if(debug)	std::cout << "Idx in loop grid: "<<myIdx+2*(int)m_cgX.at(iRx)<<" "<<myIdy+2*(int)m_cgY.at(iRx)<<" for region: "<<iRx<<std::endl;
    //To be checked fator 2 in last gridX
    if(abs(myIdx)<2*(int)m_cgX.at(iRx) && abs(myIdy)<2*(int)m_cgY.at(iRx)){
      myReg = iRx; 
      break;
    } else {
      if(iRx < 2) myIdx /= 2;
    }
  }
  if(myIdy<=0 && rY<0) myIdy--;
  if(myIdx<=0 && rX<0) myIdx--;

  idx = myIdx+2*(int)m_cgX.at(myReg);
  idy = myIdy+2*(int)m_cgY.at(myReg);
  reg = myReg;
  if(debug)  std::cout << "Final Idxs grid: "<<idx<<" "<<idy<<" for region: "<<myReg<<std::endl;
  return;
}

void MuonChamberLayout::setGridStep(){
  float dimX[5] = {240,301.363,325.155,348.947,372.739};
  float dimY[5] = {200,251.136,270.962,290.789,310.615};
  for(int iDum = 0; iDum<5; iDum++) {
    m_xS.push_back(dimX[iDum]);  m_yS.push_back(dimY[iDum]);
  }
}


void MuonChamberLayout::fillChambersVector(IDataProviderSvc* detSvc) {
  
  int idx(-1),idy(-1),reg(-1);
  bool debug = false;
  SmartDataPtr<DetectorElement> muonSys (detSvc,
					 "/dd/Structure/LHCb/Muon"); 

  //Getting stations
  IDetectorElement::IDEContainer::iterator itSt=muonSys->childBegin();
  int iS = 0;
  //Set the grid steps for the various stations
  setGridStep();
  
  for(itSt=muonSys->childBegin(); itSt<muonSys->childEnd(); itSt++){
    
    //Getting regions
    IDetectorElement::IDEContainer::iterator itRg=(*itSt)->childBegin();
    for(itRg=(*itSt)->childBegin(); itRg<(*itSt)->childEnd(); itRg++){
      
      //Getting chambers
      IDetectorElement::IDEContainer::iterator itCh=(*itRg)->childBegin();
      for(itCh=(*itRg)->childBegin(); itCh<(*itRg)->childEnd(); itCh++){

	SmartDataPtr<DeMuonChamber> deChmb(detSvc,(*itCh)->name());
	float myX = (deChmb->geometry())->toGlobal(HepPoint3D(0,0,0)).x();
	float myY = (deChmb->geometry())->toGlobal(HepPoint3D(0,0,0)).y();
	gridPosition(myX,myY,iS,idx,idy,reg);
	
	int enc = idx+4*m_cgX.at(reg)*idy+m_offSet.at(reg);
	m_chamberGrid.at(enc) = deChmb->chamberNumber();
	if(debug)  std::cout<<"Grid initialization: "<<enc<<" "<<deChmb->chamberNumber()<<std::endl;
      }
    }
    //next station
    iS++;
  }
  if(debug) std::cout<<"Filled chamber vector"<<std::endl;
  return;
}

bool MuonChamberLayout::shouldLowReg(int idX, int idY, int reg){
  bool lower = true;
  bool iXnz = (((idX <  m_cgX.at(reg))&&(idX>0)) || ((idX>=3*m_cgX.at(reg))&&(idX<4*m_cgX.at(reg))));
  bool iYnz = (((idY <  m_cgY.at(reg))&&(idY>0)) || ((idY>=4*m_cgY.at(reg))&&(idX<4*m_cgY.at(reg))));
  if(iXnz && iYnz) {lower = false;}
  return lower;
}
