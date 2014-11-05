// $Id: $
#include <boost/lexical_cast.hpp>

// DetDesc
#include "DetDesc/SolidSubtraction.h"
#include "DetDesc/SolidChild.h"
#include "DetDesc/SolidCons.h"

// Smartrefs
#include "GaudiKernel/SmartDataPtr.h"
#include "GaudiKernel/ISvcLocator.h"

// Gaudi/LHCb Math
#include "GaudiKernel/Plane3DTypes.h"
#include "GaudiKernel/Point3DTypes.h"
#include "GaudiKernel/Vector3DTypes.h"
#include "GaudiKernel/GenericMatrixTypes.h"
#include "LHCbMath/Line.h"
#include "LHCbMath/GeomFun.h"
#include "LHCbMath/LineTypes.h"
#include "GaudiKernel/GaudiException.h"

// FTDet
#include "FTDet/DeFTDetector.h" 
#include "FTDet/DeFTFibreMat.h"


/** @file DeFTFibreMat.cpp
 *
 *  Implementation of class : DeFTFibreMat
 *
 *  @author Plamen Hopchev
 *  @author Eric Cogneras
 *  @author Diego Milanes
 *  @date   2013-07-12
 */

using namespace LHCb;

typedef Gaudi::Math::Line<Gaudi::XYZPoint,Gaudi::XYZVector> Line;


//=============================================================================
// Standard constructor, initializes variables
//=============================================================================
DeFTFibreMat::DeFTFibreMat( const std::string& name ) :
  DetectorElement ( name ),
  // Layer stereo angle
  m_angle(0.0),
  m_tanAngle(0.0),
  m_cosAngle(0.0),
  m_dzDy(0.0),
  //ChannelID info
  m_FibreMatID(0),
  m_layerID(0),
  m_mat( 0 ),
  m_module( 0 ),
  m_layer( 0 ),
  m_quarter(0),       
  m_relativemodule(0),
  m_holey( false ),
  //fibremat dimension
  m_fibreMatHalfSizeX(0.),
  m_fibreMatHalfSizeY(0.),
  m_fibreMatHalfSizeZ(0.),
  //Layer dimensions
  m_layerMinX(0.),
  m_layerMaxX(0.),
  m_layerMinY(0.),
  m_layerMaxY(0.),
  m_layerMinZ(0.),
  m_layerMaxZ(0.),
  m_layerHalfSizeX(0.),
  m_layerHalfSizeY(0.),
  m_layerHalfSizeZ(0.),
  m_innerHoleRadius(0.),
  //Sipm info
  m_sipmPitchX(0.),
  m_nSipmPerModule(0),
  
  //-----params causing "private part" problem in FTDet
  //Hole geometry
  m_posHole(0,0,0),          //Hole position
  m_HoleShiftXSt(0.),
  m_HoleShiftYSt(0.),
  m_halfHole1X(0.),   //Hole in X, 4 sections
  m_halfHole2X(0.),
  m_halfHole3X(0.),
  m_halfHole4X(0.),
  m_halfHole1Y(0.),   //Hole in Y, 4 sections,
  m_halfHole2Y(0.),   //T station dependent
  m_halfHole3Y(0.),
  m_halfHole4Y(0.),
  
  //Auxiliary stuff
  m_msg(NULL)
{
  m_DeFTLocation = "/dd/Structure/LHCb/AfterMagnetRegion/T/FT";
  m_FTGeomVersion_reference = 20;
  m_BadChannelLayerFlag = 15;
}

//=============================================================================
// Destructor
//=============================================================================
DeFTFibreMat::~DeFTFibreMat(){
  if ( m_msg ) {
    delete m_msg;
    m_msg = 0;
  }
} 

//=============================================================================
// classID function
//=============================================================================
const CLID& DeFTFibreMat::clID () const {
  return DeFTFibreMat::classID();
}


//=============================================================================
// Initialization
//=============================================================================
StatusCode DeFTFibreMat::initialize(){
  
  StatusCode sc = DetectorElement::initialize(); // must be executed first
  if ( sc.isFailure() ) return sc;

  /// Create a MsgStream object with an add-hoc name (the second argument),
  /// instead of the ususual name(), which gives a too long string
  if ( 0 == m_msg ) m_msg = new MsgStream( msgSvc(), "DeFTFibreMat" );
  if( m_msg->level() <= MSG::DEBUG) debug() << "==> Initialize DeFTFibreMat" << endmsg;


  //init depending of detector geometry version
  int FTversion(-1);
  SmartDataPtr<DetectorElement> deFibreTracker (dataSvc(), m_DeFTLocation);
  if( deFibreTracker ){
    if( deFibreTracker -> exists( "FTversion" )) FTversion = deFibreTracker -> param<int>("FTversion");
  }
  if (FTversion < 0) {
    fatal() << "Can't access FT version" << endmsg;
    return StatusCode::FAILURE;
  }
  else {
    m_FTGeomversion=FTversion;
  }
    
  //---Set the FibreMat ID and stereo angle from the xml DDDB
  m_FibreMatID = this->params()->param<int>("FibreMatID");
  m_angle = this->params()->param<double>("stereoAngle");
  m_tanAngle = tan(m_angle);
  m_cosAngle = cos(m_angle);
    
  //---Set sections IDs and some flags  
  // Given the AABBCC convention for the FibreMatID, 
  // AA layer number T1( 00 01 02 03 ) T2( 04 05 06 07 ) T3 ( 08 09 10 11 )
  // BB module number 00 01 02 03 04 10(LeftH) 11(RightH) 05 06 07 08 09
  // CC 00(Top) 01(Bottom) 
  m_mat = m_FibreMatID%10;
  m_module = int( ( m_FibreMatID %10000 ) / 100 );
  m_relativemodule = 
    ( m_module == 10 || m_module == 11 )? 0 :
    ( m_module == 4  || m_module == 5  )? 1 :
    ( m_module == 3  || m_module == 6  )? 2 :
    ( m_module == 2  || m_module == 7  )? 3 :
    ( m_module == 1  || m_module == 8  )? 4 :
    ( m_module == 0  || m_module == 9  )? 5 : 99;

  if      ( (m_module <= 4 || m_module == 10) && m_mat ) m_quarter = 1;
  else if ( (m_module <= 4 || m_module == 10) && !m_mat ) m_quarter = 3;
  else if ( ((m_module >= 5 && m_module <= 9) || m_module == 11) && m_mat ) m_quarter = 0;
  else if ( ((m_module >= 5 && m_module <= 9) || m_module == 11) && !m_mat ) m_quarter = 2;
  else{
    m_quarter = -99;
    if( m_msg->level() <= MSG::DEBUG ) debug() << "Aborting calculateHits: not possible to find module/quarter" << endmsg;
    return StatusCode::FAILURE;
  }
  m_layer  = int( m_FibreMatID / 10000 ); 
  m_layerID = m_layer;
  m_holey = ( m_module == 10 || m_module == 11 );   //central fibremats with holes

  
  //-----------------Get geometrical info
  
  //-------------fibremat geometry
  
  //----------geometry v20
  if(m_FTGeomversion <= m_FTGeomVersion_reference) {
    
    //other flags
    m_RightHoleAxesXZInversion=false;   //do NOT invert Right Hole XZ axes if v2 geometry (temporary)

    const SolidBox *outerBox(0);
    if( !m_holey ){   //fibremat without holes
      outerBox = dynamic_cast<const SolidBox*>( this->geometry()->lvolume()->solid() );
      if ( 0 == outerBox ) {
        fatal() << "Can't acquire layer geometry (SolidBox not holey)" << endmsg;
        return StatusCode::FAILURE;
      }
      m_innerHoleRadius = 0.0;
    }
    else {   //with holes
      const SolidSubtraction *subtrObject = dynamic_cast<const SolidSubtraction*>( this->geometry()->lvolume()->solid() );
      if ( 0 == subtrObject ) {
        fatal() << "Can't acquire layer geometry (SolidSubtraction)" << endmsg;
        return StatusCode::FAILURE;
      }
      outerBox = dynamic_cast<const SolidBox*>( subtrObject->coverTop() );
      if ( 0 == outerBox ) {
        fatal() << "Can't acquire layer geometry (SolidBox holey)" << endmsg;
        return StatusCode::FAILURE;
      }
      //Hole geometry
      const SolidChild *tmpChild = dynamic_cast<const SolidChild*>( (*subtrObject)[0] );
      if ( 0 == tmpChild ) {
        fatal() << "Can't acquire layer geometry (SolidChild)" << endmsg;
        return StatusCode::FAILURE;
      }
      const SolidCons *innerCons = dynamic_cast<const SolidCons*>( tmpChild->solid() );
      if ( 0 == innerCons ) {
        fatal() << "Can't acquire layer geometry (SolidCons)" << endmsg;
        return StatusCode::FAILURE;
      }
      m_innerHoleRadius = innerCons->outerRadiusAtPlusZ();
    }
    m_fibreMatHalfSizeX = outerBox->xHalfLength();
    m_fibreMatHalfSizeY = outerBox->yHalfLength();
    m_fibreMatHalfSizeZ = outerBox->zHalfLength();
    
    //Fibremat center and corners (Right, Left, Bottom, Top ...) in global frame
    m_fibreMatGlobalCenter = this->geometry()->toGlobal( Gaudi::XYZPoint(0.,0.,0.) );
    m_fibreMatGlobalRT = this->geometry()->toGlobal( Gaudi::XYZPoint(-m_fibreMatHalfSizeX,m_fibreMatHalfSizeY,0) );
    m_fibreMatGlobalLT = this->geometry()->toGlobal( Gaudi::XYZPoint(m_fibreMatHalfSizeX,m_fibreMatHalfSizeY,0) );
    m_fibreMatGlobalRB = this->geometry()->toGlobal( Gaudi::XYZPoint(-m_fibreMatHalfSizeX,-m_fibreMatHalfSizeY,0) );
    m_fibreMatGlobalLB = this->geometry()->toGlobal( Gaudi::XYZPoint(m_fibreMatHalfSizeX,-m_fibreMatHalfSizeY,0) );
    
    //sipm or fibre end y position (local frame)
    if(m_mat) m_fibreMatSipmY = -m_fibreMatHalfSizeY;
    else m_fibreMatSipmY = m_fibreMatHalfSizeY;
  }

    
  //-----------geometry v4x
  if (m_FTGeomversion > m_FTGeomVersion_reference) {
    //other flags
    if(m_module==11) m_RightHoleAxesXZInversion=true;    //invert Right Hole XZ axes if v4 geometry (temporary)
    else m_RightHoleAxesXZInversion=false;
    
    //FibreMat size and position 
    m_fibreMatHalfSizeX = 0.5*this->params()->param<double>("FTFibreMatSizeXSt");
    m_fibreMatHalfSizeY = 0.5*this->params()->param<double>("FTFibreMatSizeYSt");
    m_fibreMatHalfSizeZ = 0.5*this->params()->param<double>("FTFibreMatSizeZ");

    //Fibremat center and corners (Right, Left, Bottom, Top ...) in global frame
    //with temporary patch for XZ module 11 inversion
    m_fibreMatGlobalCenter = this->geometry()->toGlobal( Gaudi::XYZPoint(0.,0.,0.) );
    if(m_RightHoleAxesXZInversion) {
      m_fibreMatGlobalRT = this->geometry()->toGlobal( Gaudi::XYZPoint(m_fibreMatHalfSizeX,m_fibreMatHalfSizeY,0) );
      m_fibreMatGlobalLT = this->geometry()->toGlobal( Gaudi::XYZPoint(-m_fibreMatHalfSizeX,m_fibreMatHalfSizeY,0) );
      m_fibreMatGlobalRB = this->geometry()->toGlobal( Gaudi::XYZPoint(m_fibreMatHalfSizeX,-m_fibreMatHalfSizeY,0) );
      m_fibreMatGlobalLB = this->geometry()->toGlobal( Gaudi::XYZPoint(-m_fibreMatHalfSizeX,-m_fibreMatHalfSizeY,0) );
    } 
    else {
      m_fibreMatGlobalRT = this->geometry()->toGlobal( Gaudi::XYZPoint(-m_fibreMatHalfSizeX,m_fibreMatHalfSizeY,0) );
      m_fibreMatGlobalLT = this->geometry()->toGlobal( Gaudi::XYZPoint(m_fibreMatHalfSizeX,m_fibreMatHalfSizeY,0) );
      m_fibreMatGlobalRB = this->geometry()->toGlobal( Gaudi::XYZPoint(-m_fibreMatHalfSizeX,-m_fibreMatHalfSizeY,0) );
      m_fibreMatGlobalLB = this->geometry()->toGlobal( Gaudi::XYZPoint(m_fibreMatHalfSizeX,-m_fibreMatHalfSizeY,0) );
    }
        
    //sipm or fibre end y position (local frame)
    if(m_mat) m_fibreMatSipmY = -m_fibreMatHalfSizeY;   //bottom
    else m_fibreMatSipmY = m_fibreMatHalfSizeY;

    //Hole(s)
    if( m_holey ) {   //fibremat with holes
      //Hole position (local frame): same for all sub-boxes
      m_HoleShiftXSt=this->params()->param<double>("HoleShiftXSt");
      m_HoleShiftYSt=this->params()->param<double>("HoleShiftYSt");
      m_posHole.SetXYZ(m_HoleShiftXSt, m_HoleShiftYSt, m_fibreMatGlobalCenter.z());
      
      //Hole sub boxes sizes (local frame)
      m_halfHole1X=this->params()->param<double>("Hole1X")/2;
      m_halfHole2X=this->params()->param<double>("Hole2X")/2;
      m_halfHole3X=this->params()->param<double>("Hole3X")/2;
      m_halfHole4X=this->params()->param<double>("Hole4X")/2;
      m_halfHole1Y=this->params()->param<double>("Hole1Y")/2;
      m_halfHole2Y=this->params()->param<double>("Hole2Y")/2;
      m_halfHole3Y=this->params()->param<double>("Hole3Y")/2;
      m_halfHole4Y=this->params()->param<double>("Hole4Y")/2;
    }
 }



  //--------------layer module geometry (local coordinates)
  double FTFibreModuleSizeX(0),FTFibreModuleSizeY(0),FTDeadHSizeZ(0),FTFibreModuleSizeZ(0),FTFibreSizeZ(0);
  IDetectorElement *parentDet = this->parentIDetectorElement();
  if(0 != parentDet) {
  
    //-------------geometry before v4x    
    if(m_FTGeomversion <= m_FTGeomVersion_reference) {
      //Left/right vertical dead zone 
      const SolidBase *sbDeadV(0);
      if( ((sc=findSolidBase(parentDet,"pvDeadVLeft",sbDeadV)) == StatusCode::SUCCESS) ||
          ((sc=findSolidBase(parentDet,"pvDeadVRight",sbDeadV)) == StatusCode::SUCCESS) )
      {
        m_moduleEdgeSizeX=sbDeadV->xMax()-sbDeadV->xMin();
      }
      else {
        fatal() << "Can't find PVolume: "<<"DeadV"<< endmsg;
        return sc;
      }
      //Horizontal dead zone
      const SolidBase *sbDeadH(0);
      if( ((sc=findSolidBase(parentDet,"pvDeadH",sbDeadH)) == StatusCode::SUCCESS) ||
          ((sc=findSolidBase(parentDet,"pvDeadHHoleL",sbDeadH)) == StatusCode::SUCCESS) ||
          ((sc=findSolidBase(parentDet,"pvDeadHHoleR",sbDeadH)) == StatusCode::SUCCESS) )
      {
        m_moduleGapH=sbDeadH->yMax()-sbDeadH->yMin();
      }
      else {
        fatal() << "Can't find PVolume (DeadH)"<< endmsg;
        return sc;
      }
      //Fibre zone
      const SolidBase *sbFibreTop(0);
      if( ((sc=findSolidBase(parentDet,"pvFibreTop",sbFibreTop)) == StatusCode::SUCCESS) ||
          ((sc=findSolidBase(parentDet,"pvFibreMatHoleLTop",sbFibreTop)) == StatusCode::SUCCESS) ||
          ((sc=findSolidBase(parentDet,"pvFibreMatHoleRTop",sbFibreTop)) == StatusCode::SUCCESS) )
      {
        FTFibreSizeZ = sbFibreTop->zMax()-sbFibreTop->zMin();
      }
      else {
        fatal() << "Can't find PVolume (FibreTop)"<< endmsg;
        return sc;
      }

      //get parent to access other useful dimensions
      double FTCarbonSizeZ(0),FTHoneycombSizeZ(0),FTKaptonSizeZ(0);
      IDetectorElement *parent2Det = parentDet->parentIDetectorElement();
      if(0 != parent2Det) {
       //Carbon
        const SolidBase *sbCarbonFullA(0);
        if( ((sc=findSolidBase(parent2Det,"pvCarbonFullA",sbCarbonFullA)) == StatusCode::SUCCESS) ||
            ((sc=findSolidBase(parent2Det,"pvCarbonHoleLA",sbCarbonFullA)) == StatusCode::SUCCESS) ||
            ((sc=findSolidBase(parent2Det,"pvCarbonHoleRA",sbCarbonFullA)) == StatusCode::SUCCESS) )
        {
          FTCarbonSizeZ = sbCarbonFullA->zMax()-sbCarbonFullA->zMin();
        }
        else {
          fatal() << "Can't find PVolume (CarbonFullA)"<< endmsg;
          return sc;
        }
        //Honeycomb
        const SolidBase *sbHoneycombFullA(0);
        if( ((sc=findSolidBase(parent2Det,"pvHoneycombFullA",sbHoneycombFullA)) == StatusCode::SUCCESS)  || 
            ((sc=findSolidBase(parent2Det,"pvHoneycombHoleLA",sbHoneycombFullA)) == StatusCode::SUCCESS) ||
            ((sc=findSolidBase(parent2Det,"pvHoneycombHoleRA",sbHoneycombFullA)) == StatusCode::SUCCESS) )
        {
          FTHoneycombSizeZ = sbHoneycombFullA->zMax()-sbHoneycombFullA->zMin();
        }
        else {
          fatal() << "Can't find PVolume (HoneycombFullA)"<< endmsg;
          return sc;
        }
       //Kapton
        const SolidBase *sbKaptonFullA(0);
        if( ((sc=findSolidBase(parent2Det,"pvKaptonFullA",sbKaptonFullA)) == StatusCode::SUCCESS) ||
            ((sc=findSolidBase(parent2Det,"pvKaptonHoleLA",sbKaptonFullA)) == StatusCode::SUCCESS) ||
            ((sc=findSolidBase(parent2Det,"pvKaptonHoleRA",sbKaptonFullA)) == StatusCode::SUCCESS) )
        {
          FTKaptonSizeZ = sbKaptonFullA->zMax()-sbKaptonFullA->zMin();
        }
        else {
          fatal() << "Can't find PVolume (KaptonFullA)"<< endmsg;
          return sc;
        }
        FTFibreModuleSizeX = 2*(m_fibreMatHalfSizeX + m_moduleEdgeSizeX);
        FTFibreModuleSizeY = sbDeadV->yMax()-sbDeadV->yMin();
        FTDeadHSizeZ = FTCarbonSizeZ + FTHoneycombSizeZ + FTKaptonSizeZ;
        FTFibreModuleSizeZ = 2*FTDeadHSizeZ+FTFibreSizeZ;
      }
      else {
        fatal() << "Can't find 2nd level parent detector element of Fibremat (geometry v2)" << endmsg;
        return StatusCode::FAILURE;
      }
    }
    
    
    //-------------geometry v4x 
    if(m_FTGeomversion > m_FTGeomVersion_reference) {
      //Left vertical dead zone
      m_moduleEdgeSizeX = parentDet->params()->param<double>("FTDeadVSizeXSt");   
      m_moduleGapH = parentDet->params()->param<double>("FTDeadHSizeYSt");
      
      //Fibre zone
      FTFibreSizeZ = this->params()->param<double>("FTFibreMatSizeZ");   
      
      IDetectorElement *parent2Det = parentDet->parentIDetectorElement();
      if(0 != parent2Det) {
        FTFibreModuleSizeX = parent2Det->params()->param<double>("FTFullModuleSizeX");   
        FTFibreModuleSizeY = parent2Det->params()->param<double>("FTFullModuleSizeY");   
        FTFibreModuleSizeZ = parent2Det->params()->param<double>("FTFullModuleSizeZ");
      }
      else {
        fatal() << "Can't find 2nd level parent detector element of Fibremat (geometry v4)" << endmsg;
        return StatusCode::FAILURE;
      }
    }
 
    //fibreModule dimensions (in local frame)
    m_FibreModuleHalfSizeX = FTFibreModuleSizeX/2;
    m_FibreModuleHalfSizeY = FTFibreModuleSizeY/2.;
    m_FibreModuleHalfSizeZ = FTFibreModuleSizeZ/2.;
    
    //final layer dimensions (in local frame)
    m_layerHalfSizeX = 12*m_FibreModuleHalfSizeX;    //to be read from DB
    m_layerHalfSizeY = m_FibreModuleHalfSizeY;
    m_layerHalfSizeZ = m_FibreModuleHalfSizeZ;
    m_layerMinX = -m_layerHalfSizeX;
    m_layerMaxX = +m_layerHalfSizeX;
    m_layerMinY = -m_layerHalfSizeY;
    m_layerMaxY = +m_layerHalfSizeY;
    m_layerMinZ = m_fibreMatGlobalCenter.Z()-m_layerHalfSizeZ;  //DBL
    m_layerMaxZ = m_fibreMatGlobalCenter.Z()+m_layerHalfSizeZ;  //use here m_fibreMatGlobalCenter, different from layerCenter

    /// Determine the slope in the global y-z plane (All geometries)
    Gaudi::Rotation3D rotToGlobal;
    Gaudi::Matrix3x3 rotToGlobalMat;
    this->geometry()->toGlobalMatrixNominal().GetRotation(rotToGlobal);
    rotToGlobal.GetRotationMatrix(rotToGlobalMat);
    m_dzDy=std::abs(asin(rotToGlobalMat(1,2)));    //rot matrix(1,2) = -sin(angrotX) (euler ZXZ)
  }
  else {
   fatal() << "Can't acquire parent detector element of FibreMat" << endmsg;
   return StatusCode::FAILURE;
  }
  
  
  //-----------sipm geometry (to be put in some DB)
  if(m_FTGeomversion <= m_FTGeomVersion_reference) {
    m_sipmSizeX=32.25;
    m_sipmEdgeSizeX=0.625;
  }
  else {
    m_sipmSizeX=32.25;
    m_sipmEdgeSizeX=0.2;
  }
  m_sipmNChannels=128;
  m_cellSizeX=0.25;
  m_sipmPitchX = m_sipmSizeX + 2*m_sipmEdgeSizeX;
  m_nSipmPerModule = round(2.*m_fibreMatHalfSizeX/m_sipmPitchX);  
  //origin of sipm numbering in local frame (number 0 SiPM closest to
  //global ref frame origin, X axis points to the "left")
  //sipm total size (pitch) signed to account for pm numbering direction wrto local frame X axis
  m_sipmOriginX = (m_quarter%2)? -m_fibreMatHalfSizeX : m_fibreMatHalfSizeX;
  m_sipmPitchXsigned   = (m_quarter%2)? m_sipmPitchX : -m_sipmPitchX;


  //debuf stuff
  if( m_msg->level() <= MSG::DEBUG) {
    debug() << "FT geometry parameters:"
    << "\n\tm_layer: " << m_layer
    << "\n\tm_module: " << m_module
    << "\n\tm_quarter: " << m_quarter
    << "\n\tm_mat: " << m_mat
    << "\n\tm_angle: " << m_angle
    << "\n\tm_dzdy: " << m_dzDy
    << "\n\tfibreModule size X: " << 2*m_FibreModuleHalfSizeX
    << "\n\tfibreModule size Y: " << 2*m_FibreModuleHalfSizeY
    << "\n\tfibreModule size Z: " << 2*m_FibreModuleHalfSizeZ
    << "\n\tlayer min X: " << m_layerMinX << "\tmax X: " << m_layerMaxX
    << "\n\tlayer min Y: " << m_layerMinY << "\tmax Y: " << m_layerMaxY
    << "\n\tlayer min Z: " << m_layerMinZ << "\tmax Z: " << m_layerMaxZ
    << "\n\tm_moduleEdgeSizeX : " << m_moduleEdgeSizeX
    << "\n\tSipm Y pos: " << m_fibreMatSipmY
    << "\n\tSipm pitch X: " << m_sipmPitchX
    << "\n\tSipm N per module: " << m_nSipmPerModule
    << endmsg;
    if (m_holey) {
      debug() << "FT Hole geometry parameters:"
      << "\n\tm_posHole: " << m_posHole
      << "\n\tm_halfHole1 X: " << m_halfHole1X << "\tY: " << m_halfHole1Y
      << "\n\tm_halfHole2 X: " << m_halfHole2X << "\tY: " << m_halfHole2Y
      << "\n\tm_halfHole3 X: " << m_halfHole3X << "\tY: " << m_halfHole3Y
      << "\n\tm_halfHole4 X: " << m_halfHole4X << "\tY: " << m_halfHole4Y
      << endmsg;
    }
  }
  return StatusCode::SUCCESS;
}



//=============================================================================
// Finalization
//=============================================================================
StatusCode DeFTFibreMat::finalize(){
  // destroy the MsgService object
  if ( m_msg ) {
    delete m_msg;
    m_msg = 0;
  }
  return StatusCode::SUCCESS;
}


//=============================================================================
// Function to determine which FT channels are traversed by a trajectory
// determined by globalPointEntry and globalPointExit. Fills a vector of
// pairs <FTChannelID, energyFraction>. The latter is the fraction of the energy
// of the MC particle deposited in the corresponding SiPM cell. A light sharing
// model is used in the determination of the energyFractions.
//=============================================================================
StatusCode DeFTFibreMat::calculateListOfFiredChannels(const LHCb::MCHit  *fthit,
                                                      VectFTPairs&         vectChanAndEnergy) const
{

  /// Create a vector of FT pairs which will hold <FTChannel, FractionalPosition>
  /// This is the 'working' quantity. In the final step of the calculateHits method
  /// we use it to create the final vectChanAndEnergy
  VectFTPairs vectChanAndFracPos;

  StatusCode sc = this->calculateHits(fthit, vectChanAndFracPos);
  if(sc.isFailure()) return StatusCode::FAILURE;

  /// Call the light-sharing method using vectChanAndFracPos,
  /// create the final vector of pairs <FTChannel, EnergyFractions>
  vectChanAndEnergy = (this->createLightSharingChannels( vectChanAndFracPos ));

  // Finally, to return the energy deposited in each channel by the hit, each energy fraction
  // is multiplied by the energy
  for (VectFTPairs::iterator itPair = vectChanAndEnergy.begin(); itPair != vectChanAndEnergy.end(); ++itPair) {
    itPair->second *= fthit->energy();
  }

  return StatusCode::SUCCESS;
}


//=============================================================================
// Function to determine the mean SiPM channel (ChannelID + fraction) associated
// to the track between entry and exit point.
// @param fthit : hit from Geant4
// @param ChanAndFrac : std::pair (ChannelID + fraction) associated to the mean channel
//=============================================================================
StatusCode DeFTFibreMat::calculateMeanChannel(const LHCb::MCHit  *fthit,
                                              FTPair&             ChanAndFrac) const
{
  /// Create a vector of FT pairs which will hold <FTChannel, FractionalPosition>
  /// This is the 'working' quantity. In the final step of the calculateHits method
  /// we use it to create the final vectChanAndEnergy
  VectFTPairs vectChanAndFracPos;
  
  
  StatusCode sc = this->calculateHits(fthit, vectChanAndFracPos);
  if(sc.isFailure()) return StatusCode::FAILURE;
  
  //Compute mean channel ID + fractional part associated to the mean fired channel of the hit
  VectFTPairs::iterator vectChanAndFracPosEdgeStart = vectChanAndFracPos.end();
  VectFTPairs::iterator vectChanAndFracPosEdgeEnd = vectChanAndFracPos.begin();
  
  for (VectFTPairs::iterator itPair = vectChanAndFracPos.begin(); itPair != vectChanAndFracPos.end(); ++itPair) {
    if( m_msg->level() <= MSG::DEBUG) debug() << "==calculateMeanChannel==  channelID="<<itPair->first <<" + " << itPair->second
                                      << "\t diff="<< itPair->first - (vectChanAndFracPos.begin())->first
                                      << "\t End-Start="<<vectChanAndFracPosEdgeEnd->first - vectChanAndFracPosEdgeStart->first 
                                      << "\t Start-End="<< vectChanAndFracPosEdgeStart->first - vectChanAndFracPosEdgeEnd->first  
                                      << "\t size="<< vectChanAndFracPos.size()
                                      <<endmsg;  
    if((itPair->first.layer()<  12) && (vectChanAndFracPosEdgeStart == vectChanAndFracPos.end())) 
      vectChanAndFracPosEdgeStart = itPair;
    
    if(itPair->first.layer() < 12) vectChanAndFracPosEdgeEnd = itPair;
    
  }
  
  // Define the output of the function : list of channels and fractional parts
  FTChannelID meanChannel = std::min(vectChanAndFracPosEdgeStart->first, vectChanAndFracPosEdgeEnd->first);
  
  double fractional = (vectChanAndFracPos.size() + (vectChanAndFracPos.begin())->second
                       +(vectChanAndFracPos.end()-1)->second - 1)/2;
  if( m_msg->level() <= MSG::DEBUG) debug() <<"==calculateMeanChannel==  vectChanAndFracPosEdgeStart->first="<<vectChanAndFracPosEdgeStart->first
                                    << "\t vectChanAndFracPosEdgeEnd->first=" << vectChanAndFracPosEdgeEnd->first
                                    << "\t Min Both diff = " 
                                    << std::min(vectChanAndFracPosEdgeStart->first-vectChanAndFracPosEdgeEnd->first,
                                                vectChanAndFracPosEdgeEnd->first-vectChanAndFracPosEdgeStart->first)
                                    << std::endl;
  
  if((vectChanAndFracPosEdgeStart != vectChanAndFracPos.begin())||(vectChanAndFracPosEdgeEnd !=(vectChanAndFracPos.end()-1)))
  { 
    fractional = fractional - vectChanAndFracPos.size() + 1 + 
      std::min(vectChanAndFracPosEdgeStart->first-vectChanAndFracPosEdgeEnd->first,
               vectChanAndFracPosEdgeEnd->first-vectChanAndFracPosEdgeStart->first);
  }
  
  
  if( m_msg->level() <= MSG::DEBUG) debug() << "==calculateMeanChannel==  Starting channelID="<<meanChannel 
                                    << "\t fractional=" << fractional
                                    <<endmsg;
  while(fractional > 0.5)
  {
    meanChannel = meanChannel+ 1;
    fractional -=1;
  }
  
  if( m_msg->level() <= MSG::DEBUG) debug() << "==calculateMeanChannel==  Final channelID="<<meanChannel 
                                    << "\t fractional=" << fractional
                                    <<endmsg;
  
  ChanAndFrac = std::make_pair(meanChannel,fractional);
  
  return StatusCode::SUCCESS;
}


//=============================================================================
// Function to determine the mean SiPM channel (ChannelID + fraction) associated
// to the track between entry and exit point.
// @param fthit : hit from Geant4
// @param ChanAndFrac : std::pair (ChannelID + fraction) associated to the mean channel
//=============================================================================
StatusCode DeFTFibreMat::calculateHits(const LHCb::MCHit  *fthit,
                                       VectFTPairs&        vectChanAndFracPos) const
{
  if( m_relativemodule == 99 ){
    if( m_msg->level() <= MSG::DEBUG) debug() << "FibreMat not found" << endmsg;
    return StatusCode::FAILURE;
  }
    
  // get hit position in global and local coordinate systems and perform checks.
  Gaudi::XYZPoint enPLocal  = this -> geometry() -> toLocal( fthit -> entry() );
  Gaudi::XYZPoint exPLocal  = this -> geometry() -> toLocal( fthit -> exit()  );
  Gaudi::XYZPoint enPGlobal = fthit -> entry() ;
  Gaudi::XYZPoint exPGlobal = fthit -> exit()  ;

  //temporary workaround for Right Hole (module 11, v4 geometry) axes inversion
  if(m_RightHoleAxesXZInversion) {
    doRHAxesInversion(enPLocal);
    doRHAxesInversion(exPLocal);
  }
  
  //test if entry and exit are in the same fibremat
  double pLocalSepX = exPLocal.X() - enPLocal.X(); double pGlobalSepX =  exPGlobal.X() - enPGlobal.X();
  double pLocalSepY = exPLocal.Y() - enPLocal.Y(); double pGlobalSepY =  exPGlobal.Y() - enPGlobal.Y();
  double pLocalSepZ = exPLocal.Z() - enPLocal.Z(); double pGlobalSepZ =  exPGlobal.Z() - enPGlobal.Z();
  double pLocalSep  = sqrt( pLocalSepX*pLocalSepX   + pLocalSepY*pLocalSepY   +  pLocalSepZ*pLocalSepZ );
  double pGlobalSep = sqrt( pGlobalSepX*pGlobalSepX + pGlobalSepY*pGlobalSepY + pGlobalSepZ*pGlobalSepZ );
  if( m_msg->level() <= MSG::DEBUG){
    debug() << "Entry Point in Global / Local: " << enPGlobal << enPLocal << endmsg;
    debug() << "Exit  Point in Global / Local: " << exPGlobal << exPLocal << endmsg;
  }
  if( std::abs( pLocalSep - pGlobalSep ) > 1.e-2 ){
    if( m_msg->level() <= MSG::DEBUG) debug() << "Aborting calculateHits: local and global entry and exit point distances "
       << "differ more than 10 microns. Likely entry and exit points in different fibremats" << endmsg;
    return StatusCode::FAILURE;
  }


  //test if entry and exit are not too close in z
  if ( std::abs( pLocalSepZ ) < 1.e-2 ) {
    if( m_msg->level() <= MSG::DEBUG) debug() << "Aborting calculateHits because z-distance "
       << "between entry and exit points is less than 10 microns." << endmsg;
    return StatusCode::FAILURE;
  }
  
  //test if entry and exit are inside fibremat global acceptance
  if( std::abs( enPLocal.X() ) > m_fibreMatHalfSizeX ||
      std::abs( enPLocal.Y() ) > m_fibreMatHalfSizeY || 
      std::abs( exPLocal.X() ) > m_fibreMatHalfSizeX ||
      std::abs( exPLocal.Y() ) > m_fibreMatHalfSizeY
      ){
    if( m_msg->level() <= MSG::DEBUG) debug() << "Aborting calculateHits: entry or exit points out of fibremat sensitive region" << endmsg;
    return StatusCode::FAILURE;
  }
  
  //hole treatment and fibrelength
  double fibrelengthMax(2*m_fibreMatHalfSizeY);
  if( m_holey ){
    if(inBeamHole(enPLocal, fibrelengthMax) || inBeamHole(exPLocal, fibrelengthMax)) {
      if( m_msg->level() <= MSG::DEBUG) debug() << "Aborting calculateHits: entry or exit points are inside "
          << "the beam pipe hole" << endmsg;
      return StatusCode::FAILURE;
    }
  }

  ///information of the hit
  unsigned int hitLayer = this->layer();
  if( m_msg->level() <= MSG::DEBUG) debug() << "LayerID = "<< hitLayer
                                    << ", Module = "       << m_module
                                    << ", Isbottom = "     << m_mat
                                    << ", quarter = "      << m_quarter
                                    << ", Stereo angle = " << this->angle() << endmsg;
  


  ///////////////////////////////////////////////////////
  /// Get cell coordinates of the entry and exit points
  ///////////////////////////////////////////////////////

  if( m_msg->level() <= MSG::DEBUG) debug() << "Local Coordinates of enP and exP: "
                                            << enPLocal.X() << ", " << exPLocal.X() << endmsg;

  unsigned int enPSipmID, enPCellID;
  unsigned int exPSipmID, exPCellID;
  double enPFraction, exPFraction;
  FTChannelID channel;
  
  cellIDCoordinates( enPLocal.X(), m_quarter, enPSipmID, enPCellID, enPFraction );
  cellIDCoordinates( exPLocal.X(), m_quarter, exPSipmID, exPCellID, exPFraction );

  if ( (enPSipmID > m_nSipmPerModule) || (exPSipmID > m_nSipmPerModule) ) {
    if( m_msg->level() <= MSG::DEBUG) debug() << "Aborting calculateHits(...) because entry "
                                      << "or exit points are outside acceptance (we get too large sipmID)" << endmsg;
    return StatusCode::FAILURE;
  }
  

  ///////////////////////////////////////////////////////
  /// Create pairs of FT channels and fractional position
  ///////////////////////////////////////////////////////

  /// Case where entry and exit points are in the same cell
  if ( (enPSipmID==exPSipmID) && (enPCellID==exPCellID) ) {
    if( m_msg->level() <= MSG::DEBUG) debug() << "Entry and Exit points are in the same cell!" << endmsg;
    
    double frac = (enPFraction + exPFraction)/2;  
    if( m_msg->level() <= MSG::DEBUG) debug() << "Average fract dist to cell center: " << frac << endmsg;

    /// Create and push-back the pair (FTChannelID, fraction)
    channel = createChannel( hitLayer, m_module, m_mat, enPSipmID, enPCellID );
    vectChanAndFracPos.push_back( std::make_pair(channel, frac) );

  } //end single cell

  /// Case where entry and exit points are in different cells
  else {
    
    if( m_msg->level() <= MSG::DEBUG) debug() << "Entry and Exit points are in different cells!" << endmsg;
    /// The procedure is to determine the crossing point between the particle
    /// trajectory and the planes separating the adjacent cells. First we determine
    /// the u-coordinate of the cell border between the first and second cells
    /// and between the last and last-but-once cells. Then we determine the crossing
    /// points. The hits in the middle cells will have fractional position equal
    /// to 0 (i.e. at the cell center).

    /// local X coordinate decreases or increases with z?
    double dUExEn = exPLocal.X() - enPLocal.X();
    int uDir = (dUExEn>0) ? 1 : -1;

    /// Cell edge next to the entry point (can be on the left or on the right)
    double enPCellSeparEdge, enPDistToSeparEdge;
    enPCellSeparEdge = enPLocal.X() - (enPFraction - 0.5*uDir) * m_cellSizeX;
    enPDistToSeparEdge = enPLocal.X() - enPCellSeparEdge;

    if( m_msg->level() <= MSG::DEBUG) debug() << "Entry point: uDir / cellSeparEdge / distToSeparEdge: "
                                      << uDir << ", " << enPCellSeparEdge << ", " << enPDistToSeparEdge << endmsg;
    
    /// Cell edge next to the exit point
    double exPCellSeparEdge, exPDistToSeparEdge;
    exPCellSeparEdge = exPLocal.X() - (exPFraction + 0.5*uDir) * m_cellSizeX;
    exPDistToSeparEdge = exPLocal.X() - exPCellSeparEdge;

    if( m_msg->level() <= MSG::DEBUG) debug() << "Exit point: uDir / cellSeparEdge / distToSeparEdge: "
                                      << uDir << ", " << exPCellSeparEdge << ", " << exPDistToSeparEdge << endmsg;
    
    //////////////////////////////////////////////////////////////////
    /// Calculate the fractional position of the particle path center inside
    /// each traversed cell. Create and push_back the relevant FT pairs
    //////////////////////////////////////////////////////////////////
    StatusCode sc;
    Gaudi::XYZPoint pIntersect(0.,0.,0.);
    double fracP1, fracP2;

    /// The cell of the entry point
    double fracPosFirstCell = 999.;
    sc = cellCrossingPoint( enPCellSeparEdge, fthit->entry(), fthit->exit(), pIntersect );
    // in case of a problem --> directly exit the function
    if ( sc.isFailure() ) {
      if( m_msg->level() <= MSG::DEBUG) debug() << "Aborting calculateHits(...), because of unsuccessful "
                                        << "call to cellCrossingPoint(EntryPoint)" << endmsg;
      return sc;
    }
    
    fracP1 = enPFraction;
    fracP2 = (dUExEn>0) ? 0.5 : -0.5; 
    fracPosFirstCell = (fracP1 + fracP2)/2;
    if( m_msg->level() <= MSG::DEBUG){
      debug() << "Entry Point, SC = " << sc << " Intersection point: " << pIntersect << endmsg;
      debug() << "Entry Point, Frac = " << fracPosFirstCell << endmsg;
    }

    /// Create and push-back the pair (FTChannelID, fraction)
    channel = createChannel( hitLayer, m_module, m_mat, enPSipmID, enPCellID );
    vectChanAndFracPos.push_back( std::make_pair(channel, fracPosFirstCell) );
    
    /// The middle cells
    double eps = 1.e-3; /// add this distance when determining the number of middle cells
    int nMiddleCells( (std::abs(exPCellSeparEdge - enPCellSeparEdge) + eps) / m_cellSizeX );
    unsigned int midCellSipmID, midCellID;
    double midCellFraction;
    int i;
        
    for (i=0; i<nMiddleCells; ++i) {
      double midCellCenterU = enPCellSeparEdge + uDir*m_cellSizeX*(i+0.5);
      if( m_msg->level() <= MSG::DEBUG) debug() << "\tMid Cell " << i << " midCellCenterU = " << midCellCenterU << endmsg;
      cellIDCoordinates( midCellCenterU, m_quarter, midCellSipmID, midCellID, midCellFraction );

      /// Create and push-back the pair (FTChannelID, fraction)
      channel = createChannel( hitLayer, m_module, m_mat, midCellSipmID, midCellID );
      vectChanAndFracPos.push_back( std::make_pair(channel, midCellFraction) );
    }// end loop over mid cells
    
    /// The cell of the exit point
    double fracPosLastCell = 999.;
    sc = cellCrossingPoint( exPCellSeparEdge, fthit->entry() , fthit->exit(), pIntersect );
    // in case of a problem --> directly exit the function
    if ( sc.isFailure() ) {
      if( m_msg->level() <= MSG::DEBUG) debug() << "Aborting calculateHits(...), because of unsuccessful "
                                        << "call to cellCrossingPoint(ExitPoint)" << endmsg;
      return sc;
    }

    fracP1 = exPFraction;
    fracP2 = (dUExEn>0) ? -0.5 : 0.5;
    fracPosLastCell = (fracP1 + fracP2)/2;
    
    if( m_msg->level() <= MSG::DEBUG){
      debug() << "Exit Point, SC = " << sc << " Intersection point: " << pIntersect << endmsg;
      debug() << "Exit Point, Frac = " << fracPosLastCell << endmsg;
    }
    
    /// Create and push-back the pair (FTChannelID, fraction)
    channel = createChannel( hitLayer, m_module, m_mat, exPSipmID, exPCellID );
    vectChanAndFracPos.push_back( std::make_pair(channel, fracPosLastCell) );
  }//end more than 1 hit cells
  
  
  if( m_msg->level() <= MSG::DEBUG) debug() << "Finished creating FTPairs\n" << endmsg;
  
  /// Printout the vector of FT pairs
  if( m_msg->level() <= MSG::DEBUG) debug() << "Size of vector of FT pairs: " << vectChanAndFracPos.size() << endmsg;
  VectFTPairs::const_iterator itPair;
  DetectorSegment tmpDetSeg;
  for (itPair = vectChanAndFracPos.begin(); itPair != vectChanAndFracPos.end(); ++itPair) {
    if( m_msg->level() <= MSG::DEBUG) debug() << itPair->first << ", FractPos: " << itPair->second << endmsg;
    /// Test the inverse function ///
    if( m_msg->level() <= MSG::DEBUG) debug() << "Test of function cellUCoordinate(FTChannelID): "
            << cellUCoordinate(itPair->first) << endmsg;
    /// Test of functions nextChannelLeft/Right
    if( m_msg->level() <= MSG::DEBUG) debug() << "Test of function nextChannelLeft:"
                                      << this->nextChannelLeft( itPair->first ) << endmsg;
    if( m_msg->level() <= MSG::DEBUG) debug() << "Test of function nextChannelRight:"
                                      << this->nextChannelRight( itPair->first ) << endmsg;
    /// Test the function FTChannel+Fraction --> DetSegment
    if( m_msg->level() <= MSG::DEBUG) debug() << "Test of function createDetSegment(FTChannelID, fracPos)" << endmsg;
    tmpDetSeg = createDetSegment( itPair->first, itPair->second );
  }

  return StatusCode::SUCCESS;
}


//=============================================================================
// This function determines the fibre lengh and relative position of the hit 
// in the fibre according to the Hit position.
// These returned value can be used to determine the attenuation coefficient to be applied
// on the deposited energy to take into acocunt the light attenuation 
// through the fibre.
// !!!Works only if entry and exit in the same FibreMat!!!
//=============================================================================
StatusCode DeFTFibreMat::hitPositionInFibre(const LHCb::MCHit  *fthit,
                                            double& meanfibreMaxLength,
                                            double& fibreLenghFrac) const
{
  Gaudi::XYZPoint enPLocal = this->geometry()->toLocal( fthit->entry() );
  Gaudi::XYZPoint exPLocal = this->geometry()->toLocal( fthit->exit() );

  //temporary workaround for Right Hole (module 11, v4 geometry) axes inversion
  if(m_RightHoleAxesXZInversion) {
    doRHAxesInversion(enPLocal);
    doRHAxesInversion(exPLocal);
  }
  
  double enfibrefullLength,exfibrefullLength;
  inBeamHole(enPLocal, enfibrefullLength);
  inBeamHole(exPLocal, exfibrefullLength);
  meanfibreMaxLength=(enfibrefullLength+exfibrefullLength)/2.;
  
  double meanY= (enPLocal.y()+exPLocal.y())/2;
  // Fractional lengh is relative to the SiPM position which is on the top
  // for top fibremats & on the bottom for bottom fibremats
  fibreLenghFrac = std::abs(meanY - m_fibreMatSipmY)/meanfibreMaxLength;

  return StatusCode::SUCCESS;
}


//=============================================================================
// Function dealing with the light sharing between neighbouring SiPM cells
//=============================================================================
VectFTPairs DeFTFibreMat::createLightSharingChannels(VectFTPairs& inputVectPairs) const
{
  VectFTPairs::const_iterator itPair;
  // First determine the weighting of the energy deposits by taking into account
  // the particle path length in the cells
  double sumPathWeights = 0;
  for (itPair = inputVectPairs.begin(); itPair != inputVectPairs.end(); ++itPair) {
    sumPathWeights += (1 - 2.*std::abs(itPair->second));
  }
  
  std::vector<double> vectPathWeights;
  if(std::abs(sumPathWeights)>1.e-6) {
    for (itPair = inputVectPairs.begin(); itPair != inputVectPairs.end(); ++itPair) {
      vectPathWeights.push_back( (1 - 2.*std::abs(itPair->second)) / sumPathWeights );
    }
  }
  else {
    for (itPair = inputVectPairs.begin(); itPair != inputVectPairs.end(); ++itPair) {
      vectPathWeights.push_back( 0.);
    }
  }
  if( m_msg->level() <= MSG::DEBUG) debug() << "Path weights: " << vectPathWeights << endmsg;


  std::vector<LHCb::FTChannelID> vectChannels;
  // append the channel before the first one with a direct deposit from the MC particle
  // the sequence of channels in the vector can be both right -> left or the opposite
  int sequenceRightToLeft = ( inputVectPairs.front().first.sipmCell() < inputVectPairs.back().first.sipmCell() );

  if ( inputVectPairs.size() > 1 && sequenceRightToLeft ) {
    vectChannels.push_back( this->nextChannelRight(inputVectPairs.front().first) );
  }
  else { vectChannels.push_back( this->nextChannelLeft(inputVectPairs.front().first) ); }

  std::vector<double> lcrFractions; // vector holding the left/center/right fractions
  std::vector< std::vector<double> > vectTriplets; // vector of the lcrFractions

  for (itPair = inputVectPairs.begin(); itPair != inputVectPairs.end(); ++itPair) {
    // append the FT channel
    vectChannels.push_back( itPair->first );
    // calculate the left/center/right sharing of each channel
    this->lightSharing(itPair->second, lcrFractions);
    if( m_msg->level() <= MSG::DEBUG) debug() << "FractPos: " << itPair->second << " ; Sharing: " << lcrFractions << endmsg;
    vectTriplets.push_back( lcrFractions );
  }

  // append the channel after the last one with a direct deposit from the MC particle
  if ( inputVectPairs.size() > 1 && sequenceRightToLeft ) {
    vectChannels.push_back( this->nextChannelLeft(inputVectPairs.back().first) );
  }
  else { vectChannels.push_back( this->nextChannelRight(inputVectPairs.back().first) ); }  
  // calculate the overall fractions for each channel
  std::vector<double> finalFractions( inputVectPairs.size()+2, 0. );

  unsigned int ind;
  for (ind=0; ind<vectTriplets.size(); ++ind) {
    // here we also apply the particle path-length weighting
    if ( inputVectPairs.size() > 1 && sequenceRightToLeft ) {
      finalFractions.at(ind)   += vectTriplets[ind][2] * vectPathWeights[ind];
      finalFractions.at(ind+1) += vectTriplets[ind][1] * vectPathWeights[ind];
      finalFractions.at(ind+2) += vectTriplets[ind][0] * vectPathWeights[ind];
    }
    else {
      finalFractions.at(ind)   += vectTriplets[ind][0] * vectPathWeights[ind];
      finalFractions.at(ind+1) += vectTriplets[ind][1] * vectPathWeights[ind];
      finalFractions.at(ind+2) += vectTriplets[ind][2] * vectPathWeights[ind];
    }
  }

  if( m_msg->level() <= MSG::DEBUG){
    debug() << "Final Fractions: " << finalFractions << endmsg;
    debug() << "Final Channels: "  << vectChannels << endmsg;
    
    debug() << "Size of vector of triplets / channels / finalFracts: " << vectTriplets.size()
            << " / " << vectChannels.size() << " / " << finalFractions.size() << endmsg;  
  }
  
  // Create the final vector of FTPairs
  // The second part of the pair is the MCP energy fraction for this channel

  VectFTPairs outputVectPairs;
  for (ind=0; ind<vectChannels.size(); ++ind) {
    outputVectPairs.push_back( std::make_pair(vectChannels.at(ind), finalFractions.at(ind)) ); 
  }
  if( m_msg->level() <= MSG::DEBUG) debug() << "Created vector of light-sharing FT pairs with size: " << outputVectPairs.size() << endmsg;

  return outputVectPairs;
}

//=============================================================================
// A linear light-sharing model. From the fractional position inside a cell
// determine the energy fractions deposited in the left/central/right SiPM cells
//=============================================================================
void DeFTFibreMat::lightSharing( double position, std::vector<double>& fractions ) const {
  fractions.clear();
  double frLeft, frCenter, frRight;
  if (position > 0) {
    frLeft  =  0.68 * position + 0.16;
    frRight = -0.32 * position + 0.16;
  }
  else {
    frLeft  =  0.32 * position + 0.16;
    frRight = -0.68 * position + 0.16;
  }
  frCenter = -0.36 * std::abs(position) + 0.68;
  fractions.push_back( frLeft );
  fractions.push_back( frCenter );
  fractions.push_back( frRight );
}


//=============================================================================
// Function encapsulating the creation of FTChannelIDs
//=============================================================================
FTChannelID DeFTFibreMat::createChannel(unsigned int hitLayer,
                                        int          module,
                                        int          mat,
                                        unsigned int sipmID,
                                        unsigned int grossCellID) const
{
  FTChannelID channel;
  // Convert the grossCellID to netCellID
  unsigned int netCellID = this->netCellID(grossCellID);

  /// Create and push_back the corresponding FT pair
  if ( netCellID > (m_sipmNChannels-1) ) {
    if( m_msg->level() <= MSG::DEBUG) debug() << "Gross cellID " << grossCellID << " corresponds to insensitive cell."
                       << " Creating invalid FT channel (the signature is: layer=" << m_BadChannelLayerFlag << ")."
                       << endmsg;
    channel = FTChannelID( m_BadChannelLayerFlag, 0, 0, 0, 0 );
  }
  else {
    channel = FTChannelID( hitLayer, module, mat, sipmID, netCellID );
  }
  return channel;
}


//========================================================================-===========
// gross cell id (count inactive cells) to net cell id (discard them in the numbering)
//====================================================================================
unsigned int DeFTFibreMat::netCellID(const unsigned int grossID) const {
  unsigned int netID=999; // any number above m_sipmNChannels
  if      ( grossID >  0 && grossID <  65 ) netID = grossID-1;
  else if ( grossID > 65 && grossID < 130 ) netID = grossID-2;
  return netID;
}

//========================================================================
// net cell id (inactive cells not counted) to gross cell id (count them)
//========================================================================
unsigned int DeFTFibreMat::grossCellID(const unsigned int netID) const {
  unsigned int grossID=999; // any number above m_sipmNChannels
  if      ( netID <  64 ) grossID = netID+1;
  else if ( netID < 128 ) grossID = netID+2;
  return grossID;
}



//=============================================================================
// Function to determine the local U coordinate at the center of an FT cell
// from a FTChannelID. In case of non-valid channel: return a large value.
// This is the reciprocal function to 'cellIDCoordinates(lCoord, quarter, ...)'.
// Make sure that the changes in one of these two functions are reflected in
// the conjugate function as well.
// Warning: works only for current fibremat
//=============================================================================
double DeFTFibreMat::cellUCoordinate(const FTChannelID& channel) const {
  double uCoord;

  // check if it is a valid channel or one that corresponds to non-sensitive cell
  if ( channel.layer() == m_BadChannelLayerFlag ) {
    if( m_msg->level() <= MSG::DEBUG) debug() << "Function cellUCoordinate: cannot determine uCoord for "
                                      << " non-valid channel " << channel << endmsg;
    uCoord = 99999.;
  }
  else {
    double qLR = (m_quarter%2) ? 1 : -1;   //right or left quarter
    double moduleSizeX = 2*m_FibreModuleHalfSizeX;
    int sipm = channel.sipmId();
  
    //right or left sipm edge (closest to global origin)
    double sipmREdge = qLR*(m_relativemodule*moduleSizeX) + 
                       qLR*(m_moduleEdgeSizeX+(sipm + !(m_quarter%2))*m_sipmPitchX);

    // Determine gross cellID
    unsigned int grossID = grossCellID(channel.sipmCell());
    // offset of the cell center wrt SiPM right edge (cellID always increases from right to left)
    double cellOffset = (grossID + 0.5) * m_cellSizeX;
  
    // correction in the case that the SiPM edge is not exactly 1 cell width
    double corr = -m_cellSizeX+m_sipmEdgeSizeX;
    cellOffset += corr;
    
    uCoord = sipmREdge+cellOffset;
  }
  return uCoord;
}



//=============================================================================
//Find local fibremat frame cell coordinate from FTChannelID
//=============================================================================
double DeFTFibreMat::cellLocalX(const FTChannelID& channel) const {
  double lCoordX;

  // check if it is a valid channel or one that corresponds to non-sensitive cell
  if ( channel.layer() == m_BadChannelLayerFlag ) {
    if( m_msg->level() <= MSG::DEBUG) debug() << "Function cellLocalCoordinate: cannot determine local Coord for "
                                              << " non-valid channel " << channel << endmsg;
    lCoordX = 99999.;
  }
  else {
    int sipm = channel.sipmId();
    double qLR = (m_quarter%2) ? 1 : -1;   //right or left quarter
  
    //right or left sipm edge (closest to global origin)
    double sipmREdgeX = qLR*(-m_fibreMatHalfSizeX + (sipm + !(m_quarter%2))*m_sipmPitchX);

    // Determine gross cellID
    unsigned int grossID = grossCellID(channel.sipmCell());
    // offset of the cell center wrt SiPM right edge (cellID always increases from right to left)
    // correction for the SiPM edge counted as 1 cell width
    double cellOffset = (grossID + 0.5 - 1) * m_cellSizeX + m_sipmEdgeSizeX;
      
    lCoordX = sipmREdgeX+cellOffset;
  }
  return lCoordX;
}





//=============================================================================
// Function to determine the sipmID, gross cellID (counts also insensitive cells
// and SiPM edge gaps) and fractional position inside a cell. Input params are
// the hit x fibremat local coordinate and quarterID.
//=============================================================================
void DeFTFibreMat::cellIDCoordinates( const double  Xlocal,
                                      unsigned int  quarter,
                                      unsigned int& sipmID,
                                      unsigned int& cellID,
                                      double&       fracDistCellCenter ) const
{
  /// Get sipmID and local position of its right edge (don't use simple division: edge effects !)
  unsigned int lsipm = 0;
  for( unsigned int i = 0; i < m_nSipmPerModule; ++i ){
    if( std::abs(Xlocal - m_sipmOriginX) > (double)i*m_sipmPitchX ) {
      lsipm = i;
    }
    else break;								  
  }
  if( lsipm > m_BadChannelLayerFlag ) {
    error() << "In function cellIDCoordinates: SipmID must be between 0 and " << m_BadChannelLayerFlag << endmsg;
    error() << " local X: " << Xlocal  << " local sipm: " << lsipm << endmsg;
  }
  else {
    sipmID = lsipm;
  }    
  
  //hit sipm 'right' edge in local frame (ie looking in beam direction)
  double sipmREdgeX = m_sipmOriginX + (double)(lsipm + !(quarter%2))*m_sipmPitchXsigned;
  
  if( m_msg->level() <= MSG::DEBUG) debug() << "quarter, sipmID, sipmREdgeX = "
                                    << quarter << ", "<< sipmID << ", " << sipmREdgeX << endmsg;
  
  /// Get cellID inside the SiPM
  double distSipmREdge = Xlocal - sipmREdgeX;
  if ( distSipmREdge < 0 ) {
    error() << "In function cellIDCoordinates: got negative distance between "
            << " the hit and the sipmEdge. Must be non-negative for points outside dead regions" << endmsg;
    error() << Xlocal << " " << sipmREdgeX << endmsg;
  }


  //cell edge, extreme (dead) channels 0 and 130, fractionnal distance
  //wrto cell center (same axis orientation as mats)
  //get cellID, sensitive cells have ids: 1-64, 66-129 incl.
  //cellID 65 is the inner SiPM hole!
  double cellREdgeX;
  if ( distSipmREdge < m_sipmEdgeSizeX ) {
    cellID = 0;
    cellREdgeX = sipmREdgeX;
    fracDistCellCenter = -0.5;
  }
  else if ( distSipmREdge > m_sipmEdgeSizeX + m_sipmSizeX ) {
    cellID = 130;
    cellREdgeX = sipmREdgeX + m_sipmEdgeSizeX + m_sipmSizeX;
    fracDistCellCenter = 0.5;
  }
  else {
    double distActiveArea = Xlocal - (sipmREdgeX + m_sipmEdgeSizeX);
    if ( distActiveArea < 0 ) {
      error() << "In function cellIDCoordinates: got negative distance between "
              << " the hit and the first sensitive cell. Must be non-negative!)" << endmsg;
    }
    cellID = (unsigned int) (1 + distActiveArea/m_cellSizeX);     // >=1 by construction
    cellREdgeX = (sipmREdgeX + m_sipmEdgeSizeX) + (cellID-1)*m_cellSizeX;
    fracDistCellCenter = (Xlocal - (cellREdgeX + m_cellSizeX/2)) / m_cellSizeX;
  }
  
  if( m_msg->level() <= MSG::DEBUG) debug() << "\n\tdistSipmREdge: " << distSipmREdge 
                                    << "\n\tGross cellID: " << cellID
                                    << "\n\tcellREdgeX: " << cellREdgeX
                                    << ", distToCellREdge = " << Xlocal - cellREdgeX
                                    << ", distToCellCenter = " << Xlocal - (cellREdgeX + m_cellSizeX/2)
                                    << "\n\tfracDistCellCenter: " << fracDistCellCenter << endmsg;
}



//=============================================================================
// Function to determine the XYZ crossing point of a line (determined by the
// points gpEntry and gpExit) and the vertical (or tilted in the case of u/v layers)
// plane between two cells.
//=============================================================================
StatusCode DeFTFibreMat::cellCrossingPoint(const double cellEdgeU,
                                           const Gaudi::XYZPoint& gpEntry,
                                           const Gaudi::XYZPoint& gpExit,
                                           Gaudi::XYZPoint& pIntersect) const
{
  /// Create the plane of the border between two cells
  /// set small, but signed yInit for the calculation of the cell x at the top/bottom of the layer
  double yInit = (gpEntry.y()>0) ? 1.e-6 : -1.e-6;
  double cellYAtVertBorder = (gpEntry.y()>0) ? m_layerMaxY : m_layerMinY;
  double cellXAtVertBorder = xAtVerticalBorder( cellEdgeU, yInit );

  double cellZMin = gpEntry.z();
  Gaudi::XYZPoint gp1(cellEdgeU, 0., cellZMin);
  Gaudi::XYZPoint gp2(cellEdgeU, 0., cellZMin+2*m_fibreMatHalfSizeZ);
  Gaudi::XYZPoint gp3(cellXAtVertBorder, cellYAtVertBorder, cellZMin);
  if( m_msg->level() <= MSG::DEBUG) debug() << "P1, P2, P3:\n\t" << gp1 << "\n\t"
                                    << gp2 << "\n\t" << gp3 << endmsg;
  
  /// Create the plane, the particle trajectory and their intersection
  Gaudi::Plane3D cellBorderPlane(gp1, gp2, gp3);
  if( m_msg->level() <= MSG::DEBUG) debug() << "CELL BORDER PLANE:" << cellBorderPlane << endmsg;
  
  Line partTraj(gpEntry, (gpExit - gpEntry).Unit());
  double paramIntersect;
  if( m_msg->level() <= MSG::DEBUG) debug() << "PART TRAJ:" << partTraj << endmsg;

  bool sc = Gaudi::Math::intersection( partTraj, cellBorderPlane, pIntersect, paramIntersect );

  if( m_msg->level() <= MSG::DEBUG){
    debug() << "Intersection P, param, line(param):"
            << sc << " : " << pIntersect << ", " << paramIntersect << endmsg;
    
    debug() << "Position(p=0), Position(p=xx): " << partTraj.position(0.)
            << partTraj(paramIntersect) << endmsg;
  }
  
    
  return StatusCode(sc);
}

//=============================================================================
// Function to create a DetectorSegment (straight line representing an FT channel)
// from a FTChannelID
// !!!to be revisited, not sure if completely coherent frames!!!
// !!! and no test of channel validity!!!
//=============================================================================
DetectorSegment DeFTFibreMat::createDetSegment(const FTChannelID& channel,
                                               double fracPos) const {

  /// Determine the x coordinate at y=0 of the det. segment
  double cellCenterX = cellUCoordinate( channel );
  double hitU = cellCenterX + fracPos*m_cellSizeX;
  double hitX0 = hitU/m_cosAngle;

  //local fibremat frame x coordinate
  double hitLocalX = cellLocalX(channel) + fracPos*m_cellSizeX;
  
  /// Determine the upper and lower boundaries of the det. segment (active fibre part)
  /// For speed, we do not take into account that below/above the beam pipe
  /// the fibres are shorter
  Gaudi::XYZPoint hitGlobalBottom = this->geometry()->toGlobal( Gaudi::XYZPoint(hitLocalX,-m_fibreMatHalfSizeY,0.) );
  Gaudi::XYZPoint hitGlobalTop = this->geometry()->toGlobal( Gaudi::XYZPoint(hitLocalX,m_fibreMatHalfSizeY,0.) );
  double hitGlobalMinY = hitGlobalBottom.Y();
  double hitGlobalMaxY = hitGlobalTop.Y();

  double hitZ0;
  if ( !m_mat ) {    //top
    hitZ0 = m_fibreMatGlobalCenter.Z() - m_cosAngle*m_layerHalfSizeY*m_dzDy/sqrt(1.+m_dzDy*m_dzDy);
  }
  else{     //bottom
    hitZ0 = m_fibreMatGlobalCenter.Z() + m_cosAngle*m_layerHalfSizeY*m_dzDy/sqrt(1.+m_dzDy*m_dzDy);
  }
  
  if( m_msg->level() <= MSG::DEBUG) debug() << "layer: " << channel.layer() << " quarter:" << m_quarter << " Z:"
                                            << m_fibreMatGlobalCenter.Z() << " Z0:" << hitZ0 
                                            << " Ymin,max:" << hitGlobalMinY << " " << hitGlobalMaxY << endmsg;
  
  return DetectorSegment( hitX0, hitZ0, -m_tanAngle, m_dzDy, hitGlobalMinY, hitGlobalMaxY );   
}


//=============================================================================
// Function to determine the x coordinate at the top/bottom of the layer
// by extrapolating the initial (x0,y0) along the fibres
//=============================================================================
double DeFTFibreMat::xAtVerticalBorder(double x0, double y0) const {
  if (std::abs(m_angle)<1.e-4) return x0;
  else {
    double yAtBorder = (y0>0) ? m_layerMaxY : m_layerMinY;
    return x0 + (yAtBorder-y0)*m_tanAngle;
  }
}

//=============================================================================
// Function to determine the y coordinate of the crossing point between
// the beam-pipe hole (circle) and the fibres. The procedure is to express
// the fibre trajectory as x = x0 + m_tanAngle * y and solve the circle
// equation x**2 + y**2 = R**2 to determine the y coords of the crossing points.
// For y>0 (y<0) we are interested only in the positive (negative) solution.
// x0 is the x-coord@y=0 (i.e. the u-coordinate). The returned status code
// signifies if there is at least one crossing point.
//=============================================================================
void DeFTFibreMat::beamPipeYCoord(const double x0, const int ySign, double& yIntersect) const {

  /// Solve the quadratic equation
  double a = 1 + m_tanAngle*m_tanAngle;
  double b = 2 * x0 * m_tanAngle;
  double c = x0*x0 - m_innerHoleRadius*m_innerHoleRadius;
  double D  = b*b - 4*a*c;
  if ( D < 0 ) {
    /// No real solutions ==> no crossing points
    // a dummy value to initialize the variable
    yIntersect = 0.;
  }
  else {
    /// We have 2 solutions (can be degenerate)
    /// For ySign=1 (ySign=-1) will return the positive (negative) solution
    yIntersect = (-b + ySign*sqrt(D)) / (2*a);
    // debug() << "y-coordinate of beam-pipe crossing point: " << yIntersect << endmsg;
  }

}

//=============================================================================
// Function to determine the y coordinate of the crossing point between
// the beam-pipe hole (circle) and the fibres. The procedure is to express
// the fibre trajectory as x = x0 + m_tanAngle * y and solve the circle
// equation x**2 + y**2 = R**2 to determine the y coords of the crossing points.
// For y>0 (y<0) we are interested only in the positive (negative) solution.
// x0 is the x-coord@y=0 (i.e. the u-coordinate). The returned status code
// signifies if there is at least one crossing point.
//=============================================================================
void DeFTFibreMat::beamPipeYCoord(const double xcoord,
                                        const double ycoord,
                                        double& yIntersect) const {
  /// Solve the quadratic equation
  double x0 = xAtYEq0(xcoord, ycoord);
  int ySign = 0;
  if(ycoord > 0) ySign = 1;
  if(ycoord < 0) ySign = -1;

  double a = 1 + m_tanAngle*m_tanAngle;
  double b = 2 * x0 * m_tanAngle;
  double c = x0*x0 - m_innerHoleRadius*m_innerHoleRadius;
  double D  = b*b - 4*a*c;
  if ( D < 0 ) {
    /// No real solutions ==> no crossing points
    // a dummy value to initialize the variable
    yIntersect = 0.;
  }
  else {
    /// We have 2 solutions (can be degenerate)
    /// For ySign=1 (ySign=-1) will return the positive (negative) solution
    yIntersect = ( -b + ySign*sqrt(D) ) / (2*a);
    //debug() << "y-coordinate of beam-pipe crossing point: " << yIntersect << endmsg;
  }
}

//=============================================================================
// Function to determine lengh of the fibre as a function of its location 
// to take into account the hole in the middle of the layer (local frame)
// KEPT for BACKCOMPATIBILITY (replaced by "inBeamHole")
//=============================================================================
double DeFTFibreMat::FibreLengh(const Gaudi::XYZPoint&  lpEntry,
                                const Gaudi::XYZPoint&  lpExit) const{
  double fibrefullLength; 
  const Gaudi::XYZPoint lpMean((lpEntry.X()+lpExit.X())/2, (lpEntry.Y()+lpExit.Y())/2, (lpEntry.Z()+lpExit.Z())/2);                 
  inBeamHole(lpMean, fibrefullLength);
  return fibrefullLength;
}


//=============================================================================
// Get the FTChannelID of the cell located on the left of a given cell (left/right assume
// an observer sitting at (0,0,0) and looking downstream (+z)). The cell
// numbering convention is that the cellID always increases with x, i.e. to the left.
//=============================================================================
FTChannelID DeFTFibreMat::nextChannelLeft(const FTChannelID& channel) const {
  unsigned int grossCellID = this->grossCellID(channel.sipmCell());
  unsigned int grossIDLeftCell = grossCellID + 1u;
  return createChannel( channel.layer(), channel.module(), channel.mat(), channel.sipmId(), grossIDLeftCell );
}

//=============================================================================
// Get the FTChannelID of the cell located on the right of a given cell (left/right assume
// an observer sitting at (0,0,0) and looking downstream (+z)). The cell
// numbering convention is that the cellID always increases with x, i.e. to the left.
//=============================================================================
FTChannelID DeFTFibreMat::nextChannelRight(const FTChannelID& channel) const {
  unsigned int grossCellID = this->grossCellID(channel.sipmCell());
  unsigned int grossIDRightCell = grossCellID - 1u;
  return createChannel( channel.layer(), channel.module(), channel.mat(), channel.sipmId(), grossIDRightCell );
}



//=============================================================================
//Find solidbase in a named pvolume
//=============================================================================
StatusCode DeFTFibreMat::findSolidBase(IDetectorElement *det, const std::string& pvolname, const SolidBase* &solidbase) {
  
  if(0==det) {
    fatal() << "Null input detector element"<< endmsg;
    return StatusCode::FAILURE;
  }
  
  const IPVolume *pvol=det->geometry()->lvolume()->pvolume(pvolname);
  if(0==pvol) return StatusCode::FAILURE;
    
  solidbase = dynamic_cast<const SolidBase*>(pvol->lvolume()->solid());
  if(0==solidbase) {
    fatal() << "Can't find SolidBase for PVolume: "<<pvolname<< endmsg;
    return StatusCode::FAILURE;
  }
  return StatusCode::SUCCESS;
}



//=============================================================================
//Beam Hole acceptance, depending on geometry versions. Also returns maximum
//fiber length at the input point
//=============================================================================
bool DeFTFibreMat::inBeamHole(const Gaudi::XYZPoint& hitLocal,
                              double& fibrelengthMax) const {
    bool inHole;
    if(m_holey) {
      if (m_FTGeomversion <= m_FTGeomVersion_reference) {    //geom v20
        double YFibreHole;
        beamPipeYCoord(hitLocal.X(), hitLocal.Y(), YFibreHole);   //may be bugged: local or global ?
        //get (maximum) fibre length
        fibrelengthMax=2.*m_fibreMatHalfSizeY - std::abs(YFibreHole)/m_cosAngle;   //Y=0 when no intersection
        //test hit y position
        Gaudi::XYZPoint hitGlobal=this->geometry()->toGlobal(hitLocal);
        if((std::pow(hitGlobal.x(),2) + std::pow(hitGlobal.y(),2)) < std::pow(m_innerHoleRadius,2)) inHole=true;

        if( m_msg->level() <= MSG::DEBUG) {
          debug() << "inBeamHole geom v2:" << " hitLocal: " << hitLocal << "in: " << inHole << " fibrelengthMax: "
                  << fibrelengthMax << endmsg;
        }
      }
      else if (m_FTGeomversion > m_FTGeomVersion_reference) {   //geom v4x
        double dxHit=std::abs(hitLocal.X()-m_posHole.X());        //absolute dx to hole center
        //Hole size for each sub-box
        double dyHole;
        if(dxHit<m_halfHole1X) dyHole=m_halfHole1Y;
        else if(dxHit<m_halfHole2X && dxHit>=m_halfHole1X) dyHole=m_halfHole2Y;
        else if(dxHit<m_halfHole3X && dxHit>=m_halfHole2X) dyHole=m_halfHole3Y;
        else if(dxHit<m_halfHole4X && dxHit>=m_halfHole3X) dyHole=m_halfHole4Y;
        else dyHole=0;
        //get (maximum) fibre length
        if(std::abs(m_fibreMatHalfSizeY-std::abs(m_posHole.Y())) < dyHole) {
          fibrelengthMax=m_fibreMatHalfSizeY+std::abs(m_posHole.Y())-dyHole;
        }
        else fibrelengthMax=2*m_fibreMatHalfSizeY;
        //test hit y position
        if(std::abs(hitLocal.Y()-m_posHole.Y())<dyHole) inHole=true;

        if( m_msg->level() <= MSG::DEBUG) {
          debug() << "inBeamHole geom v4:" << " m_posHole: " << m_posHole <<  " dyHole: " << dyHole
                  <<" hitLocal: " << hitLocal << "in: " << inHole << " fibrelengthMax: "
                  << fibrelengthMax << endmsg;
        }
      }
      else {   //no clear geometry
        fatal() << "Can't find geometry to define beam hole: v"<<m_FTGeomversion<< endmsg;
      }
    }
    else {   //not a hole fibremat
      fibrelengthMax=2*m_fibreMatHalfSizeY;
      inHole=false;
    }
    
    return inHole;
}



//temporary workaround for Right Hole (module 11) axes inversion (not to be done for v2 geometry)
void DeFTFibreMat::doRHAxesInversion(Gaudi::XYZPoint& xyzLocal) const {
  xyzLocal.SetXYZ(-xyzLocal.X(), xyzLocal.Y(), -xyzLocal.Z());
}

