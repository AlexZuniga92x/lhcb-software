// $Id$
// ============================================================================
#ifndef ANALYSIS_MODELS_H 
#define ANALYSIS_MODELS_H 1
// ============================================================================
// Include files
// ============================================================================
// LHCbMath
// ============================================================================
#include "LHCbMath/Functions.h"
// ============================================================================
// ROOT 
// ============================================================================
using std::size_t ;
// ============================================================================
// #include "RooArgList.h"
// #include "RooArgSet.h"
#include "RooAbsPdf.h"
#include "RooRealProxy.h"
#include "RooListProxy.h"
#include "RooAbsReal.h"
// ============================================================================
namespace Analysis
{
  // ==========================================================================
  /** @namespace  Models Models.h Analysis/Models.h
   *  
   *
   *  @author Vanya BELYAEV  Ivan.Belyaev@cern.ch
   *  @date   2011-11-30
   */
  namespace Models 
  {
    // ========================================================================
    /** @class BreitWigner 
     *
     *  J.D.Jackson, 
     *  "Remarks on the Phenomenological Analysis of Resonances",
     *  In Nuovo Cimento, Vol. XXXIV, N.6
     *
     *  http://www.springerlink.com/content/q773737260425652/
     *
     *  @see Gaudi::Math::BreitWigner
     *  @author Vanya BELYAEV Ivan.BElyaev@cern.ch
     *  @date 2011-11-30
     */
    class GAUDI_API BreitWigner : public RooAbsPdf 
    {
    public:
      // ======================================================================
      ClassDef(Analysis::Models::BreitWigner, 1) ;
      // ======================================================================
    public:
      // ======================================================================
      /// constructor from all parameters 
      BreitWigner ( const char*          name      , 
                    const char*          title     ,
                    RooAbsReal&          x         ,
                    RooAbsReal&          mass      ,
                    RooAbsReal&          width     ,
                    const double         m1        , 
                    const double         m2        ,
                    const unsigned short L     = 0 ) ;
      /// constructor from all parameters 
      BreitWigner ( const char*          name      , 
                    const char*          title     ,
                    RooAbsReal&          x         ,
                    RooAbsReal&          mass      ,
                    RooAbsReal&          width     ,
                    const double         m1        , 
                    const double         m2        ,
                    const unsigned short L                         ,
                    const Gaudi::Math::BreitWigner::JacksonRho rho ) ;
      /// constructor from main parameters and "shape"
      BreitWigner ( const char*          name      , 
                    const char*          title     , 
                    RooAbsReal&          x         ,
                    RooAbsReal&          mass      ,
                    RooAbsReal&          width     ,
                    const Gaudi::Math::BreitWigner& bw ) ;
      /// "copy" constructor 
      BreitWigner ( const BreitWigner& , const char* name = 0 ) ;
      /// virtual destructor 
      virtual ~BreitWigner() ;
      /// clone 
      virtual BreitWigner* clone ( const char* name ) const ; 
      // ======================================================================
    public:
      // ======================================================================
      // the actual evaluation of function 
      virtual Double_t evaluate() const ;
      // ======================================================================
    public:
      // ======================================================================
      /// get the amplitude 
      std::complex<double>            amplitude () const  ;
      /// access to underlying function 
      const Gaudi::Math::BreitWigner& function  () const { return m_bw ; }
      // ======================================================================
    protected:
      // ======================================================================
      RooRealProxy m_x     ;
      RooRealProxy m_mass  ;
      RooRealProxy m_width ;
      // ======================================================================
    private:
      // ======================================================================
      /// the actual function 
      mutable Gaudi::Math::BreitWigner m_bw ;            // the actual function 
      // ======================================================================
    } ;
    // ========================================================================
    /** @class Rho0
     *
     *  J.D.Jackson, 
     *  "Remarks on the Phenomenological Analysis of Resonances",
     *  In Nuovo Cimento, Vol. XXXIV, N.6
     *
     *  http://www.springerlink.com/content/q773737260425652/
     *
     *  @see Analysis::Models::BreitWigner
     *  @see Gaudi::Math::BreitWigner
     *  @see Gaudi::Math::Rho0
     *  @author Vanya BELYAEV Ivan.BElyaev@cern.ch
     *  @date 2011-11-30
     */
    class GAUDI_API Rho0 : public Analysis::Models::BreitWigner
    {
    public:
      // ======================================================================
      ClassDef(Analysis::Models::Rho0, 1) ;
      // ======================================================================
    public:
      // ======================================================================
      /// constructor from all parameters 
      Rho0 ( const char*          name      , 
             const char*          title     ,
             RooAbsReal&          x         ,
             RooAbsReal&          mass      ,
             RooAbsReal&          width     ,
             const double         pi_mass   ) ;
      /// "copy" constructor 
      Rho0 ( const Rho0& , const char* name = 0 ) ;
      /// virtual destructor 
      virtual ~Rho0 () ;
      /// clone 
      virtual Rho0* clone ( const char* name ) const ; 
      // ======================================================================
    } ;
    // ========================================================================
    /** @class Kstar
     *
     *  J.D.Jackson, 
     *  "Remarks on the Phenomenological Analysis of Resonances",
     *  In Nuovo Cimento, Vol. XXXIV, N.6
     *
     *  http://www.springerlink.com/content/q773737260425652/
     *
     *  @see Analysis::Models::BreitWigner
     *  @see Gaudi::Math::BreitWigner
     *  @see Gaudi::Math::Kstar
     *  @author Vanya BELYAEV Ivan.BElyaev@cern.ch
     *  @date 2011-11-30
     */
    class GAUDI_API Kstar : public Analysis::Models::BreitWigner
    {
    public:
      // ======================================================================
      ClassDef(Analysis::Models::Kstar, 1) ;
      // ======================================================================
    public:
      // ======================================================================
      /// constructor from all parameters 
      Kstar ( const char*          name      , 
              const char*          title     ,
              RooAbsReal&          x         ,
              RooAbsReal&          mass      ,
              RooAbsReal&          width     ,
              const double         k_mass    , 
              const double         pi_mass   ) ;
      /// "copy" constructor 
      Kstar ( const Kstar& , const char* name = 0 ) ;
      /// virtual destructor 
      virtual ~Kstar () ;
      /// clone 
      virtual  Kstar* clone ( const char* name ) const ; 
      // ======================================================================
    } ;
    // ========================================================================
    /** @class BW23L
     *
     *  J.D.Jackson, 
     *  "Remarks on the Phenomenological Analysis of Resonances",
     *  In Nuovo Cimento, Vol. XXXIV, N.6
     *
     *  http://www.springerlink.com/content/q773737260425652/
     *
     *  @see Gaudi::Math::BreitWigner
     *  @author Vanya BELYAEV Ivan.BElyaev@cern.ch
     *  @date 2011-11-30
     */
    class GAUDI_API BW23L : public RooAbsPdf 
    {
    public:
      // ======================================================================
      ClassDef(Analysis::Models::BW23L, 1) ;
      // ======================================================================
    public:
      // ======================================================================
      /// constructor from all parameters 
      BW23L ( const char*          name      , 
              const char*          title     ,
              RooAbsReal&          x         ,
              RooAbsReal&          mass      ,
              RooAbsReal&          width     ,
              const double         m1        , 
              const double         m2        ,
              const unsigned short l         , 
              //
              const double         m3        , 
              const double         m         , 
              const double         L         ) ;      
      /// constructor from all parameters 
      BW23L ( const char*          name      , 
              const char*          title     ,
              RooAbsReal&          x         ,
              RooAbsReal&          mass      ,
              RooAbsReal&          width     ,
              const double         m1        , 
              const double         m2        ,
              const unsigned short l                         ,
              const Gaudi::Math::BreitWigner::JacksonRho rho , 
              //
              const double         m3        , 
              const double         m         , 
              const double         L         ) ;
      /// constructor from main parameters and "shape"
      BW23L ( const char*          name      , 
              const char*          title     , 
              RooAbsReal&          x         ,
              RooAbsReal&          mass      ,
              RooAbsReal&          width     ,
              const Gaudi::Math::BW23L& bw   ) ; // shape 
      /// "copy" constructor 
      BW23L ( const BW23L& , const char* name = 0 ) ;
      /// virtual destructor 
      virtual ~BW23L() ;
      /// clone 
      virtual BW23L* clone ( const char* name ) const ; 
      // ======================================================================
    public:
      // ======================================================================
      // the actual evaluation of function 
      virtual Double_t     evaluate() const ;
      // ======================================================================
    public:
      // ======================================================================
      /// get the amplitude 
      std::complex<double>      amplitude () const  ;
      /// access to underlying function 
      const Gaudi::Math::BW23L& function  () const { return m_bw ; }
      // ======================================================================
    protected:
      // ======================================================================
      RooRealProxy m_x     ;
      RooRealProxy m_mass  ;
      RooRealProxy m_width ;
      // ======================================================================
    private:
      // ======================================================================
      /// the actual function 
      mutable Gaudi::Math::BW23L m_bw ;            // the actual function 
      // ======================================================================
    } ;
    // ========================================================================
    /** @class Flatte
     *
     *  S.M.Flatte, 
     *  "Coupled-channel analysis of the \f$\pi\eta\f$ 
     *  and \f$K\bar{K}\f$ systems near \f$K\bar{K}\f$ threshold  
     *  Phys. Lett. B63, 224 (1976 
     *
     *  http://www.sciencedirect.com/science/article/pii/0370269376906547
     *
     *  \f$\pi\pi\f$-channel
     *  @author Vanya BELYAEV Ivan.BElyaev@cern.ch
     *  @date 2011-11-30
     */
    class GAUDI_API Flatte : public RooAbsPdf
    {
    public:
      // ======================================================================
      ClassDef(Analysis::Models::Flatte, 1) ;
      // ======================================================================
    public:
      // ======================================================================
      /// constructor from all parameters 
      Flatte ( const char*          name      , 
               const char*          title     ,
               RooAbsReal&          x         ,
               RooAbsReal&          m0        ,
               RooAbsReal&          m0g1      ,
               RooAbsReal&          g2og1     ,
               const double         k_mass    , 
               const double         pi_mass   ) ;
      /// "copy" constructor 
      Flatte ( const Flatte& , const char* name = 0 ) ;
      /// virtual destructor 
      virtual ~Flatte () ;
      /// clone 
      virtual Flatte* clone ( const char* name ) const ; 
      // ======================================================================
    public:
      // ======================================================================
      // the actual evaluation of function 
      virtual Double_t evaluate() const ;
      // ======================================================================
    public:
      // ======================================================================
      /// get the amplitude 
      virtual std::complex<double> amplitude () const  ;
      /// access to underlying function 
      const Gaudi::Math::Flatte&   function  () const { return m_flatte ; }
      // ======================================================================
    protected:
      // ======================================================================
      RooRealProxy m_x     ;
      RooRealProxy m_m0    ;
      RooRealProxy m_m0g1  ;
      RooRealProxy m_g2og1 ;
      double       m_K     ;
      double       m_pi    ;
      // ======================================================================
    protected:
      // ======================================================================
      /// the actual function 
      mutable Gaudi::Math::Flatte m_flatte ;             // the actual function 
      // ======================================================================
    } ;
    // ========================================================================
    /** @class Flatte2
     *
     *  S.M.Flatte, 
     *  "Coupled-channel analysis of the \f$\pi\eta\f$ 
     *  and \f$K\bar{K}\f$ systems near \f$K\bar{K}\f$ threshold  
     *  Phys. Lett. B63, 224 (1976 
     *
     *  http://www.sciencedirect.com/science/article/pii/0370269376906547
     *
     *  KK-channel
     *  @author Vanya BELYAEV Ivan.BElyaev@cern.ch
     *  @date 2011-11-30
     */
    class GAUDI_API Flatte2 : public Analysis::Models::Flatte
    {
    public:
      // ======================================================================
      ClassDef(Analysis::Models::Flatte2, 1) ;
      // ======================================================================
    public:
      // ======================================================================
      /// constructor from all parameters 
      Flatte2 ( const char*          name      , 
                const char*          title     ,
                RooAbsReal&          x         ,
                RooAbsReal&          m0        ,
                RooAbsReal&          m0g1      ,
                RooAbsReal&          g2og1     ,
                const double         k_mass    , 
                const double         pi_mass   ) ;
      /// "copy" constructor 
      Flatte2 ( const Flatte2& , const char* name = 0 ) ;
      /// virtual destructor 
      virtual ~Flatte2 () ;
      /// clone 
      virtual Flatte2* clone ( const char* name ) const ; 
      // ======================================================================
    public:
      // ======================================================================
      /// get the amplitude 
      virtual std::complex<double> amplitude () const  ;
      // ======================================================================
    public:
      // ======================================================================
      // the actual evaluation of function 
      virtual Double_t evaluate() const ;
      // ======================================================================
    } ;
    // ========================================================================
    /** @class PhaseSpace2
     *  simple model for 2-body phase space 
     *  @author Vanya BELYAEV Ivan.BElyaev@cern.ch
     *  @date 2011-11-30
     */
    class PhaseSpace2 : public RooAbsPdf 
    {
    public:
      // ======================================================================
      ClassDef(Analysis::Models::PhaseSpace2, 1) ;
      // ======================================================================
    public:
      // ======================================================================
      /// constructor from all parameters 
      PhaseSpace2 ( const char*          name      , 
                    const char*          title     ,
                    RooAbsReal&          x         ,
                    const double         m1        , 
                    const double         m2        ) ;
      /// "copy constructor"
      PhaseSpace2 ( const PhaseSpace2& right     , 
                    const char*        name  = 0 )  ;
      /// destructor 
      virtual ~PhaseSpace2() ;
      /// clone 
      virtual PhaseSpace2* clone ( const char* name ) const ; 
      // ======================================================================
    public:
      // ======================================================================
      // the actual evaluation of function 
      virtual Double_t evaluate() const ;
      // ======================================================================
    public:
      // ======================================================================
      /// access to underlying function       /// access to underlying function 
      const Gaudi::Math::PhaseSpace2& function() const { return m_ps2 ; }
      // ======================================================================
    protected:
      // ======================================================================
      RooRealProxy m_x     ;
      // ======================================================================
    private:
      // ======================================================================
      /// the actual function 
      mutable Gaudi::Math::PhaseSpace2 m_ps2 ;           // the actual function 
      // ======================================================================
    } ;
    // ========================================================================
    /** @class PhaseSpaceLeft
     *  simple model for left-edge of N-body phase-space 
     *  @author Vanya BELYAEV Ivan.BElyaev@cern.ch
     *  @date 2011-11-30
     */
    class PhaseSpaceLeft : public RooAbsPdf 
    {
    public:
      // ======================================================================
      ClassDef(Analysis::Models::PhaseSpaceLeft, 1) ;
      // ======================================================================
    public:
      // ======================================================================
      /// constructor from all parameters 
      PhaseSpaceLeft ( const char*          name      , 
                       const char*          title     ,
                       RooAbsReal&          x         ,
                       RooAbsReal&          threshold ,
                       const unsigned short N         ) ;
      /// "copy constructor"
      PhaseSpaceLeft ( const PhaseSpaceLeft& right     , 
                       const char*           name  = 0 )  ;
      /// destructor 
      virtual ~PhaseSpaceLeft() ;
      /// clone 
      virtual PhaseSpaceLeft* clone ( const char* name ) const ; 
      // ======================================================================
    public:
      // ======================================================================
      // the actual evaluation of function 
      virtual Double_t evaluate() const ;
      // ======================================================================
    public:
      // ======================================================================
      /// access to underlying function 
      const Gaudi::Math::PhaseSpaceLeft& function() const { return m_left ; }
      // ======================================================================
    protected:
      // ======================================================================
      RooRealProxy m_x         ;
      RooRealProxy m_threshold ;
      // ======================================================================
    private:
      // ======================================================================
      /// the actual function 
      mutable Gaudi::Math::PhaseSpaceLeft m_left ;        // the actual function 
      // ======================================================================
    } ;
    // ========================================================================
    /** @class PhaseSpaceRight
     *  simple model for right-edge of L-body phase-space in N-body decays 
     *  @author Vanya BELYAEV Ivan.BElyaev@cern.ch
     *  @date 2011-11-30
     */
    class PhaseSpaceRight : public RooAbsPdf 
    {
    public:
      // ======================================================================
      ClassDef(Analysis::Models::PhaseSpaceRight, 1) ;
      // ======================================================================
    public:
      // ======================================================================
      /// constructor from all parameters 
      PhaseSpaceRight ( const char*          name      , 
                        const char*          title     ,
                        RooAbsReal&          x         ,
                        RooAbsReal&          threshold ,
                        const unsigned short L         , 
                        const unsigned short N         ) ;
      /// "copy constructor"
      PhaseSpaceRight ( const PhaseSpaceRight& right     , 
                        const char*            name  = 0 )  ;
      /// destructor 
      virtual ~PhaseSpaceRight () ;
      /// clone 
      virtual PhaseSpaceRight* clone ( const char* name ) const ; 
      // ======================================================================
    public:
      // ======================================================================
      // the actual evaluation of function 
      virtual Double_t evaluate() const ;
      // ======================================================================
    public:
      // ======================================================================
      /// access to underlying function 
      const Gaudi::Math::PhaseSpaceRight& function() const { return m_right ; }
      // ======================================================================
    protected:
      // ======================================================================
      RooRealProxy m_x         ;
      RooRealProxy m_threshold ;
      // ======================================================================
    private:
      // ======================================================================
      /// the actual function 
      mutable Gaudi::Math::PhaseSpaceRight m_right ;     // the actual function 
      // ======================================================================
    } ;
    // ========================================================================
    /** @class PhaseSpaceNL
     *  
     *  @see Gaudi::Math::PhaseSpaceNL
     *  @author Vanya BELYAEV Ivan.BElyaev@cern.ch
     *  @date 2011-11-30
     */
    class PhaseSpaceNL : public RooAbsPdf
    {    
    public:
      // ======================================================================
      ClassDef(Analysis::Models::PhaseSpaceNL, 1) ;
      // ======================================================================
    public:
      // ======================================================================
      /// constructor from all parameters 
      PhaseSpaceNL ( const char*          name      , 
                     const char*          title     ,
                     RooAbsReal&          x         ,
                     RooAbsReal&          low       ,
                     RooAbsReal&          high      ,    
                     const unsigned short N         , 
                     const unsigned short L         ) ;
      /// "copy" constructor 
      PhaseSpaceNL ( const PhaseSpaceNL& , const char* name = 0 ) ;
      /// virtual destructor 
      virtual ~PhaseSpaceNL () ;
      /// clone 
      virtual PhaseSpaceNL* clone ( const char* name ) const ; 
      // ======================================================================
    public:
      // ======================================================================
      // the actual evaluation of function 
      virtual Double_t evaluate() const ;
      // ======================================================================
    public:
      // ======================================================================
      /// access to underlying function 
      const Gaudi::Math::PhaseSpaceNL& function() const { return m_ps ; }
      // ======================================================================
    protected:
      // ======================================================================
      RooRealProxy m_x     ;
      RooRealProxy m_low   ;
      RooRealProxy m_high  ;
      // ======================================================================
    private:
      // ======================================================================
      /// the actual function 
      mutable Gaudi::Math::PhaseSpaceNL m_ps ;           // the actual function 
      // ======================================================================
    };
    // ========================================================================
    /** @class PhaseSpacePol
     *  The mass-ditribtion of L-particles from N-body phase space decays, 
     *  modulate with non-negative polynomial 
     *  @author Vanya BELYAEV Ivan.Belyaev@itep.ru
     *  @date 2013-10-05
     */
    class PhaseSpacePol : public RooAbsPdf 
    {
    public:
      // ======================================================================
      ClassDef(Analysis::Models::PhaseSpacePol, 1) ;
      // ======================================================================
      /// constructor from all parameters 
      PhaseSpacePol 
      ( const char*                      name   , 
        const char*                      title  ,
        RooAbsReal&                      x      ,
        const double                     low    , 
        const double                     high   ,
        const unsigned short             N      , 
        const unsigned short             L      , 
        RooAbsReal&                      phi1   ) ;
      /// constructor from all parameters 
      PhaseSpacePol 
      ( const char*                      name   , 
        const char*                      title  ,
        RooAbsReal&                      x      ,
        const double                     low    , 
        const double                     high   ,
        const unsigned short             N      , 
        const unsigned short             L      , 
        RooAbsReal&                      phi1   ,
        RooAbsReal&                      phi2   ) ;
      /// constructor from all parameters 
      PhaseSpacePol 
      ( const char*                      name   , 
        const char*                      title  ,
        RooAbsReal&                      x      ,
        const double                     low    , 
        const double                     high   ,
        const unsigned short             N      , 
        const unsigned short             L      , 
        RooAbsReal&                      phi1   ,
        RooAbsReal&                      phi2   , 
        RooAbsReal&                      phi3   ) ;
      /// constructor from all parameters 
      PhaseSpacePol 
      ( const char*                      name   , 
        const char*                      title  ,
        RooAbsReal&                      x      ,
        const double                     low    , 
        const double                     high   ,
        const unsigned short             N      , 
        const unsigned short             L      , 
        RooArgList&                      phis   ) ;
      // ======================================================================
      /// constructor from all parameters 
      PhaseSpacePol 
      ( const char*                      name   , 
        const char*                      title  ,
        RooAbsReal&                      x      ,
        const Gaudi::Math::PhaseSpaceNL& ps     , 
        RooAbsReal&                      phi1   ) ;
      /// constructor from all parameters 
      PhaseSpacePol 
      ( const char*                      name   , 
        const char*                      title  ,
        RooAbsReal&                      x      ,
        const Gaudi::Math::PhaseSpaceNL& ps     , 
        RooAbsReal&                      phi1   ,
        RooAbsReal&                      phi2   ) ;
      /// constructor from all parameters 
      PhaseSpacePol 
      ( const char*                      name   , 
        const char*                      title  ,
        RooAbsReal&                      x      ,
        const Gaudi::Math::PhaseSpaceNL& ps     , 
        RooAbsReal&                      phi1   ,
        RooAbsReal&                      phi2   ,
        RooAbsReal&                      phi3   ) ;
      /// constructor from all parameters 
      PhaseSpacePol 
      ( const char*                      name   , 
        const char*                      title  ,
        RooAbsReal&                      x      ,
        const Gaudi::Math::PhaseSpaceNL& ps     , 
        RooArgList&                      phis   ) ;
      // ======================================================================
      // "copy" constructor 
      // ======================================================================
      PhaseSpacePol ( const PhaseSpacePol& right    , 
                      const char*          name = 0 ) ;
      /// destructor 
      virtual ~PhaseSpacePol () ;
      /// clone 
      virtual  PhaseSpacePol* clone( const char* name )  const ;
      // ======================================================================
    public:
      // ======================================================================
      virtual Double_t evaluate () const ;
      // ======================================================================
    private:
      // ======================================================================
      RooRealProxy m_x    ;
      RooListProxy m_phis ;
      // ======================================================================
      TIterator* m_iterator;  //! do not persist
      // ======================================================================
    private:
      // ======================================================================
      /// the actual phase space function 
      Gaudi::Math::PhaseSpaceNL     m_ps       ;  // the actual function 
      /// the positive polinomial 
      mutable Gaudi::Math::Positive m_positive ;  // the positive polynomial
      // ======================================================================      
    } ;
    // ========================================================================
    /** @class PhaseSpace23L
     *  simple model for 2-body phase space from 3-body decays with 
     *  the orbital momenta:
     *
     *   \f$ f \propto q^{2\ell+1}p^{2L+1}\f$, where
     *     \f$\ell\f$ is the orbital momentum of the pair of particles, 
     *    and \f$L\f$ is the orbital momentum between the pair and 
     *    the third particle. 
     *   E.g. taking \f$\ell=0, L=1\f$, one can get the S-wave contribution for 
     *   \f$\pi^+\pi^-\f$-mass from \f$B^0\rightarrowJ/\psi\pi^+\pi^-\f$ decay.
     * 
     *  @see Gaudi::Math::PhaseSpace23L 
     *  @author Vanya BELYAEV Ivan.Belyaev@itep.ru
     *  @date 2012-04-01
     */
    class PhaseSpace23L : public RooAbsPdf 
    {
    public:
      // ======================================================================
      ClassDef(Analysis::Models::PhaseSpace23L, 1) ;
      // ======================================================================
    public:
      // ======================================================================
      /** constructor from all parameters 
       *  @param m1 the mass of the first  particle 
       *  @param m2 the mass of the second particle 
       *  @param m3 the mass of the third  particle 
       *  @param m  the mass of the mother particle (m>m1+m2+m3)
       *  @param L  the angular momentum between the first pair and the third particle
       *  @param l  the angular momentum between the first and the second particle
       */
      PhaseSpace23L ( const char*          name      , 
                      const char*          title     ,
                      RooAbsReal&          x         ,
                      const double         m1        , 
                      const double         m2        ,
                      const double         m3        ,
                      const double         m         ,
                      const unsigned short L         , 
                      const unsigned short l     = 0 ) ;
      /// "copy constructor"
      PhaseSpace23L ( const PhaseSpace23L& right , const char* name = 0 )  ;
      /// destructor 
      virtual ~PhaseSpace23L() ;
      /// clone 
      virtual PhaseSpace23L* clone ( const char* name ) const ; 
      // ======================================================================
    public:
      // ======================================================================
      // the actual evaluation of function 
      Double_t evaluate() const ;
      // ======================================================================
    public:
      // ======================================================================
      /// access to underlying function 
      const Gaudi::Math::PhaseSpace23L& function() const { return m_ps23L ; }
      // ======================================================================
    protected:
      // ======================================================================
      RooRealProxy m_x     ;
      // ======================================================================
    private:
      // ======================================================================
      /// the actual function 
      Gaudi::Math::PhaseSpace23L m_ps23L ;               // the actual function 
      // ======================================================================
    } ;
    // ========================================================================
    /** @class LASS
     *  S-wave Kpi amplitude for S-wave Kpi distribtion
     *  @see Gaudi::Math::LASS
     *  @author Vanya BELYAEV Ivan.Belyaev@itep.ru
     *  @date 2013-10-05
     */
    class GAUDI_API LASS : public RooAbsPdf 
    {
    public:
      // ======================================================================
      ClassDef(Analysis::Models::LASS, 1) ;
      // ======================================================================
    public:
      // ======================================================================
      /// constructor from all parameters 
      LASS  ( const char*          name          , 
              const char*          title         ,
              RooAbsReal&          x             ,
              RooAbsReal&          m1430         , // mass  of K*(1430) 
              RooAbsReal&          g1430         , // width of K*(1430) 
              RooAbsReal&          a             , 
              RooAbsReal&          r             , 
              RooAbsReal&          e             ,                
              const double         m1    = 493.7 ,   // mass of K 
              const double         m2    = 139.6 ) ; // mass of pi 
      /// "copy constructor"
      LASS  ( const LASS& right , const char* name = 0 )  ;
      /// destructor 
      virtual ~LASS () ;
      /// clone 
      virtual  LASS * clone ( const char* name ) const ; 
      // ======================================================================
    public:
      // ======================================================================
      // the actual evaluation of function 
      virtual Double_t evaluate() const ;
      // ======================================================================
    public:
      // ======================================================================
      /// get the amplitude 
      std::complex<double>     amplitude () const ;
      /// access to underlying function 
      const Gaudi::Math::LASS& function  () const { return m_lass ; }
      // ======================================================================
    protected:
      // ======================================================================
      /// the mass 
      RooRealProxy m_x     ;
      /// K*(1430) parameters
      RooRealProxy m_m0    ;
      RooRealProxy m_g0    ;
      /// LASS parameters 
      RooRealProxy m_a     ;
      RooRealProxy m_r     ;
      RooRealProxy m_e     ;
      // ======================================================================
    private:
      // ======================================================================
      /// the actual function 
      mutable Gaudi::Math::LASS m_lass ;              // the actual function 
      // ======================================================================
    } ;
    // ========================================================================
    /** @class LASS23L
     *  S-wave Kpi amplitude for Kpi from B-> Kpi X decays
     *  @see Gaudi::Math::LASS23L
     *  @author Vanya BELYAEV Ivan.Belyaev@itep.ru
     *  @date 2012-04-02
     */
    class GAUDI_API LASS23L : public RooAbsPdf 
    {
    public:
      // ======================================================================
      ClassDef(Analysis::Models::LASS23L, 1) ;
      // ======================================================================
    public:
      // ======================================================================
      /// constructor from all parameters 
      LASS23L ( const char*          name          , 
                const char*          title         ,
                RooAbsReal&          x             ,
                RooAbsReal&          m1430         ,
                RooAbsReal&          g1430         ,
                RooAbsReal&          a             , 
                RooAbsReal&          r             , 
                RooAbsReal&          e             ,                
                const double         m1    = 493.7 , 
                const double         m2    = 139.6 ,
                const double         m3    = 3097  , 
                const double         m     = 5278  ,
                const unsigned short L     = 1     ) ;
      /// "copy constructor"
      LASS23L ( const LASS23L& right , const char* name = 0 )  ;
      /// destructor 
      virtual ~LASS23L() ;
      /// clone 
      virtual  LASS23L* clone ( const char* name ) const ; 
      // ======================================================================
    public:
      // ======================================================================
      // the actual evaluation of function 
      virtual Double_t evaluate() const ;
      // ======================================================================
    public:
      // ======================================================================
      /// get the complex amplitude 
      std::complex<double>        amplitude () const ; // get the complex amplitude 
      /// access to underlying function 
      const Gaudi::Math::LASS23L& function  () const { return m_lass ; }
      // ======================================================================
    protected:
      // ======================================================================
      /// the mass 
      RooRealProxy m_x     ;
      /// K*(1430) parameters:
      RooRealProxy m_m0    ;
      RooRealProxy m_g0    ;
      /// LASS parameters 
      RooRealProxy m_a     ;
      RooRealProxy m_r     ;
      RooRealProxy m_e     ;
      // ======================================================================
    private:
      // ======================================================================
      /// the actual function 
      mutable Gaudi::Math::LASS23L m_lass ;              // the actual function 
      // ======================================================================
    } ;    
    // ========================================================================
    /** @class Bugg
     *  parametrisation of sigma-pole for
     *  two pion mass distribution
     *
     *  The parameterization of sigma pole by
     *  B.S.Zou and D.V.Bugg, Phys.Rev. D48 (1993) R3948.
     *
     *  @author Vanya BELYAEV Ivan.Belyaev@itep.ru
     *  @date 2012-04-01
     *  @see Gaudi::Math::Bugg
     */
    class GAUDI_API Bugg : public RooAbsPdf 
    {
    public:
      // ======================================================================
      ClassDef(Analysis::Models::Bugg, 1) ;
      // ======================================================================
    public:
      // ======================================================================
      /// constructor from all parameters 
      Bugg  ( const char*          name               , 
              const char*          title              ,
              RooAbsReal&          x                  ,
              RooAbsReal&          M                  ,   // sigma M 
              RooAbsReal&          g2                 ,   // sigma G2 
              RooAbsReal&          b1                 ,   // sigma B1 
              RooAbsReal&          b2                 ,   // sigma B2
              RooAbsReal&          a                  ,   // sigma a 
              RooAbsReal&          s1                 ,   // sigma s1 
              RooAbsReal&          s2                 ,   // sigma s2 
              const double         m1    = 139.6/1000 ) ; // mass of pi GeV 
      /// "copy constructor"
      Bugg  ( const Bugg& right , const char* name = 0 )  ;
      /// destructor 
      virtual ~Bugg () ;
      /// clone 
      virtual  Bugg* clone ( const char* name ) const ; 
      // ======================================================================
    public:
      // ======================================================================
      // the actual evaluation of function 
      virtual Double_t evaluate() const ;
      // ======================================================================
    public:
      // ======================================================================
      /// get the amplitude 
      std::complex<double>     amplitude () const ;
      /// access to underlying function 
      const Gaudi::Math::Bugg& function  () const { return m_bugg ; }
      // ======================================================================
    protected:
      // ======================================================================
      /// the mass 
      RooRealProxy m_x     ;
      /// sigma/bugg parameters 
      RooRealProxy m_M     ;
      RooRealProxy m_g2    ;
      RooRealProxy m_b1    ;
      RooRealProxy m_b2    ;
      RooRealProxy m_a     ;
      RooRealProxy m_s1    ;
      RooRealProxy m_s2    ;
      // ======================================================================
    private:
      // ======================================================================
      /// the actual function 
      mutable Gaudi::Math::Bugg m_bugg ;              // the actual function 
      // ======================================================================
    } ;
    // ========================================================================
    /** @class Bugg23L
     *  parametrisation of sigma-pole for
     *  two pion mass distribution form three body decays 
     *
     *  The parameterization of sigma pole by
     *  B.S.Zou and D.V.Bugg, Phys.Rev. D48 (1993) R3948.
     *
     *  @author Vanya BELYAEV Ivan.Belyaev@itep.ru
     *  @date 2012-04-01
     *  @see Gaudi::Math::Bugg23L
     */
    class GAUDI_API Bugg23L : public RooAbsPdf 
    {
    public:
      // ======================================================================
      ClassDef(Analysis::Models::Bugg23L, 1) ;
      // ======================================================================
    public:
      // ======================================================================
      /// constructor from all parameters 
      Bugg23L ( const char*          name               , 
                const char*          title              ,
                RooAbsReal&          x                  ,
                RooAbsReal&          M                  ,   // sigma M 
                RooAbsReal&          g2                 ,   // sigma G2 
                RooAbsReal&          b1                 ,   // sigma B1 
                RooAbsReal&          b2                 ,   // sigma B2
                RooAbsReal&          a                  ,   // sigma a 
                RooAbsReal&          s1                 ,   // sigma s1 
                RooAbsReal&          s2                 ,   // sigma s2 
                const double         m1    = 139.6/1000 ,   // mass of pi GeV 
                const double         m3 = 3097.0 / 1000 ,   //  GeV
                const double         m  = 5278.0 / 1000 ,   // GeV
                const unsigned short L  =    1          ) ;
      /// "copy constructor"
      Bugg23L ( const Bugg23L& right , const char* name = 0 )  ;
      /// destructor 
      virtual ~Bugg23L () ;
      /// clone 
      virtual  Bugg23L* clone ( const char* name ) const ; 
      // ======================================================================
    public:
      // ======================================================================
      // the actual evaluation of function 
      virtual Double_t evaluate() const ;
      // ======================================================================
    public:
      // ======================================================================
      /// get the amplitude 
      std::complex<double>        amplitude () const ;
      /// access to underlying function 
      const Gaudi::Math::Bugg23L& function  () const { return m_bugg ; }
      // ======================================================================
    protected:
      // ======================================================================
      /// the mass 
      RooRealProxy m_x     ;
      /// sigma/bugg parameters 
      RooRealProxy m_M     ;
      RooRealProxy m_g2    ;
      RooRealProxy m_b1    ;
      RooRealProxy m_b2    ;
      RooRealProxy m_a     ;
      RooRealProxy m_s1    ;
      RooRealProxy m_s2    ;
      // ======================================================================
    private:
      // ======================================================================
      /// the actual function 
      mutable Gaudi::Math::Bugg23L m_bugg ;              // the actual function 
      // ======================================================================
    } ;
    // ========================================================================
    /** @class CrystalBall
     *  The special parametrization of ``Crystal Ball-function''
     *  @see Gaudi::Math::CrystalBall
     *  @author Vanya BELYAEV Ivan.Belyaev@itep.ru
     *  @date 2012-05-13
     */
    class GAUDI_API CrystalBall : public RooAbsPdf 
    { 
    public:
      // ====================================================================== 
      ClassDef(Analysis::Models::CrystalBall, 1) ;
      // ======================================================================
    public:
      // ======================================================================
      /// constructor from all parameters 
      CrystalBall
      ( const char*          name      , 
        const char*          title     ,
        RooAbsReal&          x         ,
        RooAbsReal&          m0        ,
        RooAbsReal&          sigma     ,  
        RooAbsReal&          alpha     ,  
        RooAbsReal&          n         ) ;  // n-1
      /// "copy" constructor 
      CrystalBall ( const CrystalBall& right , const char* name = 0 ) ;
      /// virtual destructor 
      virtual ~CrystalBall () ;
      /// clone 
      virtual  CrystalBall* clone ( const char* name ) const ; 
      // ======================================================================
    public:
      // ======================================================================
      // the actual evaluation of function 
      virtual Double_t evaluate() const ;
      // ======================================================================
    public:
      // ======================================================================
      /// access to underlying function 
      const Gaudi::Math::CrystalBall& function() const { return m_cb ; }
      // ======================================================================
    protected:
      // ======================================================================
      RooRealProxy m_x      ;
      RooRealProxy m_m0     ;
      RooRealProxy m_sigma  ;
      RooRealProxy m_alpha  ;
      RooRealProxy m_n      ;
      // ======================================================================
    private:
      // ======================================================================
      /// the actual function 
      mutable Gaudi::Math::CrystalBall m_cb ;                  // the function 
      // ======================================================================
    } ;    
    // ========================================================================
    /** @class CrystalBallRS
     *  The special parametrization of ``Crystal Ball-function''
     * rigth-side crystal ball 
     *  @see Gaudi::Math::CrystalBallRightSide 
     *  @author Vanya BELYAEV Ivan.Belyaev@itep.ru
     *  @date 2012-05-13
     */
    class GAUDI_API CrystalBallRS : public RooAbsPdf 
    { 
    public:
      // ====================================================================== 
      ClassDef(Analysis::Models::CrystalBallRS, 1) ;
      // ======================================================================
    public:
      // ======================================================================
      /// constructor from all parameters 
      CrystalBallRS
      ( const char*          name      , 
        const char*          title     ,
        RooAbsReal&          x         ,
        RooAbsReal&          m0        ,
        RooAbsReal&          sigma     ,  
        RooAbsReal&          alpha     ,  
        RooAbsReal&          n         ) ;  // n-1
      /// "copy" constructor 
      CrystalBallRS ( const CrystalBallRS& right , const char* name = 0 ) ;
      /// virtual destructor 
      virtual ~CrystalBallRS () ;
      /// clone 
      virtual  CrystalBallRS* clone ( const char* name ) const ; 
      // ======================================================================
    public:
      // ======================================================================
      // the actual evaluation of function 
      virtual Double_t evaluate() const ;
      // ======================================================================
    public:
      // ======================================================================
      /// access to underlying function 
      const Gaudi::Math::CrystalBallRightSide& function() const { return m_cb ; }
      // ======================================================================
    protected:
      // ======================================================================
      RooRealProxy m_x      ;
      RooRealProxy m_m0     ;
      RooRealProxy m_sigma  ;
      RooRealProxy m_alpha  ;
      RooRealProxy m_n      ;
      // ======================================================================
    private:
      // ======================================================================
      /// the actual function 
      mutable Gaudi::Math::CrystalBallRightSide m_cb ;  // the function 
      // ======================================================================
    } ;
    // ========================================================================
    /** @class Needham
     *  The special parametrization by Matthew NEEDHAM of 
     *  ``Crystal Ball-function'' nicely suitable for \f$J/\psi\f$-peak
     *  @thank Matthew Needham 
     *  @see Gaudi::Math::Needham
     *  @author Vanya BELYAEV Ivan.Belyaev@cern.ch
     *  @date 2012-05-13
     */
    class GAUDI_API Needham : public RooAbsPdf 
    { 
    public:
      // ====================================================================== 
      ClassDef(Analysis::Models::Needham, 1) ;
      // ======================================================================
    public:
      // ======================================================================
      /// constructor from all parameters 
      Needham
      ( const char*          name      , 
        const char*          title     ,
        RooAbsReal&          x         ,
        RooAbsReal&          m0        ,
        RooAbsReal&          sigma     ,  
        RooAbsReal&          a0        ,  
        RooAbsReal&          a1        ,  
        RooAbsReal&          a2        ) ;
      /// "copy" constructor 
      Needham ( const Needham& right , const char* name = 0 ) ;
      /// virtual destructor 
      virtual ~Needham () ;
      /// clone 
      virtual  Needham* clone ( const char* name ) const ; 
      // ======================================================================
    public:
      // ======================================================================
      // the actual evaluation of function 
      virtual Double_t evaluate() const ;
      // ======================================================================
    public:
      // ======================================================================
      /// access to underlying function 
      const Gaudi::Math::Needham& function() const { return m_needham ; }
      /// get current alpha 
      double                      alpha   () const ;
      // ======================================================================
    protected:
      // ======================================================================
      RooRealProxy m_x      ;
      RooRealProxy m_m0     ;
      RooRealProxy m_sigma  ;
      RooRealProxy m_a0     ;
      RooRealProxy m_a1     ;
      RooRealProxy m_a2     ;
      // ======================================================================
    private:
      // ======================================================================
      /// the actual function 
      mutable Gaudi::Math::Needham m_needham ;                  // the function 
      // ======================================================================
    } ;    
    // ========================================================================
    /** @class CrystalBallDS 
     *  double-sided ``Crystal Ball-function'' 
     *  for description of gaussian with the tail
     *  @author Vanya BELYAEV Ivan.Belyaev@cern.ch
     *  @see Gaudi::Math::CrystalBallDoubleSided
     *  @date 2011-05-25
     */
    class GAUDI_API CrystalBallDS : public RooAbsPdf 
    { 
    public:
      // ====================================================================== 
      ClassDef(Analysis::Models::CrystalBallDS, 1) ;
      // ======================================================================
    public:
      // ======================================================================
      /// constructor from all parameters 
      CrystalBallDS
      ( const char*          name      , 
        const char*          title     ,
        RooAbsReal&          x         ,
        RooAbsReal&          m0        ,
        RooAbsReal&          sigma     ,  //    
        RooAbsReal&          alphaL    ,  // alpha_L 
        RooAbsReal&          nL        ,  //     n_L - 1   
        RooAbsReal&          alphaR    ,  // alpha_R - 1   
        RooAbsReal&          nR        ); //     n_R 
      /// "copy" constructor 
      CrystalBallDS ( const CrystalBallDS& right , const char* name = 0 ) ;
      /// virtual destructor 
      virtual ~CrystalBallDS() ;
      /// clone 
      virtual  CrystalBallDS* clone ( const char* name ) const ; 
      // ======================================================================
    public:
      // ======================================================================
      // the actual evaluation of function 
      virtual Double_t evaluate() const ;
      // ======================================================================
    public:
      // ======================================================================
      /// access to underlying function 
      const Gaudi::Math::CrystalBallDoubleSided& function() const 
      { return m_cb2 ; }
      // ======================================================================
    protected:
      // ======================================================================
      RooRealProxy m_x      ;
      RooRealProxy m_m0     ;
      RooRealProxy m_sigma  ;
      RooRealProxy m_alphaL ;
      RooRealProxy m_nL     ;
      RooRealProxy m_alphaR ;
      RooRealProxy m_nR     ;
      // ======================================================================
    private:
      // ======================================================================
      /// the actual function 
      mutable Gaudi::Math::CrystalBallDoubleSided m_cb2 ;       // the function 
      // ======================================================================
    } ;
    // ========================================================================
    /** @class Apolonios
     *  A modified gaussian with power-law tail on rigth ride and exponential
     *  tail on low-side 
     *  The function is proposed by Diego Martinez Santos 
     *  https://indico.cern.ch/getFile.py/access?contribId=2&resId=1&materialId=slides&confId=262633
     *  Here a bit modified version is used with redefined parameter <code>n</code>
     *  to be coherent with local definitions of Crystal Ball
     *  
     *  @see Gaudi::Math::Apolonios
     *  @author Vanya BELYAEV Ivane.BElyaev@itep.ru
     *  @date 2013-12-01
     */
    // ========================================================================
    class GAUDI_API Apolonios : public RooAbsPdf 
    {
      // ======================================================================
    public :
      // ======================================================================
      ClassDef(Analysis::Models::Apolonios, 1) ;
      // ======================================================================
    public:
      // ======================================================================
      /// constructor from all parameters 
      Apolonios
      ( const char*          name      , 
        const char*          title     ,
        RooAbsReal&          x         , 
        RooAbsReal&          mean      , 
        RooAbsReal&          sigma     , 
        RooAbsReal&          alpha     ,
        RooAbsReal&          n         , 
        RooAbsReal&          b         ) ;
      /// "copy" constructor 
      Apolonios  ( const Apolonios& right , const char* name = 0  ) ;
      /// virtual destructor  
      virtual ~Apolonios () ;
      /// clone 
      virtual  Apolonios* clone ( const char* name ) const ; 
      // ======================================================================
    public:
      // ======================================================================
      // the actual evaluation of function 
      virtual Double_t evaluate() const ;
      // ======================================================================
    public:
      // ======================================================================
      /// access to underlying function 
      const Gaudi::Math::Apolonios& function() const { return m_apo ; }
      // ======================================================================
    protected:
      // ======================================================================
      RooRealProxy m_x      ;
      RooRealProxy m_m0     ;
      RooRealProxy m_sigma  ;
      RooRealProxy m_alpha  ;
      RooRealProxy m_n      ;
      RooRealProxy m_b      ;
      // ======================================================================
    private:
      // ======================================================================
      /// the actual function 
      mutable Gaudi::Math::Apolonios m_apo ;                // the function 
      // ======================================================================
    } ;
    // ========================================================================
    /** @class GramCharlierA
     *  The peak with Gram-Charlier type A parameterization
     *  @see Gaudi::Math::GramCharlierA 
     *  @author Vanya BELYAEV Ivan.Belyaev@cern.ch
     *  @date 2011-12-05
     */
    class GAUDI_API GramCharlierA : public RooAbsPdf 
    {
      // ======================================================================
    public :
      // ======================================================================
      ClassDef(Analysis::Models::GramCharlierA, 1) ;
      // ======================================================================
    public:
      // ======================================================================
      /// constructor from all parameters 
      GramCharlierA 
      ( const char*          name      , 
        const char*          title     ,
        RooAbsReal&          x         , 
        RooAbsReal&          mean      , 
        RooAbsReal&          sigma     , 
        RooAbsReal&          kappa3    ,
        RooAbsReal&          kappa4    );
      /// "copy" constructor 
      GramCharlierA ( const GramCharlierA& right , const char* name = 0  ) ;
      /// virtual destructor  
      virtual ~GramCharlierA () ;
      /// clone 
      virtual  GramCharlierA* clone ( const char* name ) const ; 
      // ======================================================================
    public:
      // ======================================================================
      // the actual evaluation of function 
      virtual Double_t evaluate() const ;
      // ======================================================================
    public:
      // ======================================================================
      /// access to underlying function 
      const Gaudi::Math::GramCharlierA& function() const { return m_gca ; }
      // ======================================================================
    protected:
      // ======================================================================
      RooRealProxy m_x      ;
      RooRealProxy m_m0     ;
      RooRealProxy m_sigma  ;
      RooRealProxy m_kappa3 ;
      RooRealProxy m_kappa4 ;
      // ======================================================================
    private:
      // ======================================================================
      /// the actual function 
      mutable Gaudi::Math::GramCharlierA m_gca ;                // the function 
      // ======================================================================
    } ;  
    // ========================================================================
    /** @class BifurcatedGauss 
     *  @see Gaudi::Math::BifurkatedGauss 
     *  @author Vanya BELYAEV Ivan.BElyaev@itep.ru
     *  @date 2013-08-27
     */
    // ========================================================================
    class GAUDI_API BifurcatedGauss : public RooAbsPdf 
    {
      // ======================================================================
    public :
      // ======================================================================
      ClassDef(Analysis::Models::BifurcatedGauss, 1) ;
      // ======================================================================
    public:
      // ======================================================================
      /// constructor from all parameters 
      BifurcatedGauss 
      ( const char*          name      , 
        const char*          title     ,
        RooAbsReal&          x         , 
        RooAbsReal&          peak      , 
        RooAbsReal&          sigmaL    , 
        RooAbsReal&          sigmaR    ) ;
      /// "copy" constructor 
      BifurcatedGauss ( const BifurcatedGauss& right , const char* name = 0  ) ;
      /// virtual destructor  
      virtual ~BifurcatedGauss () ;
      /// clone 
      virtual  BifurcatedGauss* clone ( const char* name ) const ; 
      // ======================================================================
    public:
      // ======================================================================
      // the actual evaluation of function 
      virtual Double_t evaluate() const ;
      // ======================================================================
    public:
      // ======================================================================
      /// access to underlying function 
      const Gaudi::Math::BifurcatedGauss& function() const { return m_bg ; }
      // ======================================================================
    protected:
      // ======================================================================
      RooRealProxy m_x      ;
      RooRealProxy m_peak   ;
      RooRealProxy m_sigmaL ;
      RooRealProxy m_sigmaR ;
      // ======================================================================
    private:
      // ======================================================================
      /// the actual function 
      mutable Gaudi::Math::BifurcatedGauss m_bg ;               // the function 
      // ======================================================================      
    } ;
    // ========================================================================
    /** @class GenGaussV1
     *  Simple class that implements the generalized normal distribution v1
     *  @see http://en.wikipedia.org/wiki/Generalized_normal_distribution#Version_1
     *  @see Gaudi::Math::GenGaussV1 
     *  @author Vanya BELYAEV Ivan.BElyaev@itep.ru
     *  @date 2013-08-27
     */
    // ========================================================================
    class GAUDI_API GenGaussV1 : public RooAbsPdf 
    {
      // ======================================================================
    public :
      // ======================================================================
      ClassDef(Analysis::Models::GenGaussV1, 1) ;
      // ======================================================================
    public:
      // ======================================================================
      /// constructor from all parameters 
      GenGaussV1
      ( const char*          name      , 
        const char*          title     ,
        RooAbsReal&          x         , 
        RooAbsReal&          mu        , 
        RooAbsReal&          alpha     , 
        RooAbsReal&          beta      ) ;
      /// "copy" constructor 
      GenGaussV1 ( const GenGaussV1& right , const char* name = 0  ) ;
      /// virtual destructor  
      virtual ~GenGaussV1 () ;
      /// clone 
      virtual  GenGaussV1* clone ( const char* name ) const ; 
      // ======================================================================
    public:
      // ======================================================================
      // the actual evaluation of function 
      virtual Double_t evaluate() const ;
      // ======================================================================
    public:
      // ======================================================================
      /// access to underlying function 
      const Gaudi::Math::GenGaussV1& function() const { return m_ggv1 ; }
      // ======================================================================
    protected:
      // ======================================================================
      RooRealProxy m_x      ;
      RooRealProxy m_mu     ;
      RooRealProxy m_alpha  ;
      RooRealProxy m_beta   ;
      // ======================================================================
    private:
      // ======================================================================
      /// the actual function 
      mutable Gaudi::Math::GenGaussV1 m_ggv1 ;                 // the function 
      // ======================================================================      
    } ;
    // ========================================================================
    /** @class GenGaussV2
     *  Simple class that implements the generalized normal distribution v2
     *  @see http://en.wikipedia.org/wiki/Generalized_normal_distribution#Version_2
     *  @see Gaudi::Math::GenGaussV2 
     *  @author Vanya BELYAEV Ivan.BElyaev@itep.ru
     *  @date 2013-08-27
     */
    // ========================================================================
    class GAUDI_API GenGaussV2 : public RooAbsPdf 
    {
      // ======================================================================
    public :
      // ======================================================================
      ClassDef(Analysis::Models::GenGaussV2, 1) ;
      // ======================================================================
    public:
      // ======================================================================
      /// constructor from all parameters 
      GenGaussV2
      ( const char*          name      , 
        const char*          title     ,
        RooAbsReal&          x         , 
        RooAbsReal&          xi        , 
        RooAbsReal&          alpha     , 
        RooAbsReal&          kappa     ) ;
      /// "copy" constructor 
      GenGaussV2 ( const GenGaussV2& right , const char* name = 0  ) ;
      /// virtual destructor  
      virtual ~GenGaussV2 () ;
      /// clone 
      virtual  GenGaussV2* clone ( const char* name ) const ; 
      // ======================================================================
    public:
      // ======================================================================
      // the actual evaluation of function 
      virtual Double_t evaluate() const ;
      // ======================================================================
    public:
      // ======================================================================
      /// access to underlying function 
      const Gaudi::Math::GenGaussV2& function() const { return m_ggv2 ; }
      // ======================================================================
    protected:
      // ======================================================================
      RooRealProxy m_x      ;
      RooRealProxy m_xi     ;
      RooRealProxy m_alpha  ;
      RooRealProxy m_kappa  ;
      // ======================================================================
    private:
      // ======================================================================
      /// the actual function 
      mutable Gaudi::Math::GenGaussV2 m_ggv2 ;                 // the function 
      // ======================================================================      
    } ;
    // ========================================================================
    /** @class SkewGauss
     *  Simple class that implements the skew normal distribution
     *  @see http://en.wikipedia.org/wiki/Skew_normal_distribution
     *  @see Gaudi::Math::SkewGauss 
     *  @author Vanya BELYAEV Ivan.BElyaev@itep.ru
     *  @date 2013-08-27
     */
    // ========================================================================
    class GAUDI_API SkewGauss : public RooAbsPdf 
    {
      // ======================================================================
    public :
      // ======================================================================
      ClassDef(Analysis::Models::SkewGauss, 1) ;
      // ======================================================================
    public:
      // ======================================================================
      /// constructor from all parameters 
      SkewGauss
      ( const char*          name      , 
        const char*          title     ,
        RooAbsReal&          x         , 
        RooAbsReal&          xi        , 
        RooAbsReal&          omega     , 
        RooAbsReal&          alpha     ) ;
      /// "copy" constructor 
      SkewGauss ( const SkewGauss& right , const char* name = 0  ) ;
      /// virtual destructor  
      virtual ~SkewGauss () ;
      /// clone 
      virtual  SkewGauss* clone ( const char* name ) const ; 
      // ======================================================================
    public:
      // ======================================================================
      // the actual evaluation of function 
      virtual Double_t evaluate() const ;
      // ======================================================================
    public:
      // ======================================================================
      /// access to underlying function 
      const Gaudi::Math::SkewGauss& function() const { return m_sg ; }
      // ======================================================================
    protected:
      // ======================================================================
      RooRealProxy m_x      ;
      RooRealProxy m_xi     ;
      RooRealProxy m_omega  ;
      RooRealProxy m_alpha  ;
      // ======================================================================
    private:
      // ======================================================================
      /// the actual function 
      mutable Gaudi::Math::SkewGauss m_sg ;                     // the function 
      // ======================================================================      
    } ;
    // ========================================================================
    /** @class Bukin
     *  "Bukin"-function
     *  @see Gaudi::Math::Bukin
     *  @author Vanya BELYAEV Ivan.BElyaev@cern.ch
     *  @date 2011-12-05
     */
    class GAUDI_API Bukin : public RooAbsPdf 
    {
      // ======================================================================
    public :
      // ======================================================================
      ClassDef(Analysis::Models::Bukin, 1) ;
      // ======================================================================
    public:
      // ======================================================================
      /// constructor from all parameters 
      Bukin
      ( const char*          name      , 
        const char*          title     ,
        RooAbsReal&          x         , 
        RooAbsReal&          peak      , 
        RooAbsReal&          sigma     , 
        RooAbsReal&          xi        ,
        RooAbsReal&          rhoL      ,
        RooAbsReal&          rhoR      ) ;
      /// "copy" constructor 
      Bukin ( const Bukin& right , const char* name = 0  ) ;
      /// virtual destructor  
      virtual ~Bukin () ;
      /// clone 
      virtual  Bukin* clone ( const char* name ) const ; 
      // ======================================================================
    public:
      // ======================================================================
      // the actual evaluation of function 
      virtual Double_t evaluate() const ;
      // ======================================================================
    public:
      // ======================================================================
      /// access to underlying function 
      const Gaudi::Math::Bukin& function() const { return m_bukin ; }
      // ======================================================================
    protected:
      // ======================================================================
      RooRealProxy m_x      ;
      RooRealProxy m_peak   ;
      RooRealProxy m_sigma  ;
      RooRealProxy m_xi     ;
      RooRealProxy m_rhoL   ;
      RooRealProxy m_rhoR   ;
      // ======================================================================
    private:
      // ======================================================================
      /// the actual function 
      mutable Gaudi::Math::Bukin m_bukin ;                      // the function 
      // ======================================================================      
    } ;  
    // ========================================================================
    /** @class Voigt
     *  "Voigt"-function
     *  @see Gaudi::Math::Voigt
     *  @author Vanya BELYAEV Ivan.BElyaev@cern.ch
     *  @date 2011-12-05
     */
    class GAUDI_API Voigt : public RooAbsPdf 
    {
      // ======================================================================
    public :
      // ======================================================================
      ClassDef(Analysis::Models::Voigt, 1) ;
      // ======================================================================
    public:
      // ======================================================================
      /// constructor from all parameters 
      Voigt
      ( const char*          name      , 
        const char*          title     ,
        RooAbsReal&          x         , 
        RooAbsReal&          m0        , 
        RooAbsReal&          gamma     , 
        RooAbsReal&          sigma     ) ;
      /// "copy" constructor 
      Voigt ( const Voigt& right , const char* name = 0  ) ;
      /// virtual destructor  
      virtual ~Voigt () ;
      /// clone 
      virtual  Voigt* clone ( const char* name ) const ; 
      // ======================================================================
    public:
      // ======================================================================
      // the actual evaluation of function 
      virtual Double_t evaluate() const ;
      // ======================================================================
    public:
      // ======================================================================
      /// access to underlying function 
      const Gaudi::Math::Voigt& function() const { return m_voigt ; }
      // ======================================================================
    protected:
      // ======================================================================
      RooRealProxy m_x      ;
      RooRealProxy m_m0     ;
      RooRealProxy m_gamma  ;
      RooRealProxy m_sigma  ;
      // ======================================================================
    private:
      // ======================================================================
      /// the actual function 
      mutable Gaudi::Math::Voigt m_voigt ;                      // the function 
      // ======================================================================      
    };
    // ========================================================================
    /** @class PolyPositive
     *  PolyPositive polynomial
     *  @see Gaudi::Math::Positive
     *  @author Vanya BELYAEV  Ivan.Belyaev@itep.ru
     *  @date 2011-05-25
     */
    class GAUDI_API PolyPositive: public RooAbsPdf 
    {
      // ======================================================================
    public :
      // ======================================================================
      ClassDef(Analysis::Models::PolyPositive, 1) ;
      // ======================================================================
    public:
      // ======================================================================
      /// linear 
      PolyPositive
      ( const char*          name      , 
        const char*          title     ,
        RooAbsReal&          x         ,
        RooAbsReal&          phi1      , 
        const double         xmin      , 
        const double         xmax      ) ;
      /// quadric
      PolyPositive
      ( const char*          name      , 
        const char*          title     ,
        RooAbsReal&          x         ,
        RooAbsReal&          phi1      , 
        RooAbsReal&          phi2      , 
        const double         xmin      , 
        const double         xmax      ) ;
      /// cubic 
      PolyPositive
      ( const char*          name      , 
        const char*          title     ,
        RooAbsReal&          x         ,
        RooAbsReal&          phi1      , 
        RooAbsReal&          phi2      , 
        RooAbsReal&          phi3      , 
        const double         xmin      , 
        const double         xmax      ) ;
      /// quartic
      PolyPositive
      ( const char*          name      , 
        const char*          title     ,
        RooAbsReal&          x         ,
        RooAbsReal&          phi1      , 
        RooAbsReal&          phi2      , 
        RooAbsReal&          phi3      , 
        RooAbsReal&          phi4      , 
        const double         xmin      , 
        const double         xmax      ) ;
      /// quintic 
      PolyPositive
      ( const char*          name      , 
        const char*          title     ,
        RooAbsReal&          x         ,
        RooAbsReal&          phi1      , 
        RooAbsReal&          phi2      , 
        RooAbsReal&          phi3      , 
        RooAbsReal&          phi4      , 
        RooAbsReal&          phi5      , 
        const double         xmin      , 
        const double         xmax      ) ;
      /// general 
      PolyPositive
      ( const char*          name      , 
        const char*          title     ,
        RooAbsReal&          x         ,
        const RooArgList&    coeffs    ,
        const double         xmin      , 
        const double         xmax      ) ;
      /// copy
      PolyPositive
      ( const PolyPositive&     right     , 
        const char*          name = 0  ) ;
      /// destructor 
      virtual ~PolyPositive() ;
      /// clone 
      virtual  PolyPositive* clone ( const char* name ) const ; 
      // ======================================================================
    public:
      // ======================================================================
      // the actual evaluation of function 
      virtual Double_t evaluate() const ;
      // ======================================================================
    public:
      // ======================================================================
      /// access to underlying function 
      const Gaudi::Math::Positive& function() const { return m_positive ; }
      // ======================================================================
    protected :
      // ======================================================================
      RooRealProxy m_x    ;
      RooListProxy m_phis ;
      // ======================================================================
      TIterator* m_iterator;  //! do not persist
      // ======================================================================
    private:
      // ======================================================================
      /// the actual function 
      mutable Gaudi::Math::Positive m_positive ;               // the function 
      // ======================================================================
    } ;
    // ========================================================================
    /** @class ExpoPositive
     *  exponential multiplied on positive polynomial
     *  @see Gaudi::Math::Positive
     *  @author Vanya BELYAEV  Ivan.Belyaev@itep.ru
     *  @date 2011-05-25
     */
    class GAUDI_API ExpoPositive: public RooAbsPdf 
    {
      // ======================================================================
    public :
      // ======================================================================
      ClassDef(Analysis::Models::ExpoPositive, 1) ;
      // ======================================================================
    public:
      // ======================================================================
      /// linear 
      ExpoPositive
      ( const char*          name      , 
        const char*          title     ,
        RooAbsReal&          x         ,
        RooAbsReal&          tau       , 
        RooAbsReal&          phi1      , 
        const double         xmin      , 
        const double         xmax      ) ;
      /// quadric
      ExpoPositive
      ( const char*          name      , 
        const char*          title     ,
        RooAbsReal&          x         ,
        RooAbsReal&          tau       , 
        RooAbsReal&          phi1      , 
        RooAbsReal&          phi2      , 
        const double         xmin      , 
        const double         xmax      ) ;
      /// cubic 
      ExpoPositive
      ( const char*          name      , 
        const char*          title     ,
        RooAbsReal&          x         ,
        RooAbsReal&          tau       , 
        RooAbsReal&          phi1      , 
        RooAbsReal&          phi2      , 
        RooAbsReal&          phi3      , 
        const double         xmin      , 
        const double         xmax      ) ;
      /// quartic
      ExpoPositive
      ( const char*          name      , 
        const char*          title     ,
        RooAbsReal&          x         ,
        RooAbsReal&          tau       , 
        RooAbsReal&          phi1      , 
        RooAbsReal&          phi2      , 
        RooAbsReal&          phi3      , 
        RooAbsReal&          phi4      , 
        const double         xmin      , 
        const double         xmax      ) ;
      /// quintic 
      ExpoPositive
      ( const char*          name      , 
        const char*          title     ,
        RooAbsReal&          x         ,
        RooAbsReal&          tau       , 
        RooAbsReal&          phi1      , 
        RooAbsReal&          phi2      , 
        RooAbsReal&          phi3      , 
        RooAbsReal&          phi4      , 
        RooAbsReal&          phi5      , 
        const double         xmin      , 
        const double         xmax      ) ;
      /// general 
      ExpoPositive
      ( const char*          name      , 
        const char*          title     ,
        RooAbsReal&          x         ,
        RooAbsReal&          tau       , 
        const RooArgList&    coeffs    ,
        const double         xmin      , 
        const double         xmax      ) ;
      /// copy
      ExpoPositive
      ( const ExpoPositive&     right     , 
        const char*          name = 0  ) ;
      /// destructor 
      virtual ~ExpoPositive() ;
      /// clone 
      virtual  ExpoPositive* clone ( const char* name ) const ; 
      // ======================================================================
    public:
      // ======================================================================
      // the actual evaluation of function 
      virtual Double_t evaluate() const ;
      // ======================================================================
    public: // integrals  
      // ======================================================================      
      virtual Int_t    getAnalyticalIntegral
        ( RooArgSet&     allVars      , 
          RooArgSet&     analVars     ,
          const char* /* rangename */ ) const ;
      virtual Double_t analyticalIntegral 
        ( Int_t          code         ,  
          const char*    rangeName    ) const ;
      // ======================================================================
    protected :
      // ======================================================================
      RooRealProxy m_x    ;
      RooRealProxy m_tau  ;
      RooListProxy m_phis ;
      // ======================================================================
      TIterator* m_iterator;  //! do not persist
      // ======================================================================
    private:
      // ======================================================================
      /// the actual function 
      mutable Gaudi::Math::ExpoPositive m_positive ;           // the function 
      // ======================================================================
    } ;
    // ========================================================================
    /** @class StudentT 
     *  Student-T distribution
     *
     *  \f[  f(y) = \frac{1}{\sqrt{\pi n}} \frac { \Gamma( \frac{n+1}{2}) } { \Gamma( \frac{n}{2}  ) }
     *  \left( 1 + \frac{y^2}{n} \right)^{ -\frac{n+1}{2}} \f], 
     *  where \f$ y = \frac{x - \mu}{\sigma} \f$  
     * 
     *  @see Gaudi::Math::StudentT
     *  @author Vanya BELYAEV  Ivan.Belyaev@itep.ru
     *  @date 2013-01-05
     */
    class GAUDI_API StudentT: public RooAbsPdf 
    {
      // ======================================================================
    public :
      // ======================================================================
      ClassDef(Analysis::Models::StudentT, 1) ;
      // ======================================================================
    public:
      // ======================================================================
      /// constructor from all parameters 
      StudentT ( const char*          name      , 
                 const char*          title     ,
                 RooAbsReal&          x         ,
                 RooAbsReal&          mu        ,
                 RooAbsReal&          sigma     ,
                 RooAbsReal&          n         ) ;
      /// "copy constructor"
      StudentT ( const StudentT&      right     , 
                 const char*          name  = 0 )  ;
      /// destructor 
      virtual ~StudentT() ;
      /// clone 
      virtual  StudentT* clone ( const char* name ) const ; 
      // ======================================================================
    public:
      // ======================================================================
      // the actual evaluation of function 
      virtual Double_t evaluate() const ;
      // ======================================================================
    public:
      // ======================================================================
      /// access to underlying function 
      const Gaudi::Math::StudentT& function() const { return m_stt ; }
      // ======================================================================
    protected: 
      // ======================================================================
      RooRealProxy m_x        ;
      RooRealProxy m_mu       ;
      RooRealProxy m_sigma    ;
      RooRealProxy m_n        ;
      // ======================================================================
    private:
      // ======================================================================
      /// the actual function 
      mutable Gaudi::Math::StudentT m_stt ;           // the actual function 
      // ======================================================================
    } ;
    // ========================================================================
    /** @class GammaDist 
     *  Gamma-distribution shape/scale parameters 
     *  http://en.wikipedia.org/wiki/Gamma_distribution
     *  @author Vanya BELYAEV Ivan.Belyaev@itep.ru
     *  @date   2013-05-11
     *  @see Gaudi::Math::GammaDist 
     */
    class GAUDI_API GammaDist : public RooAbsPdf 
    {
      // ======================================================================
    public :
      // ======================================================================
      ClassDef(Analysis::Models::GammaDist, 1) ;
      // ======================================================================
    public:
      // ======================================================================
      /// constructor from all parameters
      GammaDist ( const char*          name      , 
                  const char*          title     ,
                  RooAbsReal&          x         , 
                  RooAbsReal&          k         ,
                  RooAbsReal&          theta     ) ;
      /// "copy constructor"
      GammaDist ( const GammaDist&     right     , 
                  const char*          name  = 0 )  ;
      /// destructor 
      virtual ~GammaDist () ;
      /// clone 
      virtual  GammaDist* clone ( const char* name ) const ; 
      // ======================================================================
    public:
      // ======================================================================
      // the actual evaluation of function 
      virtual Double_t evaluate() const ;
      // ======================================================================
    public:
      // ======================================================================
      /// access to underlying function 
      const Gaudi::Math::GammaDist& function() const { return m_gamma ; }
      // ======================================================================
    protected: 
      // ======================================================================
      RooRealProxy m_x        ;
      RooRealProxy m_k        ;
      RooRealProxy m_theta    ;
      // ======================================================================
    private:
      // ======================================================================
      /// the actual function
      mutable Gaudi::Math::GammaDist m_gamma ; // the actual function
      // ======================================================================
    } ;
    // ========================================================================
    /** @class GenGammaDist 
     *  Generalized Gamma-distribution with additional shift parameter 
     *  http://en.wikipedia.org/wiki/Generalized_gamma_distribution
     *  special cases : 
     *   - p == 1      : Gamma  distribution
     *   - p == k      : Weibull distribution
     *   - p == k == 1 : Exponential distribution
     *   - p == k == 2 : Rayleigh    distribution
     *  @see Gaudi::Math::GenGammaDist
     *  @author Vanya BELYAEV Ivan.Belyaev@itep.ru
     *  @date   2013-05-11
     *  @see Gaudi::Math::GammaDist 
     */
    class GAUDI_API GenGammaDist : public RooAbsPdf 
    {
      // ======================================================================
    public :
      // ======================================================================
      ClassDef(Analysis::Models::GenGammaDist, 1) ;
      // ======================================================================
    public:
      // ======================================================================
      /// constructor from all parameters
      GenGammaDist 
      ( const char*          name      , 
        const char*          title     ,
        RooAbsReal&          x         , 
        RooAbsReal&          k         ,
        RooAbsReal&          theta     ,
        RooAbsReal&          p         ,
        RooAbsReal&          low       ) ;
      /// "copy constructor"
      GenGammaDist ( const GenGammaDist&  right     , 
                     const char*          name  = 0 )  ;
      /// destructor 
      virtual ~GenGammaDist () ;
      /// clone 
      virtual  GenGammaDist* clone ( const char* name ) const ; 
      // ======================================================================
    public:
      // ======================================================================
      // the actual evaluation of function 
      virtual Double_t evaluate() const ;
      // ======================================================================
    public:
      // ======================================================================
      /// access to underlying function 
      const Gaudi::Math::GenGammaDist& function() const { return m_ggamma ; }
      // ======================================================================
    protected: 
      // ======================================================================
      RooRealProxy m_x        ;
      RooRealProxy m_k        ;
      RooRealProxy m_theta    ;
      RooRealProxy m_p        ;
      RooRealProxy m_low      ;
      // ======================================================================
    private:
      // ======================================================================
      /// the actual function
      mutable Gaudi::Math::GenGammaDist m_ggamma ; // the actual function
      // ======================================================================
    } ;
    // ========================================================================
    /** @class Amoroso
     *  Another view on generalized gamma distribtion
     *  http://arxiv.org/pdf/1005.3274
     *  @see Gaudi::Math::Amoroso
     *  @author Vanya BELYAEV Ivan.BElyaev@cern.ch
     *  @date 2011-12-05
     */
    class GAUDI_API Amoroso : public RooAbsPdf 
    {
      // ======================================================================
    public :
      // ======================================================================
      ClassDef(Analysis::Models::Amoroso, 1) ;
      // ======================================================================
    public:
      // ======================================================================
      /// constructor from all parameters 
      Amoroso
      ( const char*          name      , 
        const char*          title     ,
        RooAbsReal&          x         , 
        RooAbsReal&          theta     , 
        RooAbsReal&          alpha     , 
        RooAbsReal&          beta      ,
        RooAbsReal&          a         ) ;
      /// "copy" constructor 
      Amoroso ( const Amoroso& right , const char* name = 0  ) ;
      /// virtual destructor  
      virtual ~Amoroso () ;
      /// clone 
      virtual  Amoroso* clone ( const char* name ) const ; 
      // ======================================================================
    public:
      // ======================================================================
      // the actual evaluation of function 
      virtual Double_t evaluate() const ;
      // ======================================================================
    public:
      // ======================================================================
      /// access to underlying function 
      const Gaudi::Math::Amoroso& function() const { return m_amoroso ; }
      // ======================================================================
    protected:
      // ======================================================================
      RooRealProxy m_x      ;
      RooRealProxy m_theta  ;
      RooRealProxy m_alpha  ;
      RooRealProxy m_beta   ;
      RooRealProxy m_a      ;
      // ======================================================================
    private:
      // ======================================================================
      /// the actual function 
      mutable Gaudi::Math::Amoroso m_amoroso ;                  // the function 
      // ======================================================================      
    } ;
    // ========================================================================
    /** @class LogGammaDist 
     *  Distribution for log(x), where x follows 
     *  gamma-distribution shape/scale parameters 
     *  http://en.wikipedia.org/wiki/Gamma_distribution
     *  @author Vanya BELYAEV Ivan.Belyaev@itep.ru
     *  @date   2013-05-11
     *  @see Gaudi::Math::LogGammaDist 
     */
    class GAUDI_API LogGammaDist : public RooAbsPdf 
    {
      // ======================================================================
    public :
      // ======================================================================
      ClassDef(Analysis::Models::LogGammaDist, 1) ;
      // ======================================================================
    public:
      // ======================================================================
      /// constructor from all parameters
      LogGammaDist ( const char*          name      , 
                     const char*          title     ,
                     RooAbsReal&          x         , 
                     RooAbsReal&          k         ,
                     RooAbsReal&          theta     ) ;
      /// "copy constructor"
      LogGammaDist ( const LogGammaDist&  right     , 
                     const char*          name  = 0 )  ;
      /// destructor 
      virtual ~LogGammaDist () ;
      /// clone 
      virtual  LogGammaDist* clone ( const char* name ) const ; 
      // ======================================================================
    public:
      // ======================================================================
      // the actual evaluation of function 
      virtual Double_t evaluate() const ;
      // ======================================================================
    public:
      // ======================================================================
      /// access to underlying function 
      const Gaudi::Math::LogGammaDist& function() const { return m_gamma ; }
      // ======================================================================
    protected: 
      // ======================================================================
      RooRealProxy m_x        ;
      RooRealProxy m_k        ;
      RooRealProxy m_theta    ;
      // ======================================================================
    private:
      // ======================================================================
      /// the actual function
      mutable Gaudi::Math::LogGammaDist m_gamma ; // the actual function
      // ======================================================================
    } ;  
    // ========================================================================
    /** @class Log10GammaDist 
     *  Distribution for log10(x), where x follows 
     *  gamma-distribution shape/scale parameters 
     *  http://en.wikipedia.org/wiki/Gamma_distribution
     *  @author Vanya BELYAEV Ivan.Belyaev@itep.ru
     *  @date   2013-05-11
     *  @see Gaudi::Math::Log10GammaDist 
     */
    class GAUDI_API Log10GammaDist : public RooAbsPdf 
    {
      // ======================================================================
    public :
      // ======================================================================
      ClassDef(Analysis::Models::Log10GammaDist, 1) ;
      // ======================================================================
    public:
      // ======================================================================
      /// constructor from all parameters
      Log10GammaDist ( const char*           name      , 
                       const char*           title     ,
                       RooAbsReal&           x         , 
                       RooAbsReal&           k         ,
                       RooAbsReal&           theta     ) ;
      /// "copy constructor"
      Log10GammaDist ( const Log10GammaDist& right     , 
                       const char*           name  = 0 )  ;
      /// destructor 
      virtual ~Log10GammaDist () ;
      /// clone 
      virtual  Log10GammaDist* clone ( const char* name ) const ; 
      // ======================================================================
    public:
      // ======================================================================
      // the actual evaluation of function 
      virtual Double_t evaluate() const ;
      // ======================================================================
    public:
      // ======================================================================
      /// access to underlying function 
      const Gaudi::Math::Log10GammaDist& function() const { return m_gamma ; }
      // ======================================================================
    protected: 
      // ======================================================================
      RooRealProxy m_x        ;
      RooRealProxy m_k        ;
      RooRealProxy m_theta    ;
      // ======================================================================
    private:
      // ======================================================================
      /// the actual function
      mutable Gaudi::Math::Log10GammaDist m_gamma ; // the actual function
      // ======================================================================
    } ;  
    // ========================================================================
    /** @class LogGamma
     *  - http://arxiv.org/pdf/1005.3274
     *  - Prentice, R. L. (1974). A log gamma model and its maximum likelihood
     *                            estimation. Biometrika 61, 539
     *  - Johnson, N. L., Kotz, S., and Balakrishnan, N. (1995). Continuous
     *            univariate distributions, 2nd ed. Vol. 2. Wiley, New York.
     *  - Bartlett, M. S. and G., K. M. (1946). The statistical analysis of
     *                  variance-heterogeneity and the logarithmic transformation. 
     *                 J. Roy. Statist. Soc. Suppl. 8, 1, 128.
     *
     *  dot not mix with Analysis::Models::LogGammaDist        
     *  @author Vanya BELYAEV Ivan.Belyaev@itep.ru
     *  @date   2013-05-11
     *  @see Gaudi::Math::LogGamma 
     */
    class GAUDI_API LogGamma : public RooAbsPdf 
    {
      // ======================================================================
    public :
      // ======================================================================
      ClassDef(Analysis::Models::LogGamma, 1) ;
      // ======================================================================
    public:
      // ======================================================================
      /// constructor from all parameters
      LogGamma ( const char*          name      , 
                 const char*          title     ,
                 RooAbsReal&          x         , 
                 RooAbsReal&          nu        ,
                 RooAbsReal&          lambda    ,
                 RooAbsReal&          alpha     ) ;
      /// "copy constructor"
      LogGamma ( const LogGamma&      right     , 
                 const char*          name  = 0 )  ;
      /// destructor 
      virtual ~LogGamma () ;
      /// clone 
      virtual  LogGamma* clone ( const char* name ) const ; 
      // ======================================================================
    public:
      // ======================================================================
      // the actual evaluation of function 
      virtual Double_t evaluate() const ;
      // ======================================================================
    public:
      // ======================================================================
      /// access to underlying function 
      const Gaudi::Math::LogGamma& function() const { return m_lgamma ; }
      // ======================================================================
    protected: 
      // ======================================================================
      RooRealProxy m_x        ;
      RooRealProxy m_nu       ;
      RooRealProxy m_lambda   ;
      RooRealProxy m_alpha    ;
      // ======================================================================
    private:
      // ======================================================================
      /// the actual function
      mutable Gaudi::Math::LogGamma m_lgamma ; // the actual function
      // ======================================================================
    } ;
    // ========================================================================
    /** @class BetaPrime
     *  http://en.wikipedia.org/wiki/Beta_prime_distribution
     *  @author Vanya BELYAEV Ivan.Belyaev@itep.ru
     *  @date   2013-05-11
     *  @see Gaudi::Math::BetaPrime 
     */
    class GAUDI_API BetaPrime : public RooAbsPdf 
    {
      // ======================================================================
    public :
      // ======================================================================
      ClassDef(Analysis::Models::BetaPrime, 1) ;
      // ======================================================================
    public:
      // ======================================================================
      /// constructor from all parameters
      BetaPrime 
      ( const char*           name      , 
        const char*           title     ,
        RooAbsReal&           x         , 
        RooAbsReal&           alpha     ,
        RooAbsReal&           beta      ) ;
      /// "copy constructor"
      BetaPrime 
      ( const BetaPrime&      right     , 
        const char*           name  = 0 )  ;
      /// destructor 
      virtual ~BetaPrime () ;
      /// clone 
      virtual  BetaPrime* clone ( const char* name ) const ; 
      // ======================================================================
    public:
      // ======================================================================
      // the actual evaluation of function 
      virtual Double_t evaluate() const ;
      // ======================================================================
    public:
      // ======================================================================
      /// access to underlying function 
      const Gaudi::Math::Log10GammaDist& function() const { return m_gamma ; }
      // ======================================================================
    protected: 
      // ======================================================================
      RooRealProxy m_x        ;
      RooRealProxy m_k        ;
      RooRealProxy m_theta    ;
      // ======================================================================
    private:
      // ======================================================================
      /// the actual function
      mutable Gaudi::Math::Log10GammaDist m_gamma ; // the actual function
      // ======================================================================
    } ;
    // ========================================================================
    // 2D-models 
    // ========================================================================
    /** @class Poly2DPositive
     *  Poly2DPositive polynomial
     *  @see Gaudi::Math::Positive2D
     *  @author Vanya BELYAEV  Ivan.Belyaev@itep.ru
     *  @date 2011-05-25
     */
    class GAUDI_API Poly2DPositive: public RooAbsPdf 
    {
      // ======================================================================
    public :
      // ======================================================================
      ClassDef(Analysis::Models::Poly2DPositive, 1) ;
      // ======================================================================
    public:
      // ======================================================================
      /// linear 
      Poly2DPositive
      ( const char*          name      , 
        const char*          title     ,
        RooRealVar&          x         ,
        RooRealVar&          y         ,
        const unsigned short nX        , 
        const unsigned short nY        , 
        RooArgList&          phis      ) ; // at least (n+1)*(n+2)-1 elements 
      /// copy
      Poly2DPositive
      ( const Poly2DPositive&     right     , 
        const char*          name = 0  ) ;
      /// destructor 
      virtual ~Poly2DPositive() ;
      /// clone 
      virtual  Poly2DPositive* clone ( const char* name ) const ; 
      // ======================================================================
    public:
      // ======================================================================
      // the actual evaluation of function 
      virtual Double_t evaluate() const ;
      // ======================================================================
    public:  // integrals 
      // ======================================================================
      virtual Int_t    getAnalyticalIntegral
        ( RooArgSet&     allVars      , 
          RooArgSet&     analVars     ,
          const char* /* rangename */ ) const ;
      virtual Double_t analyticalIntegral 
        ( Int_t          code         , 
          const char*    rangeName    ) const ;
      // ======================================================================
    public:
      // ======================================================================
      /// access to underlying function 
      const Gaudi::Math::Positive2D& function() const { return m_positive ; }
      // ======================================================================
    protected :
      // ======================================================================
      RooRealProxy m_x    ;
      RooRealProxy m_y    ;
      RooListProxy m_phis ;
      // ======================================================================
      TIterator* m_iterator;  //! do not persist
      // ======================================================================
    private:
      // ======================================================================
      /// the actual function 
      mutable Gaudi::Math::Positive2D m_positive ;              // the function 
      // ======================================================================
    } ;
    // ========================================================================
    /** @class Poly2DSymPositive
     *  Poly2DSymPositive polynomial
     *  @see Gaudi::Math::Positive2D
     *  @author Vanya BELYAEV  Ivan.Belyaev@itep.ru
     *  @date 2011-05-25
     */
    class GAUDI_API Poly2DSymPositive: public RooAbsPdf 
    {
      // ======================================================================
    public :
      // ======================================================================
      ClassDef(Analysis::Models::Poly2DSymPositive, 1) ;
      // ======================================================================
    public:
      // ======================================================================
      /// linear 
      Poly2DSymPositive
      ( const char*          name      , 
        const char*          title     ,
        RooRealVar&          x         ,
        RooRealVar&          y         ,
        const unsigned short n         , 
        RooArgList&          phis      ) ; // at least (nX+1)*(nY+1)-1 elements 
      /// copy
      Poly2DSymPositive
      ( const Poly2DSymPositive&     right     , 
        const char*          name = 0  ) ;
      /// destructor 
      virtual ~Poly2DSymPositive() ;
      /// clone 
      virtual  Poly2DSymPositive* clone ( const char* name ) const ; 
      // ======================================================================
    public:
      // ======================================================================
      // the actual evaluation of function 
      virtual Double_t evaluate() const ;
      // ======================================================================
    public:  // integrals 
      // ======================================================================
      virtual Int_t    getAnalyticalIntegral
        ( RooArgSet&     allVars      , 
          RooArgSet&     analVars     ,
          const char* /* rangename */ ) const ;
      virtual Double_t analyticalIntegral 
        ( Int_t          code         , 
          const char*    rangeName    ) const ;
      // ======================================================================
    public:
      // ======================================================================
      /// access to underlying function 
      const Gaudi::Math::Positive2DSym& function() const { return m_positive ; }
      // ======================================================================
    protected :
      // ======================================================================
      RooRealProxy m_x    ;
      RooRealProxy m_y    ;
      RooListProxy m_phis ;
      // ======================================================================
      TIterator* m_iterator;  //! do not persist
      // ======================================================================
    private:
      // ======================================================================
      /// the actual function 
      mutable Gaudi::Math::Positive2DSym m_positive ;           // the function 
      // ======================================================================
    } ;
    // ========================================================================
    /** @class PS22DPol
     * 
     *  F(x,y) = PS2(x)*PS2(y)*PPOL(x,y)
     *
     *  @author Vanya BELYAEV  Ivan.Belyaev@itep.ru
     *  @date 2011-05-25
     */
    class GAUDI_API PS22DPol: public RooAbsPdf 
    {
      // ======================================================================
    public :
      // ======================================================================
      ClassDef(Analysis::Models::PS22DPol, 1) ;
      // ======================================================================
    public:
      // ======================================================================
      /// linear 
      PS22DPol
      ( const char*                     name      , 
        const char*                     title     ,
        RooRealVar&                     x         ,
        RooRealVar&                     y         ,
        const Gaudi::Math::PhaseSpace2& ps1       , 
        const Gaudi::Math::PhaseSpace2& ps2       ,
        const unsigned short nX                   , 
        const unsigned short nY                   , 
        RooArgList&          phis      ) ; // at least (nX+1)*(nY+1)-1 elements 
      /// copy
      PS22DPol
      ( const PS22DPol&       right     , 
        const char*          name = 0  ) ;
      /// destructor 
      virtual ~PS22DPol() ;
      /// clone 
      virtual  PS22DPol* clone ( const char* name ) const ; 
      // ======================================================================
    public:
      // ======================================================================
      // the actual evaluation of function 
      virtual Double_t evaluate() const ;
      // ======================================================================
    public:
      // ======================================================================
      /// access to underlying function(s) 
      const Gaudi::Math::Positive2D&   function() const { return m_positive ; }
      const Gaudi::Math::PhaseSpace2&  ps1     () const { return m_ps1      ; }
      const Gaudi::Math::PhaseSpace2&  ps2     () const { return m_ps2      ; }      
      // ======================================================================
    protected :
      // ======================================================================
      RooRealProxy m_x    ;
      RooRealProxy m_y    ;
      RooListProxy m_phis ;
      // ======================================================================
      TIterator* m_iterator;  //! do not persist
      // ======================================================================
    private:
      // ======================================================================
      /// the actual functions()
      mutable Gaudi::Math::Positive2D m_positive ;              // the function 
      Gaudi::Math::PhaseSpace2  m_ps1  ;
      Gaudi::Math::PhaseSpace2  m_ps2  ;
      // ======================================================================
    } ;
    // ========================================================================
    /** @class PSnl2DPol
     * 
     *  F(x,y) = PSnl(x)*PSnl(y)*PPOL(x,y)
     *
     *  @author Vanya BELYAEV  Ivan.Belyaev@itep.ru
     *  @date 2011-05-25
     */
    class GAUDI_API PSnl2DPol: public RooAbsPdf 
    {
      // ======================================================================
    public :
      // ======================================================================
      ClassDef(Analysis::Models::PSnl2DPol, 1) ;
      // ======================================================================
    public:
      // ======================================================================
      /// linear 
      PSnl2DPol
      ( const char*                      name      , 
        const char*                      title     ,
        RooRealVar&                      x         ,
        RooRealVar&                      y         ,
        const Gaudi::Math::PhaseSpaceNL& ps1       , 
        const Gaudi::Math::PhaseSpaceNL& ps2       ,
        const unsigned short nX                    , 
        const unsigned short nY                    , 
        RooArgList&          phis      ) ; // at least (nX+1)*(nY+1)-1 elements 
      /// copy
      PSnl2DPol
      ( const PSnl2DPol&       right     , 
        const char*            name = 0  ) ;
      /// destructor 
      virtual ~PSnl2DPol() ;
      /// clone 
      virtual  PSnl2DPol* clone ( const char* name ) const ; 
      // ======================================================================
    public:
      // ======================================================================
      // the actual evaluation of function 
      virtual Double_t evaluate() const ;
      // ======================================================================
    public:
      // ======================================================================
      /// access to underlying function(s) 
      const Gaudi::Math::Positive2D&   function() const { return m_positive ; }
      const Gaudi::Math::PhaseSpaceNL& ps1     () const { return m_ps1      ; }
      const Gaudi::Math::PhaseSpaceNL& ps2     () const { return m_ps2      ; }      
      // ======================================================================
    protected :
      // ======================================================================
      RooRealProxy m_x    ;
      RooRealProxy m_y    ;
      RooListProxy m_phis ;
      // ======================================================================
      TIterator* m_iterator;  //! do not persist
      // ======================================================================
    private:
      // ======================================================================
      /// the actual functions()
      mutable Gaudi::Math::Positive2D m_positive ;              // the function 
      Gaudi::Math::PhaseSpaceNL       m_ps1  ;
      Gaudi::Math::PhaseSpaceNL       m_ps2  ;
      // ======================================================================
    } ;
    // ========================================================================
    /** @class PS2s2DPol
     * 
     *  F(x,y) = PS2(x)*PS2(y)*PPOL(x,y)
     *
     *  @author Vanya BELYAEV  Ivan.Belyaev@itep.ru
     *  @date 2011-05-25
     */
    class GAUDI_API PS2s2DPol: public RooAbsPdf 
    {
      // ======================================================================
    public :
      // ======================================================================
      ClassDef(Analysis::Models::PS2s2DPol, 1) ;
      // ======================================================================
    public:
      // ======================================================================
      /// linear 
      PS2s2DPol
      ( const char*                     name      , 
        const char*                     title     ,
        RooRealVar&                     x         ,
        RooRealVar&                     y         ,
        const Gaudi::Math::PhaseSpace2& ps        , 
        const unsigned short            n                   , 
        RooArgList&                     phis      ) ; // at least (n+1)*(n+2)-1 elements 
      /// copy
      PS2s2DPol
      ( const PS2s2DPol&     right     , 
        const char*          name = 0  ) ;
      /// destructor 
      virtual ~PS2s2DPol() ;
      /// clone 
      virtual  PS2s2DPol* clone ( const char* name ) const ; 
      // ======================================================================
    public:
      // ======================================================================
      // the actual evaluation of function 
      virtual Double_t evaluate() const ;
      // ======================================================================
    public:
      // ======================================================================
      /// access to underlying function(s) 
      const Gaudi::Math::Positive2DSym&function() const { return m_positive ; }
      const Gaudi::Math::PhaseSpace2&  ps      () const { return m_ps       ; }
      // ======================================================================
    protected :
      // ======================================================================
      RooRealProxy m_x    ;
      RooRealProxy m_y    ;
      RooListProxy m_phis ;
      // ======================================================================
      TIterator* m_iterator;  //! do not persist
      // ======================================================================
    private:
      // ======================================================================
      /// the actual functions()
      mutable Gaudi::Math::Positive2DSym m_positive ;          // the function 
      Gaudi::Math::PhaseSpace2           m_ps  ;
      // ======================================================================
    } ;
    // ========================================================================
    /** @class PSnls2DPol
     * 
     *  F(x,y) = PS2(x)*PS2(y)*PPOL(x,y)
     *
     *  @author Vanya BELYAEV  Ivan.Belyaev@itep.ru
     *  @date 2011-05-25
     */
    class GAUDI_API PSnls2DPol: public RooAbsPdf 
    {
      // ======================================================================
    public :
      // ======================================================================
      ClassDef(Analysis::Models::PSnls2DPol, 1) ;
      // ======================================================================
    public:
      // ======================================================================
      /// linear 
      PSnls2DPol
      ( const char*                      name      , 
        const char*                      title     ,
        RooRealVar&                      x         ,
        RooRealVar&                      y         ,
        const Gaudi::Math::PhaseSpaceNL& ps        , 
        const unsigned short             n                   , 
        RooArgList&                      phis      ) ; // at least (n+1)*(n+2)-1 elements 
      /// copy
      PSnls2DPol
      ( const PSnls2DPol&     right     , 
        const char*          name = 0  ) ;
      /// destructor 
      virtual ~PSnls2DPol() ;
      /// clone 
      virtual  PSnls2DPol* clone ( const char* name ) const ; 
      // ======================================================================
    public:
      // ======================================================================
      // the actual evaluation of function 
      virtual Double_t evaluate() const ;
      // ======================================================================
    public:
      // ======================================================================
      /// access to underlying function(s) 
      const Gaudi::Math::Positive2DSym&function() const { return m_positive ; }
      const Gaudi::Math::PhaseSpaceNL& ps      () const { return m_ps       ; }
      // ======================================================================
    protected :
      // ======================================================================
      RooRealProxy m_x    ;
      RooRealProxy m_y    ;
      RooListProxy m_phis ;
      // ======================================================================
      TIterator* m_iterator;  //! do not persist
      // ======================================================================
    private:
      // ======================================================================
      /// the actual functions()
      mutable Gaudi::Math::Positive2DSym m_positive ;          // the function 
      Gaudi::Math::PhaseSpaceNL          m_ps  ;
      // ======================================================================
    } ;
  } //                                        end of namespace Analysis::Models
  // ==========================================================================
} //                                                  end of namespace Analysis 
// ============================================================================
//                                                                      The END 
// ============================================================================
#endif // ANALYSIS_MODELS_H
// ============================================================================
