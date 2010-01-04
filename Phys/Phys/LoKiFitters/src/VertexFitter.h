// $Id: VertexFitter.h,v 1.5 2010-01-04 16:50:56 ibelyaev Exp $
// ============================================================================
#ifndef LOKIFITTERS_VERTEXFITTER_H 
#define LOKIFITTERS_VERTEXFITTER_H 1
// ============================================================================
// Include files
// ============================================================================
// STD & STL 
// ============================================================================
#include <algorithm>
// ============================================================================
// GaudiKernel
// ============================================================================
#include "GaudiKernel/ToolFactory.h"
// ============================================================================
// GaudiAlg
// ============================================================================
#include "GaudiAlg/GaudiTool.h"
// ============================================================================
// Event 
// ============================================================================
#include "Event/Particle.h"
#include "Event/Vertex.h"
// ============================================================================
// Kernel
// ============================================================================
#include "Kernel/IVertexFit.h"
#include "Kernel/IParticleTransporter.h"
#include "Kernel/IParticlePropertySvc.h"
// ============================================================================
#include "Kernel/NodesPIDs.h"
// ============================================================================
// LHCbMath
// ============================================================================
#include "LHCbMath/MatrixUtils.h"
#include "LHCbMath/MatrixTransforms.h"
// ============================================================================
// ROOT/Mathlib
// ============================================================================
#include "Math/Functions.h"
// ============================================================================
// Local 
// ============================================================================
#include "KalmanFilter.h"
// ============================================================================
namespace LoKi
{
  // ==========================================================================
  /** @class VertexFitter 
   *  the most trivial implementation of Kalman-filter based vertex fitter.
   *  It should be very efficient (CPU).
   * 
   *  @todo proper implementation of IVertexFit:remove 
   *
   *  <b>"Vertex Fitter as Kalman Filter"</b>
   *
   *  Let  \f$\vec{p}_k\f$ be a vector containing the parameters of the 
   *  \f$k\f$-track. Let  \f$V_k\f$ be the covariance matrix of these parameters, 
   *  and  \f$G_k\f$ is <b>the formal</b> inverse matrix: \f$G_k = V_k^{-1}\f$.
   *  For the trivial case of \f$dim(\vec{p}_k)=5\f$ both matrices 
   *  \f$V_k\f$ and \f$G_k\f$ could be defined properly and are non-singular.
   *
   *  Here we'll try to extend the standard Kalman Filter formalism 
   *  for <b>almost</b> linear model, by embedding the 
   *  minimal \f$dim=5\f$-model into more dimensions. 
   *  In particular we choose the \f$dim(\vec{p}_k)=7\f$:
   *  \f[ \vec{p} = p^T = 
   *      \left( \vec{x} , \vec{q} \right) = 
   *      \left( x , y , z , p_x , p_y , p_z , E \right)  
   *  \f] 
   *
   * The vertex fit determines the new parameters 
   * \f$\vec{x}\f$- position of the common vertex and
   * \f$\vec{q}\f$- 4-momentum of the track, 
   * constrained to originate in \f$\vec{x}\f$.
   *
   * 
   * The covariance matrix of fit parameters could be split into sub-matrices:
   *
   *  - the vertex part \f$ 3\times3\f$:       \f$C = cov\left( \vec{x} \right)\f$  
   *  - particle momentum part \f$4\times4\f$: \f$D_k = cov\left( \vec{q}_k \right)\f$ 
   *  - correlation between the vertex position and the particle 
   *    momentum part \f$3\times4\f$: \f$E_k = cor\left( \vec{x},\vec{q}_k \right)\f$ 
   *
   *  - correlation between the momenta of different particles \f$4\times4\f$:
   *    \f$S_{k,l} = cor\left( \vec{q}_l,\vec{q}_k \right)\f$  
   *
   *  Such choice of \f$\vec{p}\f$ and \f$\vec{q}\f$ 
   *  allows to obtain almost trivial projection matrices \f$A\f$ and 
   *  \f$B\f$:
   *  \f[ \vec{p}_k \left( \vec{x}_k , \vec{q}_k \right) = 
   *      c_k^0 +  A_k \vec{x} + B_k \vec{q},  \f]
   *  where 
   *  \f$ A_k = \left[ \frac{\partial \vec{p} }{\partial \vec{x}} \right] \f$ and 
   *  \f$ B_k  = \left[ \frac{\partial \vec{p} }{\partial \vec{q}} \right] \f$.
   *  Clearly with the the chosen  parameterization of \f$\vec{p}\f$ one gets:
   *  \f$A_k = A = \bigl( \begin{smallmatrix}1 \\ 0\end{smallmatrix}\bigr)\f$, 
   *  \f$B_k = B = \bigl( \begin{smallmatrix}0 \\ 1\end{smallmatrix}\bigr)\f$. 
   *
   *
   *  The Kalman filter works as:
   * 
   *  - \f$ C^{-1}_k = C^{-1}_{k-1} + A^T_{k}G_{k}^{B}A_{k} \f$ 
   *  - \f$ E_k      = -F_{k}C_{k}  \f$ 
   *  - \f$ D_{k}    = W_{k} - E_{k}F^{T}_{k} \f$
   *  
   *  where:
   *  
   *  - \f$ W_{k}     = \left( B^T_{k}G_{k}B_{k}\right)^{-1}  \f$ 
   *  - \f$ G^{B}_{k} = G_{k} - G_{k}B_{k}W_{k}B^T_{k}G_{k} \f$ 
   *  - \f$ F_{k}     = W_{k}B^T_{k}G_{k}A_{k} \f$ 
   *
   *  New fit values are:
   *
   * - \f$ \vec{x}_{k} = C_{k}\left(C^{-1}_{k-1}\vec{x}{k-1}
   *           + A^T_{k}G^B_{k}\left(\vec{p}_{k}-c^0_{k}\right)\right) \f$
   * - \f$ \vec{q}_{k} = W_{k}B^T_{k}G_{k}\left(\vec{p}_{k}
   *           - c^0_k-A_k\vec{x}_k\right) \f$ 
   * 
   *  New \f$\chi^2\f$ is :
   *   \f$ \chi^2_{k} = \chi^2_{k-1} + \chi_1 +\chi_2\f$, where 
   *  - \f$ \chi_1 = \vec{\delta_1}^{T} G_k \vec{\delta_1} \f$ 
   *  - \f$ \chi_2 = \vec{\delta_2}^{T} C^{-1}_{k-1} \vec{\delta_2} \f$ 
   *  - \f$ \vec{\delta}_1 = \vec{p}-c^0_{k}-A_{k}\vec{x}-B_{k}\vec{q}_{k}\f$ 
   *  - \f$ \vec{\delta}_2 = \vec{x}_{k}-\vec{x}_{k-1} \f$ 
   * 
   *  The Kalman smoothing step is fairly trivial:
   *  
   *  - \f$ \vec{x}^{n}_{k} = \vec{x}_{n} \f$
   *  - \f$ \vec{q}^{n}_{k} = W_{k}B^T_{k}G_{k}\left[\vec{p}-A_{k}\vec{x}_{n}\right] \f$
   *  
   *  After the iteration the covariance matrices are calculated as:
   *  
   *  - \f$ C^{n}_{k} = C_{n} \f$ 
   *  - \f$ E^{n}_{k} = -F_{k}C_{n} \f$ 
   *  - \f$ D^{n}_{k} = W_k - E^{n}_{k}F^T_{k} \f$ 
   *  - \f$ cov(\vec{q}^{n}_{k},\vec{q}^{n}_{l}) = F_{k}C_{n}F^T_{l} \f$ 
   *
   *  Since for our parameterization 
   *  \f$A_k = A = \bigl( \begin{smallmatrix}1 \\ 0\end{smallmatrix}\bigr)\f$, 
   *  \f$B_k = B = \bigl( \begin{smallmatrix}0 \\ 1\end{smallmatrix}\bigr)\f$, 
   *  all formulare become relaively simle:
   *  
   *  Assuming that 
   *  \f$ G_{k} = \bigl( \begin{smallmatrix} 
   *       G_x  & G_{xp} \\ G^T_{xp} & G_{p} \end{smallmatrix}\bigr), \f$
   *  one has
   *  
   *  - \f$ W_k = G_p^{-1} \f$ 
   *  - \f$ G^B_k = \bigl( \begin{smallmatrix}
   *   G_x - G_{xp}G_{p}^{-1}G^{T}_{xp} & 0 \\ 0 & 0 \end{smallmatrix} \bigr) \f$ 
   *  - \f$ F_k       = G_p^{-1} G^{T}_{xp} \f$ 
   *  - \f$ C_k^{-1}  = C_{k-1}^{-1} + \left( G_x - G_{xp}G_p^{-1}G_{xp}^T\right) \f$ 
   *  - \f$ D_k       = G_p^{-1}+ G_p^{-1}G^T_{xp}C_kG_{xp}G_p^{-1} \f$ 
   *  - \f$ \vec{x}_x = C_k\left( C^{-1}_{k-1}\vec{x}_{k-1}+
   *            \left( G_x - G_{xp}G^{-1}_{p}G^{T}_{xp} , 0 \right) 
   *            \left( \vec{p}_k - c^0_k \right) \right) \f$ 
   *  - \f$ \chi^2_{k} = \chi^2_{k-1} + \delta_{\vec{x}}^T 
   *           \left[ G_x - G_{xp}G_{p}^{-1}G^T_{xp} \right] \delta_{\vec{x}} 
   *           + ... \f$ 
   *
   *
   *  In an obvious way:
   *
   *  - \f$ G_x - G_{xp}G_{p}^{-1}G_{xp} = V^{-1}_{x}     \f$ 
   *  - \f$ G_{p}^{-1} = V_{p} - V^T_{xp}V^{-1}_{x}V_{xp} \f$ 
   *  - \f$ G^{-1}G^T_{xp} = -V^T_{xp}V^{-1}_{x}          \f$ 
   *
   *  @author Vanya BELYAEV ibelyaev@physics.syr.edu
   *  @date 2006-05-24
   */
  // ==========================================================================
  class VertexFitter 
    : public          GaudiTool 
    , public virtual IVertexFit 
  {
    // ========================================================================
    /// the friend factory for instantiation 
    friend class ToolFactory<VertexFitter> ;
    // ========================================================================
  public:
    // ========================================================================
    /// error codes
    enum {
      // ======================================================================
      /// Invalid Particle 
      InvalidParticle         = 701 ,                       // Invalid Particle 
      /// Invalid Data
      InvalidData             = 702 ,                       //     Invalid Data 
      /// Error in Matrix Inversion 
      ErrorInMatrixInversion  = 703 ,             //  Error in Matrix Inversion
      /// No Convergency
      NoConvergency           = 710 ,             // No Convergency is detected 
      /// Not Implemented Yet 
      NotImplemented          = 720               //        not yet implemented 
      // ======================================================================
    } ;
    // ========================================================================
  protected:
    // ========================================================================
    typedef LoKi::KalmanFilter::Entry    Entry    ;
    typedef LoKi::KalmanFilter::Entries  Entries  ;
    typedef Entries::iterator            EIT      ;
    // ========================================================================
  public:  
    // ========================================================================
    /** The vertex fitting method without creation of a Particle 
     *
     *  @code
     *
     *  // locate  the tool 
     *  const IVertexFit* fitter = tool<IVertexFit>( ... ) ;
     * 
     *  // container of particles to be fitetr into common vertex 
     *  LHCb::Particle::ConstVector daughters = ... ;
     *  
     *  Vertex vertex ;
     *  StatusCode sc = fitter->fit( daughters , vertex ) 
     *  if( sc.isFailure() ) { Warning("Error in vertex fit", sc ) ; }
     *   
     *  // check chi2 of common vertex fit 
     *  double chi2 = vertex->chi2() ;
     *
     *  @endcode 
     *
     *  @see IVertexFit 
     *  @see Particle
     *  @see Vertex
     *  @see GaudiAlgorithm::tool
     *  @see GaudiAlgorithm::Warning
     *
     *  @param daughters vector of daughter particles  (input)
     *  @param vertex result of vertex fit             (output) 
     *  @return status code 
     */
    virtual StatusCode fit 
    ( LHCb::Vertex&                      vertex    , 
      const LHCb::Particle::ConstVector& daughters ) const ;  
    // ========================================================================
  public:
    // ========================================================================
    /** The vertex fitting method with the creation of LHCb::Particle 
     *  ("classical")
     *
     *  @code
     *
     *  // locate  the tool 
     *  const IVertexFit* fitter = tool<IVertexFit>( ... ) ;
     * 
     *  // container of particles to be fitted into common vertex 
     *  LHCb::Particle::ConstVector daughters = ... ;
     *  
     *  Vertex   vertex   ;
     *  LHCb::Particle particle ; 
     *  StatusCode sc = fitter->fit( daughters , particle , vertex ) 
     *  if ( sc.isFailure() ) { Warning ( "Error in vertex fit", sc ) ; }
     *   
     *  // check chi2 of common vertex fit 
     *  double chi2 = vertex -> chi2() ;
     *
     *  @endcode 
     *  
     *  @see IVertexFit 
     *  @see Particle
     *  @see Vertex
     *  @see GaudiAlgorithm::tool
     *  @see GaudiAlgorithm::Warning
     *
     *  @param daughters vector of daughter particles  (input)
     *  @param particle  result of vertex fit             (output) 
     *  @param vertex    result of vertex fit             (output) 
     *  @return status code 
     */
    virtual StatusCode fit 
    ( const LHCb::Particle::ConstVector& daughters ,
      LHCb::Vertex&          vertex    , 
      LHCb::Particle&        particle  ) const ;  
    // ========================================================================
  public:
    // ========================================================================
    /** add the particle to the vertex and refit the vertex 
     * 
     *  @code
     *
     *  // locate  the tool 
     *  const IVertexFit* fitter = get<IVertexFit>( ... ) ;
     *    
     *  LHCb::Vertex*   vertex  = ... ;
     *  LHCb::Particle* oneMore = ... ;
     *
     *  StatusCode sc = fitter -> add ( oneMore , vertex ) ;
     *  if( sc.isFailure() ) { Warning("Error in 'add' ", sc ) ; }
     *  
     *  // check chi2 of common vertex fit 
     *  double chi2 = vertex -> chi2 () ;
     *
     *  @endcode 
     *
     *  @see IVertexFit 
     *  @param particle particle to be added
     *  @param vertex   vertex to be modified 
     *  @return status code 
     */
    virtual StatusCode add
    ( const LHCb::Particle*  particle , 
      LHCb::Vertex&          vertex   ) const ;  
    // ========================================================================  
  public:
    // ========================================================================  
    /** remove the particle from the vertex and refit the vertex 
     *
     *  @code
     *
     *  // locate  the tool 
     *  const IVertexFit* fitter = get<IVertexFit>( ... ) ;
     *    
     *  LHCb::Vertex*   vertex = ... ;
     *  LHCb::Particle* remove = ... ;
     *
     *  StatusCode sc = fitter -> remove ( remove , vertex ) ;
     *  if( sc.isFailure() ) { Warning("Error in 'remove' ", sc ) ; }
     *  
     *  // check chi2 of common vertex fit 
     *  double chi2 = vertex -> chi2 () ;
     *
     *  @endcode 
     *
     *  @param particle particle to be removed
     *  @param vertex   vertex to be modified 
     *  @return status code 
     */
    virtual StatusCode remove
    ( const LHCb::Particle*  particle , 
      LHCb::Vertex&          vertex   ) const ;
    // ========================================================================
  public:  
    // ========================================================================
    /** The major method for "combining" the daughter particles 
     *  into "mother" particle.
     *
     *  The container of input particles is "combined" into
     *  "mother" particle and its decay vertex.
     *
     *  The intermediate constrains (mass, vertex, mass-vertex,
     *  etc.. could be applied in the process of "combining", 
     *  dependent on the used implementation)
     * 
     *  @code 
     *
     *  // locate the tool  
     *  const IParticleCombiner* combiner = get<IParticleCombiner>( ... ) 
     * 
     *  // loop over the first daughter particle 
     *  for( ... ipi1 = ... ; ... ; ++ipi1 ) 
     *  {
     *     for( ... ipi2 = ... ; ... ; ++ipi2 ) 
     *     {
     *        const LHCb::Particle* pi1 = *ipi1 ;
     *        const LHCb::Particle* pi2 = *ipi2 ;
     *   
     *        IParticleCombiner::LHCb::Particle::ConstVector daughters ;
     *        daughters.push_back ( pi1 ) ;
     *        daughters.push_back ( pi2 ) ;
     *   
     *        LHCb::Particle K0S  ; 
     *        Vertex   Vrtx ; 
     *   
     *        StatusCode sc = combiner -> combine ( daughters , K0S , Vrtx ) ;
     *        if( sc.isFailure() ) 
     *        { Warning ( "Error in K0S fit, skip the pair", sc ) ; }  
     * 
     *        .... 
     *     }
     *  }
     *
     *  @endcode 
     *
     *  @see IParticleCombiner 
     *
     *  @param daughters the vector of daughter particles    (input)
     *  @param mother    the "mother" particle               (output)
     *  @param vertex    the decay vertex of mother particle (output)
     *  @return status code 
     */
    virtual StatusCode combine
    ( const LHCb::Particle::ConstVector&  daughters ,  
      LHCb::Particle&                     mother    , 
      LHCb::Vertex&                       vertex    ) const 
    { return fit ( daughters , vertex , mother ) ; } 
    // ========================================================================
  public:
    // ========================================================================
    /** The basic method for "refit" of the particle 
     *
     *  @code 
     *
     *  // locate the tool 
     *  const IParticleReFitter* refitter = tool<IParticleRefitter>( ... ) ;
     * 
     *  // particle to be refit 
     *  LHCb::Particle* p = ... ;
     *
     *  StatusCode sc = refitter -> reFit ( *p ) ;
     *  if ( sc.isFailure() ) { Warning ( "Error in reFit" , sc ) ; }
     *
     *  @endcode 
     *
     *  @see IParticleReFitter 
     *
     *  @see Particle
     *  @see GaudiAlgorithm::tool
     *  @see GaudiAlgorithm::Warning
     *
     *  @param particle reference to the particle 
     *  @return statsu code 
     */
    virtual StatusCode reFit ( LHCb::Particle& particle ) const  
    {
      // play a bit with extra-info
      if ( particle.hasInfo ( LHCb::Particle::Chi2OfParticleReFitter ) ) 
      { particle.eraseInfo ( LHCb::Particle::Chi2OfParticleReFitter )  ; }
      //
      LHCb::Vertex* vertex = particle.endVertex() ;
      const IVertexFit* vFit = this ;
      StatusCode sc = vFit->fit ( vertex->outgoingParticles().begin() , 
                                  vertex->outgoingParticles().end  () , 
                                  *vertex , particle                  ) ; 
      if ( sc.isFailure() ) { return Error("reFit(): error from fit()" , sc )  ; }
      //
      // in the case of success update the extra-info:
      if ( particle.hasInfo ( LHCb::Particle::Chi2OfVertexConstrainedFit ) ) 
      { 
        particle.addInfo 
          (  LHCb::Particle::Chi2OfParticleReFitter ,
             particle.info ( LHCb::Particle::Chi2OfVertexConstrainedFit , -1000 ) ) ;
      }
      //
      return StatusCode::SUCCESS ;
    } 
    // ========================================================================
  public:
    // ========================================================================
    /// the standard initialization of the tool 
    virtual StatusCode initialize () ; // the standard initialization of the tool 
    /// the standard finalization of the tool 
    virtual StatusCode finalize   () ; // the standard finalization of the tool 
    // ========================================================================
  protected:
    // ========================================================================
    /// standard constructor 
    VertexFitter 
    ( const std::string& type   , 
      const std::string& name   , 
      const IInterface*  parent ) ;
    /// virtual & protected destrcutor
    virtual ~VertexFitter() {}                // virtual & protected destructor
    // ========================================================================
  private:
    // ========================================================================
    /// the default constructor is disabled 
    VertexFitter () ;                    // the default constructor is disabled 
    /// copiy constructoir is disabled 
    VertexFitter ( const VertexFitter& ) ; // copy is disabled 
    /// assignement operator is disabled 
    VertexFitter& operator=( const VertexFitter& ); // disabled 
    // ========================================================================
  protected:
    // ========================================================================
    /// load the data into internal representation 
    StatusCode _load      
    ( const LHCb::Particle* particle , Entry& entry ) const ;
    /// load the data from the daughter particles into the internal structures 
    StatusCode _load ( const LHCb::Particle::ConstVector& daughters ) const ;
    /// add one particle at the end of the queue
    StatusCode _add 
    ( const LHCb::Particle* child , const double newZ ) const ;
    /// transport the data to a certain position 
    StatusCode _transport ( Entry& entry , const double newZ ) const 
    { return LoKi::KalmanFilter::transport ( entry , newZ , transporter() ) ; }
    /// transport all data to a certain position 
    StatusCode _transport ( const double newZ ) const 
    {
      for ( EIT entry = m_entries.begin() ; m_entries.end() != entry ; ++entry ) 
      { 
        StatusCode sc = _transport ( *entry , newZ ) ; 
        if ( sc.isFailure() ) 
        { Warning ("_transport(): the error from transport() , ignore", sc ) ; }
      }
      return StatusCode::SUCCESS ;
    }
    /// make few Kalman filter iterations 
    StatusCode _iterate ( const size_t nMax , const Gaudi::Vector3& x ) const ;
    /// make a seed 
    StatusCode _seed ( const LHCb::Vertex* vertex  ) const ;
    // ========================================================================
  protected:
    // ========================================================================
    /// get the transporter 
    inline IParticleTransporter* transporter() const 
    {
      if ( 0 != m_transporter ) { return m_transporter ; }
      m_transporter = tool<IParticleTransporter> ( m_transporterName , this ) ;
      return m_transporter ;
    } 
    // ========================================================================
    /// get the particle type 
    LoKi::KalmanFilter::ParticleType particleType ( const LHCb::Particle* p ) const ;
    // ========================================================================
  private:
    // ========================================================================
    /// maximal number of iterations for the vertex fit 
    unsigned short m_nIterMaxI    ; // maximal number of iteration for vertex fit 
    /// maximal number of iterations for "add" 
    unsigned short m_nIterMaxII   ; // maximal number of iteration for "add" 
    /// maximal number of iterations for "remove" 
    unsigned short m_nIterMaxIII  ; // maximal number of iteration for "remove" 
    /// distance (stop-iteration criterion)
    double         m_DistanceMax  ; // distance (stop-iteration criterion)
    /// chi2 distance (stop iteration criterion)
    double         m_DistanceChi2 ; // chi2 distance (stop iteration criterion)
    /// propagator/extrapolator/transporter name
    std::string                        m_transporterName ;
    /// the transporter itself
    mutable IParticleTransporter*      m_transporter     ;
    /// fiducial volume for valid seed/vertex 
    double                             m_seedZmin        ;
    double                             m_seedZmax        ;
    double                             m_seedRho         ;
    /// local auxillary storages 
    mutable Entries                    m_entries  ;
    mutable const LHCb::Vertex*        m_vertex   ;
    // seed (evaluated internally) 
    mutable Gaudi::Vector3             m_seed     ;
    mutable Gaudi::SymMatrix3x3        m_seedci   ;
    // internal helper matrices 
    mutable Gaudi::SymMatrix2x2        m_cixy     ;
    mutable Gaudi::SymMatrix4x4        m_cmom     ;
    mutable Gaudi::Matrix4x4           m_cmom1    ;
    mutable Gaudi::Matrix4x3           m_mpcov    ;
    // ========================================================================
  private:
    // ========================================================================
    /// particle property service 
    mutable const LHCb::IParticlePropertySvc* m_ppSvc      ;
    /// Gamma-like particles 
    mutable Decays::Nodes::Pid                m_gammaLike  ;
    /// Long-lived particles 
    mutable Decays::Nodes::LongLived_         m_longLived  ;
    /// Short-lived particles 
    mutable Decays::Nodes::ShortLived_        m_shortLived ;
    // ========================================================================
  } ;
  // ==========================================================================
} //                                                      end of namespace LoKi
// ============================================================================
// The END
// ============================================================================
#endif // LOKIFITTERS_VERTEXFITTER_H
// ============================================================================
