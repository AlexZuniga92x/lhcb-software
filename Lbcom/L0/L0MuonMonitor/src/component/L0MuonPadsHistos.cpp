// $Id: L0MuonPadsHistos.cpp,v 1.1 2008-07-24 09:36:53 jucogan Exp $
// Include files 

// from Gaudi
#include "GaudiKernel/ToolFactory.h" 
#include "Kernel/MuonTileID.h"

// local
#include "L0MuonPadsHistos.h"

//-----------------------------------------------------------------------------
// Implementation file for class : L0MuonPadsHistos
//
// 2008-04-02 : 
//-----------------------------------------------------------------------------

// Declaration of the Tool Factory
DECLARE_TOOL_FACTORY( L0MuonPadsHistos );


//=============================================================================
// Standard constructor, initializes variables
//=============================================================================
L0MuonPadsHistos::L0MuonPadsHistos( const std::string& type,
                                      const std::string& name,
                                      const IInterface* parent )
  : GaudiHistoTool ( type, name , parent )
{
  declareInterface<L0MuonPadsHistos>(this);
  
  m_stationLayouts[L0Muon::MonUtilities::M1]=MuonLayout(24,8);
  m_stationLayouts[L0Muon::MonUtilities::M2]=MuonLayout(48,8);
  m_stationLayouts[L0Muon::MonUtilities::M3]=MuonLayout(48,8);
  m_stationLayouts[L0Muon::MonUtilities::M4]=MuonLayout(12,8);
  m_stationLayouts[L0Muon::MonUtilities::M5]=MuonLayout(12,8);
  
  for (int sta=0; sta<L0Muon::MonUtilities::NStations; ++sta) {
    m_hmap[sta]=NULL;  
    m_hmultiBx[sta]=NULL;
    for (int reg=0; reg<L0Muon::MonUtilities::NRegions; ++reg) {
      m_hmultiBx_region[sta][reg]=NULL;
    }
  }
  
}

//=============================================================================
// Destructor
//=============================================================================
L0MuonPadsHistos::~L0MuonPadsHistos() {} 

//=============================================================================

void L0MuonPadsHistos::bookHistos(bool shortname) {
  for (int i=0; i<L0Muon::MonUtilities::NStations; ++i) bookHistos(i, shortname);
}
 
void L0MuonPadsHistos::bookHistos(int sta,bool shortname) {
  std::string toolname="";
  if (!shortname) toolname=name();

  std::string hname;

  int xgrid=m_stationLayouts[sta].xGrid();
  int ygrid=m_stationLayouts[sta].yGrid();
  int nx=xgrid*16;
  int ny=ygrid*16;

  hname = L0Muon::MonUtilities::hname_pads_map(sta,toolname);
  AIDA::IHistogram2D * h2D=book2D(hname,hname,-nx,nx,2*nx,-ny,ny,2*ny);
  m_hmap[sta]= h2D;

  hname = L0Muon::MonUtilities::hname_pads_multiBx(sta,toolname);
  m_hmultiBx[sta]= book2D(hname,hname,-7.5,7.5,15,0.5,10.5,10);;
  for (int reg=0; reg<L0Muon::MonUtilities::NRegions; ++reg) {
    hname = L0Muon::MonUtilities::hname_pads_multiBx_region(sta,reg,toolname);
    m_hmultiBx_region[sta][reg]= book2D(hname,hname,-7.5,7.5,15,0.5,10.5,10);;
  }
} 

void L0MuonPadsHistos::fillHistos(const std::vector<LHCb::MuonTileID> &pads, int ts) 
{ 
  int multi[L0Muon::MonUtilities::NStations][L0Muon::MonUtilities::NRegions];
  for (int sta=0; sta<L0Muon::MonUtilities::NStations; ++sta) {
    for (int reg=0; reg<L0Muon::MonUtilities::NRegions; ++reg) {
      multi[sta][reg]=0;
    }
  }
  
  std::vector<LHCb::MuonTileID>::const_iterator itpads;
  for (itpads=pads.begin();itpads<pads.end();++itpads){
    int sta = itpads->station();
    if (m_hmap[sta]==NULL) continue;
    int qua = itpads->quarter();
    int X  =itpads->nX();
    int Y  =itpads->nY();
    int reg=itpads->region();
    int f=1<<reg;
    for (int ix=X*f; ix<X*f+f; ++ix){
      for (int iy=Y*f; iy<Y*f+f; ++iy){
        int x= ix;
        int y= iy;
        L0Muon::MonUtilities::flipCoord(x,y,qua);
        fill(m_hmap[sta],x,y,1);
      }
    }
    ++multi[sta][reg];
  }

  for (int sta=0; sta<L0Muon::MonUtilities::NStations; ++sta) {
    int n=0;
    for (int reg=0; reg<L0Muon::MonUtilities::NRegions; ++reg) {
      if (m_hmultiBx_region[sta][reg]==NULL) continue;
      n+=multi[sta][reg];
      fill(m_hmultiBx_region[sta][reg],ts,multi[sta][reg],1);
    }
    if (m_hmultiBx[sta]==NULL) continue;
    fill(m_hmultiBx[sta],ts,n,1);
  }
  
}
