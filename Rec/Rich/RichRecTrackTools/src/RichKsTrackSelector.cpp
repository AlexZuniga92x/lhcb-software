
//-----------------------------------------------------------------------------
/** @file RichKsTrackSelector.cpp
 *
 *  Implementation file for RICH reconstruction tool : Rich::Rec::KsTrackSelector
 *
 *  CVS Log :-
 *  $Id: RichKsTrackSelector.cpp,v 1.2 2009-06-10 16:12:58 jonrob Exp $
 *
 *  @author Chris Jones   Christopher.Rob.Jones@cern.ch
 *  @date   12/08/2006
 */
//-----------------------------------------------------------------------------

// from Gaudi
#include "GaudiKernel/ToolFactory.h"

// local
#include "RichKsTrackSelector.h"

// All code is in general Rich reconstruction namespace
using namespace Rich::Rec;

//-----------------------------------------------------------------------------

// Declaration of the Tool Factory
DECLARE_TOOL_FACTORY( KsTrackSelector );

//=============================================================================
// Standard constructor, initializes variables
//=============================================================================
KsTrackSelector::KsTrackSelector( const std::string& type,
                                  const std::string& name,
                                  const IInterface* parent )
  : BaseTrackSelector ( type, name , parent ) { }

//=============================================================================
// Destructor
//=============================================================================
KsTrackSelector::~KsTrackSelector() {}

//=============================================================================


