// $Id: TupleToolStripping.h,v 1.1 2010-04-26 12:49:59 pkoppenb Exp $
#ifndef TUPLETOOLSTRIPPING_H
#define TUPLETOOLSTRIPPING_H 1

// Include files
// from Gaudi
#include "GaudiAlg/GaudiTool.h"

#include "TupleToolBase.h"
#include "Kernel/IEventTupleTool.h"            // Interface

/** @class TupleToolStripping TupleToolStripping.h
 *
 *
 *  @author Patrick Koppenburg
 *  @date   2010-04-23
 */
class TupleToolStripping : public TupleToolBase, 
                           virtual public IEventTupleTool
{

public:

  /// Standard constructor
  TupleToolStripping( const std::string& type,
                      const std::string& name,
                      const IInterface* parent);

  virtual ~TupleToolStripping( ); ///< Destructor

  virtual StatusCode fill( Tuples::Tuple& );

private:

  std::string m_location ; ///< HDR location

};

#endif // TUPLETOOLSTRIPPING_H
