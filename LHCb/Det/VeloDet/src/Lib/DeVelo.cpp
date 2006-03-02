// $Id: DeVelo.cpp,v 1.60 2006-03-02 14:21:29 mtobin Exp $
//
// ============================================================================
#define  VELODET_DEVELO_CPP 1
// ============================================================================
// from STL (for std::sort)
#include <algorithm>

// from Kernel
#include "Kernel/SystemOfUnits.h"
#include "Kernel/PhysicalConstants.h"
// trajectory class from LHCbKernel
#include "Kernel/CircleTraj.h"
#include "Kernel/LineTraj.h"

// From Gaudi
#include "GaudiKernel/Bootstrap.h"
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/PropertyMgr.h"
#include "GaudiKernel/IJobOptionsSvc.h"
#include "GaudiKernel/ISvcLocator.h"
#include "GaudiKernel/IUpdateManagerSvc.h"

#include "DetDesc/Condition.h"


// Local
#include "VeloDet/DeVelo.h"

/** @file DeVelo.cpp
 *
 *  Implementation of class :  DeVelo
 *
 *  @author David Hutchcroft David.Hutchcroft@cern.ch
 *  @author Mark Tobin Mark.Tobin@cern.ch
 *  @author Kurt Rinnert kurt.rinnert@cern.ch
 */


// **  Standard Constructors

DeVelo::DeVelo( const std::string& name ) :  
  DetectorElement(name),
  m_tell1ToSensorsConditionName("TELL1ToSensors")
{
  ; 
} 

//
// Standard Destructor
DeVelo::~DeVelo() {
  // should be all handled by the TDS and the automatic deletion of the vectors
}

// ============================================================================
// object identification
// ============================================================================
const CLID& DeVelo::clID () const { return DeVelo::classID() ; }


// ============================================================================
// intialization method
// ============================================================================
StatusCode DeVelo::initialize() {

  // Trick from old DeVelo to set the output level
  PropertyMgr* pmgr = new PropertyMgr();
  int outputLevel=0;
  pmgr->declareProperty("OutputLevel", outputLevel);
  IJobOptionsSvc* jobSvc;
  ISvcLocator* svcLoc = Gaudi::svcLocator();
  StatusCode sc = svcLoc->service("JobOptionsSvc", jobSvc);
  jobSvc->setMyProperties("DeVelo", pmgr);
  if ( 0 < outputLevel ) {
    msgSvc()->setOutputLevel("DeVelo", outputLevel);
  }
  delete pmgr;
  MsgStream msg( msgSvc(), "DeVelo" );
  msg << MSG::DEBUG << "Initialising DeVelo " << endreq;
  // Initialise the detector element
  sc = DetectorElement::initialize();
  if( sc.isFailure() ) { 
    msg << MSG::ERROR << "Failure to initialize DetectorElement" << endreq;
    return sc ; 
  }
  // get all of the pointers to the child detector elements
  std::vector<DeVeloSensor*> veloSensors = findVeloSensors();
  
  msg << MSG::DEBUG << "Found " << veloSensors.size() 
      << " sensors in the XML" << endreq;

  std::vector<DeVeloSensor*>::iterator iDESensor;
  int detElemCount=0;
  m_nRSensors=m_nPhiSensors=m_nPileUpSensors=0;

  // JPP sensors no longer pre-sorted by Z in XML so sort them before
  // storing.
  std::sort(veloSensors.begin(), veloSensors.end(), less_Z());
  
  for(iDESensor = veloSensors.begin() ; iDESensor != veloSensors.end() ; 
      ++iDESensor){
    // Sensors are pre-sorted in XML such that they increase with z position
    m_vpSensor.push_back(*iDESensor);
    unsigned int index=m_vpSensor.size()-1;
    msg << MSG::DEBUG << "type " << (*iDESensor)->fullType() 
        << " index " << index
        << " R " << (*iDESensor)->isR() 
        << " PHI " << (*iDESensor)->isPhi()
        << " PU " << (*iDESensor)->isPileUp() << endmsg;
    if((*iDESensor)->isR()){
      m_vpRSensor.push_back(dynamic_cast<DeVeloRType*>((*iDESensor)));
      m_nRSensors++;
      m_RIndex.push_back(index);
    } else if((*iDESensor)->isPhi()){
      m_vpPhiSensor.push_back(dynamic_cast<DeVeloPhiType*>((*iDESensor)));
      m_nPhiSensors++;
      m_PhiIndex.push_back(index);
    } else if((*iDESensor)->isPileUp()){
      m_vpPUSensor.push_back(dynamic_cast<DeVeloRType*>((*iDESensor)));
      m_nPileUpSensors++;
      m_PUIndex.push_back(index);
    } else {
      msg << MSG::ERROR << "Sensor type is unknown" << endreq;
    }
    m_validSensors[m_vpSensor[index]->sensorNumber()]=true;
    msg << MSG::DEBUG << "Sensor number " << m_vpSensor[index]->sensorNumber()
        << " pSensor " << (*iDESensor)->sensorNumber()
        << " type " << m_vpSensor[index]->fullType() 
        << " z = " << m_vpSensor[index]->z()
        << endreq;
    detElemCount++;
  }
  // Check indices are correct
  if(msg.level() == MSG::VERBOSE) {
    for(unsigned int i=0; m_RIndex.size()>i; i++){
      msg << MSG::VERBOSE << "Index of R sensors " << i << " " 
          <<  m_RIndex[i];
      msg << " sensor number " << m_vpSensor[m_RIndex[i]]->sensorNumber()
          << endmsg;
    }
    
  }
  
  
  // Build a list of phi sensors associated to R
  // Dog leg shape requires both phi of the station
  // need to sort sensors into accending order in z
  // get cute and use the STL sort routine with a custom comparator
  //  std::sort(m_vpSensor.begin(), m_vpSensor.end(), less_Z());

  for(unsigned int iSensor=0; iSensor < m_vpSensor.size() ; ++iSensor){
    m_sensorZ.push_back(m_vpSensor[iSensor]->z());
    unsigned int sensor = m_vpSensor[iSensor]->sensorNumber();

    msg << MSG::DEBUG << "Index " << iSensor << " Sensor number " << sensor
        << " is type " << m_vpSensor[iSensor]->fullType() 
        << " at global z = " << m_vpSensor[iSensor]->z()
        << " and in VELO frame " 
        << geometry()->toLocal(Gaudi::XYZPoint(0,
                                                0,
                                                m_vpSensor[iSensor]->z())).z()
        << endreq;
    // Find phi sensors associated to R in each station (group of 4 sensors)
    int station=(iSensor-4)/4;
    unsigned int firstInStation=0;
    if(0 <= station) firstInStation= 4+4*static_cast<unsigned int>(station);
    if(m_vpSensor[iSensor]->isR()){
      for(unsigned int isens=firstInStation; isens<firstInStation+4; isens++){
        unsigned int aSensor = m_vpSensor[isens]->sensorNumber();
        if(m_vpSensor[isens]->isPhi()) {
          m_vpSensor[iSensor]->associateSensor(aSensor);
        }
      }
    }
  }
  
  if(m_nRSensors < 2 || m_nPhiSensors < 2 || m_nPileUpSensors < 2){
    msg << MSG::ERROR 
        << " This code requies at least two of each type of sensor"
        << endreq;
    return StatusCode::FAILURE;
  } else {
    msg << MSG::DEBUG 
        << " There are " << m_nRSensors << " R type, " 
        << m_nPhiSensors << " Phi type and "
        << m_nPileUpSensors << " pileup type sensors " << endreq;
  }
  /*  msg << "Going to attempt to recalculate z positions\n";
      recalculateZs();*/
  return StatusCode::SUCCESS;
}

// return the sensor number for a point (global frame)
unsigned int DeVelo::sensorNumber(const Gaudi::XYZPoint& point) const {
  double z = point.z();
  for(unsigned int index=0;numberSensors()>index;index++){
    if(0.250*mm > fabs(z - m_sensorZ[index])) {
      return sensorNumber(index);
    }
  }
  MsgStream msg(msgSvc(), "DeVelo");
  msg << MSG::ERROR << "sensorNumber: no sensor at z = " 
      << z << endmsg;
  return 999;
}

// return the sensitive volume if for a point in the global frame
const int DeVelo::sensitiveVolumeID(const Gaudi::XYZPoint& point) const {
  MsgStream msg(msgSvc(), "DeVelo");
  std::vector<DeVeloSensor*>::const_iterator iDeVeloSensor;
  for(iDeVeloSensor=m_vpSensor.begin(); iDeVeloSensor!=m_vpSensor.end(); ++iDeVeloSensor){
    Gaudi::XYZPoint localPoint;
    StatusCode sc=(*iDeVeloSensor)->globalToLocal(point,localPoint);
    if(sc.isSuccess()) {
      double z = localPoint.z();
      if(0.20*mm > fabs(z)) {
        return ((*iDeVeloSensor)->sensorNumber());
      }
    }
  }
  msg << MSG::ERROR << "sensitiveVolumeID: no sensitive volume at z = " 
      << point.z() << endmsg;
  return -999;
}

// Return the index of a sensor in the vector of pointers to the sensors 
// which increase with sensor number
unsigned int DeVelo::sensorIndex(unsigned int sensor) const
{
  if(64 > sensor) return m_RIndex[sensor];
  else if(128 > sensor) return m_PhiIndex[sensor-64];
  else return m_PUIndex[sensor-128];
}

// return pointer to sensor
const DeVeloSensor* DeVelo::sensor(unsigned int sensorNumber) const
{
  return m_vpSensor[sensorIndex(sensorNumber)];
}

// Gives the VeloChannelID and offset (in fraction of a pitch width) 
// associated to a 3D position. with pitch width in mm
// Sign convention is offset is +- 0.5 
// with +ve in increasing strip number  (global frame) 
StatusCode DeVelo::pointToChannel(const Gaudi::XYZPoint &point, 
                                  LHCb::VeloChannelID &channel,
                                  double &localOffset,
                                  double &pitch) const {
  unsigned int sensor = sensorNumber(point);
  return pointToChannel(point,sensor,channel,localOffset,pitch);
}

// pointToChannel if sensor known (global frame)
StatusCode DeVelo::pointToChannel(const Gaudi::XYZPoint &point, 
                                  const unsigned int &sensor,
                                  LHCb::VeloChannelID &channel,
                                  double &localOffset,
                                  double &pitch) const {
  unsigned int index=sensorIndex(sensor);
  return m_vpSensor[index]->pointToChannel(point,channel,localOffset,pitch);
}

// Residual of 3D point to a VeloChannelID
// returns offset in mm from closest point on channel 
/*StatusCode  DeVelo::residual(const Gaudi::XYZPoint &point, 
                             const LHCb::VeloChannelID &channel,
                             double &residual,
                             double &chi2) const{
  
  return this->residual(point,channel,0.,0.5,residual,chi2);
  }*/
StatusCode  DeVelo::residual(const Gaudi::XYZPoint &point, 
                             const LHCb::VeloChannelID &channel,
                             double &residual,
                             double &chi2) const {
  unsigned int index=sensorIndex(channel.sensor());
  return m_vpSensor[index]->residual(point,channel,residual,chi2);
}

// Residual of 3D point to a VeloChannelID + offset in fraction of a channel
// and width of the cluster in channel widths (for the chi2)
// returns offset in mm, and chi^2 from position to point 
StatusCode DeVelo::residual(const Gaudi::XYZPoint &point, 
                            const LHCb::VeloChannelID &channel,
                            const double &localOffset,
                            const double &width,
                            double &residual,
                            double &chi2) const {
  unsigned int index=sensorIndex(channel.sensor());
  return m_vpSensor[index]->residual(point,channel,localOffset,
                                     width,residual,chi2);
}

// Convert global 3D Point to local 3D point in frame of the sensor 
StatusCode DeVelo::globalToLocal(const unsigned int &sensor,
                                 const Gaudi::XYZPoint &global,
                                 Gaudi::XYZPoint &local) {
  return m_vpSensor[sensorIndex(sensor)]->globalToLocal(global,local);
}

// Convert local 3D Point to global 3D point in frame of LHCb
StatusCode DeVelo::localToGlobal(const unsigned int &sensor,
                                 const Gaudi::XYZPoint &local,
                                 Gaudi::XYZPoint &global) const {
  return m_vpSensor[sensorIndex(sensor)]->localToGlobal(local,global);
}

// Get the nth (signed) neighbour strip to a given VeloChannelID
StatusCode DeVelo::neighbour(const LHCb::VeloChannelID &startChannel,
                             const int &offset,
                             LHCb::VeloChannelID &channel) const {
  unsigned int index=sensorIndex(startChannel.sensor());
  return m_vpSensor[index]->neighbour(startChannel,offset,channel);
}

// Check the distance in strips between two channelIDs
StatusCode DeVelo::channelDistance(const LHCb::VeloChannelID &startChannel,
                                   const LHCb::VeloChannelID &endChannel,
                                   int &offset) const {
  unsigned int sen1= startChannel.sensor();
  unsigned int sen2= endChannel.sensor();
  if(sen1 != sen2){
    offset = 2048;
    return StatusCode::FAILURE;
  }else{
    sen1 = sensorIndex(sen1);
    return m_vpSensor[sen1]->channelDistance(startChannel,endChannel,offset);
  }
}

// Returns the vector of phi sensor numbers one can match with the 
// specified R sensor number and vice versa
StatusCode DeVelo::sensorAssociated( unsigned int sensor, 
                               std::vector<unsigned int> &assocSensor ) const
{
  if(isRSensor(sensor)){
    assocSensor = m_vpSensor[sensorIndex(sensor)]->associatedSensors();
  } else {
    return StatusCode::FAILURE;
  }
  return StatusCode::SUCCESS;
}

// returns the number of zones in this sensor
unsigned int DeVelo::numberOfZones( unsigned int sensor) const {
  return m_vpSensor[sensorIndex(sensor)]->numberOfZones();
}

// returns the phi "zone" of the r strip or r zone of phi strip
unsigned int DeVelo::zoneOfStrip( LHCb::VeloChannelID channel) const{
  unsigned int index=sensorIndex(channel.sensor());
  return m_vpSensor[index]->zoneOfStrip(channel.strip());
}

// returns the phi "zone" of the r strip or r zone of phi strip
unsigned int DeVelo::zoneOfStrip( unsigned int strip, 
                                  unsigned int sensor ) const {
  return m_vpSensor[sensorIndex(sensor)]->zoneOfStrip(strip);
}

// Number of strips in each zone
unsigned int DeVelo::stripsInZone( unsigned int sensor, 
                                   unsigned int zone ) const {
  return m_vpSensor[sensorIndex(sensor)]->stripsInZone(zone);
}

// returns the local radius of the strip
double DeVelo::rOfStrip( LHCb::VeloChannelID channel ) const {
  // check whether sensor is R type using m_vpSensor[sensor]->type()
  //  write method bool DeVelo::isR(unsigned int sensor), isPhi etc.
  unsigned int sensor=channel.sensor();
  unsigned int index=sensorIndex(sensor);
  unsigned int strip=channel.strip();
  if(DeVeloRType* rPtr = dynamic_cast<DeVeloRType*>(m_vpSensor[index])){
    return rPtr->rOfStrip(strip);
  }else{
    MsgStream msg(msgSvc(), "DeVelo");
    msg << MSG::ERROR << "rOfStrip: asked for non-valid R sensor " 
        << channel.sensor() << endmsg;
    return 0.;
  }
}
// returns the local radius of the strip+fractional distance to strip
double DeVelo::rOfStrip( LHCb::VeloChannelID channel, double fraction) const
{
  // check whether sensor is R type using m_vpSensor[sensor]->type()
  //  write method bool DeVelo::isR(unsigned int sensor), isPhi etc.
  unsigned int index=sensorIndex(channel.sensor());
  if(DeVeloRType* rPtr = dynamic_cast<DeVeloRType*>(m_vpSensor[index])){
    return rPtr->rOfStrip(channel.strip(),fraction);
  }else{
    MsgStream msg(msgSvc(), "DeVelo");
    msg << MSG::ERROR << "rOfStrip: asked for non-valid R sensor " 
        << channel.sensor() << endmsg;
    return 0.;
  }
}

// returns the R pitch at the given channelID
double DeVelo::rPitch( LHCb::VeloChannelID channel ) const
{
  unsigned int index=sensorIndex(channel.sensor());
  if(DeVeloRType* rPtr = dynamic_cast<DeVeloRType*>(m_vpSensor[index])){
    return rPtr->rPitch(channel.strip());
  }else{
    MsgStream msg(msgSvc(), "DeVelo");
    msg << MSG::ERROR << "rPitch: asked for non-valid R sensor " 
        << channel.sensor() << endmsg;
    return 0.;
  }
}

// returns the R pitch at the given channelID
double DeVelo::rPitch( LHCb::VeloChannelID channel, double fraction) const
{
  unsigned int sensor=channel.sensor();
  unsigned int index=sensorIndex(sensor);
  if(DeVeloRType* rPtr = dynamic_cast<DeVeloRType*>(m_vpSensor[index])){
    return rPtr->rPitch(channel.strip(),fraction);
  }else{
    MsgStream msg(msgSvc(), "DeVelo");
    msg << MSG::ERROR << "rPitch: asked for non-valid R sensor " 
        << channel.sensor() << endmsg;
    return 0.;
  }
}

// returns the R pitch at a given radius
double DeVelo::rPitchAtLocalR( LHCb::VeloChannelID channel, double radius) const
{
  unsigned int sensor=channel.sensor();
  unsigned int index=sensorIndex(sensor);
  if(DeVeloRType* rPtr = dynamic_cast<DeVeloRType*>(m_vpSensor[index])){
    return rPtr->rPitch(radius);
  }else{
    MsgStream msg(msgSvc(), "DeVelo");
    msg << MSG::ERROR << "rPitchAtLocalR: asked for non-valid R sensor " 
        << channel.sensor() << endmsg;
    return 0.;
  }
}

// returns the local phi of the strip at the specified radius for this sensor.
double DeVelo::phiOfStrip( LHCb::VeloChannelID channel, double radius) const {
  return phiOfStrip(channel,0.,radius);
}

// returns the local phi of the strip +fractional distance to strip
// at the specified radius for this sensor.
double DeVelo::phiOfStrip( LHCb::VeloChannelID channel,
                           double fraction, double radius) const
{
  unsigned int sensor=channel.sensor();
  unsigned int index=sensorIndex(sensor);
  if(DeVeloPhiType* phiPtr = dynamic_cast<DeVeloPhiType*>(m_vpSensor[index])){
    return phiPtr->phiOfStrip(channel.strip(),fraction,radius);
  }else{
    MsgStream msg(msgSvc(), "DeVelo");
    msg << MSG::ERROR << "phiOfStrip: asked for non-valid phi sensor " 
        << channel.sensor() << endmsg;
    return -999;
  }
}

// returns the angle of the strip wrt the x axis for
// the strip+fractional distance to strip
double DeVelo::angleOfStrip( LHCb::VeloChannelID channel,double fraction) const
{
  
  unsigned int sensor=channel.sensor();
  unsigned int index=sensorIndex(sensor);
  if(DeVeloPhiType* phiPtr = dynamic_cast<DeVeloPhiType*>(m_vpSensor[index])){
    return phiPtr->angleOfStrip(channel.strip(),fraction);
  }else{
    MsgStream msg(msgSvc(), "DeVelo");
    msg << MSG::ERROR << "angleOfStrip: asked for non-valid phi sensor " 
        << channel.sensor() << endmsg;
    return -999;
  }
}

// The stereo angle of the phi strips in radians,
// signed so that positive indicates phi increases with radius 
double DeVelo::phiStereo( LHCb::VeloChannelID channel, double radius) const
{
  unsigned int index=sensorIndex(channel.sensor());
  if(DeVeloPhiType * phiPtr = 
     dynamic_cast<DeVeloPhiType*>(m_vpSensor[index])){
    return phiPtr->phiOffset(radius);
  }else{
    MsgStream msg(msgSvc(), "DeVelo");
    msg << MSG::ERROR << "phiStereo: asked for non-valid phi sensor " 
        << channel.sensor() << endmsg;
    return -999;
  }
}

// returns the Phi pitch (in mm) at the given radius (sensor local) 
double DeVelo::phiPitch(LHCb::VeloChannelID channel, double radius) const
{
  unsigned int index=sensorIndex(channel.sensor());
  DeVeloPhiType * phiPtr = 
    dynamic_cast<DeVeloPhiType*>(m_vpSensor[index]);
  if(phiPtr){
    return phiPtr->phiPitch(radius);
  }else{
    MsgStream msg(msgSvc(), "DeVelo");
    msg << MSG::ERROR << "phiPitch: asked for non-valid phi sensor " 
        << channel.sensor() << endmsg;
    return 0.;
  }
}

// returns the Phi pitch (in radians) for a given strip
double DeVelo::phiPitch(LHCb::VeloChannelID channel) const
{
  unsigned int index=sensorIndex(channel.sensor());  
  DeVeloPhiType * phiPtr = 
    dynamic_cast<DeVeloPhiType*>(m_vpSensor[index]);
  if(phiPtr){
    return phiPtr->phiPitch(channel.strip());
  }else{
    MsgStream msg(msgSvc(), "DeVelo");
    msg << MSG::ERROR << "phiPitch: asked for non-valid phi sensor " 
        << channel.sensor() << endmsg;
    return 0.;
  }
}

double DeVelo::distToOrigin( LHCb::VeloChannelID channel ) const
{
  unsigned int index=sensorIndex(channel.sensor());
  DeVeloPhiType * phiPtr = 
    dynamic_cast<DeVeloPhiType*>(m_vpSensor[index]);
  if(phiPtr){
    return phiPtr->distToOrigin(channel.strip());
  }else{
    MsgStream msg(msgSvc(), "DeVelo");
    msg << MSG::ERROR << "distToOrigin: asked for non-valid phi sensor " 
        << channel.sensor() << endmsg;
    return 0.;
  }
}

// return the minimum sensitive radius of an R wafer, local frame
double DeVelo::rMin(unsigned int sensor) const {
  return m_vpSensor[sensorIndex(sensor)]->innerRadius();
}
// return the maximum sensitive radius of an R wafer, local frame
double DeVelo::rMax(unsigned int sensor) const {
  return m_vpSensor[sensorIndex(sensor)]->outerRadius();
}

// return the minimum sensitive radius of an R wafer in a zone, local frame
// 4 zones (different phi) for R sensors and 2 zones (different R and stereo)
// for the phi sensors 
double DeVelo::rMin(unsigned int sensor, unsigned int zone) const{
  return m_vpSensor[sensorIndex(sensor)]->rMin(zone);
}
// return the maximum sensitive radius of an R wafer in a zone, local frame
// 4 zones (different phi) for R sensors and 2 zones (different R and stereo)
// for the phi sensors 
double DeVelo::rMax(unsigned int sensor, unsigned int zone) const {
  return m_vpSensor[sensorIndex(sensor)]->rMax(zone);
}

// Smallest Phi (local frame) of the r strips in the zone
double DeVelo::phiMin(unsigned int sensor, unsigned int zone) const{
  if(isRSensor(sensor)){
    DeVeloRType * rPtr = 
      dynamic_cast<DeVeloRType*>(m_vpSensor[sensorIndex(sensor)]);
    return rPtr->phiMinZone(zone);
  }else{
    MsgStream msg(msgSvc(), "DeVelo");
    msg << MSG::ERROR << "phiMin: asked for non-valid R sensor " 
        << sensor << endmsg;
    return -999.;
  }
}

// Largest Phi (local frame) of the R strips in the zone
double DeVelo::phiMax(unsigned int sensor, unsigned int zone) const{
  if(isRSensor(sensor)){
    DeVeloRType * rPtr = 
      dynamic_cast<DeVeloRType*>(m_vpSensor[sensorIndex(sensor)]);
    return rPtr->phiMaxZone(zone);
  }else{
    MsgStream msg(msgSvc(), "DeVelo");
    msg << MSG::ERROR << "phiMax: asked for non-valid R sensor " 
        << sensor << endmsg;
    return -999.;
  }
}

// minimum phi at R (overlap in x) for a given zone
double DeVelo::phiMin(unsigned int sensor, 
                      unsigned int zone, 
                      double radius) const {
  if(isRSensor(sensor)){
    DeVeloRType * rPtr = 
      dynamic_cast<DeVeloRType*>(m_vpSensor[sensorIndex(sensor)]);
    return rPtr->phiMinZone(zone,radius);
  }else{
    MsgStream msg(msgSvc(), "DeVelo");
    msg << MSG::ERROR << "phiMin: asked for non-valid R sensor " 
        << sensor << endmsg;
    return -999.;
  }
}

// maximum phi at R (overlap in x) for a given zone
double DeVelo::phiMax(unsigned int sensor, 
                      unsigned int zone,
                      double radius) const {
  if(isRSensor(sensor)){
    DeVeloRType * rPtr = 
      dynamic_cast<DeVeloRType*>(m_vpSensor[sensorIndex(sensor)]);
    return rPtr->phiMaxZone(zone,radius);
  }else{
    MsgStream msg(msgSvc(), "DeVelo");
    msg << MSG::ERROR << "phiMax: asked for non-valid R sensor " 
        << sensor << endmsg;
    return -999.;
  }
}

// returns the silicon thickness
double DeVelo::siliconThickness ( unsigned int sensor ) const {
  return m_vpSensor[sensorIndex(sensor)]->siliconThickness();
}

// returns the number of strips per sensor.
unsigned int DeVelo::numberStrips(unsigned int sensor) const {
  return m_vpSensor[sensorIndex(sensor)]->numberOfStrips();
}
    
// returns the capacitance of the strip.
double DeVelo::stripCapacitance(LHCb::VeloChannelID channel) const {
  unsigned int index=sensorIndex(channel.sensor());
  return m_vpSensor[index]->stripCapacitance(channel.strip());
}

// Access to a strip's geometry, for Panoramix
// from strip number and R sensor number, returns Z, R and a phi range.
// in local frame 
StatusCode DeVelo::stripLimitsR( unsigned int sensor, 
                                 unsigned int strip,
                                 double& z, 
                                 double& radius, 
                                 double& phiMin, 
                                 double& phiMax )  const {
  z = zSensor(sensor);
  LHCb::VeloChannelID channel(sensor,strip);
  if(isRSensor(sensor) || isPileUpSensor(sensor)){    
    DeVeloRType * rPtr = 
      dynamic_cast<DeVeloRType*>(m_vpSensor[sensorIndex(sensor)]);
    StatusCode sc=rPtr->stripLimits(strip,radius,phiMin,phiMax);
    if(m_vpSensor[sensorIndex(sensor)]->isRight()){
      phiMin += pi;
      phiMax += pi;
    }
    return sc;
  }else {
      MsgStream msg( msgSvc(), "DeVelo" );
      msg << MSG::ERROR 
          << "Asked for phi type sensor as if r/pu type" << endreq;
      return StatusCode::FAILURE;
  }
  return StatusCode::SUCCESS;
}
  
// from strip number and phi sensor number, returns the two end points
// in local frame 
StatusCode DeVelo::stripLimitsPhi( unsigned int sensor, 
                                   unsigned int strip,
                                   Gaudi::XYZPoint& begin, 
                                   Gaudi::XYZPoint& end ) const {
  DeVeloPhiType * phiPtr = 
    dynamic_cast<DeVeloPhiType*>(m_vpSensor[sensorIndex(sensor)]);
  if(phiPtr){
    StatusCode sc = phiPtr->stripLimits(strip, begin, end);
   return sc;
  } else {
    MsgStream msg( msgSvc(), "DeVelo" );
    msg << MSG::ERROR 
        << "Asked for r/pu type sensor as if phi type" << endreq;
    return StatusCode::FAILURE;
  }
  return StatusCode::SUCCESS;
}
//=============================================================================
// Construct 3d point in global frame from R/phi channels 
// (rFrac is fractional distance to strip (+/-0.5))
//=============================================================================
StatusCode DeVelo::makeSpacePoint( LHCb::VeloChannelID rChan, 
                                   double rFrac,
                                   LHCb::VeloChannelID phiChan,
                                   double phiFrac,
                                   Gaudi::XYZPoint& point, 
                                   double&  rPitch,
                                   double&  phiPitch ) const {
  point.SetCoordinates( 0., 0., 0. );
  rPitch   = 0.;
  phiPitch = 0.;

  MsgStream msg( msgSvc(), "DeVelo" );
  msg << MSG::VERBOSE << ">>>>>>>> Inputs; sensors, R " << rChan.sensor()
      << " strip " << (rChan.strip()+rFrac)
      << " Phi " << phiChan.sensor()
      << " strip " << (phiChan.strip()+phiFrac)
      << " at z = " << zSensor(rChan.sensor())
      << endmsg;

  // check that the sensor types are valid
  if(LHCb::VeloChannelID::RType != rChan.type()) return StatusCode::FAILURE;
  if(LHCb::VeloChannelID::PhiType != phiChan.type()) return StatusCode::FAILURE;
  
  unsigned int rSensor=rChan.sensor();
  unsigned int phiSensor=phiChan.sensor();
  // Nearby sensor...
  double zR   = m_sensorZ[sensorIndex(rSensor)];
  double zPhi = m_sensorZ[sensorIndex(phiSensor)];
  
  // Compute R from strip.
  unsigned int rZone; rZone=zoneOfStrip(rChan);
  double localR=rOfStrip(rChan, rFrac);
  // check some matching in the detector region.
  double rAtPhi = localR * ( zPhi - m_zVertex ) / ( zR - m_zVertex );

  double innerPitch=this->rPitch(LHCb::VeloChannelID(rSensor,0));
  double tolPhiBoundary = 5. * innerPitch;

  double innerRadius=rMin(rSensor);
  double outerRadius=rMax(rSensor);
  
  if(innerRadius-tolPhiBoundary > rAtPhi) return StatusCode::FAILURE;
  if(outerRadius+tolPhiBoundary < rAtPhi) return StatusCode::FAILURE;

  // Coherence in the Phi detector region, with some tolerance
  double phiBoundRadius=rMin(phiSensor,0);
  unsigned int nbPhiInner=stripsInZone(phiSensor,0);
  if(phiBoundRadius+tolPhiBoundary < rAtPhi) {
    if(nbPhiInner > phiChan.strip()) {
      return false;
    }
  } else if(phiBoundRadius-tolPhiBoundary > rAtPhi) {
    if (nbPhiInner <= phiChan.strip()) {
      return false;
    }
  }
  double phiLocal = phiOfStrip(phiChan, phiFrac, rAtPhi);
  if(isDownstreamSensor(phiSensor)) phiLocal = -phiLocal;
  // Test for R compatibility
  double phiMin = phiLocal + 0.02;    // Tolerance for tests
  double phiMax = phiLocal - 0.02;    // tolerance for tests
  unsigned int iFind=0;
  for(unsigned int iZone=0;iZone<numberOfZones(rChan.sensor());iZone++){
    if(iZone == static_cast<unsigned int>(rZone)) {
      double zoneMin = this->phiMin(rChan.sensor(),iZone);
      double zoneMax = this->phiMax(rChan.sensor(),iZone);
      if ((zoneMin < phiMin) && ( zoneMax > phiMax ) ){
        iFind = iZone;
      }
    }
  }
  if(static_cast<unsigned int>(rZone) != iFind) return StatusCode::FAILURE;
  // Convert a local point to a global point, using the 
  // localToGlobal method of the R sensor
  double x=localR*cos(phiLocal);
  double y=localR*sin(phiLocal);
  //  double x=rAtPhi*cos(phiLocal);
  //  double y=rAtPhi*sin(phiLocal);
  StatusCode sc=localToGlobal(rSensor,Gaudi::XYZPoint(x,y,0),point);
  if(!sc) return StatusCode::FAILURE;
  // Compute the pitches. 
  rPitch = this->rPitch(rChan);
  phiPitch = this->phiPitch(phiChan, rAtPhi);
  return StatusCode::SUCCESS;
}
//==============================================================================
//  Return true if the two zones are matching for R sensors. 
//  Also returns true for neighbouring phi zones
//==============================================================================
bool DeVelo::matchingZones (unsigned int zone1, unsigned int zone2) const {
  if(0 == zone1) return (2 > zone2);
  else if(1 == zone1) return (3 > zone2);
  else if(2 == zone1) return (0 < zone2);
  else if(3 == zone1) return (1 < zone2);
  return false;
}
//=========================================================================
// REPLICATE OLD DEVELO CODE FOR TRIGGER....
//=========================================================================
//  Returns a range of strip matching the point, and the conversion factors
//=========================================================================
void DeVelo::trgPhiMatchingStrips( int sensor, double radius, 
                                   int rSensor, int zone, double angularTol,
                                   double& stripMin, double& stripMax, 
                                   double& pitch, double& offset ) const {
  stripMin = -1.;
  stripMax = -1.;
  pitch    = 0.;
  offset   = 0.;

  MsgStream msg( msgSvc(), "DeVelo" );
  msg << MSG::VERBOSE << ">>>>>>>> Inputs; sensors, phi " << sensor 
      << " R " << rSensor 
      << " at z = " << zSensor(rSensor)+zSensor(sensor)/2
      << " zone " << zone 
      << " radius " << radius
      << " angularTol " << angularTol << endmsg;

  if(isRSensor(sensor)) return;    // R sensor
  if(rMin(sensor) > radius ) return;
  if(rMax(sensor) < radius ) return;
  bool isInner=false;
  if(rMax(sensor,0) > radius){
    isInner = true;
  }
  offset = phiStereo(LHCb::VeloChannelID(sensor,0),radius);
  if(isInner){
    pitch = phiPitch(LHCb::VeloChannelID(sensor,0));
  } else {
    pitch = phiPitch(LHCb::VeloChannelID(sensor,stripsInZone(sensor,0)));
  }
  if ( isDownstreamSensor(sensor) ) {
    pitch  = -pitch;
    offset = -offset;
  }
  //== tolerance in phi angle to match R and Phi...
  double absDz =  fabs(m_vpSensor[sensorIndex(sensor)]->z() - 
                       m_vpSensor[sensorIndex(rSensor)]->z());
  double deltaPhi = angularTol * absDz / radius;
  
  double phiMin;
  double phiMax;
  if(0 == zone || 3 == zone){
    phiMin = this->phiMin(rSensor,zone,radius);
    phiMax = this->phiMax(rSensor,zone,radius);
  } else {
    phiMin = this->phiMin(rSensor,static_cast<unsigned int>(zone));
    phiMax = this->phiMax(rSensor,static_cast<unsigned int>(zone));
  }
  phiMin += -deltaPhi - offset;
  phiMax += deltaPhi - offset;
  
  // For unusual pairing, rotate Phi ranges to match the R zone...
  // But only in the appropriate zones...
  if(xSide(sensor) != xSide(rSensor)){
    if ( 0 == zone ) {
      phiMin += pi;
      phiMax += pi;
    } else if ( 3 == zone ) {
      phiMin -= pi;
      phiMax -= pi;
    } else {
      return;
    }
  }

  stripMin = phiMin / pitch;
  stripMax = phiMax / pitch;
  if ( stripMax < stripMin ) {
    double temp = stripMin;
    stripMin = stripMax;
    stripMax = temp;
  }
  //== Phi strips are defined from 0 to nbstrips. Strip center is at +.5
  unsigned int nbPhiInner=stripsInZone(sensor,0);
  unsigned int nbStrips=numberStrips(sensor);
  if ( isInner ) {
    if ( 0. > stripMax ) {
      stripMin = -1.;
      stripMax = -1.;
    } else if ( 0. > stripMin ) {
      stripMin = 0.;
    } else if ( nbPhiInner <= stripMin ) {
      stripMin = -1.;
      stripMax = -1.;
    } else if ( nbPhiInner <= stripMax ) {
      stripMax = nbPhiInner;
    }
  } else {
    stripMin += nbPhiInner;
    stripMax += nbPhiInner;
    offset   -= nbPhiInner * pitch;
    if ( nbPhiInner > stripMax ) {
      stripMin = -1.;
      stripMax = -1.;
    } else if ( nbPhiInner > stripMin ) {
      stripMin = nbPhiInner;
    } else if ( nbStrips <= stripMin ) {
      stripMin = -1.;
      stripMax = -1.;
    } else if ( nbStrips <= stripMax ) {
      stripMax = nbStrips;
    }
  }
  if ( isRightSensor(sensor) ) offset += pi;
  msg << MSG::VERBOSE << "Outputs; strip Min " << stripMin 
      << " max " << stripMax << " pitch " << pitch 
      << " offset " << offset/degree << endmsg;
}

// returns the phi of the strip at the specified radius for this sensor.
double DeVelo::trgPhiOfStrip( LHCb::VeloChannelID channel,double radius ) const {
  return trgPhiOfStrip(channel,0.,radius);
}

// returns the local phi of the strip +fractional distance to strip
// at the specified radius for this sensor.
double DeVelo::trgPhiOfStrip( LHCb::VeloChannelID channel,
                               double fraction, 
                              double radius ) const {
  unsigned int sensor=channel.sensor();
  unsigned int index=sensorIndex(sensor);
  if(DeVeloPhiType* phiPtr = dynamic_cast<DeVeloPhiType*>(m_vpSensor[index])){
    return phiPtr->trgPhiOfStrip(channel.strip(),fraction,radius);
  }else{
    MsgStream msg(msgSvc(), "DeVelo");
    msg << MSG::ERROR << "trgPhiOfStrip: asked for non-valid phi sensor " 
        << channel.sensor() << endmsg;
    return -999.;
  }
}

// returns the angle of the strip wrt the x axis for
// the strip+fractional distance to strip
double DeVelo::trgPhiDirectionOfStrip( LHCb::VeloChannelID channel,
                               double fraction ) const {
  unsigned int sensor=channel.sensor();
  unsigned int index=sensorIndex(sensor);
  if(DeVeloPhiType* phiPtr = dynamic_cast<DeVeloPhiType*>(m_vpSensor[index])){
    return phiPtr->trgPhiDirectionOfStrip(channel.strip(),fraction);
  }else{
    return StatusCode::FAILURE;
  }
}
//=============================================================================
std::vector<DeVeloSensor*> DeVelo::findVeloSensors()
{

  std::vector<DeVeloSensor*> mySensors;
  
  scanDetectorElement(this, mySensors);
  return mySensors;
  
}
//=============================================================================
void DeVelo::scanDetectorElement(IDetectorElement* detElem, 
                                 std::vector<DeVeloSensor*>& sensors)
{
  MsgStream msg( msgSvc(), "DeVelo" );  
  std::vector<IDetectorElement*> veloSensors =
    detElem->childIDetectorElements();

  msg << MSG::DEBUG << "scanDetectorElement" << endreq;
  
  std::vector<IDetectorElement*>::iterator iVeloSensors=veloSensors.begin();

  for (;iVeloSensors!=veloSensors.end(); ++iVeloSensors ) {
    msg << MSG::DEBUG << std::setw(12) << std::setiosflags(std::ios::left)
        << (*iVeloSensors)->name() << endreq;
    DeVeloSensor* pSensor = dynamic_cast<DeVeloSensor*>((*iVeloSensors));
    if (pSensor) {
      sensors.push_back(pSensor);
      msg << MSG::DEBUG << "Storing detector " <<   (*iVeloSensors)->name()
          << endreq;
      
    }
    
    scanDetectorElement(*iVeloSensors, sensors);
  }
}
//=============================================================================
// Re-cache sensor geometry and calculate z positions for all sensors
void DeVelo::recalculateZs()
{
  MsgStream msg( msgSvc(), "DeVelo" );  
  m_sensorZ.clear();
  std::vector<DeVeloSensor*>::iterator iDeVeloSensor;
  for(iDeVeloSensor=m_vpSensor.begin(); iDeVeloSensor!=m_vpSensor.end(); ++iDeVeloSensor){
    (*iDeVeloSensor)->cacheGeometry();
    double z = (*iDeVeloSensor)->z();
    m_sensorZ.push_back(z);
    msg << MSG::DEBUG << "Sensor number " << (*iDeVeloSensor)->sensorNumber() 
        << " is at z = " << z << "mm"
        << " sensVolID " << (sensitiveVolumeID(Gaudi::XYZPoint(0,0,z)))
        << " vector size is " << m_sensorZ.size()
        << " with last entry " << m_sensorZ.back()
        << endmsg;
  }
  /*  msg << MSG::DEBUG << "Now try to change each sensor" << endmsg;
      for(unsigned int i=0;i<m_sensorZ.size();i++){
      m_sensorZ[i]=-2234;
      msg << MSG::DEBUG << "Index " << i 
      << " sensor number " << m_vpSensor[i]->sensorNumber()
      << " at z " << m_sensorZ[i];
      recalculateZ(sensorNumber(i));
      msg << " is now at "<< m_sensorZ[i] << endmsg;
      }*/
}
//==============================================================================
// Re-cache geometry and calculate z position of a given sensor
void DeVelo::recalculateZ(unsigned int sensor)
{
  unsigned int index=sensorIndex(sensor);
  m_vpSensor[index]->cacheGeometry();
  m_sensorZ[index]=m_vpSensor[index]->z();
}

//=========================================================================
// members related to condition caching   
//=========================================================================

const DeVeloSensor* DeVelo::sensorByTell1Id(unsigned int tell1Id) const
{
  std::map<unsigned int, const DeVeloSensor*>::const_iterator mi;

  mi =  m_sensorByTell1Id.find(tell1Id);

  if (m_sensorByTell1Id.end() == mi) return 0;

  return (*mi).second;
}

StatusCode DeVelo::registerConditionCallBacks() 
{
  StatusCode sc; 
  MsgStream msg(msgSvc(), "DeVelo");

  // TELL1 to sensor mapping condition
  updMgrSvc()->registerCondition(this,
                                 condition(m_tell1ToSensorsConditionName.c_str()).path(),
                                 &DeVelo::updateTell1ToSensorsCondition);
  
  sc = updMgrSvc()->update(this);
  if(!sc.isSuccess()) {
    msg << MSG::ERROR 
        << "Failed to update VELO conditions!"
        << endreq;
    return sc;
  }

  return StatusCode::SUCCESS;
}

StatusCode DeVelo::updateTell1ToSensorsCondition()
{
  m_tell1ToSensorsCondition = condition(m_tell1ToSensorsConditionName.c_str());
  const std::vector<int>& tell1Ids 
    = m_tell1ToSensorsCondition->paramAsIntVect("Tell1Id");
  const std::vector<int>& sensorNumbers 
    = m_tell1ToSensorsCondition->paramAsIntVect("SensorId");
  
  m_sensorByTell1Id.clear();

  std::vector<int>::const_iterator i = tell1Ids.begin();
  std::vector<int>::const_iterator j = sensorNumbers.begin();

  for (; i != tell1Ids.end() && j != sensorNumbers.end(); ++i, ++j) {
    unsigned int tell1Id      = static_cast<unsigned int>(*i);
    unsigned int sensorNumber = static_cast<unsigned int>(*j);

    m_sensorByTell1Id[tell1Id] = sensor(sensorNumber);
  }

  if (i != tell1Ids.end() || j != sensorNumbers.end()) {
    MsgStream msg(msgSvc(), "DeVelo");
    msg << MSG::ERROR 
        << "Number of TELL1 and sensor IDs do not match!"
        << endreq;
    return StatusCode::FAILURE;
  }

  return StatusCode::SUCCESS;
}

LHCb::Trajectory* DeVelo::trajectory(const LHCb::LHCbID& lID, 
                                     const double offset) const {

  // look up the trajectory
 
  LHCb::Trajectory* tTraj = 0;

  if ( !lID.isVelo()){
     throw GaudiException( "The LHCbID is not of VELO type!",
                           "DeVELO.cpp",StatusCode::FAILURE );
  }
  
  LHCb::VeloChannelID id = lID.veloID();

  if( id.isRType() || id.isPileUp() ){
    // r type is a circle
    double z = 0.;
    double radius = 0.;
    double phiMin = 0.;
    double phiMax = 0.;
    StatusCode sc = this->stripLimitsR( id.sensor(), id.strip(), z, 
					radius, phiMin, phiMax );
    if(!sc){
      throw GaudiException( "The trajectory could not be made",
			    "DeVELO.cpp",StatusCode::FAILURE );
    }
    // offset is offset on R
    radius += rPitch(id) * offset;

    // start with coords of center and both ends in local frame
    Gaudi::XYZPoint lOrigin(0.,0.,0.);
    Gaudi::XYZPoint lEnd1(radius*cos(phiMin),radius*sin(phiMin),z);
    Gaudi::XYZPoint lEnd2(radius*cos(phiMax),radius*sin(phiMax),z);
    
    // move to global frame
    Gaudi::XYZPoint gOrigin, gEnd1, gEnd2;
    localToGlobal(id.sensor(), lOrigin, gOrigin);
    localToGlobal(id.sensor(), lEnd1, gEnd1);
    localToGlobal(id.sensor(), lEnd2, gEnd2);

    // put into trajectory
    tTraj = new LHCb::CircleTraj(gOrigin,gEnd1-gOrigin,gEnd2-gOrigin,radius);

  }else{

    // phi type is a line
    Gaudi::XYZPoint lEnd1, lEnd2;
    StatusCode sc = this->stripLimitsPhi(id.sensor(), id.strip(),
					 lEnd1,lEnd2);
    if(!sc){
      throw GaudiException( "The trajectory could not be made",
			    "DeVELO.cpp",StatusCode::FAILURE );
    }

    // need to also grab next strip in local frame to get offset effect
    Gaudi::XYZPoint lNextEnd1, lNextEnd2;
    // check direction of offset
    if(offset >= 0.){
      sc = this->stripLimitsPhi(id.sensor(), id.strip()+1,
				lNextEnd1,lNextEnd2);
      if(!sc){
	throw GaudiException( "The trajectory could not be made",
			      "DeVELO.cpp",StatusCode::FAILURE );
      }
      lEnd1 += (lNextEnd1-lEnd1)*offset;
      lEnd2 += (lNextEnd2-lEnd2)*offset;
    }else{
      sc = this->stripLimitsPhi(id.sensor(), id.strip()-1,
				lNextEnd1,lNextEnd2);
      if(!sc){
	throw GaudiException( "The trajectory could not be made",
			      "DeVELO.cpp",StatusCode::FAILURE );
      }
      lEnd1 += (lEnd1-lNextEnd1)*offset;
      lEnd2 += (lEnd2-lNextEnd2)*offset;
    }

    Gaudi::XYZPoint gEnd1, gEnd2;
    localToGlobal(id.sensor(), lEnd1, gEnd1);
    localToGlobal(id.sensor(), lEnd2, gEnd2);

    // put into trajectory
    tTraj = new LHCb::LineTraj(gEnd1,gEnd2);
  }
    
  return tTraj;  
}
