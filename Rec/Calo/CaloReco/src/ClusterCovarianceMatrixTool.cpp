// $Id: ClusterCovarianceMatrixTool.cpp,v 1.6 2008-06-30 15:36:33 odescham Exp $
// ============================================================================
// CVS tag $Name: not supported by cvs2svn $ 
// ============================================================================
// Include files
// GaudiKernel
#include "GaudiKernel/ToolFactory.h"
// CaloDet 
#include "CaloDet/DeCalorimeter.h"
//Event
#include "Event/CaloCluster.h"
// CaloUtil
#include "CaloUtils/CovarianceEstimator.h"
// local
#include "ClusterCovarianceMatrixTool.h"

// ============================================================================
/** @file
 *
 *  Implementation file for class ClusterCovarianceMatrixTool
 *
 *  @date 02/11/2001 
 *  @author Vanya Belyaev Ivan.Belyaev@itep.ru
 */
// ============================================================================

DECLARE_TOOL_FACTORY( ClusterCovarianceMatrixTool );

// ============================================================================
/** Standard constructor
 *  @param type tool type (useless)
 *  @param name tool name
 *  @param parent pointer to parent object (service, algorithm or tool)  
 */
// ============================================================================
ClusterCovarianceMatrixTool::ClusterCovarianceMatrixTool
( const std::string& type   ,
  const std::string& name   ,
  const IInterface*  parent )
  : GaudiTool( type , name , parent )
    , m_estimator (      ) 
    , m_a         ( "", 0.09 )
    , m_gainErr   ( "", 0    )
    , m_noiseIn   ( "", 0    ) 
    , m_noiseCo   ( "", 0    )
    , m_detData   ( DeCalorimeterLocation::Ecal )
{
  // interface!
  declareInterface<ICaloClusterTool> (this);
  // try to get properties from the parent 
  DoubleProperty resolution      ( "Resolution"      , m_a       ) ;
  DoubleProperty gainError       ( "GainError"       , m_gainErr ) ;
  DoubleProperty noiseIncoherent ( "NoiseIncoherent" , m_noiseIn ) ;
  DoubleProperty noiseCoherent   ( "NoiseCoherent"   , m_noiseCo ) ;
  StringProperty detData         ( "Detector"        , m_detData ) ;
  ///
  if( 0 != parent )
    {
      const IProperty* prop = 
        dynamic_cast<const IProperty*> ( parent );
      if( 0 != prop )
        {
          StatusCode sc;
          sc=prop->getProperty( &resolution      );
          sc=prop->getProperty( &gainError       );
          sc=prop->getProperty( &noiseIncoherent );
          sc=prop->getProperty( &noiseCoherent   );
          sc=prop->getProperty( &detData         );
          if(sc.isFailure())warning()<<"Unable to get properties"<<endreq;
        }
    }
  ///
  m_a       = resolution      .value() ;
  m_gainErr = gainError       .value() ;
  m_noiseIn = noiseIncoherent .value() ;
  m_noiseCo = noiseCoherent   .value() ;
  m_detData = detData         .value() ;
  ///
  declareProperty( "Resolution"       , m_a       );
  declareProperty( "GainError"        , m_gainErr );
  declareProperty( "NoiseIncoherent"  , m_noiseIn );
  declareProperty( "NoiseCoherent"    , m_noiseCo );
  declareProperty( "Detector"        , m_detData  );
};
// ============================================================================
/** destructor, virtual and protected 
 */
// ============================================================================
ClusterCovarianceMatrixTool::~ClusterCovarianceMatrixTool()
{
};

// ============================================================================
/** standard initialization method 
 *  @return status code 
 */
// ============================================================================
StatusCode ClusterCovarianceMatrixTool::initialize ()
{
  StatusCode sc = GaudiTool::initialize ();
  /// 
  if( sc.isFailure() ) 
    { return Error("Could not initialize the base class!") ; }
  ///
  m_det = getDet<DeCalorimeter>( m_detData) ;
  
  m_estimator.setDetector( m_det     ) ;
  m_estimator.setA       ( m_a       ) ;
  m_estimator.setGainS   ( m_gainErr ) ;
  m_estimator.setNoiseIn ( m_noiseIn ) ;
  m_estimator.setNoiseCo ( m_noiseCo ) ;
  
  info()      << " Has initialized with parameters: "              << endreq 
              << " \t 'Detector'         = '" << m_detData << "'"  << endreq 
              << " \t 'Resolution'       = '" << m_a       << "'"  << endreq 
              << " \t 'GainError'        = '" << m_gainErr << "'"  << endreq 
              << " \t 'NoiseIncoherent'  = '" << m_noiseIn << "'"  << endreq 
              << " \t 'NoiseCoherent'    = '" << m_noiseCo << "'"  << endreq 
              << " \t Estimator is          " << m_estimator       << endreq ;
  return StatusCode::SUCCESS ;
};
// ============================================================================

// ============================================================================
/** standard finalization method 
 *  @return status code 
 */
// ============================================================================
StatusCode ClusterCovarianceMatrixTool::finalize   ()
{  
  return GaudiTool::finalize ();
};
// ============================================================================

// ============================================================================
/** The main processing method (functor interface) 
 *  @param cluster pointer to CaloCluster object to be processed
 *  @return status code 
 */  
// ============================================================================
StatusCode 
ClusterCovarianceMatrixTool::operator() ( LHCb::CaloCluster* cluster ) const 
{
  /// check the argument 
  if( 0 == cluster                ) 
    { return Error( "CaloCluster*   points to NULL!") ; }
  if( 0 == m_estimator.detector() ) 
    { return Error( "DeCalorimeter* points to NULL!") ; }
  /// apply the estimator 
  return m_estimator( cluster );
};
// ============================================================================

// ============================================================================
/** The main processing method 
 *  @param cluster pointer to CaloCluster object to be processed
 *  @return status code 
 */  
// ============================================================================
StatusCode 
ClusterCovarianceMatrixTool::process ( LHCb::CaloCluster* cluster ) const 
{ return (*this)( cluster ); };
// ============================================================================

// ============================================================================
// The End 
// ============================================================================


