// $Id: IBTaggingTool.h,v 1.1 2005-07-04 15:45:32 pkoppenb Exp $
#ifndef BTAGGINGTOOL_IBTAGGINGTOOL_H
#define BTAGGINGTOOL_IBTAGGINGTOOL_H 1

// Include files
// from STL
#include <string>
// from Gaudi
#include "Kernel/DVAlgorithm.h"
#include "GaudiKernel/IAlgTool.h"
#include "Event/FlavourTag.h"

static const InterfaceID IID_IBTaggingTool("IBTaggingTool", 1 , 0);

/** @class IBTaggingTool IBTaggingTool.h
 *
 *  v1.2
 *  @author Marco Musy (Milano)
 *  @date   2004-12-14
 */

class IBTaggingTool : virtual public IAlgTool {

public:

  /// Retrieve interface ID
  static const InterfaceID& interfaceID() { return IID_IBTaggingTool; };

  virtual FlavourTag* tag( const Particle* ) = 0;
  virtual FlavourTag* tag( const  Particle*, const  Vertex* ) = 0;
  virtual FlavourTag* tag( const  Particle*, const  Vertex*,
			   ParticleVector& ) = 0;
};
#endif // BTAGGINGTOOL_IBTAGGINGTOOL_H
