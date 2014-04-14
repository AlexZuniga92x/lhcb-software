// ============================================================================
// Include files 
// ============================================================================
// local
// ============================================================================
#include "GaudiKernel/CArrayAsProperty.h"
#include "CaloTrackMatch.h"
// ============================================================================
/** @file 
 *  Implementation file for class CaloTrackMatch
 *  
 *  Properties of the class:
 *  BadValue - bad value of the chi2,
 *  ConditionName - string with the path to the file containing e- and e+ X-correction condition parameters,
 *  AlphaPOut - track-cluster matching X-correction parameters for Outer zone of the ECAL in case (q*polarity) > 0
 *  AlphaNOut - track-cluster matching X-correction parameters for Outer zone of the ECAL in case (q*polarity) < 0
 *  AlphaPMid - track-cluster matching X-correction parameters for Middle zone of the ECAL in case (q*polarity) > 0
 *  AlphaNMid - track-cluster matching X-correction parameters for Middle zone of the ECAL in case (q*polarity) < 0
 *  AlphaPInn - track-cluster matching X-correction parameters for Inner zone of the ECAL in case (q*polarity) > 0
 *  AlphaNInn - track-cluster matching X-correction parameters for Inner zone of the ECAL in case (q*polarity) < 0
 *
 *  @author Oleg STENYAKIN  oleg.stenyakin@cern.ch
 *  @date   2014-03-03
 *
 */
// ============================================================================
/// standard constructor 
// ============================================================================
CaloTrackMatch::CaloTrackMatch
( const std::string& type   ,
  const std::string& name   ,
  const IInterface*  parent )
  : Calo::CaloTrackTool ( type, name , parent )
  , m_bad ( 1.e+10 )
  , m_state() 
{
  declareProperty ("BadValue" , m_bad                                     ) ;

  declareProperty ("ConditionName", m_conditionName = "/dd/Conditions/ParticleID/Calo/ElectronXCorrection",
                   "set this property to an empty string to disable the use of CondDB" ) ;
  declareProperty ("AlphaPOut"   , m_alphaPOut, "electron X-correction params for (q*polarity) > 0 Outer"  ) ;
  declareProperty ("AlphaNOut"   , m_alphaNOut, "electron X-correction params for (q*polarity) < 0 Outer"  ) ;
  declareProperty ("AlphaPMid"   , m_alphaPMid, "electron X-correction params for (q*polarity) > 0 Middle" ) ;
  declareProperty ("AlphaNMid"   , m_alphaNMid, "electron X-correction params for (q*polarity) < 0 Middle" ) ;
  declareProperty ("AlphaPInn"   , m_alphaPInn, "electron X-correction params for (q*polarity) > 0 Inner"  ) ;
  declareProperty ("AlphaNInn"   , m_alphaNInn, "electron X-correction params for (q*polarity) < 0 Inner"  ) ;
}
// ============================================================================
/// destructor 
// ============================================================================
CaloTrackMatch::~CaloTrackMatch(){}
// ============================================================================
StatusCode CaloTrackMatch::i_updateAlpha()
{
  // allow a user to disable the CondDB and reset the x-corrections with setProperty on the fly (if ever needed)
  if ( m_conditionName.empty() ){
    warning() << "attempt to update X-correction parameters by UpdMgrSvc while CondDB access disabled" << endmsg;
    return StatusCode::SUCCESS;
  }

  // safety protection against SIGSEGV
  if ( !m_cond ){ fatal() << "CaloTrackMatch::i_updateAlpha m_cond == 0" << endmsg; return StatusCode::FAILURE; }
  try {
    m_alphaPOut = m_cond->paramAsDoubleVect( "alphaPOut" );
    m_alphaNOut = m_cond->paramAsDoubleVect( "alphaNOut" );
    m_alphaPMid = m_cond->paramAsDoubleVect( "alphaPMid" );
    m_alphaNMid = m_cond->paramAsDoubleVect( "alphaNMid" );
    m_alphaPInn = m_cond->paramAsDoubleVect( "alphaPInn" );
    m_alphaNInn = m_cond->paramAsDoubleVect( "alphaNInn" );
  }
  catch ( GaudiException &exc ){
    fatal() << "X-correction update failed! msg ='" << exc << "'" << endmsg;
    return StatusCode::FAILURE;
  }
  if ( msgLevel(MSG::INFO) )
    debug() << "CondDB update of CaloTrackMatch X-correction parameters with '" << m_conditionName << "':"
            <<"\nalphaPOut: "<< Gaudi::Utils::toString ( m_alphaPOut ) 
            <<" alphaNOut: " << Gaudi::Utils::toString ( m_alphaNOut )
            <<"\nalphaPMid: "<< Gaudi::Utils::toString ( m_alphaPMid )
            <<" alphaNMid: " << Gaudi::Utils::toString ( m_alphaNMid )
            <<"\nalphaPInn: "<< Gaudi::Utils::toString ( m_alphaPInn )
            <<" alphaNInn: " << Gaudi::Utils::toString ( m_alphaNInn ) << endmsg;

  return StatusCode::SUCCESS;
}

// ============================================================================
StatusCode CaloTrackMatch::initialize()
{
  StatusCode sc = Calo::CaloTrackTool::initialize();
  if ( sc.isFailure() ) { return sc ; }

  if (! existDet<DataObject>(detSvc(), m_conditionName) ){
    warning() << "Condition '" << m_conditionName << "' not found -- switch off the use of the CondDB for CaloTrackMatch!" << endmsg; 
    m_conditionName = "";
  }
 
  if ( ! m_conditionName.empty() ){
    registerCondition(m_conditionName, m_cond, &CaloTrackMatch::i_updateAlpha);
    sc = runUpdate();  // ask UpdateManagerSvc to load the condition w/o waiting for the next BeginEvent incident
  }
  else
    warning() << "ConditionName empty -- reading of the CaloTrackMatch X-correction parameters from the CondDB has been disabled!" << endmsg;

  m_magFieldSvc = svc<ILHCbMagnetSvc>( "MagneticFieldSvc", true ) ;

  return StatusCode::SUCCESS;
}
// ============================================================================
// The END 
// ============================================================================


