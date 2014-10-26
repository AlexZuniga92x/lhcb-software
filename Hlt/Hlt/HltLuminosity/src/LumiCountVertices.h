// $Id: LumiCountVertices.h,v 1.2 2008-08-25 10:59:04 panmanj Exp $
#ifndef LUMICOUNTVERTICES_H
#define LUMICOUNTVERTICES_H 1

// Include files
// from Gaudi
#include "GaudiAlg/GaudiAlgorithm.h"
// from LHCB
#include "Event/Vertex.h"
#include "Event/RecVertex.h"

/** @class LumiCountVertices LumiCountVertices.h
 *
 *
 *  @author Jaap Panman
 *  @date   2008-07-21
 */
class LumiCountVertices : public GaudiAlgorithm
{
  public:
    /// Standard constructor
    LumiCountVertices( const std::string& name, ISvcLocator* pSvcLocator );

    ~LumiCountVertices() override = default; ///< Destructor

    StatusCode initialize() override; ///< Algorithm initialization
    StatusCode execute() override;    ///< Algorithm execution

  private:
    std::string m_InputSelectionName;
    std::string m_CounterName;
    std::string m_OutputContainerName;
    int m_Counter;
};
#endif // LUMICOUNTVERTICES_H
