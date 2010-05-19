// $Id: TupleToolPrimaries.cpp,v 1.10 2010-05-19 06:45:32 pkoppenb Exp $
// Include files

// from Gaudi
#include "GaudiKernel/ToolFactory.h"

// local
#include "TupleToolPrimaries.h"

#include "GaudiAlg/Tuple.h"
#include "GaudiAlg/TupleObj.h"
#include "Kernel/DVAlgorithm.h"

#include <Kernel/GetDVAlgorithm.h>
#include <Kernel/DVAlgorithm.h>
#include <Event/RecVertex.h>

//#include <functional>

//-----------------------------------------------------------------------------
// Implementation file for class : TupleToolPrimaries
//
// 2007-11-07 : Jeremie Borel
//-----------------------------------------------------------------------------

// Declaration of the Tool Factory
// actually acts as a using namespace TupleTool
DECLARE_TOOL_FACTORY( TupleToolPrimaries );

using namespace LHCb;

//=============================================================================
// Standard constructor, initializes variables
//=============================================================================
TupleToolPrimaries::TupleToolPrimaries( const std::string& type,
					const std::string& name,
					const IInterface* parent )
  : TupleToolBase ( type, name , parent )
  //, m_dva(0)
{
  declareInterface<IEventTupleTool>(this);
  declareProperty("InputLocation", m_pvLocation = "" , 
                  "PV location to be used. If empty, take default");
  
}

//=============================================================================
//=============================================================================

StatusCode TupleToolPrimaries::initialize(){
  if( !TupleToolBase::initialize() ) return StatusCode::FAILURE;

  if ( "" == m_pvLocation){
    const IOnOffline* oo = tool<IOnOffline>("OnOfflineTool",this);
    m_pvLocation = oo->primaryVertexLocation();
    debug() << "Will be looking for PVs at " << m_pvLocation << endmsg ;
  } 
  //m_dva = Gaudi::Utils::getDVAlgorithm ( contextSvc() ) ;
  //if (0==m_dva) return Error("Couldn't get parent DVAlgorithm", StatusCode::FAILURE);

  return StatusCode::SUCCESS;
}

//=============================================================================
//=============================================================================

StatusCode TupleToolPrimaries::fill( Tuples::Tuple& tuple ) 
{
  const std::string prefix=fullName();
  const unsigned int maxPV = 100 ;

  if (msgLevel(MSG::VERBOSE)) verbose() << "Storing PVs with prefix " << prefix << endmsg ;
  
  std::vector<double>  pvx, pvy, pvz;
  std::vector<double>  epvx, epvy, epvz;
  std::vector<double>  pvchi2, pvndof, pvntracks;
  
  const RecVertex::Container* PV = 0 ;
  //if ( ""==m_pvLocation ) PV = m_dva->primaryVertices();   // default
  if (exist<RecVertex::Container>(m_pvLocation))
  {     // user given
    PV = get<RecVertex::Container>(m_pvLocation);
  }
  if (0!=PV)
  {
    
    if( PV->size() > maxPV )
    {
      Warning("More than 100 primaries, no PVs will be stored.");
    } else {
      
      for(RecVertex::Container::const_iterator i = PV->begin() ; PV->end()!=i ; ++i )
      {
        if (msgLevel(MSG::VERBOSE)) verbose() << "PV: " <<  (*i)->position() << endmsg ;
        pvx.push_back( (*i)->position().X() );
        pvy.push_back( (*i)->position().Y() );
        pvz.push_back( (*i)->position().Z() );
        epvx.push_back( std::sqrt((*i)->covMatrix()(0,0)) );
        epvy.push_back( std::sqrt((*i)->covMatrix()(1,1)) );
        epvz.push_back( std::sqrt((*i)->covMatrix()(2,2)) );
        pvchi2.push_back((*i)->chi2());
        pvndof.push_back((*i)->nDoF());
        pvntracks.push_back((double)(*i)->tracks().size());
      }
    }
    
  }
  
  bool test=true;
  test &= tuple->farray( prefix+"PVX", pvx, prefix+"nPV",  maxPV );
  test &= tuple->farray( prefix+"PVY", pvy, prefix+"nPV",  maxPV );
  test &= tuple->farray( prefix+"PVZ", pvz, prefix+"nPV",  maxPV );
  test &= tuple->farray( prefix+"PVXERR", epvx, prefix+"nPV",  maxPV );
  test &= tuple->farray( prefix+"PVYERR", epvy, prefix+"nPV",  maxPV );
  test &= tuple->farray( prefix+"PVZERR", epvz, prefix+"nPV",  maxPV );
  
  test &= tuple->farray( prefix+"PVCHI2", pvchi2, prefix+"nPV",  maxPV );
  test &= tuple->farray( prefix+"PVNDOF", pvndof, prefix+"nPV",  maxPV );
  test &= tuple->farray( prefix+"PVNTRACKS", pvntracks, prefix+"nPV",  maxPV );
  
  if (msgLevel(MSG::DEBUG)) debug() << "Stored " << PV->size() << " PVs " << test << endmsg ;
  return StatusCode(test);
  //  return StatusCode::SUCCESS;
}
