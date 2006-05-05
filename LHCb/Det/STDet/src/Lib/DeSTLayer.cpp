
#include "STDet/DeSTLayer.h"
#include "DetDesc/IGeometryInfo.h"
#include "DetDesc/SolidBox.h"

#include "Kernel/PhysicalConstants.h"

/** @file DeSTLayer.cpp
*
*  Implementation of class :  DeSTLayer
*
*    @author Matthew Needham
*/


DeSTLayer::DeSTLayer( const std::string& name ) :
  DeSTBaseElement( name )
{ 
  // constructer
}


DeSTLayer::~DeSTLayer() {
  // destructer
}

StatusCode DeSTLayer::initialize() {

  // initialize method
  MsgStream msg(msgSvc(), name() );

  StatusCode sc = DeSTBaseElement::initialize();
  if (sc.isFailure() ){
    msg << MSG::ERROR << "Failed to initialize detector element" << endreq; 
  }
  else {

    m_id = param<int>("layerID");
    m_angle = param<double>("stereoangle");
    m_cosAngle = cos(m_angle);
    m_sinAngle = sin(m_angle);
    cachePlane();
  }

  return StatusCode::SUCCESS;
}

std::ostream& DeSTLayer::printOut( std::ostream& os ) const{

  // stream to cout
  os << " Layer: "  << m_id << std::endl;
  os << "stereo angle " << m_angle
     << std::endl;

  return os;
}

MsgStream& DeSTLayer::printOut( MsgStream& os ) const{

  // stream to Msg service
  os << " Station : "  << m_id << endreq;
  os << "stereo angle " << m_angle
     << endreq;

  return os;
}

void DeSTLayer::cachePlane(){

 Gaudi::XYZPoint p1 = globalPoint(0,0,0);
 Gaudi::XYZPoint p2 = globalPoint(3*m,0,0);
 Gaudi::XYZPoint p3 = globalPoint(0,3*cm,0);

 m_plane = Gaudi::Plane3D(p1,p2,p3);

}

