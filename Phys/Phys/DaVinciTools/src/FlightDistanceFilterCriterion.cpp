// Include files 

// from Gaudi
#include "GaudiKernel/ToolFactory.h"

// local
#include "FlightDistanceFilterCriterion.h"
#include "DaVinciTools/IGeomDispCalculator.h"

//-----------------------------------------------------------------------------
// Implementation file for class : FlightDistanceFilterCriterion
//
// 15/07/2004 : Luis Fernandez
//-----------------------------------------------------------------------------

// Declaration of the Tool Factory
static const  ToolFactory<FlightDistanceFilterCriterion>          s_factory ;
const        IToolFactory& FlightDistanceFilterCriterionFactory = s_factory ;

//=============================================================================
// Standard constructor, initializes variables
//=============================================================================
FlightDistanceFilterCriterion::FlightDistanceFilterCriterion(
                                                      const std::string& type,
                                                      const std::string& name,
                                                      const IInterface* parent )
  : GaudiTool ( type, name , parent ) {

  // declare additional interface
  declareInterface<IFilterCriterion>(this);

  // declare properties

  //================================================================
  declareProperty( "CutAllPVs", m_CutAllPVs = false);
  declareProperty( "CutBestPV", m_CutBestPV = false);
  //================================================================

  //================================================================
  // Min unsigned Flight distance w.r.t all PVs
  declareProperty( "MinFallPVs", m_minFallPVs = -1. );
  // Min unsigned Flight Significance w.r.t all PVs
  declareProperty( "MinFSallPVs", m_minFSallPVs = -1. );
  // Min signed Flight distance w.r.t all PVs
  declareProperty( "MinSignedFallPVs", m_minSignedFallPVs = -100000. );
  // Min signed Flight Significance w.r.t all PVs
  declareProperty( "MinSignedFSallPVs", m_minSignedFSallPVs = -100000. );
  //================================================================

  //================================================================
  // Min unsigned Flight distance w.r.t Best PV
  declareProperty( "MinFPV", m_minFPV = -1. );  
  // Min unsigned Flight Significance w.r.t Best PV
  declareProperty( "MinFSPV", m_minFSPV = -1. );
  // Min signed Flight distance w.r.t Best PV
  declareProperty( "MinSignedFPV", m_minSignedFPV = -100000. );
  // Min signed Flight Significance w.r.t Best PV
  declareProperty( "MinSignedFSPV", m_minSignedFSPV = -100000. );
  //---
  // Max unsigned Flight distance w.r.t Best PV
  declareProperty( "MaxFPV", m_maxFPV = -1. );  
  // Max unsigned Flight Significance w.r.t Best PV
  declareProperty( "MaxFSPV", m_maxFSPV = -1. );
  // Max signed Flight distance w.r.t Best PV
  declareProperty( "MaxSignedFPV", m_maxSignedFPV = -100000. );
  // Max signed Flight Significance w.r.t Best PV
  declareProperty( "MaxSignedFSPV", m_maxSignedFSPV = -100000. );


  //================================================================
}

//=============================================================================
// initialize() method
//=============================================================================
StatusCode FlightDistanceFilterCriterion::initialize() {

  StatusCode sc = GaudiTool::initialize();
  debug() << ">>>   FlightDistanceFilterCriterion::initialize() " << endreq;
  sc = service( "EventDataSvc", m_EDS, true );

  if( sc.isFailure() ) {
    err() << " Unable to retrieve EventDataSvc " << endreq;
    return sc;
  }
  sc = toolSvc()->retrieveTool("GeomDispCalculator", m_vtxDisTool, this);
  if(sc.isFailure()){
    err() << " Unable to retrieve GeomDispCalculator tool" << endreq;
    return sc;
  }
  sc = toolSvc()->retrieveTool("GeomDispCalculator", m_ipTool, this);
  if(sc.isFailure()){
    err() << " Unable to retrieve GeomDispCalculator tool" << endreq;
    return sc;
  }

  if(m_CutAllPVs && m_CutBestPV){
    fatal() << ">>>   CANNOT cut w.r.t all PVs AND Best PV. " 
      "Choose either mode." << endreq;
    return StatusCode::FAILURE;
  }

  //================================================================
  if(m_CutAllPVs){
    debug() << ">>>   Will apply cuts w.r.t all PVs. Cuts are:" << endreq;

    if(m_minFallPVs>0.){
      debug() << ">>>   Minimum unsigned F w.r.t all PVs: " 
              << m_minFallPVs << " mm" << endreq;
    }
    
    if(m_minFSallPVs>0.){
      debug() << ">>>   Minimum unsigned FS w.r.t all PVs: " 
              << m_minFSallPVs << " sigma" << endreq;
    }
    
    if(m_minSignedFallPVs>-100000.){
      debug() << ">>>   Minimum signed F w.r.t all PVs: " 
              << m_minSignedFallPVs << " mm" << endreq;
    }
    
    if(m_minSignedFSallPVs>-100000.){
      debug() << ">>>   Minimum signed FS w.r.t all PVs: " 
              << m_minSignedFSallPVs << " sigma" << endreq;
    }
  }
  //================================================================

  //================================================================
  if(m_CutBestPV){
    debug() << ">>>   Will apply cuts w.r.t Best PV. Cuts are:" << endreq;
    
    // Min cuts
    if(m_minFPV>0.){
      debug() << ">>>   Minimum unsigned F w.r.t Best PV: " 
              << m_minFPV << " mm" << endreq;
    }
    
    if(m_minFSPV>0.){
      debug() << ">>>   Minimum unsigned FS w.r.t Best PV: " 
              << m_minFSPV << " sigma" << endreq;
    }
    
    if(m_minSignedFPV>-100000.){
      debug() << ">>>   Minimum signed F w.r.t Best PV: " 
              << m_minSignedFPV << " mm" << endreq;
    }
    
    if(m_minSignedFSPV>-100000.){
      debug() << ">>>   Minimum signed FS w.r.t Best PV: " 
              << m_minSignedFSPV << " sigma" << endreq;
    }

    // Max cuts
    if(m_maxFPV>0.){
      debug() << ">>>   Maximum unsigned F w.r.t Best PV: " 
              << m_maxFPV << " mm" << endreq;
    }
    if(m_maxFSPV>0.){
      debug() << ">>>   Maximum unsigned FS w.r.t Best PV: " 
              << m_maxFSPV << " sigma" << endreq;
    }
    
    if(m_maxSignedFPV>-100000.){
      debug() << ">>>   Maximum signed F w.r.t Best PV: " 
              << m_maxSignedFPV << " mm" << endreq;
    }
    
    if(m_maxSignedFSPV>-100000.){
      debug() << ">>>   Maximum signed FS w.r.t Best PV: " 
              << m_maxSignedFSPV << " sigma" << endreq;
    }
  }
  //================================================================

  return StatusCode::SUCCESS;
}
//=============================================================================
// Test if filter is satisfied
//=============================================================================
bool FlightDistanceFilterCriterion::isSatisfied( const Particle* const & part ){

  SmartDataPtr<Vertices> PV(m_EDS,VertexLocation::Primary);

  verbose() << ">>>> Looping on " << PV->size() << " PVs" << endreq;
  VertexVector::const_iterator iv;
  int npv = 0;

  // PV w.r.t. which the composite has the smallest IPS
  Vertex *BestPV = 0;
  
  const Vertex* secv = part->endVertex();
  if (!secv){ 
    verbose() << "No decay vertex found" << endreq;
    return (false);
  }
  
  verbose() << "Found decay vertex z [mm] = " 
          << secv->position().z()<< endreq;

  double normIPSMin = -1.;
  
  bool minFallPVs_happy  = true;
  bool minFPV_happy  = true;

  // set to false if needed
  bool maxFPV_happy  = !((m_maxFPV>0.)
                         ||(m_maxFSPV>0.)
                         ||(m_maxSignedFPV>-100000.)
                         ||(m_maxSignedFSPV>-100000.));

  // loop over PVs
  for (iv=PV->begin();iv!=PV->end();++iv) {
    Vertex* pv = *iv;
    if( pv->type()!=Vertex::Primary) continue;

    npv++;
    verbose() << "Primary vertex number = " << npv << endreq;
    verbose() << "Primary vertex z [mm] = " 
              << pv->position().z()<< endreq;

    //================================================================
    // Find the PV w.r.t. which the composite has the smallest IPS
    // Calculate the IPS of the composite candidate 
    if(m_CutBestPV){
      double ip, ipe;
      StatusCode scip = m_ipTool->calcImpactPar(*part,*pv,ip,ipe);
      if(scip.isFailure()) continue;
      double normIPS = ip/ipe;
      verbose() << "IPS = " << normIPS << " sigma" << endreq;
      if(normIPSMin<0||normIPS<normIPSMin) {
        normIPSMin=normIPS;
        BestPV=pv;
      }
    }
    //================================================================

    //================================================================
    if(m_CutAllPVs){
      double f = -1. , fe = -1.;
      StatusCode sc = m_vtxDisTool->calcVertexDis(*pv, *secv, f, fe);
      if (sc.isFailure()) continue;
      double fs = fabs(f/fe);
      
      // the unsigned flight distance
      verbose() << "unsigned flight distance = " 
                << f << " +/-" << fe << " mm" << endreq ;
      
      // the unsigned flight significance
      verbose() << "unsigned flight significance = " << fs << endreq ;
      
      // the signed flight distance
      double normf = (secv->position()-pv->position()).z()>0 ? f : -f;
      verbose() << "signed flight distance = " << normf 
                << " +/-" << fe << " mm" << endreq ;
      
      // the signed flight significance
      double normfs = (secv->position()-pv->position()).z()>0 ? fs : -fs;
      verbose() << "signed flight significance = " << normfs << endreq ;
      
      
      if (// Check the unsigned flight distance w.r.t all PVs
          (m_minFallPVs>0. && f < m_minFallPVs)
          ||
          // Check the unsigned flight significance w.r.t all PVs
          (m_minFSallPVs>0. && fs < m_minFSallPVs)
          ||
          // Check the unsigned flight distance w.r.t all PVs
          (m_minSignedFallPVs > -100000. && normf < m_minSignedFallPVs)
          ||
          // Check the unsigned flight significance w.r.t all PVs
          (m_minSignedFSallPVs > -100000. && normfs < m_minSignedFSallPVs)){
        minFallPVs_happy  = false;
        verbose() << "Breaking because of bad min Flight w.r.t all PVs" 
                  << endreq ;
        break; // found at least one PV not satisfying one of the criteria
      }
    }
    //================================================================
    
  } // loop over PVs
  
  //================================================================
  if(m_CutAllPVs){  
    verbose() << "Happy for minimum F w.r.t all PVs: " 
              << minFallPVs_happy << endreq ;
  }
  //================================================================  

  //================================================================
  // Calculate the IPS of the composite candidate w.r.t the Best PV
  if(m_CutBestPV && BestPV){
    double ip, ipe;
    StatusCode scip = m_ipTool->calcImpactPar(*part,*BestPV,ip,ipe);
    if(scip.isFailure())return(false);
    
    double normIPS = ip/ipe;
    verbose() << "IPS w.r.t best PV = " << normIPS << " sigma" << endreq;
    
    double f = -1. , fe = -1.;
    StatusCode sc = m_vtxDisTool->calcVertexDis(*BestPV, *secv, f, fe);
    if (sc.isFailure()) return(false);
    double fs = fabs(f/fe);
    
    // the unsigned flight distance
    verbose() << "unsigned flight distance w.r.t Best PV = " 
              << f << " +/-" << fe << " mm" << endreq ;
    
    // the unsigned flight significance
    verbose() << "unsigned flight significance w.r.t Best PV = " 
              << fs << endreq ;
    
    // the signed flight distance
    double normf = (secv->position()-BestPV->position()).z()>0 ? f : -f;
    verbose() << "signed flight distance w.r.t Best PV = " << normf 
              << " +/-" << fe << " mm" << endreq ;
    
    // the signed flight significance
    double normfs = (secv->position()-BestPV->position()).z()>0 ? fs : -fs;
    verbose() << "signed flight significance w.r.t Best PV = " 
              << normfs << endreq ;

    // Min cuts    
    if (// Check the unsigned flight distance w.r.t best PV
        (m_minFPV>0. && f < m_minFPV)
        ||
        // Check the unsigned flight significance w.r.t best PV
        (m_minFSPV>0. && fs < m_minFSPV)
        ||
        // Check the unsigned flight distance w.r.t best PV
        (m_minSignedFPV > -100000. && normf < m_minSignedFPV)
        ||
        // Check the unsigned flight significance w.r.t best PV
        (m_minSignedFSPV > -100000. && normfs < m_minSignedFSPV)
        ){
      minFPV_happy  = false;
    }

    verbose() << "Happy for minimum F w.r.t Best PV: " 
              << minFPV_happy << endreq ;

    // Max cuts
    // When all the max cuts are set
    if (// Check the unsigned flight distance w.r.t best PV
        (m_maxFPV>0. && f < m_maxFPV)
        &&
        // Check the unsigned flight significance w.r.t best PV
        (m_maxFSPV>0. && fs < m_maxFSPV)
        &&
        // Check the unsigned flight distance w.r.t best PV
        (m_maxSignedFPV > -100000. && normf < m_maxSignedFPV)
        &&
        // Check the unsigned flight significance w.r.t best PV
        (m_maxSignedFSPV > -100000. && normfs < m_maxSignedFSPV)

        ){
      maxFPV_happy  = true;
    }
    // When not all the max cuts are set
    else{
      // special case, change value of flag to true here
      maxFPV_happy  = true;
      
      if (// Check the unsigned flight distance w.r.t best PV
          (m_maxFPV>0. && f > m_maxFPV)
          ||
          // Check the unsigned flight significance w.r.t best PV
          (m_maxFSPV>0. && fs > m_maxFSPV)
          ||
          // Check the unsigned flight distance w.r.t best PV
          (m_maxSignedFPV > -100000. && normf > m_maxSignedFPV)
          ||
          // Check the unsigned flight significance w.r.t best PV
          (m_maxSignedFSPV > -100000. && normfs > m_maxSignedFSPV)
          ){
        maxFPV_happy  = false;
      }
    }
    
    verbose() << "Happy for maximum F w.r.t Best PV: " 
            << maxFPV_happy << endreq ;
    
    verbose() << "Happy for minimum and maximum F w.r.t Best PV: " 
            << (minFPV_happy && maxFPV_happy) << endreq ;
  } // if m_CutBestPV
  
    //================================================================
  
  return (minFallPVs_happy && minFPV_happy && maxFPV_happy);
}

//=============================================================================
// Test if filter is satisfied
//=============================================================================
bool FlightDistanceFilterCriterion::operator()( const Particle* const & part ) {
  return this->isSatisfied( part );
}

//=============================================================================
