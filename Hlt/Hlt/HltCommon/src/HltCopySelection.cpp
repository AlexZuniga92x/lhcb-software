// $Id: HltCopySelection.cpp,v 1.4 2009-08-06 06:55:53 graven Exp $
// Include files 
#include <algorithm>
#include <iterator>
// from Gaudi
#include "GaudiKernel/AlgFactory.h"
#include "HltCopySelection.h"

#include "Event/Particle.h"
#include "Event/Track.h"
//-----------------------------------------------------------------------------
// Implementation file for class : HltCopySelection
//
// 2009-03-31 : Gerhard Raven
//-----------------------------------------------------------------------------

// explicit instantiation of class, and the corresponding
// declaration of the Algorithm Factory
template class HltCopySelection<LHCb::Particle>;
typedef HltCopySelection<LHCb::Particle> HltCopyParticleSelection;
DECLARE_ALGORITHM_FACTORY( HltCopyParticleSelection );

template class HltCopySelection<LHCb::Track>;
typedef HltCopySelection<LHCb::Track> HltCopyTrackSelection;
DECLARE_ALGORITHM_FACTORY( HltCopyTrackSelection );

//=============================================================================
// Standard constructor, initializes variables
//=============================================================================
template <typename T>
HltCopySelection<T>::HltCopySelection( const std::string& name,
                                    ISvcLocator* pSvcLocator)
  : HltAlgorithm ( name , pSvcLocator )
  , m_selection(*this)
{
  m_selection.declareProperties( );
}
//=============================================================================
// Destructor
//=============================================================================
template <typename T>
HltCopySelection<T>::~HltCopySelection() {} 

//=============================================================================
// Initialization
//=============================================================================
template <typename T>
StatusCode 
HltCopySelection<T>::initialize() {
  
  StatusCode sc = HltAlgorithm::initialize(); // must be executed first
  if ( sc.isFailure() ) return sc;  // error printed already by GaudiAlgorithm
  m_selection.retrieveSelections();
  m_selection.registerSelection();
  counter("#input");
  // declareInfo("#input",&counter("#input"),std::string("Candidates seen by ") + name());
  return sc;
}

//=============================================================================
// Main execution
//=============================================================================

template <typename T>
StatusCode 
HltCopySelection<T>::execute() {
  counter("#input")  +=  m_selection.template input<1>()->size();
  std::copy( m_selection.template input<1>()->begin(), 
             m_selection.template input<1>()->end(), 
             std::back_inserter(*m_selection.output()) );
  setFilterPassed( !m_selection.output()->empty() );
  return StatusCode::SUCCESS;
}
