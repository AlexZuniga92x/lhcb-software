// $Id: VeloClusterPosition.cpp,v 1.17 2007-12-04 16:40:01 cattanem Exp $
// Include files

// stl
#include <vector>

// from Gaudi
#include "GaudiKernel/ToolFactory.h"
#include "GaudiKernel/SystemOfUnits.h"

// Math
#include "LHCbMath/LHCbMath.h"
#include "GaudiKernel/Point3DTypes.h"

// Velo
#include "VeloDet/DeVelo.h"
#include "Event/VeloCluster.h"

// local
#include "VeloClusterPosition.h"

// boost
#include <boost/assign/std/vector.hpp>

//-----------------------------------------------------------------------------
// Implementation file for class : VeloClusterPosition
//
// 2005-09-30 : Tomasz Szumlak 
// 
//-----------------------------------------------------------------------------

// Declaration of the Tool Factory
DECLARE_TOOL_FACTORY( VeloClusterPosition );

using namespace boost::assign;

typedef IVeloClusterPosition::toolInfo toolInfo;
typedef IVeloClusterPosition::Direction Direction;
typedef std::pair<double, double> Pair;

static const Gaudi::XYZVector ZVersor(0., 0., 1.);

//=============================================================================
// Standard constructor, initializes variables
//=============================================================================
VeloClusterPosition::VeloClusterPosition(const std::string& type,
                               const std::string& name,
                               const IInterface* parent)
  : GaudiTool(type, name, parent),
    m_veloDet ( 0 ),
    m_projAngles ( 24, 0. ),
    m_minAngle ( 0. ),
    m_maxAngle ( 0. ),
    m_trackDir ( ),
    m_gloPoint ( ),
    m_fracPos ( 0. ),
	m_p0(0),
	m_p1(0)
{
  declareInterface<IVeloClusterPosition>(this);
  // default paramertrizations are of form error=slope*pitch+const
  // the first value represents const and is given in um
  // the second value is slope and is dimensionless
  // we use the same parametrizations for both R and Phi sensors
  // default error parametrization is meant to be used in function
  // where only cluster is passed in
  m_defaultResolution.push_back(-6.301);
  m_defaultResolution.push_back(0.27);
  // parameters for calculation error determined in projected angle bins
  m_p0Values+=-2.5, -3.591, -3.430, -4.112, -4.964, -5.705, -6.301, -7.065,
              -7.353, -7.055, -6.168, -4.814, -3.075, -1.067, 0.798, 2.753,
              4.446, 5.116, 6.411, 7.124, 7.683, 7.573, 9.946, 10.97;
  m_p1Values+=0.29, 0.29, 0.28, 0.28, 0.28, 0.27, 0.27, 0.269,
              0.239, 0.23, 0.215, 0.18, 0.15, 0.11, 0.09, 0.06, 
              0.04, 0.03, 0.0226, 0.0154, 0.0154, 0.0235, 0.0008, -0.0079;
  // parameters determined for different pitch bins error type 2
  m_errAnglePara+=0.038, 0.00033, -25.52, 0.167, 1.447, 0.0069, 0.121, 0.00032;
  //
  declareProperty("DefaultResolution", m_defaultResolution);
  declareProperty("CalculateExactProjAngle", m_calculateExactProjAngle=false);
  declareProperty("ErrorParaType", m_errorParaType=PITCH_PARA);
  declareProperty("P0Values", m_p0Values);
  declareProperty("P1Values", m_p1Values);
  declareProperty("SplineType", m_splineType="Cspline");
}
//=============================================================================
// Destructor
//=============================================================================
VeloClusterPosition::~VeloClusterPosition(){
};
//=============================================================================
StatusCode VeloClusterPosition::initialize()
{
  debug()<< " ==> initialize() " <<endmsg;
  //
  StatusCode sc=GaudiTool::initialize();
  if ( sc.isFailure() ) return sc;
  // get Velo detector
  m_veloDet=getDet<DeVelo>( DeVeloLocation::Default );
  // assign angles in rad from 0 to 400 mrad
  std::vector<double>::iterator it=m_projAngles.begin();
  int value=0;
  while(it!=m_projAngles.end()){
    (*it)=static_cast<double>(value*Gaudi::Units::pi/RAD_TO_DEG);
    ++it;
    ++value;
  }
  assert(m_projAngles.size()==m_p0Values.size());
  assert(m_projAngles.size()==m_p1Values.size());
  //std::copy(m_projAngles.begin(), m_projAngles.end(),
  //            std::ostream_iterator<double>(std::cout, "\n"));
  m_minAngle=m_projAngles.front();
  m_maxAngle=m_projAngles.back();
  m_p0=new GaudiMath::SimpleSpline(m_projAngles, m_p0Values, splineType());
  m_p1=new GaudiMath::SimpleSpline(m_projAngles, m_p1Values, splineType());
  //
  return ( StatusCode::SUCCESS );
}
//=============================================================================
//  Finalize
//=============================================================================
StatusCode VeloClusterPosition::finalize() {

  debug() << "==> Finalize" << endmsg;
  if(m_p0!=0) {delete m_p0; m_p0=0;}
  if(m_p1!=0) {delete m_p1; m_p0=0;}

  return ( GaudiTool::finalize() );  // must be called after all other actions
}
//=============================================================================
// position calculation, depending on sensor type the clusterPos
// is radial or angular variable
//=============================================================================
double VeloClusterPosition::meanResolution(const double& pitch) const
{ 
  debug()<< " ==> resolution() " <<endmsg;
  //
  double p_0=m_defaultResolution[0];
  double p_1=m_defaultResolution[1];
  //
  double resolution=p_0+(p_1*pitch);
  //
  return ( resolution );
}
//=========================================================================
double VeloClusterPosition::fracPosLA(const LHCb::VeloCluster* cluster) const
{
  debug()<< " ==> fracPosLA()" <<endmsg;
  //
  double centre=0., sum=0., fractionalPos;
  int intDistance=0;
  int stripNumber=cluster->size();
  std::vector<LHCb::VeloChannelID> chanCont=cluster->channels();
  //
  const DeVeloSensor* sens=m_veloDet->sensor(chanCont[0].sensor());
  for(int i=0; i<stripNumber; i++){
    StatusCode sc=sens->channelDistance(chanCont[0],chanCont[i],intDistance);
    sc.ignore(); // channels in a cluster are always in same sensor!
    centre+=static_cast<float>(intDistance)*cluster->adcValue(i);
    sum+=cluster->adcValue(i);
  }
  //
  if(stripNumber==1){
    fractionalPos=0.0;
  }else{
    centre=centre/sum;
    intDistance=int(LHCbMath::round(centre));
    fractionalPos=centre-intDistance;
  }
  // redefine fractional position accordingly - tell1 raw bank format
  if(fractionalPos<0.) fractionalPos+=1;
  //
  return  ( fractionalPos );
}
//=========================================================================
toolInfo VeloClusterPosition::position(const LHCb::VeloCluster* cluster) const
{
  debug()<< " ==> position(cluster) " <<endmsg;
  //
  toolInfo myInfo;
  // calculate fractional position in units of 'strip'
  double fractionalPos=fracPosLA(cluster);
  double errorPos=0., pitch=0.;
  LHCb::VeloChannelID centreChannel=cluster->channelID();
  const DeVeloSensor* sens=m_veloDet->sensor(centreChannel.sensor());
  if(sens==0){
    Error("No valid pointer to sensor");
    myInfo.strip=LHCb::VeloChannelID(0);
    myInfo.fractionalPosition=0.;
    myInfo.fractionalError=0.;
    return ( myInfo );
  }
  if(sens->isR()||sens->isPileUp()){
    const DeVeloRType* rSens=dynamic_cast<const DeVeloRType*>(sens);
    pitch=rSens->rPitch(centreChannel.strip(), fractionalPos);
    errorPos=meanResolution(pitch/Gaudi::Units::micrometer);
    errorPos/=(pitch/Gaudi::Units::micrometer);
  }else if(sens->isPhi()){
    const DeVeloPhiType* phiSens=dynamic_cast<const DeVeloPhiType*>(sens);
    double minRadius=0., maxRadius=0.;
    unsigned int zoneOfCluster=0;
    zoneOfCluster=sens->zoneOfStrip(centreChannel.strip());
    minRadius=sens->rMin(zoneOfCluster);
    maxRadius=sens->rMax(zoneOfCluster);
    double meanRadius=0.5*(maxRadius+minRadius);
    pitch=phiSens->phiPitch(meanRadius);
    errorPos=meanResolution(pitch/Gaudi::Units::micrometer);
    errorPos/=(pitch/Gaudi::Units::micrometer);
  }
  //
  myInfo.strip=centreChannel;
  myInfo.fractionalPosition=fractionalPos;
  myInfo.fractionalError=errorPos;
  //
  return ( myInfo );
}
//============================================================================
toolInfo VeloClusterPosition::position(const LHCb::VeloCluster* cluster,
                                       const Gaudi::XYZPoint& aGlobalPoint,
                                       const Direction& aDirection) const
{
  debug()<< " ==> position(cluster, point, direction) " <<endmsg;
  // this struct will be returned as output
  toolInfo anInfo;
  // get information about sensor form passed cluster
  unsigned int sensorNumber=cluster->channelID().sensor();
  const DeVeloSensor* sensor=m_veloDet->sensor(sensorNumber);
  if(sensor==0){
    Error("No valid pointer to sensor");
    anInfo.strip=LHCb::VeloChannelID(0);
    anInfo.fractionalPosition=0.;
    anInfo.fractionalError=0.;
    return ( anInfo );
  }
  // calculate fractional position
  double m_fracPos=fracPosLA(cluster);
  LHCb::VeloChannelID centreChan=cluster->channelID();
  // fill partially the toolInfo structure
  anInfo.strip=centreChan;
  anInfo.fractionalPosition=m_fracPos;
  // error estimate
  double error=0.;
  m_trackDir=Gaudi::XYZVector(aDirection.first, aDirection.second, 1.);
  m_gloPoint=aGlobalPoint;
  //-- return projected angle and local pitch
  Pair aPair=projectedAngle(sensor, centreChan);
  m_projectedAngle=aPair.first;
  double pitch=aPair.second;
  //-- error estimate
  error=errorEstimate(m_projectedAngle/Gaudi::Units::rad,
                      pitch/Gaudi::Units::micrometer);
  anInfo.fractionalError=error;
  //
  return ( anInfo );
}
//============================================================================
double VeloClusterPosition::angleOfTrack(const Direction& localSlopes,
                                  Gaudi::XYZVector& parallel2Track) const
{
  debug()<< " ==> trackAngle() " <<endmsg;
  // vectors
  Gaudi::XYZVector perpPi1(1., 0., -(localSlopes.first));
  Gaudi::XYZVector perpPi2(0., 1., -(localSlopes.second));
  // vector parallel to the track
  Gaudi::XYZVector parallel=perpPi1.Cross(perpPi2);
  double modParallel=sqrt(parallel.Mag2());
  // and normalized parallel to track
  Gaudi::XYZVector normParallel(parallel.x()/modParallel,
                                parallel.y()/modParallel,
                                parallel.z()/modParallel
  );
  parallel2Track=normParallel;
  double cosOfInsertion=parallel2Track.Dot(ZVersor);
  double alphaOfInsertion=acos(cosOfInsertion);
  //
  return ( alphaOfInsertion );
}
//============================================================================
double VeloClusterPosition::errorEstimate(const double projAngle,
                                          const double pitch) const
{
  debug()<< " ==> errorEstimate() " <<endmsg;
  //
  double angle=projAngle;
  double error=0.;
  //-- make cutoff at unphysical angles
  if(angle<m_minAngle){ angle=-angle; }
  //-- pitch projection of the residuals
  if(m_errorParaType==PITCH_PARA){
    if(angle>m_maxAngle){ angle=m_maxAngle; }
    double p0=m_p0->eval(angle);
    double p1=m_p1->eval(angle);
    error=p0+p1*pitch;
    error/=pitch;
  }
  //-- this set of parametrizations use angle projection
  if(m_errorParaType==ANGLE_PARA){
    if(angle>m_maxAngle){
      error=0.28;
    }else{
     double p0=m_errAnglePara[0]+m_errAnglePara[1]*pitch;
     double p1=m_errAnglePara[2]+m_errAnglePara[3]*pitch;
     double p2=m_errAnglePara[4]+m_errAnglePara[5]*pitch;
     double p3=m_errAnglePara[6]+m_errAnglePara[7]*pitch;
    //-- make some fine tuning - difference between sigma snd RMS
     error=1.08*(p0*sin(p1*angle+p2)+p3); 
    }
  }
  //
  return ( error );
}
//============================================================================
double VeloClusterPosition::projectedAngle() const
{
  debug()<< " ==> projectedAngle() " <<endmsg;
  //
  return ( m_projectedAngle/Gaudi::Units::degree );
}
//============================================================================
toolInfo VeloClusterPosition::position(const LHCb::VeloCluster* cluster,
                                       const LHCb::StateVector& aState) const
{
  debug()<< " ==> position (VectorState) " <<endmsg;
  unsigned int sensorNumber=cluster->channelID().sensor();
  const DeVeloSensor* sensor=m_veloDet->sensor(sensorNumber);
  double z=sensor->z();
  // build space point in global ref. frame
  Gaudi::XYZPoint aPoint(aState.x(), aState.y(), z);
  // build state pair
  Direction aDirection;
  aDirection.first=aState.tx();
  aDirection.second=aState.ty();
  return ( position(cluster, aPoint, aDirection) );
}
//============================================================================
GaudiMath::Interpolation::Type VeloClusterPosition::splineType() const
{
  debug()<< " splineType() " <<endmsg;
  //
  GaudiMath::Interpolation::Type aType;
  using namespace GaudiMath::Interpolation;
  if(m_splineType=="Cspline"){ 
    aType=Cspline;
  }else if(m_splineType=="Linear"){
    aType=Linear;
  }else if(m_splineType=="Polynomial"){
    aType=Polynomial;
  }else{
    aType=Linear;
  }
  //
  return aType;
}
//============================================================================
Pair VeloClusterPosition::projectedAngle(const DeVeloSensor* sensor,
                          const LHCb::VeloChannelID centreChan) const
{
  debug()<< " ==> projectedAngle(sensor) " <<endmsg;
  //-- returned pair consists of projected angle and local pitch
  //-- both values are needed to estimate error
  double projectedAngle=0., localPitch=0.;
  Pair locPair;
  // transform global point to the local reference frame
  Gaudi::XYZPoint aLocPoint=sensor->globalToLocal(m_gloPoint);
  unsigned int centreStrip=centreChan.strip();
  //----------------
  //   R sensor
  //----------------
  if(sensor->isR()||(sensor->isPileUp())){ 
    const DeVeloRType* rSensor=dynamic_cast<const DeVeloRType*>(sensor);
    double rOfCluCentre=rSensor->rOfStrip(centreStrip, m_fracPos);
    localPitch=rSensor->rPitch(rOfCluCentre);
    if(m_calculateExactProjAngle){  //-- exact calculations for R
      Gaudi::XYZVector parallel2Track;
      Direction locDir=localTrackDirection(m_trackDir, sensor);
      //-- track angle
      double alphaOfTrack=angleOfTrack(locDir, parallel2Track);
      //-- vector normal to the strip - sensor type specific
      Gaudi::XYZVector perp2RStrip(aLocPoint.x(), aLocPoint.y(), 0.);
      double cosTrackOnNormal=parallel2Track.Dot(perp2RStrip.Unit());
      //-- projection of track on normal to local strip
      double trackOnNormal=fabs(cosTrackOnNormal);
      //-- projection of track on Z axis
      double trackOnZ=cos(alphaOfTrack);
      projectedAngle=atan(trackOnNormal/trackOnZ);
      locPair=std::make_pair(projectedAngle, localPitch);
    }else{                          //-- approximated calculations for R
      // for R sensor, this is a good approximation, sqrt(tx**2+ty**2)
      projectedAngle=m_trackDir.rho();
      locPair=std::make_pair(projectedAngle, localPitch);
    }
  }
  //----------------
  //   Phi sensor
  //----------------
  if(sensor->isPhi()){
    const DeVeloPhiType* phiSensor=dynamic_cast<const DeVeloPhiType*>(sensor);
    double radiusOnPhi=aLocPoint.rho();
    // make velo trajectory
    std::auto_ptr<LHCb::Trajectory> traj=
      m_veloDet->trajectory(LHCb::LHCbID(centreChan), m_fracPos);
    double trajEnd=traj->endPoint().rho();
    double trajBeg=traj->beginPoint().rho();
    double rMax=trajEnd;
    if(trajEnd<trajBeg) {rMax=trajBeg;}
    double rMin=trajEnd;
    if(trajEnd>trajBeg) {rMin=trajBeg;}
    if(radiusOnPhi>rMax) {radiusOnPhi=rMax;}
    if(radiusOnPhi<rMin) {radiusOnPhi=rMin;}       
    localPitch=phiSensor->phiPitch(radiusOnPhi);
    if(m_calculateExactProjAngle){      //-- exact calculations for Phi
      Gaudi::XYZVector parallel2Track;
      std::pair<Gaudi::XYZPoint, Gaudi::XYZPoint> strip;
      strip=phiSensor->localStripLimits(centreStrip);
      Gaudi::XYZVector perp2PhiStrip((strip.first.y()-strip.second.y()),
                                     (strip.second.x()-strip.first.x()), 0.);
      Direction locDir=localTrackDirection(m_trackDir, sensor);
      double alphaOfTrack=angleOfTrack(locDir, parallel2Track);
      double cosTrackOnNormal=parallel2Track.Dot(perp2PhiStrip.Unit());
      double trackOnNormal=fabs(cosTrackOnNormal);
      double trackOnZ=cos(alphaOfTrack);
      projectedAngle=atan(trackOnNormal/trackOnZ);
      locPair=std::make_pair(projectedAngle, localPitch);
    }else{                              //-- approximated calculations for Phi
      // this is an approximation, but should be ok since sensors are never 
      // tilted so much that it matters
      double stereoAngle=
        acos(traj->direction(0.5).Dot(m_gloPoint)/m_gloPoint.rho());
      // for phi sensors, projection angle is diluted by stereo angle
      projectedAngle=m_trackDir.rho()*stereoAngle;
      locPair=std::make_pair(projectedAngle, localPitch);
    }
  }
  return ( locPair );
}
//============================================================================
Direction VeloClusterPosition::localTrackDirection(
          const Gaudi::XYZVector& gloTrackDir,
          const DeVeloSensor* sensor) const
{
  debug()<< " ==> localTrackDirection " <<endmsg;
  //-- translate global slopes into local
  using namespace std;
  Direction locTrackDir;
  if(sensor->isLeft()&&(!sensor->isDownstream())){
    locTrackDir=make_pair(gloTrackDir.x(), gloTrackDir.y());
  }else if(sensor->isLeft()&&sensor->isDownstream()){
    locTrackDir=make_pair((-1)*gloTrackDir.x(), gloTrackDir.y());
  }else if(sensor->isRight()&&sensor->isDownstream()){
    locTrackDir=make_pair(gloTrackDir.x(), (-1)*gloTrackDir.y());
  }else if(sensor->isRight()&&(!sensor->isDownstream())){
    locTrackDir=make_pair((-1)*gloTrackDir.x(), (-1)*gloTrackDir.y());
  }
  return ( locTrackDir );
}
//
