#include "DimInfoHistos.h"
#include "TH1.h"
#include "TH2.h"
#include <iostream>
#ifdef WIN32
namespace win {
#include <windows.h>
}
# define mysleep win::Sleep
#else
# define mysleep usleep
#endif

//constructor
DimInfoHistos::DimInfoHistos(std::string hSvcname, int refreshTime)
  : DimInfo((hSvcname).c_str(),refreshTime,-1.0),
    m_hist(0),m_hist2d(0),m_dimension(0),m_data(0),m_hasData(false){

  m_rtime=refreshTime;
  int len = hSvcname.length();
  m_histoname=hSvcname.substr(3,len-3);
  if(       "H1D" == hSvcname.substr(0,3) ) m_dimension = 1;
  else  if( "H2D" == hSvcname.substr(0,3) ) m_dimension = 2;
  else  {
  	std::cerr << "DimInfoHistos(" << m_histoname << "): Unexpected histogram dimension: " 
              << m_dimension << std::endl;
    exit(1);
  }
  
//  std::cerr << "DimInfoHistos(" << m_histoname << ") leaving constructor. " << std::endl;
  
}


DimInfoHistos::DimInfoHistos(std::string hSvcname) : 
  DimInfo((hSvcname).c_str(),-1.0){
  std::cerr << "DimInfoHistos(" << hSvcname << ") called with no refresh time. Using 5 s" << std::endl;
  DimInfoHistos(hSvcname,5);
}

//destructor
DimInfoHistos::~DimInfoHistos() {  
//  std::cerr << "DimInfoHistos(" << m_histoname << "): Destructor called" << std::endl;
  // if (m_hist!=0) delete m_hist;  // Leave the deletion to root, after closing any root file.
}

void DimInfoHistos::infoHandler() 
{ 
 // std::cerr << "DimInfoHistos(" << m_histoname << ") " << " histo address: " << m_hist << std::endl;
  //mysleep(m_rtime);
  
//  int dim = getSize()/sizeof(float);
  m_data = (float*) getData();
//  for(int i=0;i<dim;i++) {
//    std::cerr << "DimInfoHistos(" << m_histoname << "): i = " << i 
//              << " data[i] = " << m_data[i] << std::endl;
//  	}
  if( m_dimension != m_data[0] ){
    std::cerr 
      << "DimInfoHistos(" << m_histoname << "): Conflicting histogram dimensions. From DimService dim= " 
      << m_data[0] << std::endl;
    exit(2);
  }
  // Initialize histos
	if(( 0 == m_hist )&(0==m_hist2d)) {
    if( 1 == m_dimension ) {
    //   std::cerr << "DimInfoHistos(" << m_histoname << "): Making ROot histogram" << std::endl;
       // 1Hd m_data: dimension,nXBins,xMin,xMax,2*(UNDERFLOW,"in range" bins, OVERFLOW): entries and errors
      m_hist=new TH1F(m_histoname.c_str(),m_histoname.c_str(),(int)m_data[1],m_data[2],m_data[3]);	
    }      
    else if( 2 == m_dimension ) {
      // 2Hd data: dimension,nXBins,xMin,xMax,nYBins,yMin,yMax,
      // 2*((UNDERFLOW,"in range" bins, OVERFLOW)_x*(UNDERFLOW,"in range" bins,OVERFLOW)_y)
   //   std::cerr << "DimInfoHistos(" << m_histoname << "): Making ROot histogram" << std::endl;
      m_hist2d=new TH2F(m_histoname.c_str(),m_histoname.c_str(),(int)m_data[1],m_data[2],
                      m_data[3],(int)m_data[4],m_data[5],m_data[6]);	
    } 
	}
	if(( 0 == m_hist )&(0==m_hist2d)) {
    std::cerr << "DimInfoHistos(" << m_histoname << "): Unable to create root histogram"  << std::endl;
    exit(3);
	}
  if( 1 == m_dimension ) setH1Data();
  if( 2 == m_dimension ) setH2Data();
} 


TH1* DimInfoHistos::getHisto() {
//  std::cerr << "DimInfoHistos(" << m_histoname << "), get_hist " << m_hist 
//            << " hasData: " << std::boolalpha << m_hasData << std::endl;
  if(m_hasData == true ) return m_hist;
  return 0;
}

TH2* DimInfoHistos::get2DHisto() {
//  std::cerr << "DimInfoHistos(" << m_histoname << "), get_hist " << m_hist 
//            << " hasData: " << std::boolalpha << m_hasData << std::endl;
  if(m_hasData == true ) return m_hist2d;
  return 0;
}

void DimInfoHistos::setH1Data(){
  // Set total number of entries
  m_hist->SetEntries(m_data[4]);
//  std::cerr << "DimInfoHistos(" << m_histoname << "), setH1Data: total entries: " << m_data[4] << std::endl;
  
  // Remember root histo: bin 0: underflows, bin N+1: overflows
  int nofbins = m_hist->GetNbinsX();
  int iData=5;
  for (int i=0;i<=nofbins+1;i++) {
    m_hist->SetBinContent(i,m_data[iData++]);
//    std::cerr << "DimInfoHistos(" << m_histoname << "), setH1Data: entries: index in DimInfo data: "
//              << iData-1 << " value: " << m_data[iData-1] << std::endl;
  }
  
  for (int i=0;i<=nofbins+1;i++) {
    m_hist->SetBinError(i,m_data[iData++]);
    //std::cerr << "DimInfoHistos(" << m_histoname << "), setH1Data: errors: index in DimInfo data:  "
    //          << iData-1 << " value: " << m_data[iData-1] <<std::endl;
  }
  // Debug: Check the root histo just filled
//  for (int i=0;i<=nofbins+1;i++) {
//    std::cerr << "DimInfoHistos(" << m_histoname << "), setH1Data: Root histo content: bin: " << i 
//              << " entries: " << m_hist->GetBinContent(i) << " error: " << m_hist->GetBinError(i) << std::endl;
//  }   
  m_hasData=true;
}

void DimInfoHistos::setH2Data(){ 
  // Set total number of entries
  //  std::cerr << "DimInfoHistos(" << m_histoname << "), setH2Data: total entries: " << m_data[7] << std::endl;
  m_hist2d->SetEntries(m_data[7]);
  
  int nofbinsX = m_hist2d->GetNbinsX();
  int nofbinsY = m_hist2d->GetNbinsY();
  int iData=8;
  for (int i=0; i<=nofbinsX+1; ++i) {
    for (int j=0; j<=nofbinsY+1; ++j) {
      m_hist2d->SetBinContent(i,j,m_data[iData++]); 
      //std::cerr << "DimInfoHistos(" << m_histoname << "), setH2Data: entries:  xbin "<< i << " ybin " << j 
      //          << " index in DimInfo data: " << iData-1 << " value: " << m_data[iData-1] << std::endl;
    }
  }
  
  for (int i=0; i<=nofbinsX+1; ++i) {
    for (int j=0; j<=nofbinsY+1; ++j) {
      m_hist2d->SetBinError(i,j,m_data[iData++]); 
      //std::cerr << "DimInfoHistos(" << m_histoname << "), setH2Data: errors: xbin "<< i << " ybin " << j 
      //          << " index in DimInfo data: " << iData-1 << " value: " << m_data[iData-1] << std::endl;
    }
  }
  
  // Debug: Check the root histo just filled
 // for (int i=0; i<=nofbinsX+1; ++i) {
 //   for (int j=0; j<=nofbinsY+1; ++j) {
 //     std::cerr << "DimInfoHistos(" << m_histoname << "), setH2Data: Root histo content: xbin: " 
 //              << i  << " ybin: " << j 
 //               << " entries: " << m_hist->GetBinContent(i,j) 
 //               << " error: " << m_hist->GetBinError(i,j) << std::endl;
 //   }
 // }
  
  m_hasData=true;
}


