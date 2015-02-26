#ifdef __INTEL_COMPILER         // Disable ICC warning
  #pragma warning(disable:654)  // ITrackExtrapolator::propagate" only partially overridden
  #pragma warning(push)
#endif
#include "LHCbMath/FastRoots.h"
#include "TrackRungeKuttaExtrapolator.h"
#ifdef __INTEL_COMPILER         // Re-enable ICC warning 654
  #pragma warning(pop)
#endif
#include "GaudiKernel/PhysicalConstants.h"
#include "GaudiKernel/ToolFactory.h"
#include "boost/optional.hpp"
#include <sstream>

//
//
// 
//       (   1   0   dXdTx0   dXdTy0     dXdQoP0   )
//       (   0   1   dYdTx0   dYdTy0     dYdQoP0   )
//   J = (   0   0   dTxdTx0  dTxdTy0    dTxdQoP0  )
//       (   0   0   dTydTx0  dTxdTy0    dTydQoP0  )
//       (   0   0      0        0         1       )
//
//


namespace {

  typedef ROOT::Math::SVector<double,4> RKTrackVector ;

  struct RKState
  {
    ROOT::Math::SVector<double,4> parameters;
    double qop ;
    double z ;
    double& x() { return parameters(0) ; }
    double& y() { return parameters(1) ; }
    double& tx() { return parameters(2) ; }
    double& ty() { return parameters(3) ; }
    
    // avoid function declared but not referenced warnings from icc
    //    double x() const { return parameters(0) ; }
    //    double y() const { return parameters(1) ; }
    double tx() const { return parameters(2) ; }
    double ty() const { return parameters(3) ; }

  } ;

  struct RKStage
  {
    RKState       state ;
    RKState       derivative ;
    TrackFieldExtrapolatorBase::FieldVector Bfield ;
  } ;

  struct RKCache
  {
    RKStage stage[7] ;
    int laststep ;
    int step ;
    RKCache() : laststep(-1), step(0) {}
  } ;


  struct RKJacobian
  {
    ROOT::Math::SMatrix<double,4,3> matrix;
    double& dXdTx0()  { return matrix(0,0) ; }
    double& dYdTx0()  { return matrix(1,0) ; }
    double& dTxdTx0() { return matrix(2,0) ; }
    double& dTydTx0() { return matrix(3,0) ; }
    
    double& dXdTy0()  { return matrix(0,1) ; }
    double& dYdTy0()  { return matrix(1,1) ; }
    double& dTxdTy0() { return matrix(2,1) ; }
    double& dTydTy0() { return matrix(3,1) ; }
    
    double& dXdQoP0()  { return matrix(0,2) ; }
    double& dYdQoP0()  { return matrix(1,2) ; }
    double& dTxdQoP0() { return matrix(2,2) ; }
    double& dTydQoP0() { return matrix(3,2) ; }

    // avoid function declared but not referenced warnings from icc
    //    double dXdTx0()  const { return matrix(0,0) ; }
    //    double dYdTx0()  const { return matrix(1,0) ; }
    double dTxdTx0() const { return matrix(2,0) ; }
    double dTydTx0() const { return matrix(3,0) ; }
    
    // avoid function declared but not referenced warnings from icc
    //    double dXdTy0()  const { return matrix(0,1) ; }
    //    double dYdTy0()  const { return matrix(1,1) ; }
    double dTxdTy0() const { return matrix(2,1) ; }
    double dTydTy0() const { return matrix(3,1) ; }
    
    // avoid function declared but not referenced warnings from icc
    //    double dXdQoP0()  const { return matrix(0,2) ; }
    //    double dYdQoP0()  const { return matrix(1,2) ; }
    double dTxdQoP0() const { return matrix(2,2) ; }
    double dTydQoP0() const { return matrix(3,2) ; }
  } ; 

  inline void addVector4( ROOT::Math::SVector<double,4>& lhs, 
                          double a, 
                          const ROOT::Math::SVector<double,4>& rhs)
  {
    // explicitely writes out m += a * n, because for some reason this is badly optimized on gcc 3.4
    lhs(0) += a * rhs(0) ;
    lhs(1) += a * rhs(1) ;
    lhs(2) += a * rhs(2) ;
    lhs(3) += a * rhs(3) ;
  }

  inline void assignVector4( ROOT::Math::SVector<double,4>& lhs, 
                             double a, 
                             const ROOT::Math::SVector<double,4>& rhs)
  {
    // explicitely writes out m = a * n, because for some reason this is badly optimized on gcc 3.4
    lhs(0) = a * rhs(0) ;
    lhs(1) = a * rhs(1) ;
    lhs(2) = a * rhs(2) ;
    lhs(3) = a * rhs(3) ;
  }

  inline void addMatrix43( ROOT::Math::SMatrix<double,4,3>& lhs, 
                           double a, 
                           const ROOT::Math::SMatrix<double,4,3>& rhs)
  {
    // explicitely writes out m += a * n, because for some reason this is badly optimized on gcc 3.4
    lhs(0,0) += a * rhs(0,0) ;
    lhs(1,0) += a * rhs(1,0) ;
    lhs(2,0) += a * rhs(2,0) ;
    lhs(3,0) += a * rhs(3,0) ;
    lhs(0,1) += a * rhs(0,1) ;
    lhs(1,1) += a * rhs(1,1) ;
    lhs(2,1) += a * rhs(2,1) ;
    lhs(3,1) += a * rhs(3,1) ;
    lhs(0,2) += a * rhs(0,2) ;
    lhs(1,2) += a * rhs(1,2) ;
    lhs(2,2) += a * rhs(2,2) ;
    lhs(3,2) += a * rhs(3,2) ;
    //   double* xlhs = &( m(0,0) ) ;
    //     const double* xrhs = &( n(0,0) ) ;
    //     for(int i=0;i<12; ++i, ++xlhs, ++xrhs)
    //       *xlhs += a * *xrhs ;
  }

  inline void assignMatrix43( ROOT::Math::SMatrix<double,4,3>& lhs, 
                              double a, 
                              const ROOT::Math::SMatrix<double,4,3>& rhs)
  {
    // explicitely writes out m = a * n, because for some reason this is badly optimized on gcc 3.4
    lhs(0,0) = a * rhs(0,0) ;
    lhs(1,0) = a * rhs(1,0) ;
    lhs(2,0) = a * rhs(2,0) ;
    lhs(3,0) = a * rhs(3,0) ;
    lhs(0,1) = a * rhs(0,1) ;
    lhs(1,1) = a * rhs(1,1) ;
    lhs(2,1) = a * rhs(2,1) ;
    lhs(3,1) = a * rhs(3,1) ;
    lhs(0,2) = a * rhs(0,2) ;
    lhs(1,2) = a * rhs(1,2) ;
    lhs(2,2) = a * rhs(2,2) ;
    lhs(3,2) = a * rhs(3,2) ;
  }

}

// *********************************************************************************************************
// Butcher tables for various adaptive Runge Kutta methods. These are all taken from wikipedia.
// *********************************************************************************************************

namespace CashKarp {
  static const double b5[6] = {37.0/378.0    , 0., 250.0/621.0,     125.0/594.0,     0.,            512.0/1771.0};
  static const double b4[6] = {2825.0/27648.0, 0., 18575.0/48384.0, 13525.0/55296.0, 277.0/14336.0, 1.0/4.0};
  static const double a[15] = {1.0/5.0, 
                         3.0/40.0,       9.0/40.0, 
                         3.0/10.0,      -9.0/10.0, 6.0/5.0,
                         -11.0/54.0,     5.0/2.0, -70.0/27.0, 35.0/27.0,
                         1631.0/55296.0, 175.0/512.0, 575.0/13824.0,44275.0/110592.0, 253.0/4096.0};
}

namespace DormandPrince { 
  static const double a[21] = {1/5.,
                         3/40.,          9/40.,
                         44/45.,        -56/15.,      32/9.,
                         19372/6561.,   -25360/2187., 64448/6561.,  -212/729.,
                         9017/3168.,    -355/33.,     46732/5247.,  49/176.,    -5103/18656.,
                         35/384.,        0.,          500/1113.,    125/192.,   -2187/6784.,   11/84. } ;
  // I think wikipedia swapped them here, but it doesn't seem to make much difference. I don't understand that.
  static const double b5[7] = {5179/57600.,    0.,          7571/16695.,  393/640.,   -92097/339200., 187/2100.,  1/40.} ;
  static const double b4[7] = {35/384.,        0.,          500/1113.,    125/192.,   -2187/6784.,    11/84.,     0.} ;
}

namespace Fehlberg {
  static const double a[15] = {1/4.,
                         3/32.,         9/32.,
                         1932/2197.,         -7200/2197.,         7296/2197.,
                         439/216.,         -8.,             3680/513.,         -845/4104.,
                         -8/27.,         2.,                 -3544/2565.,         1859/4104.,         -11/40. } ;
  static const double b4[6] = {25/216.,         0.,         1408/2565.,         2197/4104.,         -1/5.,            0. } ;
  static const double b5[6] = {16/135.,         0.,         6656/12825.,         28561/56430.,         -9/50.,    2/55. } ;
}

namespace BogackiShampine {
  static const double a[6] = {1/2.,
                        0.,   3/4.,
                        2/9., 1/3., 4/9. } ;
  static const double b2[4] = {7/24., 1/4.,         1/3.,         1/8.} ;
  static const double b3[4] = {2/9.,  1/3.,         4/9.,         0 } ;
}

namespace HeunEuler {
  static const double a[1]  = {1.} ;
  static const double b1[2] = {1.0, 0. } ;
  static const double b2[2] = {0.5, 0.5} ;
}

// *********************************************************************************************************


DECLARE_TOOL_FACTORY( TrackRungeKuttaExtrapolator )


TrackRungeKuttaExtrapolator::~TrackRungeKuttaExtrapolator()
{
}

/// TrackRungeKuttaExtrapolator constructor.
TrackRungeKuttaExtrapolator::TrackRungeKuttaExtrapolator(const std::string& type,
                                                         const std::string& name,
                                                         const IInterface* parent):
  TrackFieldExtrapolatorBase(type, name, parent)
{
  declareInterface<ITrackExtrapolator>( this );
  declareProperty( "Tolerance" , m_toleranceX =  0.001*Gaudi::Units::mm );
  declareProperty( "RelToleranceTx" , m_relToleranceTx = 5e-5 );
  //declareProperty( "AbsToleranceTx" , m_absToleranceTx = 1e-6 );
  declareProperty( "MinStep" , m_minRKStep        =  10*Gaudi::Units::mm );
  declareProperty( "MaxStep" , m_maxRKStep        =   1*Gaudi::Units::m );
  declareProperty( "InitialStep", m_initialRKStep =   1*Gaudi::Units::m );
  declareProperty( "MaxNumSteps" , m_maxNumRKSteps = 1000 );
  declareProperty( "StepScaleSafetyFactor", m_safetyFactor = 1.0 ) ;
  declareProperty( "Sigma", m_sigma = 5.5 ) ;
  //declareProperty( "MinStepScale", m_minStepScale = 0.26 ) ;
  declareProperty( "MinStepScale", m_minStepScale = 0.125 ) ;
  declareProperty( "MaxStepScale", m_maxStepScale = 4.0  ) ;
  declareProperty( "RKScheme", m_rkscheme = "CashKarp" ) ;
  declareProperty( "CorrectNumSteps", m_correctNumSteps = false ) ;
  declareProperty( "NumericalJacobian", m_numericalJacobian = false ) ;
  declareProperty( "MaxSlope"     , m_maxSlope      = 10. );
  declareProperty( "MaxCurvature", m_maxCurvature = 1/Gaudi::Units::m );
  
}

StatusCode
TrackRungeKuttaExtrapolator::finalize()
{
  if( UNLIKELY( msgLevel(MSG::DEBUG) ) ) {
    debug() << "Number of calls:     " << m_numcalls << endmsg ;
    debug() << "Min step length:     " << m_totalstats.minstep << endmsg ;
    debug() << "Max step length:     " << m_totalstats.maxstep << endmsg ;
    debug() << "Av step length:      " << m_totalstats.sumstep/(m_totalstats.numstep-m_totalstats.numfailedstep) << endmsg ;  
    debug() << "Av num step:         " << m_totalstats.numstep/double(m_numcalls) << endmsg ;
    debug() << "Fr. failed steps:    " << m_totalstats.numfailedstep/double(m_totalstats.numstep) << endmsg ;
    debug() << "Fr. increased steps: " << m_totalstats.numincreasedstep/double(m_totalstats.numstep) << endmsg ;
  }
  
  return TrackFieldExtrapolatorBase::finalize() ;
}

StatusCode
TrackRungeKuttaExtrapolator::initialize()
{
  StatusCode sc = TrackFieldExtrapolatorBase::initialize();
  if( UNLIKELY( msgLevel(MSG::DEBUG) ) )
    debug() << "Using RK scheme: " << m_rkscheme << endmsg ;
  if(        m_rkscheme == "CashKarp" ) {
    m_a  = CashKarp::a ;
    m_b4 = CashKarp::b4 ; 
    m_b5 = CashKarp::b5 ;
    m_firstSameAsLast = false ;
    m_numStages = 6 ;
  } else if( m_rkscheme == "Fehlberg" ) {
    m_a  = Fehlberg::a ;
    m_b4 = Fehlberg::b4 ; 
    m_b5 = Fehlberg::b5 ;
    m_firstSameAsLast = false ;
    m_numStages = 6 ;
  } else if( m_rkscheme == "DormandPrince" ) {
    m_a  = DormandPrince::a ;
    m_b4 = DormandPrince::b4 ; 
    m_b5 = DormandPrince::b5 ;
    m_firstSameAsLast = true ;
    m_numStages = 7 ;
  } else if( m_rkscheme == "BogackiShampine" ) {
    m_a  = BogackiShampine::a ;
    m_b4 = BogackiShampine::b2 ;
    m_b5 = BogackiShampine::b3 ;
    m_firstSameAsLast = false ;
    m_numStages = 4 ;
  } else if( m_rkscheme == "HeunEuler" ) {
    m_a  = HeunEuler::a ;
    m_b4 = HeunEuler::b1 ;
    m_b5 = HeunEuler::b2 ;
    m_firstSameAsLast = false ;
    m_numStages = 2 ;
  } else {
    error() << "No such RK scheme: \"" << m_rkscheme << "\"" << endmsg ;
    sc = StatusCode::FAILURE ;
  }
  
  // reset counters
  m_totalstats = RKStatistics() ;
  m_numcalls = 0 ;
  return sc ;
}

// Propagate a state vector from zOld to zNew
// Transport matrix is calulated when transMat pointer is not NULL
StatusCode 
TrackRungeKuttaExtrapolator::propagate( Gaudi::TrackVector& state,
                                        double zin,
                                        double zout,
                                        Gaudi::TrackMatrix* transMat,
                                        LHCb::ParticleID /*pid*/ ) const
{
  // Bail out if already at destination
  if( std::abs(zin-zout) < TrackParameters::propagationTolerance ) { 
    if( transMat ) *transMat = Gaudi::TrackMatrix( ROOT::Math::SMatrixIdentity() );
    return StatusCode::SUCCESS ;
  }
  
  boost::optional<RKJacobian> jacobian; 
  if (transMat) jacobian = RKJacobian();

  // translate the state to one we use in the runge kutta. note the factor c.
  RKState rkstate = { { state(0), state(1), state(2), state(3) },
      state(4) * Gaudi::Units::c_light, zin };

  StatusCode sc = StatusCode::SUCCESS ;
  RKErrorCode success = m_numericalJacobian && jacobian  
    ? extrapolateNumericalJacobian( rkstate, zout, *jacobian) 
    : extrapolate( rkstate, zout, jacobian.get_ptr() ) ;
  if( success == RKSuccess ) {
    // translate the state back
    state(0) = rkstate.x() ;
    state(1) = rkstate.y() ;
    state(2) = rkstate.tx() ;
    state(3) = rkstate.ty() ;

    if( transMat ) {
      *transMat = Gaudi::TrackMatrix() ;
      (*transMat)(0,0) = 1 ;
      (*transMat)(1,1) = 1 ;
      (*transMat)(4,4) = 1 ;
      for( int irow=0; irow<4; ++irow) {
        for( int icol=0; icol<3; ++icol) 
          (*transMat)(irow,icol+2) = jacobian->matrix(irow,icol) ;
        (*transMat)(irow,4) *= Gaudi::Units::c_light ;
      }
    }
  } else {
    std::stringstream str ;
    str << "RungeKuttaExtrapolator failed with code: " << success ;
    sc = Warning(str.str(),StatusCode::FAILURE,0) ;
  }
  
  return sc ;
}


TrackRungeKuttaExtrapolator::RKErrorCode
TrackRungeKuttaExtrapolator::extrapolate( RKState& state,
                                          double zout,
                                          RKJacobian* jacobian,
                                          std::vector<double>* stepvector) const
{
  ++m_numcalls ;
  // initialize the jacobian
  if(jacobian) {
    jacobian->dTxdTx0() = 1 ;
    jacobian->dTydTy0() = 1 ;
  }
  
  // now start stepping. first try with a single step. this may not be
  // very optimal inside the magnet.
  double totalStep = zout - state.z ;
  //double toleranceTx = std::abs(m_toleranceX/totalStep) ;
  double toleranceX  = m_toleranceX ;
  double toleranceTx = toleranceX/std::abs(totalStep) ;

  double absstep = std::min( std::abs(totalStep), m_initialRKStep ) ;
  int direction = totalStep > 0 ? +1 : -1 ;
  bool laststep = absstep < m_minRKStep ;
  
  //std::cout << "input state vector: " << state.vector << std::endl ;
  //double lasterr(0) ;

  RKCache       rkcache ;
  RKTrackVector err, totalErr ;
  RKStatistics  stats ;
  RKErrorCode rc = RKSuccess ;

  while( rc==RKSuccess && std::abs(state.z - zout) > TrackParameters::propagationTolerance ) {
    
    // make a single range-kutta step
    RKState prevstate = state ;
    evaluateRKStep( absstep * direction, state, err, rkcache ) ;

    // decide if the error is small enough
     
    // always accept the step if it is smaller than the minimum step size
    bool success = (absstep <= m_minRKStep) ;
    if( !success ) {

      if ( m_correctNumSteps ) {
        double estimatedN = std::abs(totalStep) / absstep ;
        toleranceX  = (m_toleranceX/estimatedN/m_sigma) ;
        toleranceTx = toleranceX/std::abs(totalStep) ;
        //(m_toleranceX/10000)/estimatedN/m_sigma ;
      }
      
      // apply the acceptance criterion.
      double normdx  = std::abs( err(0) ) / toleranceX ;
      double normdy  = std::abs( err(1) ) / toleranceX ;
      double deltatx = state.tx() - prevstate.tx() ;
      double normdtx = std::abs( err(2) ) / ( toleranceTx + std::abs( deltatx ) * m_relToleranceTx ) ;
      double errorOverTolerance = std::max( normdx, std::max( normdy, normdtx ) ) ;
      success = (errorOverTolerance <= m_sigma) ;
      //     std::cout << "step: " << rkcache.step << " " << success << " " 
      //                 << prevstate.z << " "
      //                 << state.z << " " << absstep << " "
      //                 << errorOverTolerance << std::endl ;

      // do some stepping monitoring, before adapting step size
      if(success) {
        stats.sumstep += absstep ;
        if(!laststep) stats.minstep = std::min( stats.minstep, absstep ) ;
        stats.maxstep = std::max( stats.maxstep, absstep ) ;
      } else {
        ++stats.numfailedstep ;
      }
      
      // adapt the stepsize if necessary. the powers come from num.recipees.
      double stepfactor(1) ;
      if( errorOverTolerance > 1 ) { // decrease step size
        stepfactor = std::max( m_minStepScale, m_safetyFactor / std::sqrt(std::sqrt(errorOverTolerance))); // was : * std::pow( errorOverTolerance , -0.25 ) ) ;
      } else {                       // increase step size
        if( errorOverTolerance > 0 ) 
          stepfactor = std::min( m_maxStepScale, m_safetyFactor * FastRoots::invfifthroot(errorOverTolerance) ) ; // was: * std::pow( errorOverTolerance, -0.2) ) ;
        else 
          stepfactor = m_maxStepScale ;
        ++stats.numincreasedstep ;
      }
      absstep *= stepfactor ;
      
      // apply another limitation criterion
      absstep = std::min(absstep, m_maxRKStep);
      absstep = std::max(absstep, m_minRKStep);

    }
    
    if(success) {
      // if we need the jacobian, evaluate it only for successful steps
      double thisstep = state.z - prevstate.z ; // absstep has already been changed!
      if( jacobian ) evaluateRKStepJacobian( thisstep, *jacobian, rkcache ) ;
      // update the step, to invalidate the cache (or reuse the last stage)
      rkcache.step += 1 ;
      if(stepvector) stepvector->push_back( thisstep ) ;
      //addVector4(totalErr,1,err) ;
      stats.err += err ;
    } else {
      // if this step failed, don't update the state
      state = prevstate ;
    }
    
    // check that we don't step beyond the target
    if( absstep - direction * (zout - state.z) > 0 ) {
      absstep = std::abs(zout - state.z) ;
      laststep = true ;
    } 

    // final check: bail out for vertical or looping tracks
    if( std::max(std::abs(state.tx()), std::abs(state.ty())) > m_maxSlope ) {
      if( UNLIKELY( msgLevel(MSG::DEBUG) ) )
        debug() << "State has very large slope, probably curling: tx, ty = "
                << state.tx() << ", " << state.ty() 
                << " z_origin, target, current: "
                << zout - totalStep << " " << zout << " " << state.z 
                << endmsg ;
      rc = RKCurling ;
    } else if( std::abs(state.qop * rkcache.stage[0].Bfield.y() ) > m_maxCurvature ) {
      if( UNLIKELY( msgLevel(MSG::DEBUG) ) )
        debug() << "State has too small curvature radius: "
                << state.qop * rkcache.stage[0].Bfield.y() 
                << " z_origin, target, current: "
                << zout - totalStep << " " << zout << " " << state.z 
                << endmsg ;
      rc = RKCurling ;
    } else if( stats.numfailedstep + rkcache.step  >= m_maxNumRKSteps ) {
      if( UNLIKELY( msgLevel(MSG::DEBUG) ) )
        debug() << "Exceeded max numsteps. " << endmsg ;
      rc = RKExceededMaxNumSteps ;
    }
  }

  stats.numstep = rkcache.step ;
  m_stats = stats ;
  m_totalstats += stats ;
  
  return rc ;
}


void TrackRungeKuttaExtrapolator::evaluateRKStep( double dz,
                                                  RKState& pin,
                                                  RKTrackVector& err,
                                                  RKCache& cache) const 
{
  //  debug() << "z-component of input: " 
  //          << pin.z << " " << dz << endmsg ;
  RKTrackVector k[7] ;
  int firststage(0) ;
  
  // previous step failed, reuse the first stage
  if( cache.laststep == cache.step ) {
    firststage = 1 ;
    //k[0]       = dz * cache.stage[0].derivative.parameters ;
    assignVector4( k[0], dz, cache.stage[0].derivative.parameters ) ;
    //assert( std::abs(pin.z - cache.stage[0].state.z) < 1e-4 ) ;
  }
  // previous step succeeded and we can reuse the last stage (Dormand-Prince)
  else if ( cache.step > 0 && m_firstSameAsLast ) {
    firststage = 1 ;
    cache.stage[0] = cache.stage[m_numStages-1] ;
    //k[0]           = dz * cache.stage[0].derivative.parameters ;
    assignVector4( k[0], dz, cache.stage[0].derivative.parameters ) ;
  }
  cache.laststep = cache.step ;

  for( size_t m = firststage ; m!=m_numStages ; ++m ) {
    // evaluate the state
    cache.stage[m].state = pin ;
    for( size_t n=0; n<m; ++n) {
      auto index = m*(m-1)/2 + n ;
      cache.stage[m].state.z          += m_a[index] * dz ;
      //cache.stage[m].state.parameters += m_a[index] * k[n] ;
      addVector4( cache.stage[m].state.parameters, m_a[index], k[n]) ;
    }
    
    // evaluate the derivatives
    //std::cout << "stage " << m << " --> " << cache.stage[m].state.z << std::endl ;
    cache.stage[m].Bfield = fieldVector( Gaudi::XYZPoint(cache.stage[m].state.x(), 
                                                         cache.stage[m].state.y(), 
                                                         cache.stage[m].state.z ) ) ;
    evaluateDerivatives( cache.stage[m].state, cache.stage[m].Bfield, cache.stage[m].derivative ) ;
    
    //k[m]   = dz * cache.stage[m].derivative.parameters ;
    assignVector4(k[m],dz,cache.stage[m].derivative.parameters) ;
  }
  
  // update state and error
  err(0) = err(1) = err(2) = err(3) = 0 ;
  for( size_t m = 0 ; m!=m_numStages; ++m ) {
    // this is the difference between the 4th and 5th order
    //err            += (m_b5[m] - m_b4[m] ) * k[m] ;
    addVector4(err, (m_b5[m] - m_b4[m]), k[m]) ;
    // this is the fifth order change
    //pin.parameters += m_b5[m] * k[m] ;
    addVector4(pin.parameters, m_b5[m], k[m]) ;
  }
  
  pin.z += dz ;
}


void TrackRungeKuttaExtrapolator::evaluateRKStepJacobian( double dz,
                                                          RKJacobian& jacobian,
                                                          const RKCache& cache) const
{
  // evaluate the jacobian. not that we never resue last stage
  // here. that's not entirely consistent (but who cares)
  RKJacobian derivative, jtmp;
  ROOT::Math::SMatrix<double,4,3> k[7] ;
  for( size_t m = 0 ; m<m_numStages; ++m ) {
    // evaluate the derivative
    jtmp = jacobian ;
    for( size_t n=0; n<m; ++n) {
      auto index = m*(m-1)/2 + n ;
      //jtmp.matrix += m_a[index] * k[n] ;
      addMatrix43( jtmp.matrix, m_a[index],  k[n] ) ;
    }
    
    // evaluate the derivatives. reuse the parameters and bfield from the cache
    evaluateJacobianDerivatives( cache.stage[m].state, jtmp, cache.stage[m].Bfield, derivative ) ;
    //k[m] = dz * derivative.matrix ;
    assignMatrix43( k[m], dz,  derivative.matrix) ;
  }
  
  for( size_t m = 0 ; m<m_numStages; ++m ) 
    //jacobian.matrix += m_b5[m] * k[m] ;
    addMatrix43( jacobian.matrix, m_b5[m], k[m]  ) ;
}

void
TrackRungeKuttaExtrapolator::evaluateDerivatives(const RKState& state,
                                                 const FieldVector& field,
                                                 RKState& deriv ) const
{
  double tx  = state.tx() ;
  double ty  = state.ty() ;
  double qop = state.qop ;
  double Bx = field.x() ;
  double By = field.y() ;
  double Bz = field.z() ;

  double norm = std::sqrt( 1 + tx*tx + ty*ty ) ;
  double Ax = norm * (  ty * ( tx*Bx + Bz ) - ( 1 + tx*tx) * By ) ;
  double Ay = norm * ( -tx * ( ty*By + Bz ) + ( 1 + ty*ty) * Bx ) ;
  
  // this is 'dState/Dz'
  deriv.x() = tx ;
  deriv.y() = ty ;
  deriv.tx() = qop * Ax ;
  deriv.ty() = qop * Ay ;
  deriv.qop = 0 ;
  deriv.z   = 1 ;
}


void                 
TrackRungeKuttaExtrapolator::evaluateJacobianDerivatives( const RKState& state,
                                                          const RKJacobian& jacobian,
                                                          const FieldVector& field,
                                                          RKJacobian& jacobianderiv ) const
{
  double Bx = field.x() ;
  double By = field.y() ;
  double Bz = field.z() ;
  double tx   = state.tx() ;
  double ty   = state.ty() ;
  double qop  = state.qop ;
  double tx2 = tx*tx ;
  double ty2 = ty*ty ;
  double n2 = 1 + tx2 + ty2 ;
  double n  = std::sqrt( n2 ) ;
  double Ax = n * (  ty * (tx*Bx + Bz ) - (1+tx2) * By ) ;
  double Ay = n * ( -tx * (ty*By + Bz ) + (1+ty2) * Bx ) ;
  
  // now we compute 'dJacobian/dZ'
  double dAxdTx = Ax*tx/n2 + n * ( ty * Bx - 2*tx*By ) ;
  double dAxdTy = Ax*ty/n2 + n * ( tx* Bx + Bz ) ;
  
  double dAydTx = Ay*tx/n2 + n * ( -ty*By - Bz ) ;
  double dAydTy = Ay*ty/n2 + n * ( -tx*By + 2*ty * Bx) ;

  // we'll do the factors of c later
  
  // derivatives to Tx0
  // derivatives to Tx0
  jacobianderiv.dXdTx0()  = jacobian.dTxdTx0() ;
  jacobianderiv.dYdTx0()  = jacobian.dTydTx0() ;
  jacobianderiv.dTxdTx0() = qop * (  jacobian.dTxdTx0() * dAxdTx +
                                     jacobian.dTydTx0() * dAxdTy ) ;
  jacobianderiv.dTydTx0() = qop * (  jacobian.dTxdTx0() * dAydTx +
                                     jacobian.dTydTx0() * dAydTy ) ;
  
  // derivatives to Ty0
  jacobianderiv.dXdTy0()  = jacobian.dTxdTy0() ;
  jacobianderiv.dYdTy0()  = jacobian.dTydTy0() ;
  jacobianderiv.dTxdTy0() = qop * (  jacobian.dTxdTy0() * dAxdTx +
                                     jacobian.dTydTy0() * dAxdTy ) ;
  jacobianderiv.dTydTy0() = qop * (  jacobian.dTxdTy0() * dAydTx +
                                     jacobian.dTydTy0() * dAydTy ) ;

  // derivatives to qopc
  jacobianderiv.dXdQoP0()  = jacobian.dTxdQoP0() ;
  jacobianderiv.dYdQoP0()  = jacobian.dTydQoP0() ;
  jacobianderiv.dTxdQoP0() = Ax + qop * (  jacobian.dTxdQoP0() * dAxdTx +
                                           jacobian.dTydQoP0() * dAxdTy ) ;
  jacobianderiv.dTydQoP0() = Ay + qop * (  jacobian.dTxdQoP0() * dAydTx +
                                           jacobian.dTydQoP0() * dAydTy ) ;
  
}

TrackRungeKuttaExtrapolator::RKErrorCode
TrackRungeKuttaExtrapolator::extrapolateNumericalJacobian( RKState& state,
                                                           double zout,
                                                           RKJacobian& jacobian) const
{
  // call the stanndard method but store the steps taken
  size_t cachednumstep(m_stats.numstep),cachednumfailedstep(m_stats.numfailedstep) ;

  RKState inputstate(state) ;
  std::vector<double> stepvector;
  stepvector.reserve(256) ;
  RKErrorCode success = extrapolate(state,zout,&jacobian,&stepvector) ;
  if ( success==RKSuccess ) {
    // now make small changes in tx,ty,qop
    double delta[3] = {0.01,0.01,1e-8} ;
    for(int col=0; col<3; ++col) {
      RKState astate(inputstate) ;
      switch(col) {
      case 0: astate.tx() += delta[0] ; break ;
      case 1: astate.ty() += delta[1] ; break ;
      case 2: astate.qop  += delta[2] ; break ;
      }
      RKCache cache ;
      RKTrackVector err ;
      for(size_t j=0; j<stepvector.size() ; ++j) {
        evaluateRKStep( stepvector[j],astate,err,cache ) ; 
        ++cache.step ;
      }
      if( !(std::abs(state.z - astate.z) < TrackParameters::propagationTolerance ) ) {
        std::cout << "problem in numerical integration. " << std::endl ;
        std::cout << "zin: " << inputstate.z << " "
                  << " zout: " << zout << " "
                  << " state.z: " << state.z << " "
                  << " dstate.z: " << astate.z << std::endl ;
        std::cout << "num step: "
                  << stepvector.size() << " "
                  << m_stats.numstep - cachednumstep << " "
                  << m_stats.numfailedstep - cachednumfailedstep << std::endl ;
      }
      assert(std::abs(state.z - astate.z) < TrackParameters::propagationTolerance ) ;
      
      for(int row=0; row<4; ++row) 
        jacobian.matrix(row,col) = (astate.parameters(row) - state.parameters(row)) / delta[col] ;
    }
  }
  return success ;
}
