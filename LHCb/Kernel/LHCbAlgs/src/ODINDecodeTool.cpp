// $Id: ODINDecodeTool.cpp,v 1.2 2009-11-06 16:47:45 marcocle Exp $
// Include files
#include "ODINCodecBaseTool.h"
#include "GaudiKernel/SerializeSTL.h"

/** @class ODINDecodeTool ODINDecodeTool.h
 *
 *  Tool to decode the ODIN raw bank, fill a ODIN object with its data and
 *  register it to the transient store.
 *
 *  @see ODINCodecBaseTool for the properties.
 *
 *  @author Marco Clemencic
 *  @date   2009-02-02
 */
class ODINDecodeTool : public ODINCodecBaseTool {
public:
  /// Standard constructor
  ODINDecodeTool(const std::string& type,
                 const std::string& name,
                 const IInterface* parent);

  virtual ~ODINDecodeTool(); ///< Destructor

  /// Do the conversion
  virtual void execute();
};

//=============================================================================
// IMPLEMENTATION
//=============================================================================

#include <sstream>
// from Gaudi
#include "GaudiKernel/ToolFactory.h"
// from LHCb
#include "Event/ODIN.h"
#include "Event/RawBank.h"
#include "Event/RawEvent.h"

// Declaration of the Tool Factory
DECLARE_TOOL_FACTORY( ODINDecodeTool );

//=============================================================================
// Standard constructor, initializes variables
//=============================================================================
ODINDecodeTool::ODINDecodeTool( const std::string& type,
                                const std::string& name,
                                const IInterface* parent )
  : ODINCodecBaseTool(type, name, parent) {
}
//=============================================================================
// Destructor
//=============================================================================
ODINDecodeTool::~ODINDecodeTool() {
}
//=============================================================================
// Main function
//=============================================================================
void ODINDecodeTool::execute() {

  LHCb::ODIN* odin = 0;

  // Check if there is already an ODIN object
  if (exist<LHCb::ODIN>(m_odinLocation)) {
    if (m_force) {
      // Use the already registered object
      debug() << "Modify existing ODIN object" << endmsg;
      odin = get<LHCb::ODIN>(m_odinLocation);
    } else {
      // ODIN already there and we are not supposed to touch it
      debug() << "Keep existing ODIN object" << endmsg;
      return;
    }
  }

  debug() << "Getting RawEvent" << endmsg;
  LHCb::RawEvent* rawEvent = NULL;
  for (std::vector<std::string>::const_iterator p = m_rawEventLocations.begin(); p != m_rawEventLocations.end(); ++p) {
    if (exist<LHCb::RawEvent>(*p)){
      rawEvent = get<LHCb::RawEvent>(*p);
    }
  }
  if (!rawEvent) {
    using namespace GaudiUtils;
    // Throw a meaningful exception it the bank is not found;
    std::ostringstream out;
    out << "Cannot find RawEvent in " << m_rawEventLocations;
    Exception(out.str(), StatusCode(StatusCode::FAILURE, true));
  }

  // Check if have an ODIN bank...
  const std::vector<LHCb::RawBank*>& odinBanks = rawEvent->banks(LHCb::RawBank::ODIN);
  if(odinBanks.size()){ // ... good, we can decode it
    odin = this->i_decode(*odinBanks.begin(), odin);
    if (odin && (!odin->registry())){ // register ODIN object if valid and not yet registered
      put(odin, m_odinLocation);
    }
  }
  else {
    Warning("Cannot find ODIN bank in RawEvent");
  }
}

//=============================================================================
