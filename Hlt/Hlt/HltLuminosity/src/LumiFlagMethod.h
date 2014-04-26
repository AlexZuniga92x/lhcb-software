// $Id: LumiFlagMethod.h,v 1.1 2010-01-29 13:38:12 panmanj Exp $
#ifndef LUMIFLAGMETHOD_H
#define LUMIFLAGMETHOD_H 1

// Include files
// from Gaudi
#include "GaudiAlg/GaudiAlgorithm.h"

/** @class LumiFlagMethod LumiFlagMethod.h
 *
 *
 *  @author Jaap Panman
 *  @date   2008-07-21
 */
class LumiFlagMethod : public GaudiAlgorithm
{
  public:
    /// Standard constructor
    LumiFlagMethod( const std::string& name, ISvcLocator* pSvcLocator );

    ~LumiFlagMethod() override = default; ///< Destructor

    StatusCode initialize() override; ///< Algorithm initialization
    StatusCode execute() override;    ///< Algorithm execution
    StatusCode finalize() override;   ///< Algorithm finalization

  private:
    std::string m_OutputContainerName;

    std::string m_ValueName;
    std::string m_CounterName;
    int m_Counter;
    int m_Value;
};
#endif // LUMIFLAGMETHOD_H
