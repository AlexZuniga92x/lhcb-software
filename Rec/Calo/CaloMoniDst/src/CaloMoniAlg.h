#ifndef CALOMONIDST_CALOMONIALG_H 
#define CALOMONIDST_CALOMONIALG_H 1
// ============================================================================
// Include files
// ============================================================================
// from Gaudi
// ============================================================================
#include "GaudiKernel/AlgFactory.h"
#include "GaudiAlg/GaudiHistoAlg.h"
#include "CaloUtils/Calo2Dview.h"
#include "Kernel/CaloCellID.h"
#include "GaudiKernel/HashMap.h"
#include "AIDA/IAxis.h"
#include "AIDA/IHistogram1D.h"
#include "AIDA/IHistogram2D.h"
// ============================================================================
// AIDA 
// ============================================================================
#include "AIDA/IHistogram1D.h"

// @class CaloMoniAlg CaloMoniAlg.h
//
//   @see GaudiHistoAlg
//   @see GaudiAlgorithm
//   @see      Algorithm
//   @see     IAlgorithm

class CaloMoniAlg : public Calo2Dview{
public:
// Standard constructor
//   @param   name        algorithm name
//   @param   pSvcLocator pointer to service locator
  CaloMoniAlg( const std::string &name, ISvcLocator *pSvcLocator );  
  virtual ~CaloMoniAlg();
  virtual StatusCode initialize();    ///< Algorithm initialization
  virtual StatusCode finalize();    ///< Algorithm initialization
  
// address/location/name in Transient Store of input data container
  const std::string              &inputData() const { return m_inputData; }
// vector of input data containers
  const std::vector<std::string> &inputs()    const { return m_inputs; }
// address/location/name in Transient Store of detector data
  const std::string              &detData()   const { return m_detData; }
// set address/location/name in Transient Store of input data container
  void setInputData( const std::string &addr ) { m_inputData = addr; }
// add address/location/name in Transient Store of input data container
  void addToInputs(  const std::string &addr ) { m_inputs.push_back( addr ); }
// set address/location/name in Transient Store of detector data
  void setDetData(   const std::string &addr ) { m_detData = addr; }
// booking histogram
  inline void initCounters(){
    m_count = 0;
    for(unsigned int i = 0 ;i != m_nAreas ; ++i){
      m_mcount[i]=0;
    }
  }
  inline void count(LHCb::CaloCellID id = LHCb::CaloCellID() ){
    m_count++;
    if( !(id == LHCb::CaloCellID()) ){
      int area = id.area();
      m_mcount[area]++;
    }
  }
  inline void fillCounters(std::string unit){
    //info() << "filling " << unit << " " << m_count <<  endmsg  ;
    fill(h1[unit], m_count , 1);
    if( m_split ){
      for(unsigned int i = 0;i != m_nAreas;++i)
      {
        //std::string area = CaloCellCode::CaloAreaFromNum( CaloCellCode::CaloNumFromName( m_detData ), i );
        std::string area = CaloCellCode::caloArea ( CaloCellCode::caloNum ( m_detData ), i );
        if( !validArea( area )||  m_mcount[i] == 0)continue;
        GaudiAlg::HistoID id(area + "/"+unit);
        fill(h1[id], m_mcount[i] , 1);
      }    
    }
  }
  inline void hBook1( const std::string hid,
                      const std::string titl,
                      const double low=0,
                      const double high=100,
                      const unsigned long bins=100 ){ 
    if(!doHisto(hid))return;
    if(m_split){
      for(unsigned int i = 0;i != m_nAreas;++i){
        // std::string area = CaloCellCode::CaloAreaFromNum( CaloCellCode::CaloNumFromName( m_detData ), i );
        std::string area = CaloCellCode::caloArea ( CaloCellCode::caloNum( m_detData ), i );
        if( !validArea( area ))continue;
        GaudiAlg::HistoID id(area + "/" + hid);
        std::string tit = titl + " (" + area + ")";
        h1[id] = book1D( id, tit, low, high, bins );
      }
      h1[hid] = book1D( hid, titl, low, high, bins );
    }else{
      h1[hid] = book1D( hid, titl, low, high, bins );
    }    
  }
  
  inline void hBook2( const std::string hid,
                      const std::string titl,
                      const double lowx=0,
                      const double highx=100,
                      const unsigned long binsx=100,
                      const double lowy=0,
                      const double highy=100,
                      const unsigned long binsy=100 ){ 
    if(!doHisto(hid))return;
    if( m_split ){
      for(unsigned int i = 0;i != m_nAreas;++i){
        // std::string area = CaloCellCode::CaloAreaFromNum( CaloCellCode::CaloNumFromName( m_detData ), i );
        std::string area = CaloCellCode::caloArea ( CaloCellCode::caloNum( m_detData ), i );
        if( !validArea( area ))continue;
        GaudiAlg::HistoID id(area + "/" + hid);
        std::string tit = titl + " (" + area + ")";
        h2[id] = book2D( id, tit, lowx, highx, binsx, lowy, highy, binsy );
      }
      h2[hid] = book2D( hid, titl, lowx, highx, binsx, lowy, highy, binsy );
    }else{
      h2[hid] = book2D( hid, titl, lowx, highx, binsx, lowy, highy, binsy );
    }
  }
  
// fill histogram
  inline void hFill1(std::string hid, double value, double w=1. ){ 
    if(!doHisto(hid))return;
    AIDA::IHistogram1D* h  = h1[hid];
    if( NULL == h )return;
    double bins = (double) h->axis().bins();
    double step = h->axis().upperEdge() - h->axis().lowerEdge();
    step = ( bins == 0) ? 0 : step/bins/2.;
    if(m_sat){
      if( value < h->axis().lowerEdge() )value = h->axis().lowerEdge()+step;
      if( value > h->axis().upperEdge() )value = h->axis().upperEdge()-step;
    }
    fill(h,value,w);
  }
  inline void hFill2(std::string hid, double x, double y, double w=1. ){ 
    if(!doHisto(hid))return;
    AIDA::IHistogram2D* h  = h2[hid];
    if( NULL == h )return;
    double xbins = (double) h->xAxis().bins();
    double xstep = h->xAxis().upperEdge() - h->xAxis().lowerEdge();
    xstep = ( xbins == 0) ? 0 : xstep/xbins/2.;
    double ybins = (double) h->yAxis().bins();
    double ystep = h->yAxis().upperEdge() - h->yAxis().lowerEdge();
    ystep = ( ybins == 0) ? 0 : ystep/ybins/2.;

    if(m_sat2D){
      if( x < h->xAxis().lowerEdge() )x = h->xAxis().lowerEdge()+xstep;
      if( x > h->xAxis().upperEdge() )x = h->xAxis().upperEdge()-xstep;
      if( y < h->yAxis().lowerEdge() )y = h->yAxis().lowerEdge()+ystep;
      if( y > h->yAxis().upperEdge() )y = h->yAxis().upperEdge()-ystep;
    }
    fill(h,x,y,w); 
  }
  inline void hFill1(LHCb::CaloCellID cellID , std::string hid, double value, double w=1. ){ 
    if(!doHisto(hid))return;
    AIDA::IHistogram1D* h  = h1[hid];
    if( NULL == h )return;
    double bins = (double) h->axis().bins();
    double step = h->axis().upperEdge() - h->axis().lowerEdge();
    step = ( bins == 0) ? 0 : step/bins/2.;
    if(m_sat){
      if( value < h->axis().lowerEdge() )value = h->axis().lowerEdge()+step;
      if( value > h->axis().upperEdge() )value = h->axis().upperEdge()-step;
    }
    if( m_split && !(cellID == LHCb::CaloCellID()) ) {
      // std::string area = CaloCellCode::CaloAreaFromNum( CaloCellCode::CaloNumFromName( m_detData ), cellID.area() );
      std::string area = CaloCellCode::caloArea ( CaloCellCode::caloNum( m_detData ), cellID.area() );
      if( validArea( area ) ){
        GaudiAlg::HistoID id(area + "/" + hid);
        info() << " id " << id <<endreq;
        IHistogram1D* hh = h1[id];
        if( NULL == hh )return;
        fill(hh,value,w);
      }
      fill(h,value,w);
    }else{
      fill(h,value,w);
    } 
  }
  inline void hFill2( LHCb::CaloCellID cellID , std::string hid, double x, double y, double w=1. ){ 
    if(!doHisto(hid))return;
    AIDA::IHistogram2D* h  = h2[hid];
    if( NULL == h )return;
    double xbins = (double) h->xAxis().bins();
    double xstep = h->xAxis().upperEdge() - h->xAxis().lowerEdge();
    xstep = ( xbins == 0) ? 0 : xstep/xbins/2.;
    double ybins = (double) h->yAxis().bins();
    double ystep = h->yAxis().upperEdge() - h->yAxis().lowerEdge();
    ystep = ( ybins == 0) ? 0 : ystep/ybins/2.;
    if(m_sat2D){
      if( x < h->xAxis().lowerEdge() )x = h->xAxis().lowerEdge()+xstep;
      if( x > h->xAxis().upperEdge() )x = h->xAxis().upperEdge()-xstep;
      if( y < h->yAxis().lowerEdge() )y = h->yAxis().lowerEdge()+ystep;
      if( y > h->yAxis().upperEdge() )y = h->yAxis().upperEdge()-ystep;
    }
    if( m_split && !(cellID == LHCb::CaloCellID()) ){
      // std::string area = CaloCellCode::CaloAreaFromNum( CaloCellCode::CaloNumFromName( m_detData ), cellID.area() );
      std::string area = CaloCellCode::caloArea ( CaloCellCode::caloNum ( m_detData ), cellID.area() );
      if( validArea( area )  ){
        GaudiAlg::HistoID id(area + "/" + hid);
        AIDA::IHistogram2D* hh  = h2[id];
        fill(hh,x,y,w); 
      }      
      fill(h,x,y,w); 
    }else{
      fill(h,x,y,w); 
    } 
  }
protected:
  //
  // Histogram Map
  //std::map< const std::string, AIDA::IHistogram1D * > h1;
  //std::map< const std::string, AIDA::IHistogram2D * > h2;
  GaudiUtils::HashMap< std::string, AIDA::IHistogram1D * > h1;
  GaudiUtils::HashMap< std::string, AIDA::IHistogram2D * > h2;
  //
private:
  // address/location/name in Transient Store of input data container
  std::string              m_inputData;
  // vector of addresses in the case of 'few' inputs
  std::vector<std::string> m_inputs;
// address/location/name in Transient Store of detector data
  std::string              m_detData;
  
protected:
  //
  double m_energyMin;
  double m_etMin;
  double m_massMin;
  double m_yMin;
  double m_xMin;
  double m_multMin;
  double m_sizeMin;
  double m_energyMax;
  double m_etMax;
  double m_massMax;
  double m_yMax;
  double m_xMax;
  double m_multMax;
  double m_sizeMax;
  int m_energyBin;
  int m_etBin;
  int m_massBin;
  int m_yBin;
  int m_xBin;
  int m_multBin;
  int m_sizeBin;

  double m_eFilter;
  double m_etFilter;
  double m_massFilterMin; 
  double m_massFilterMax;
  std::vector<std::string> m_histoList;
  std::vector<std::string> m_removeHisto;
  //  bool m_split;
  std::vector<std::string> m_areas;
  unsigned int m_nAreas;
  unsigned int m_count;
  std::vector<unsigned int> m_mcount;
  bool m_sat;
  bool m_sat2D;
  
  
  bool doHisto(std::string histo){
    bool ok = false;
    for( std::vector<std::string>::iterator ih = m_histoList.begin() ; m_histoList.end() != ih ; ih++){
      if( histo == *ih || "All" == *ih ){
        ok= true;
        break;
      }
    }
    for( std::vector<std::string>::iterator ih = m_removeHisto.begin() ; m_removeHisto.end() != ih ; ih++){
      if( histo == *ih){
        ok=false;
        break;
      }
    }
    return ok;
  }
  bool validArea( std::string area ){
    for(std::vector<std::string>::iterator  i = m_areas.begin();i != m_areas.end();++i){
      if( *i == area)return true;
    }    
    return false;
  }
};
#endif // CALOMONIDST_CALOMONIALG_H
