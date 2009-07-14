// $Id: STSelectByCharge.cpp,v 1.2 2009-07-14 11:04:20 lnicolas Exp $
 
// Kernel
#include "GaudiKernel/ToolFactory.h"
 
// Event
#include "Event/STCluster.h"

//boost
#include "boost/numeric/conversion/bounds.hpp"

#include "STSelectByCharge.h"


DECLARE_TOOL_FACTORY( STSelectByCharge);
 
STSelectByCharge::STSelectByCharge( const std::string& type, 
                                    const std::string& name,
                                    const IInterface* parent ) :
  ST::ToolBase(type, name, parent)
{

  declareProperty("minCharge", m_minCharge = 0.0 );
  declareProperty("maxCharge", m_maxCharge = boost::numeric::bounds<double>::highest()); 
  declareInterface<ISTClusterSelector>(this);
}

STSelectByCharge::~STSelectByCharge()
{
  //destructer
}

StatusCode STSelectByCharge::initialize() {
  
  StatusCode sc = ST::ToolBase::initialize();
  if (sc.isFailure()) return Error("Failed to initialize", sc);

  info() << "Min Charge set to " << m_minCharge << " / max charge set to " 
	 << m_maxCharge << endmsg;

  return StatusCode::SUCCESS;
}


bool STSelectByCharge::select( const LHCb::STCluster* cluster ) const{
  return (*this) (cluster);  
}
  
bool STSelectByCharge::operator()( const LHCb::STCluster* cluster ) const{
  const double charge = cluster->totalCharge();
  return charge > m_minCharge && charge < m_maxCharge;
}
