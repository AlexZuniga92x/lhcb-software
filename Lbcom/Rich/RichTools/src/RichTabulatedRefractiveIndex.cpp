
//-----------------------------------------------------------------------------
/** @file RichTabulatedRefractiveIndex.cpp
 *
 *  Implementation file for class : RichTabulatedRefractiveIndex
 *
 *  @author Chris Jones   Christopher.Rob.Jones@cern.ch
 *  @date 15/03/2002
 */
//-----------------------------------------------------------------------------

// local
#include "RichTabulatedRefractiveIndex.h"

// From Gaudi
#include "GaudiKernel/ToolFactory.h"
#include "GaudiKernel/SystemOfUnits.h"

DECLARE_NAMESPACE_TOOL_FACTORY( Rich, TabulatedRefractiveIndex );

// Standard constructor
Rich::TabulatedRefractiveIndex::
TabulatedRefractiveIndex ( const std::string& type,
                           const std::string& name,
                           const IInterface* parent )
  : RichToolBase  ( type, name, parent         ),
    m_riches      ( Rich::NRiches              ),
    m_detParams   ( NULL                       ),
    m_radiators   ( Rich::NRadiatorTypes,
                    (const DeRichRadiator *)(NULL) ),
    m_hltMode     ( false                      )
{
  // interface
  declareInterface<IRefractiveIndex>(this);
}

StatusCode Rich::TabulatedRefractiveIndex::initialize()
{
  // Initialise base class
  const StatusCode sc = RichToolBase::initialize();
  if ( sc.isFailure() ) return sc;

  // Get tools
  acquireTool( "RichDetParameters", m_detParams );

  // Rich1 and Rich2
  m_riches[Rich::Rich1] = getDet<DeRich1>( DeRichLocations::Rich1 );
  m_riches[Rich::Rich2] = getDet<DeRich2>( DeRichLocations::Rich2 );

  // HLT mode or not ?
  m_hltMode = contextContains("HLT");

  return sc;
}

double
Rich::TabulatedRefractiveIndex::refractiveIndex( const Rich::RadiatorType rad,
                                                 const double energy ) const
{
  return deRad(rad)->refractiveIndex(energy,m_hltMode);
}

double
Rich::TabulatedRefractiveIndex::refractiveIndex( const Rich::RadiatorType rad,
                                                 const double energyBot,
                                                 const double energyTop ) const
{
  const Rich::DetectorType rich = ( rad == Rich::Rich2Gas ? Rich::Rich2 : Rich::Rich1 );
  return refractiveIndex( rad,
                          m_riches[rich]->nominalHPDQuantumEff()->meanX(energyBot,energyTop) /
                          Gaudi::Units::eV );
}

double
Rich::TabulatedRefractiveIndex::refractiveIndex( const Rich::RadiatorType rad ) const
{
  return refractiveIndex( rad, m_detParams->meanPhotonEnergy(rad) );
}

double
Rich::TabulatedRefractiveIndex::
refractiveIndex ( const RichRadIntersection::Vector & intersections,
                  const double energy ) const
{
  // loop over all radiator intersections and calculate the weighted average
  // according to the path length in each radiator
  double refIndex(0), totPathL(0);
  for ( RichRadIntersection::Vector::const_iterator iR = intersections.begin();
        iR != intersections.end(); ++iR )
  {
    const double pLength = (*iR).pathLength();
    refIndex += pLength * (*iR).radiator()->refractiveIndex(energy,m_hltMode);
    totPathL += pLength;
  }
  return ( totPathL>0 ? refIndex/totPathL : refIndex );
}

double
Rich::TabulatedRefractiveIndex::
refractiveIndex ( const RichRadIntersection::Vector & intersections ) const
{
  // loop over all radiator intersections and calculate the weighted average
  // according to the path length in each radiator
  double refIndex(0), totPathL(0);
  for ( RichRadIntersection::Vector::const_iterator iR = intersections.begin();
        iR != intersections.end(); ++iR )
  {
    const double energy  = m_detParams->meanPhotonEnergy((*iR).radiator()->radiatorID());
    const double pLength = (*iR).pathLength();
    refIndex += pLength * (*iR).radiator()->refractiveIndex(energy,m_hltMode);
    totPathL += pLength;
  }
  return ( totPathL>0 ? refIndex/totPathL : refIndex );
}

double
Rich::TabulatedRefractiveIndex::
refractiveIndexRMS ( const RichRadIntersection::Vector & intersections ) const
{
  // loop over all radiator intersections and calculate the weighted average
  // according to the path length in each radiator
  double refIndexRMS(0), totPathL(0);
  for ( RichRadIntersection::Vector::const_iterator iR = intersections.begin();
        iR != intersections.end(); ++iR )
  {
    const double pLength = (*iR).pathLength();
    const Rich::TabulatedProperty1D * index = (*iR).radiator()->refIndex(m_hltMode);
    refIndexRMS += pLength * index->rms( index->minX(), index->maxX(), 100 );
    totPathL += pLength;
  }
  return ( totPathL>0 ? refIndexRMS/totPathL : refIndexRMS );
}

double
Rich::TabulatedRefractiveIndex::
refractiveIndexSD ( const RichRadIntersection::Vector & intersections ) const
{
  // loop over all radiator intersections and calculate the weighted average
  // according to the path length in each radiator
  double refIndexSD(0), totPathL(0);
  for ( RichRadIntersection::Vector::const_iterator iR = intersections.begin();
        iR != intersections.end(); ++iR )
  {
    const double pLength = (*iR).pathLength();
    const Rich::TabulatedProperty1D * index = (*iR).radiator()->refIndex(m_hltMode);
    refIndexSD += pLength * index->standardDeviation( index->minX(), index->maxX(), 100 );
    totPathL   += pLength;
  }
  return ( totPathL>0 ? refIndexSD/totPathL : refIndexSD );
}
