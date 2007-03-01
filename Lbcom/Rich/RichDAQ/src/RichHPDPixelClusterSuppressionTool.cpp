
//-----------------------------------------------------------------------------
/** @file RichHPDPixelClusterSuppressionTool.cpp
 *
 * Implementation file for class : RichHPDPixelClusterSuppressionTool
 *
 * CVS Log :-
 * $Id: RichHPDPixelClusterSuppressionTool.cpp,v 1.18 2007-03-01 19:39:07 jonrob Exp $
 *
 * @author Chris Jones   Christopher.Rob.Jones@cern.ch
 * @date   21/03/2006
 */
//-----------------------------------------------------------------------------

#include "GaudiKernel/ToolFactory.h"

// local
#include "RichHPDPixelClusterSuppressionTool.h"

// RICH DAQ
using namespace Rich::DAQ;

DECLARE_TOOL_FACTORY( HPDPixelClusterSuppressionTool );

// Standard constructor
HPDPixelClusterSuppressionTool::
HPDPixelClusterSuppressionTool( const std::string& type,
                                const std::string& name,
                                const IInterface* parent )
  : HighOccHPDSuppressionTool ( type, name, parent ),
    m_clustTool               ( NULL )
{
  // Define interface
  declareInterface<IPixelSuppressionTool>(this);
  // job options
  declareProperty( "MaxPixelClusterSize",    m_maxPixClusterSize  = 10 );
  declareProperty( "MinHPDOccForClustering", m_minHPDocc          = 10 );
}

StatusCode HPDPixelClusterSuppressionTool::initialize()
{
  // Sets up various tools and services
  const StatusCode sc = HighOccHPDSuppressionTool::initialize();
  if ( sc.isFailure() ) return sc;

  // get the clustering tool
  acquireTool( "ClusteringTool", m_clustTool, this );

  // sanity checks
  if ( m_maxPixClusterSize > m_minHPDocc ) m_minHPDocc = m_maxPixClusterSize;

  debug() << "  Max HPD pixel cluster size            = " << m_maxPixClusterSize << endreq
          << "  Min HPD occupancy for clustering      = " << m_minHPDocc << endreq;

  return sc;
}

bool
HPDPixelClusterSuppressionTool::
applyPixelSuppression( const LHCb::RichSmartID hpdID,
                       LHCb::RichSmartID::Vector & smartIDs ) const
{
  // check overall HPD suppression and update running average occupancy
  bool suppress =
    HighOccHPDSuppressionTool::applyPixelSuppression( hpdID, smartIDs );
  if ( suppress ) return true;

  // number of pixels before suppression
  const unsigned int startSize = smartIDs.size();

  // if occ below min for clustering, just return false (no suppression)
  if ( startSize < m_minHPDocc ) return false;

  // find the clusters
  HPDPixelClusters::ConstSharedPtn clusters = m_clustTool->findClusters( smartIDs );
 
  // Print out clustering results
  if ( msgLevel(MSG::VERBOSE) )
  {
    verbose() << hpdID << endreq << *clusters << endreq;
  }

  // apply pixel cluster suppression
  clusters->suppressIDs(smartIDs,m_maxPixClusterSize);

  // was anything removed ?
  suppress = ( startSize != smartIDs.size() );

  if ( m_sumPrint && suppress )
  {
    // Print message
    std::ostringstream hpd;
    hpd << hpdID;
    Warning( "Partially suppressed HPD "+hpd.str(), StatusCode::SUCCESS, 10 );
  }

  // return status
  return suppress;
}

