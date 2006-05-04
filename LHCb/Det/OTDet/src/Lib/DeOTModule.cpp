// $Id: DeOTModule.cpp,v 1.14 2006-05-04 16:50:31 janos Exp $
/// Kernel
#include "Kernel/Point3DTypes.h"
#include "Kernel/SystemOfUnits.h"
#include "Kernel/LineTraj.h"

/// DetDesc
#include "DetDesc/IGeometryInfo.h"
#include "DetDesc/SolidBox.h"

/// GSL
#include "gsl/gsl_math.h"

/// local
#include "OTDet/DeOTModule.h"

/** @file DeOTModule.cpp
 *
 *  Implementation of class :  DeOTModule
 *
 *  @author Jeroen van Tilburg jtilburg@nikhef.nl
 */

using namespace LHCb;

DeOTModule::DeOTModule(const std::string& name) :
  DetectorElement(name),
  m_stationID(0u),
  m_layerID(0u),
  m_quarterID(0u),
  m_moduleID(0u),
  m_elementID(0u),
  m_uniqueModuleID(0u),
  m_nStraws(0u),
  m_stereoAngle(0.0),
  m_sinAngle(0.0),
  m_cosAngle(0.0),
  m_xPitch(0.0),
  m_zPitch(0.0),
  m_cellRadius(0.0),
  m_inefficientRegion(0.0),
  m_nModules(0),
  m_ySizeModule(0.0),
  m_yHalfModule(0.0), 
  m_xMinLocal(0.0),
  m_xMaxLocal(0.0),
  m_yMinLocal(0.0),
  m_yMaxLocal(0.0),
  m_xInverted(false),
  m_yInverted(false) {
  
  /// Constructor 
}

DeOTModule::~DeOTModule() { /// Destructor 
}

const CLID& DeOTModule::clID() const { 
  return classID();
}


StatusCode DeOTModule::initialize() 
{
 
  IDetectorElement* quarter = this->parentIDetectorElement();
  IDetectorElement* layer = quarter->parentIDetectorElement();
  IDetectorElement* station = layer->parentIDetectorElement();
  IDetectorElement* ot = station->parentIDetectorElement();

  // Get specific parameters from the module
  m_moduleID = (unsigned int) param<int>("moduleID");
  m_quarterID = (unsigned int) quarter->params()->param<int>("quarterID");
  m_layerID = (unsigned int) layer->params()->param<int>("layerID");
  m_stationID = (unsigned int) station->params()->param<int>("stationID");
  m_nStraws = (unsigned int) param<int>("nStraws");
  m_stereoAngle = layer->params()->param<double>("stereoAngle");
  m_sinAngle = sin(m_stereoAngle);
  m_cosAngle = cos(m_stereoAngle);  
  OTChannelID aChan = OTChannelID(m_stationID, m_layerID, 
                                  m_quarterID, m_moduleID, 0u);
  setElementID(aChan);
  m_uniqueModuleID = aChan.uniqueModule();

  // Get some general parameters for the OT
  m_xPitch = ot->params()->param<double>("strawPitchX");
  m_zPitch = ot->params()->param<double>("strawPitchZ");
  m_cellRadius = ot->params()->param<double>("cellRadius");
  m_inefficientRegion = ot->params()->param<double>("inefficientRegion");
  m_nModules = (unsigned int) ot->params()->param<int>("nModules");

  // Get the lenght of the module
  const ILVolume* lv = (this->geometry())->lvolume();
  const ISolid* solid = lv->solid();
  const SolidBox* mainBox = dynamic_cast<const SolidBox*>(solid);
  m_ySizeModule = mainBox->ysize();
  m_yHalfModule = mainBox->yHalfLength();

  m_xMaxLocal = (0.5*m_nStraws+0.25)*m_xPitch;
  m_xMinLocal = -m_xMaxLocal;
  m_yMaxLocal = m_yHalfModule;
  m_yMinLocal = -m_yMaxLocal;
  
  determineSense();
  
  /// cache trajectories/planes
  cacheInfo();

  return StatusCode::SUCCESS;
}

void DeOTModule::findStraws(const Gaudi::XYZPoint& entryPoint, 
                            const Gaudi::XYZPoint& exitPoint, 
                            std::vector<unsigned int>& straws) const {
  /// This is in local cooridinates of a module
  double xOffset = -(m_nStraws/2. + 0.25) * m_xPitch;
  double lo = (entryPoint.x()-xOffset)/m_xPitch; 
  double hi = (exitPoint.x()-xOffset)/m_xPitch;
  
  if (lo > hi) std::swap(lo , hi);

  int exStraw = 1; ///< Add extra straws to the the left and right
  unsigned int strawLo = GSL_MAX_INT(0, int(std::floor(lo)) - exStraw);
  unsigned int strawHi = GSL_MIN_INT(int(m_nStraws)-1, int(std::ceil(hi)) + exStraw);

  /// Make sure straws vector is empty 
  straws.clear(); ///< This should erase all elements, if any.
  straws.reserve(strawHi-strawLo);
  
  /// Now let's fill the vector. Remember straw numbering starts at 1, i.e. i+1
  for (unsigned int i = strawLo; i <= strawHi; ++i) straws.push_back(i+1);
}

StatusCode DeOTModule::findDoca(const Gaudi::XYZPoint& entryPoint,
				const Gaudi::XYZVector& pUnit,
				const Gaudi::XYZPoint& wireBottom,
				const Gaudi::XYZVector& wUnit,
				double& lambda, double& mu,
				Gaudi::XYZVector& doca) const { 
  Gaudi::XYZVector PenMinWb = (entryPoint - wireBottom);
  double e_pDote_w = pUnit.Dot(wUnit);
  
  double dnom = 1.0 - gsl_pow_2(e_pDote_w);
  static const double eps = 0.0001;
  
  if ( dnom < eps ) {
    /// You never know :-)
    MsgStream msg( msgSvc(), name() );
    msg << MSG::WARNING << "Track is parallel to wire" << endreq;
    return StatusCode::FAILURE;
  }
  
  lambda = (-1/dnom)*(PenMinWb.Dot(pUnit) - PenMinWb.Dot(wUnit)*e_pDote_w);
  mu = (1/dnom)*(PenMinWb.Dot(wUnit) - PenMinWb.Dot(pUnit)*e_pDote_w);
  doca = PenMinWb + lambda*pUnit - mu*wUnit;

  return StatusCode::SUCCESS;
}

/// This gives you the x position of the wire
double DeOTModule::localUOfStraw(const int aStraw) const {
  int tmpStraw = aStraw;
  double uLeftStraw =  -(m_nStraws/2. - 0.25) * m_xPitch;
  if (this->monoLayerB(aStraw)) { // monoLayer B
    tmpStraw -= m_nStraws;
    uLeftStraw += 0.5 * m_xPitch;
  }
  tmpStraw -= 1; // straw numbering starts at 1
  
  return uLeftStraw + tmpStraw * m_xPitch;
}

StatusCode DeOTModule::calculateHits(const Gaudi::XYZPoint& entryPoint,
                                     const Gaudi::XYZPoint& exitPoint,
                                     std::vector<OTChannelID>& channels,
                                     std::vector<double>& driftDistances) const {

  /// Before we do anything crazy let's first check if the 
  /// entry and exit points are inside the module.
  if ( !isInside(entryPoint) || !isInside(exitPoint) ) return StatusCode::FAILURE;
  
  /// Make sure channels and driftdistances vectors are empty 
  channels.clear(); ///< This should erase all elements, if any.
  driftDistances.clear(); ///< This should erase all elements, if any.
  
  /// OK, so apparently the points are inside, so let's get the
  /// the range of straws that might contain hits. First we need to
  /// convert the points to the local coordinate system of the module.
  const IGeometryInfo* gi = this->geometry();
  Gaudi::XYZPoint enP = gi->toLocal(entryPoint);
  Gaudi::XYZPoint exP = gi->toLocal(exitPoint);

  /// Now let's get the range of straws
  std::vector<unsigned int> rStraws;
  findStraws(enP, exP, rStraws);

  /// Unit vector parallel to entry and exit points
  Gaudi::XYZVector e_p = (exP - enP).unit();

  /// Need this to check that enZ and exZ aren't sort of in the same plane,
  /// i.e. not a curly track. These are typically low momentum (50 MeV) 
  /// electrons.
  double enZ = enP.z();
  double exZ = exP.z();
  bool samePlane = std::abs(enZ-exZ) < 1.*m_cellRadius;
  
  /// Now, let's loop over the straws and check if they do contain hits
  /// and asign OTChannelID's to those that do.
  /// This going to be the same for monolayer A as well as monolayer B, except
  /// for the channel numbering. So it makes sense to define a method
  /// findHitStraws(...)
    
  Gaudi::XYZPoint wB;
  Gaudi::XYZPoint wT;
  
  wB.SetY(-m_yHalfModule);
  wT.SetY(m_yHalfModule);
     
  double mu;
  double lambda;
  Gaudi::XYZVector doca;
  /// Need this to check if hits are in efficient regions
  bool efficientY;
  
  if (!samePlane) { // Track in cell
    /// first layer
    wB.SetZ(-0.5*m_zPitch);
    wT.SetZ(-0.5*m_zPitch);
    std::vector<unsigned int>::iterator iStraw;
    for (iStraw = rStraws.begin(); iStraw != rStraws.end(); ++iStraw) {
      /// Wire bottom
      wB.SetX(localUOfStraw((*iStraw)));
      /// Wire top
      wT.SetX(localUOfStraw((*iStraw))); 
      StatusCode sc = findDoca(enP, e_p, wB, (wT - wB).unit(), lambda, mu, doca); 
      if (sc == StatusCode::SUCCESS) {
	double dist = driftDistance(doca);
	efficientY = isEfficientA(-m_yHalfModule+mu); 
    	/// Do we have a hit?
    	if (fabs(dist) < m_cellRadius && efficientY) {
    	  channels.push_back(OTChannelID(m_stationID, m_layerID,
    					 m_quarterID, m_moduleID, (*iStraw)));
    	  driftDistances.push_back(dist);
    	}
      }
    }
    
    /// second layer
    wB.SetZ(0.5*m_zPitch);
    wT.SetZ(0.5*m_zPitch);
    for (iStraw = rStraws.begin(); iStraw != rStraws.end(); ++iStraw) {
      /// Wire bottom
      wB.SetX(localUOfStraw((*iStraw)+m_nStraws));
      /// Wire top
      wT.SetX(localUOfStraw((*iStraw)+m_nStraws));
      StatusCode sc = findDoca(enP, e_p, wB, (wT - wB).unit(), lambda, mu, doca); 
      /// Do we have a hit?
      if (sc == StatusCode::SUCCESS) {
	double dist = driftDistance(doca);
	efficientY = isEfficientB(-m_yHalfModule+mu);
	if (fabs(dist) < m_cellRadius && efficientY) {
	  channels.push_back(OTChannelID(m_stationID, m_layerID,
					 m_quarterID, m_moduleID, m_nStraws+(*iStraw)));
	  driftDistances.push_back(dist);
	}
      }
    } 
  } else { // curly track
    /// This is the old curly track code. I still need to figure out
    /// how to treat this better and make it more readable.
    //msg << MSG::DEBUG << "==> We have a curly track" << endreq;
    //msg << MSG::DEBUG << "Pen_x = " << enP.x() << " Pen_y = " << enP.y() 
    //<< " Pen_z = " << enP.z() << endreq;
    //msg << MSG::DEBUG << "Pex_x = " << exP.x() << " Pex_y = " << exP.y() 
    //<< " Pex_z = " << exP.z() << endreq;
    
    double x1 = enP.x();
    double z1 = enP.z();
    double x2 = exP.x();
    double z2 = exP.z();

    double uLow = x1;
    double uHigh = x2;      
    if ( uLow > uHigh ) std::swap(uLow, uHigh);
        
    double distXY = ( exP - enP ).Rho();
    double z3Circ,zCirc,uCirc,rCirc;
    double zfrac,zint;

    // zfrac is between 0 and 1. 2.7839542167 means nothing.
    // This seems to acts as a random number generator.
    // if distance xy entry-exit is small generate z3 close
    // to the z1 and z2 ( z1 is close to z2)
    double u1 = (entryPoint.z() > exitPoint.z()) ? exitPoint.x() : entryPoint.x();
    double v1 = (entryPoint.z() > exitPoint.z()) ? exitPoint.y() : entryPoint.y();
    zfrac = modf( (fabs(u1)+fabs(v1))/2.7839542167, &zint);
    if (distXY > 2.0 * m_xPitch ) {
      z3Circ = 2.0 * (zfrac-0.5)*m_zPitch;
    } else {
      if ( z1 < 0 ) {
        z3Circ = - zfrac * m_zPitch; 
      } else {
        z3Circ = zfrac * m_zPitch; 
      }
    }

    sCircle(z1, x1, z2, x2, z3Circ, zCirc, uCirc, rCirc);
    
    // monolayer A
    int strawA = hitStrawA(uLow);
    double zStrawA = localZOfStraw(strawA);
    double uStep = uLow;
    while ( (uStep < uHigh) && strawA != 0 ) {
      uStep = localUOfStraw(strawA);
      double distCirc = 
        sqrt((zCirc-zStrawA)*(zCirc-zStrawA) + (uCirc-uStep)*(uCirc-uStep));
      double distTmp = fabs(distCirc-rCirc);
      int straw = strawA;
      double ambTmp = - (uStep-(x1+x2)/2.) * (distCirc-rCirc);

      if ( distTmp < m_cellRadius ) {
        if (ambTmp < 0.0 ) distTmp *= -1.0;
	channels.push_back(OTChannelID(m_stationID, m_layerID, 
				       m_quarterID, m_moduleID, straw));
        driftDistances.push_back(distTmp);
      }
      strawA = nextRightStraw(straw);
    }

    // monolayer B
    int strawB = hitStrawB(uLow);
    double zStrawB = localZOfStraw(strawB);
    uStep = uLow;
    while ( (uStep < uHigh) && strawB != 0 ) {
      uStep = localUOfStraw(strawB);
      double distCirc = 
        sqrt((zCirc-zStrawB)*(zCirc-zStrawB) + (uCirc-uStep)*(uCirc-uStep));
      double distTmp = fabs(distCirc-rCirc);
      int straw = strawB;

      double ambTmp = - (uStep-(x1+x2)/2.) * (distCirc-rCirc);
      if ( distTmp < m_cellRadius ) {
        if (ambTmp < 0.0) distTmp *= -1.0;
        channels.push_back(OTChannelID( m_stationID, m_layerID, 
					m_quarterID, m_moduleID, straw));
        driftDistances.push_back(distTmp);
      }
      strawB = nextRightStraw(straw);
    }
  } // curling tracks
  
  return StatusCode::SUCCESS;
}

void DeOTModule::sCircle(const double z1, const double u1, const double z2, 
                         const double u2, const double z3c,
                         double& zc, double& uc, double& rc) const {
  
  double zw=(z1+z2)/2.;
  double uw=(u2+u1)/2.;
  
  zc=0.5*(z3c*z3c-zw*zw-(u1-uw)*(u1-uw))/(z3c-zw);
  uc=uw;
  rc=fabs(zc-z3c);
}

double DeOTModule::distanceToWire(const unsigned int aStraw, 
                                  const Gaudi::XYZPoint& aPoint, 
                                  const double tx, const double ty) const {
  // go to the local coordinate system
  Gaudi::XYZVector vec(tx, ty, 1.);
  Gaudi::XYZPoint localPoint = (this->geometry())->toLocal(aPoint);
  Gaudi::XYZVector localVec = (this->geometry())->toLocal(aPoint+vec) - localPoint;

  // calculate distance to the straw
  double z = this->localZOfStraw(aStraw);
  double u = localPoint.x() + localVec.x() * (z - localPoint.z());
  double cosU = 1./sqrt(1. + gsl_pow_2(localVec.x()/localVec.z()));
  double dist = (u - localUOfStraw(aStraw)) * cosU;
  
  return dist;
}

double DeOTModule::distanceAlongWire(const double xHit, 
                                     const double yHit) const { 
  Gaudi::XYZPoint globalPoint(xHit, yHit, 0);
  Gaudi::XYZPoint localPoint = (this->geometry())->toLocal(globalPoint);

  // For the upper modules of the station the readout is above.
  return ( this->topModule() ) ? 
    m_yHalfModule - localPoint.y() : m_yHalfModule + localPoint.y();
}

Gaudi::XYZPoint DeOTModule::centerOfStraw(const unsigned int straw) const {
  /// get the global coordinate of the middle of the channel
  Gaudi::XYZPoint localPoint(this->localUOfStraw(straw), 0., 
                             this->localZOfStraw(straw));
  return (this->geometry())->toGlobal(localPoint);
}

Gaudi::XYZPoint DeOTModule::centerOfModule() const {
  /// get the global coordinate of the middle of the module
  Gaudi::XYZPoint localPoint(0., 0., 0.);
  return (this->geometry())->toGlobal(localPoint);
}

double DeOTModule::z() const {
  /// Get the global z-coordinate of the module
  return centerOfModule().z();
}

void DeOTModule::clear() {
  m_midTraj[0].reset();
  m_midTraj[1].reset();
}

void DeOTModule::determineSense() {
  Gaudi::XYZPoint g1 = globalPoint(m_xMinLocal, m_yMinLocal, 0.0);
  Gaudi::XYZPoint g2 = globalPoint(m_xMaxLocal, m_yMinLocal, 0.0);
  m_xInverted = (g1.x() > g2.x());
  
  Gaudi::XYZPoint g3 = globalPoint(m_xMinLocal, m_yMaxLocal, 0.0);
  m_yInverted = (g1.y() > g3.y());
}

void DeOTModule::cacheInfo() {
  clear();
  
  double xUpper = m_xMaxLocal;
  double xLower = m_xMinLocal;
  if (m_xInverted) std::swap(xUpper, xLower);
  double yUpper = m_yMaxLocal;
  double yLower = m_yMinLocal;
  if (m_yInverted) std::swap(yUpper, yLower);

  /// Direction, points to readout 
  Gaudi::XYZPoint g1 = globalPoint(xLower, yLower, 0.0);
  Gaudi::XYZPoint g2 = globalPoint(xLower, yUpper, 0.0);
  m_dir = g2 - g1;
  
  /// trajs of middle of monolayers
  Gaudi::XYZPoint g3[2];
  /// 0 -> first monolayer
  g3[0] = globalPoint(xLower, 0.0, -m_zPitch);
  /// 1 -> second monolayer
  g3[1] = globalPoint(xLower, 0.0, +m_zPitch);
  Gaudi::XYZPoint g4[2]; 
  /// first monolayer
  g4[0] = globalPoint(xUpper, 0.0, -m_zPitch);
  /// second monolayer
  g4[1] = globalPoint(xUpper, 0.0, +m_zPitch);
  /// first monolayer
  m_midTraj[0].reset(new LineTraj(g3[0], g4[0]));
  /// second monolayer
  m_midTraj[1].reset(new LineTraj(g3[1], g4[1]));
 
  /// range -> wire length
  /// wire lenght = module length
  /// first mono layer
  m_range[0] = std::make_pair(-m_yHalfModule, m_yHalfModule);
  /// second mono layer
  m_range[1] = std::make_pair(-m_yHalfModule, m_yHalfModule);
  // correct for inefficient regions in long modules
  if (longModule() && topModule()) m_range[0].first=-m_yHalfModule+m_inefficientRegion;
  if (longModule() && bottomModule()) m_range[1].second=m_yHalfModule-m_inefficientRegion;
  
  /// plane
  m_plane = Gaudi::Plane3D(g1, g2, g4[0] + 0.5*(g4[1]-g4[0]));
}

std::auto_ptr<LHCb::Trajectory> DeOTModule::trajectoryFirstWire(int monolayer) const {
  double lUwire = (monolayer==0?localUOfStraw(1):(s3Module()?localUOfStraw(33):localUOfStraw(65)));
  Gaudi::XYZPoint firstWire = m_midTraj[monolayer]->position(lUwire+m_midTraj[monolayer]->beginRange());
  return std::auto_ptr<LHCb::Trajectory>(new LineTraj(firstWire, m_dir, m_range[monolayer]));
}

std::auto_ptr<LHCb::Trajectory> DeOTModule::trajectoryLastWire(int monolayer) const {
  double lUwire = (monolayer==1?(s3Module()?localUOfStraw(64):localUOfStraw(128)):localUOfStraw(64));
  Gaudi::XYZPoint lastWire = m_midTraj[monolayer]->position(lUwire+m_midTraj[monolayer]->beginRange());
  return std::auto_ptr<LHCb::Trajectory>(new LineTraj(lastWire, m_dir, m_range[monolayer]));
}

/// Returns a Trajectory representing the wire identified by the LHCbID
/// The offset is zero for all OT Trajectories
std::auto_ptr<LHCb::Trajectory> DeOTModule::trajectory(const OTChannelID& aChan,
						       const double /*offset*/) const {
  if (!contains(aChan)) {
    throw GaudiException("Failed to make trajectory!", "DeOTModule.cpp",
				StatusCode::FAILURE);
  }
  
  unsigned int aStraw = aChan.straw();
  
  int mono = (monoLayerA(aStraw)?0:1);
  
  double uLstraw = localUOfStraw(aChan.straw());
  Gaudi::XYZPoint posWire = m_midTraj[mono]->position(uLstraw+m_midTraj[mono]->beginRange());
  
  return std::auto_ptr<Trajectory>(new LineTraj(posWire, m_dir, m_range[mono]));
}
