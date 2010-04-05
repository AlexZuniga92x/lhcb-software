// $Id: STOfflinePosition.cpp,v 1.20 2010-04-05 09:57:33 mneedham Exp $
 
// Kernel
#include "GaudiKernel/ToolFactory.h"
#include "GaudiKernel/cbrt.h"  // for cbrt on Windows
#include "GaudiKernel/IIncidentSvc.h"
#include "GaudiKernel/SystemOfUnits.h"
 
// Event
#include "Event/STDigit.h"
#include "Event/STCluster.h"

// Boost
#include <boost/assign/std/vector.hpp>
#include <boost/foreach.hpp>

// Mathematical tools
#include <math.h>
#include "LHCbMath/Lomont.h"

#include "Kernel/STFun.h"
#include "Kernel/STDataFunctor.h"

#include "STOfflinePosition.h"

#include "Kernel/ILHCbMagnetSvc.h"
#include "GaudiKernel/Vector3DTypes.h"
#include "GaudiKernel/Point3DTypes.h"

#include "STDet/DeSTSector.h"

using namespace boost::assign;
using namespace LHCb;
using namespace LHCb::Math;


DECLARE_TOOL_FACTORY( STOfflinePosition );
 
STOfflinePosition::STOfflinePosition(const std::string& type, 
                                     const std::string& name,
                                     const IInterface* parent) :
  ST::ToolBase( type, name, parent )
{
  m_errorVec += 0.196, 0.147, 0.177, 0.037;
  declareProperty("ErrorVec",m_errorVec);
  declareProperty("SharingCorr",m_sharingCorr = -112.); 
  declareProperty("CubicSharingCorr2",m_cubicSharingCorr2 = 5.4); 
  declareProperty("LinSharingCorr2",m_linSharingCorr2 = 1.1);
  declareProperty("LinSharingCorr4",m_linSharingCorr4 = 0.67);
  declareProperty("MaxNtoCorr",m_maxNtoCorr = 4);
  declareProperty("trim", m_trim = 0.0);
  declareProperty("MergeClusters", m_mergeClusters = false );
  declareSTConfigProperty("InputData", m_clusterLocation , 
                          STClusterLocation::TTClusters);
  declareProperty("APE", m_APE = 0.0); // Alignment Precision Error

  declareProperty("applyLorentzCorrection", m_applyLorentzCorrection = false);
  declareProperty("lorentzFactor", m_lorentzFactor = 0.025/Gaudi::Units::tesla);

  declareInterface<ISTClusterPosition>(this);
  
}

STOfflinePosition::~STOfflinePosition() 
{
  //destructer
}

StatusCode STOfflinePosition::initialize()
{
  StatusCode sc = ST::ToolBase::initialize();
  if (sc.isFailure()) return Error("Failed to initialize", sc);

  info() << "APE set to " << m_APE << endmsg;
  info() << "Error Vec ";
  BOOST_FOREACH(double val, m_errorVec) {
    info() << val <<  " " ;
  } 
  info() << endmsg;


  // Add incident at begin of each event
  if( m_mergeClusters ) incSvc()->addListener( this, IncidentType::BeginEvent );

  // mag field for lorentz shift...
  if (m_applyLorentzCorrection == true) {
    m_fieldSvc = svc<ILHCbMagnetSvc>("MagneticFieldSvc", true);
  }
  return StatusCode::SUCCESS; 
};

void STOfflinePosition::handle ( const Incident& incident )
{
  if ( IncidentType::BeginEvent == incident.type() ) m_clusters = 0;
}

ISTClusterPosition::Info STOfflinePosition::estimate(const STCluster* 
                                                     aCluster) const 
{
  STCluster::ADCVector adcVector;
  STChannelID firstChan;

  // Merge adc values of neighbouring clusters
  if( m_mergeClusters ) {

    // Get the clusters
    if ( !m_clusters ) m_clusters = get<STClusters>( m_clusterLocation ) ;

    if ( m_clusters ) {
      // Get the iterator belonging to the current cluster
      STClusters::const_iterator iterClus = 
        std::lower_bound( m_clusters->begin(), m_clusters->end(), aCluster, 
                          STDataFunctor::Less_by_Channel<const STCluster*>());

      // Find the left neighbour
      int firstChannel = (*iterClus)->firstChannel();
      --iterClus;
      while( iterClus >= m_clusters->begin() && 
             firstChannel - (*iterClus)->lastChannel() == 1 ) {
        firstChannel = (*iterClus)->firstChannel();
        --iterClus;
      }
      ++iterClus;

      // Add the adc values and save the first channel
      adcVector = (*iterClus)->stripValues();
      firstChan = (*iterClus)->firstChannel();      

      // Add the right neighbours
      int lastChannel = (*iterClus)->lastChannel();
      ++iterClus;
      while( iterClus != m_clusters->end() && 
             (*iterClus)->firstChannel() - lastChannel == 1 ) {
        adcVector.insert( adcVector.end(), (*iterClus)->stripValues().begin(),
                          (*iterClus)->stripValues().end() );
        lastChannel = (*iterClus)->lastChannel();
        ++iterClus;
      }
    }
  } else { // No merging of neighbouring clusters
    adcVector = aCluster->stripValues();
    firstChan = aCluster->firstChannel();
  }

  std::pair<double, unsigned int> info = STFun::position( adcVector, m_trim );
  double stripNum = info.first;

  STChannelID theChan = STChannelID(firstChan.type(), firstChan.station(),
                                    firstChan.layer(), firstChan.detRegion(),
                                    firstChan.sector(), 
                                    (unsigned int)stripNum+firstChan.strip());
  
  double stripFrac = stripFraction( stripNum - floor(stripNum),
                                    info.second );
                                                                             
  if (m_applyLorentzCorrection == true){
    lorentzShift(theChan, stripFrac);
  }  

  ISTClusterPosition::Info theInfo; 
  theInfo.strip = theChan;
  theInfo.fractionalPosition = stripFrac;
  theInfo.fractionalError = error(info.second);
  theInfo.clusterSize = info.second;
  return theInfo;

}

ISTClusterPosition::Info
STOfflinePosition::estimate(const SmartRefVector<STDigit>& digits) const
{  
  std::pair<double, unsigned int> info = STFun::position(digits,m_trim);
  double stripNum = info.first;

  STChannelID firstChan = digits.front()->channelID();
  STChannelID theChan = STChannelID(firstChan.type(), firstChan.station(),
                                    firstChan.layer(), firstChan.detRegion(),
                                    firstChan.sector(), (unsigned int)stripNum);

  double stripFrac = stripFraction( stripNum - floor(stripNum),
                                    info.second );
                                                                             
  if (m_applyLorentzCorrection == true){
    lorentzShift(theChan, stripFrac);
  }

  ISTClusterPosition::Info theInfo;
  theInfo.strip = theChan;
  theInfo.fractionalPosition = stripFrac;
  theInfo.fractionalError = error(info.second);
  theInfo.clusterSize = info.second;

  return theInfo;
}

double STOfflinePosition::error(const unsigned int nStrips) const
{
  // estimate of error                                                           
  double eValue =0.0;
  nStrips < m_errorVec.size() ?  eValue = m_errorVec[nStrips-1] : eValue = m_errorVec.back();
  if (m_APE > 0.0){
    eValue = sqrt(eValue*eValue + m_APE*m_APE );
  }
  return eValue;
}

double STOfflinePosition::stripFraction(const double stripNum,
                                        const unsigned int clusterSize) const
{
  // 'S- shape correction' for non-linear charge sharing
  double corStripPos = stripNum - floor(stripNum);
  if ( (clusterSize>1) && ((int)clusterSize <= m_maxNtoCorr) ) {

    if ( m_sharingCorr > 0.0 ) { // Old charge sharing correction
      corStripPos = chargeSharingCorr( corStripPos );
    } else { // New charge sharing correction
      if (clusterSize == 2) {
        // Linear plus cubic term
        corStripPos = (corStripPos-0.5)*m_linSharingCorr2 +
          pow((corStripPos-0.5), 3.0)*m_cubicSharingCorr2 + 0.5;    
      } else if( clusterSize == 3 ) {
        // Cubic term
        corStripPos = pow((corStripPos-0.5), 3.0)*4.0+0.5;
      } else if( clusterSize == 4 ) {
        // Linear term only
        corStripPos = (corStripPos-0.5)*m_linSharingCorr4+0.5;
      }    
    }
  }

  return corStripPos;
}


// Old charge
double STOfflinePosition::chargeSharingCorr(const double origDist) const
{
  // non-linear charge sharing correction
  double newDist = origDist-0.5;

  // FIXME: No correction should indeed be applied for symmetric, uneven-sized
  //        clusters (i.e. origDist==0.0), but this makes the correction
  //        function non-continuous. Try to find a better function that still
  //        leaves the zero intact. --- JvT 09.01.2009

  // No correction in case of symmetric, uneven-sized clusters
  if (m_sharingCorr > 0.0 && !lomont_compare_double(origDist, 0.0, 100) ) {
    if (newDist<0.) {
      newDist = -cbrt(-newDist/m_sharingCorr);
    }
    else {
      newDist = cbrt(newDist/m_sharingCorr);
    }
  } // apply corr

  return newDist+0.5;
  
}

void STOfflinePosition::lorentzShift(const STChannelID& chan , double& fracPosition) const{

  // calculate the lorentz shift, 

  const DeSTSector* sector = findSector(chan);  
  const Gaudi::XYZVector field = m_fieldSvc->fieldVector(sector->globalCentre()) ; 

  // note fracPosition local frame (pitch units), lorentz shift global frame (mm)
  double dx = sector->thickness() * field.y() * m_lorentzFactor/sector->pitch();
  if (sector->xInverted() == true) dx *= -1.0; 
  fracPosition += dx ;
}
