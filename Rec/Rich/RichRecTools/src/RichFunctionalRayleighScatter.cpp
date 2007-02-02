
//-----------------------------------------------------------------------------
/** @file RichFunctionalRayleighScatter.cpp
 *
 *  Implementation file for tool : Rich::Rec::FunctionalRayleighScatter
 *
 *  CVS Log :-
 *  $Id: RichFunctionalRayleighScatter.cpp,v 1.14 2007-02-02 10:10:40 jonrob Exp $
 *
 *  @author Chris Jones   Christopher.Rob.Jones@cern.ch
 *  @date   15/03/2002
 */
//-----------------------------------------------------------------------------

// local
#include "RichFunctionalRayleighScatter.h"

// Gaudi
#include "GaudiKernel/ToolFactory.h"
#include "GaudiKernel/PhysicalConstants.h"

// All code is in general Rich reconstruction namespace
using namespace Rich::Rec;

//-----------------------------------------------------------------------------

DECLARE_TOOL_FACTORY( FunctionalRayleighScatter );

// Standard constructor
FunctionalRayleighScatter::
FunctionalRayleighScatter ( const std::string& type,
                            const std::string& name,
                            const IInterface* parent )
  : RichRecToolBase( type, name, parent ),
    m_eVToMicron   ( 0 ),
    m_AeroClarity  ( 0 )
{
  // interface
  declareInterface<IRayleighScatter>(this);
}

StatusCode FunctionalRayleighScatter::initialize()
{

  // Sets up various tools and services
  const StatusCode sc = RichRecToolBase::initialize();
  if ( sc.isFailure() ) { return sc; }

  // Get Rich1 Detector element
  const DeRich1 * Rich1DE = getDet<DeRich1>( DeRichLocation::Rich1 );

  // Rayleigh scattering parameters
  m_eVToMicron  = ( Gaudi::Units::h_Planck/(Gaudi::Units::joule*Gaudi::Units::s) *
                    Gaudi::Units::c_light /(Gaudi::Units::m/Gaudi::Units::s)     /
                    Gaudi::Units::e_SI ) / Gaudi::Units::nanometer ;
  m_AeroClarity = Rich1DE->param<double>( "AerogelClarity" )/Gaudi::Units::cm;

  // Informational Printout
  debug() << " Using analytic implementation"   << endreq
          << " eV to mm conversion factor   = " << m_eVToMicron << endreq
          << " Aerogel clarity              = " << m_AeroClarity << endreq;

  return sc;
}

StatusCode FunctionalRayleighScatter::finalize()
{
  // Execute base class method
  return RichRecToolBase::finalize();
}

double
FunctionalRayleighScatter::photonScatteredProb( const LHCb::RichRecSegment * segment,
                                                const double energy ) const
{

  // check this is aerogel
  if ( Rich::Aerogel != segment->trackSegment().radiator() ) return 0;

  // check energy is valid
  if ( energy <= 0 ) return 0;

  // check path length is valid
  const double path = segment->trackSegment().pathLength();
  if ( path <= 0 ) return 0;

  // compute and return prob
  const double lambda   = m_eVToMicron/energy;
  const double scatLeng = lambda*lambda*lambda*lambda/m_AeroClarity;

  return ( 1 - (scatLeng/path)*(1.0-exp(-path/scatLeng)) );
}
