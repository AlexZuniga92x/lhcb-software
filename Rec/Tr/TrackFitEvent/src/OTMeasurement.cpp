// $Id: OTMeasurement.cpp,v 1.18 2008-05-29 14:03:58 smenzeme Exp $
// Include files

// OTDet
#include "OTDet/DeOTModule.h"
#include "Event/OTLiteTime.h"

// local
#include "Event/OTMeasurement.h"

using namespace Gaudi;
using namespace LHCb;

//-----------------------------------------------------------------------------
// Implementation file for class : OTMeasurement
//
// 2005-04-07 : Jose Hernando, Eduardo Rodrigues
// Author: Rutger van der Eijk
// Created: 07-04-1999
//-----------------------------------------------------------------------------

/// Standard constructor, initializes variables
OTMeasurement::OTMeasurement( const LHCb::OTLiteTime& time,
                              const DeOTModule& module,
                              int ambiguity,
                              const LHCb::StateVector& /*refVector*/)
  : Measurement( Measurement::OT, LHCb::LHCbID(time.channel())),
    m_ottime(time),
    m_module(&module), 
    m_deltaTimeOfFlight(0),
    m_ambiguity(ambiguity)
{
  this->init();
}

/// Standard constructor, without the reference vector
OTMeasurement::OTMeasurement( const LHCb::OTLiteTime& time,
                              const DeOTModule& module,
                              int ambiguity )
  : Measurement( Measurement::OT, LHCb::LHCbID(time.channel()), &module),
    m_ottime(time),
    m_deltaTimeOfFlight(0),
    m_ambiguity(ambiguity)
{
  this->init() ;
}

void OTMeasurement::init() 
{
  m_trajectory   = module().trajectory( channel() );
  Gaudi::XYZPoint center = m_trajectory->position( 0.5*( m_trajectory->beginRange()+m_trajectory->endRange())) ;
  m_z = center.z() ;
  
  // anything else can only be done if we have a reference.
  // I don't know what to do with these. They are part of the base class. We should just remove thenm somehow.
  // m_errMeasure = m_measure = 0;
}


double LHCb::OTMeasurement::stereoAngle() const
{
  return module().angle() ;
}

OTDet::RadiusWithError  LHCb::OTMeasurement::driftRadiusWithError( double arclen ) const 
{
  // Get the distance to the readout
  double distToReadout = trajectory().endRange() - arclen;
  // Drifttime
  double drifttime = calibratedTime() - deltaTimeOfFlight() - distToReadout / module().propagationVelocity() ;
  // The rest is done by the module
  return module().driftRadiusWithError( drifttime ) ;
}

OTDet::RadiusWithError  LHCb::OTMeasurement::driftRadiusWithErrorFromY( double globalY ) const 
{
  // Get the distance to the readout
  double deltaY = trajectory().endPoint().y() - globalY ;
  // Drifttime
  double drifttime = calibratedTime() - deltaTimeOfFlight() - deltaY / module().propagationVelocityY() ;
  // The rest is done by the module
  return module().driftRadiusWithError( drifttime ) ;
}

double LHCb::OTMeasurement::propagationTimeFromY( double globalY ) const
{
  // Get the distance to the readout
  double deltaY = trajectory().endPoint().y() - globalY ;
  return deltaY / module().propagationVelocityY() ;
}

void LHCb::OTMeasurement::setTimeOfFlight(double tof)
{
  m_deltaTimeOfFlight = tof - module().strawReferenceTimeOfFlight( channel().straw() ) ;
}

double LHCb::OTMeasurement::timeOfFlight() const
{
  return m_deltaTimeOfFlight + module().strawReferenceTimeOfFlight( channel().straw() ) ;
}



