// $Id: TupleToolTrigger.h,v 1.12 2010-03-16 01:11:28 rlambert Exp $
#ifndef JBOREL_TUPLETOOLTRIGGER_H
#define JBOREL_TUPLETOOLTRIGGER_H 1

// Include files
// from Gaudi
#include "TupleToolTriggerBase.h"
#include "Kernel/IEventTupleTool.h"            // Interface

/** @class TupleToolTrigger TupleToolTrigger.h jborel/TupleToolTrigger.h
 *
 * \brief Fill the trigger informations for the DecayTreeTuple.
 *
 * - L0Decision : LHCb::L0DUReport->decision()
 * - Hlt1Global : Global HLT1 decision
 * - Hlt2Global : Global HLT2 decision
 *
 * If verbose is true the tool needs somehow to find a list of triggers to fill.
 * In this case it uses the base class TupleToolTriggerBase to sort everything out. 
 *
 * If \b VerboseL0 = true
 * L0Decision_xxx : LHCb::L0DUReport->channelDecisionByName(xxx)
 *  
 * If \b VerboseHlt1 = true
 * Hlt1_xxx_Decision : filled ... [ -1, 0, 1 ] = [not run, decision false, decision true]
 * 
 * If \b VerboseHlt2 = true
 * Hlt2_xxx_Decision : filled ... [ -1, 0, 1 ] = [not run, decision false, decision true]
 * 
 * Verbose flag is a shortcut to turn all verbosity on.
 *  
 * 
 *  @author Jeremie Borel
 *  @date   2007-11-07
 */
class TupleToolTrigger : public TupleToolTriggerBase, virtual public IEventTupleTool {
public:
  /// Standard constructor
  TupleToolTrigger( const std::string& type,
		      const std::string& name,
		      const IInterface* parent);

  virtual ~TupleToolTrigger( ){}; ///< Destructor

  StatusCode initialize() ;
  
  //implimented in the baseclass
  StatusCode fill( Tuples::Tuple& tuple )
  {
    return TupleToolTriggerBase::fill( tuple);
    
  };

private:

  ///fill verbose information for the L0
  StatusCode fillL0( Tuples::Tuple& );
  ///fill verbose information for the HLT
  StatusCode fillHlt( Tuples::Tuple&, const std::string &);
  StatusCode fillRoutingBits( Tuples::Tuple& );
 
  StatusCode fillBasic(Tuples::Tuple& tuple );
  
  StatusCode fillVerbose(Tuples::Tuple& tuple );
  
  bool m_fillHlt;     ///< fill Hlt
  bool m_fillL0;      ///< fill L0
  //bool  m_allSteps ; ///< Fill also intermediate steps
  std::vector<unsigned int> m_routingBits ; ///< Routing bits to fill
  
};
#endif // JBOREL_TUPLETOOLTRIGGER_H
