 // $Id: $
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
      /// "copy" constructor 
      BreitWigner ( const BreitWigner& , const char* name = 0 ) ;
      /// virtual destructor 
      virtual ~BreitWigner() ;
      // ======================================================================
    protected:
      // ======================================================================
      RooRealProxy m_x     ;
      RooRealProxy m_mass  ;
      RooRealProxy m_width ;
      // ======================================================================
      // the actual evaluation of function 
      Double_t evaluate() const ;
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
      // the actual evaluation of function 
      virtual Double_t evaluate() const ;
      // ======================================================================
    protected:
      // ======================================================================
      /// the actual function 
      mutable Gaudi::Math::Flatte m_flatte ;             // the actual function 
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
      // ======================================================================
    protected:
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
      PhaseSpace2 ( const PhaseSpace2& right , const char* name )  ;
      /// destructor 
      virtual ~PhaseSpace2() ;
      // ======================================================================
    public:
      // ======================================================================
      RooRealProxy m_x     ;
      // ======================================================================
      // the actual evaluation of function 
      Double_t evaluate() const ;
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
      PhaseSpaceLeft ( const PhaseSpaceLeft& right , const char* name )  ;
      /// destructor 
      virtual ~PhaseSpaceLeft() ;
      // ======================================================================
    public:
      // ======================================================================
      RooRealProxy m_x         ;
      RooRealProxy m_threshold ;
      // ======================================================================
      // the actual evaluation of function 
      Double_t evaluate() const ;
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
      PhaseSpaceRight ( const PhaseSpaceRight& right , const char* name )  ;
      /// destructor 
      virtual ~PhaseSpaceRight () ;
      // ======================================================================
    public:
      // ======================================================================
      RooRealProxy m_x         ;
      RooRealProxy m_threshold ;
      // ======================================================================
      // the actual evaluation of function 
      Double_t evaluate() const ;
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
      // ======================================================================
    protected:
      // ======================================================================
      RooRealProxy m_x     ;
      RooRealProxy m_low   ;
      RooRealProxy m_high  ;
      // ======================================================================
      // the actual evaluation of function 
      Double_t evaluate() const ;
      // ======================================================================
    private:
      // ======================================================================
      /// the actual function 
      mutable Gaudi::Math::PhaseSpaceNL m_ps ;           // the actual function 
      // ======================================================================
    };
    // ========================================================================
    /** @class CrystalBallDS 
     *  double-sided ``Crystal Ball-function'' 
     *  for description of gaussian with the tail
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
        RooAbsReal&          alphaL    ,  // alpha_L - 1   
        RooAbsReal&          nL        ,  //     n_L - 1   
        RooAbsReal&          alphaR    ,  // alpha_R - 1   
        RooAbsReal&          nR        ); //     n_R - 1 
      /// "copy" constructor 
      CrystalBallDS ( const CrystalBallDS& right , const char* name = 0 ) ;
      /// virtual destructor 
      virtual ~CrystalBallDS() ;
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
      // the actual evaluation of function 
      Double_t evaluate() const ;
      // ======================================================================
    private:
      // ======================================================================
      /// the actual function 
      mutable Gaudi::Math::CrystalBallDoubleSided m_cb2 ;       // the function 
      // ======================================================================
    } ;
    // ========================================================================
    /** @class PolyPositive
     *  Non-negative polynomial: 
     *  the polinomial that  has no roots
     *  for the specified interval of observable
     *  for description of gaussian with the tail
     *  @see Gaudi::Math::PositiveN 
     *  @date 2011-05-25
     */
    class GAUDI_API PolyPositive : public RooAbsPdf 
    {
      // ======================================================================
    public :
      // ======================================================================
      ClassDef(Analysis::Models::PolyPositive, 1) ;
      // ======================================================================
    public:
      // ======================================================================
      /// linear polinomial
      PolyPositive 
      ( const char*          name      , 
        const char*          title     ,
        RooAbsReal&          x         ,
        RooAbsReal&          alpha1    , 
        const double         xmin      , 
        const double         xmax      ) ;
      /// quadric
      PolyPositive 
      ( const char*          name      , 
        const char*          title     ,
        RooAbsReal&          x         ,
        RooAbsReal&          alpha1    , 
        RooAbsReal&          alpha2    , 
        const bool           roots     ,  // allow roots ?
        const double         xmin      , 
        const double         xmax      ) ;
      /// qubic 
      PolyPositive 
      ( const char*          name      , 
        const char*          title     ,
        RooAbsReal&          x         ,
        RooAbsReal&          alpha1    , 
        RooAbsReal&          alpha2    , 
        RooAbsReal&          alpha3    , 
        const unsigned short N         ,  // roots ?
        const double         xmin      , 
        const double         xmax      ) ;
      /// quartic
      PolyPositive 
      ( const char*          name      , 
        const char*          title     ,
        RooAbsReal&          x         ,
        RooAbsReal&          alpha1    , 
        RooAbsReal&          alpha2    , 
        RooAbsReal&          alpha3    , 
        RooAbsReal&          alpha4    , 
        const unsigned short N         ,  // roots ?
        const double         xmin      , 
        const double         xmax      ) ;
      /// general 
      PolyPositive 
      ( const char*          name      , 
        const char*          title     ,
        RooAbsReal&          x         ,
        const RooArgList&    coeffs    ,
        const unsigned short N         ,  // roots ?
        const double         xmin      , 
        const double         xmax      ) ;
      /// copy 
      PolyPositive 
      ( const PolyPositive&  right     , 
        const char*          name = 0  ) ;
      /// destructor 
      virtual ~PolyPositive() ;
      // ======================================================================
    protected :
      // ======================================================================
      RooRealProxy m_x      ;
      RooListProxy m_alphas ;
      // ======================================================================
      TIterator* m_iterator;  //! do not persist
      // ======================================================================
      // the actual evaluation of function 
      Double_t evaluate() const ;
      // ======================================================================
    private:
      // ======================================================================
      /// the actual function 
      mutable Gaudi::Math::PositiveN m_pos ;                    // the function 
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
      // ======================================================================
    protected:
      // ======================================================================
      RooRealProxy m_x      ;
      RooRealProxy m_m0     ;
      RooRealProxy m_sigma  ;
      RooRealProxy m_kappa3 ;
      RooRealProxy m_kappa4 ;
      // ======================================================================
      // the actual evaluation of function 
      Double_t evaluate() const ;
      // ======================================================================
    private:
      // ======================================================================
      /// the actual function 
      mutable Gaudi::Math::GramCharlierA m_gca ;                // the function 
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
      // the actual evaluation of function 
      Double_t evaluate() const ;
      // ======================================================================
    private:
      // ======================================================================
      /// the actual function 
      mutable Gaudi::Math::Bukin m_bukin ;                      // the function 
      // ======================================================================      
    } ;  
    // ========================================================================
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
      // ======================================================================
    protected:
      // ======================================================================
      RooRealProxy m_x      ;
      RooRealProxy m_m0     ;
      RooRealProxy m_gamma  ;
      RooRealProxy m_sigma  ;
      // ======================================================================
      // the actual evaluation of function 
      Double_t evaluate() const ;
      // ======================================================================
    private:
      // ======================================================================
      /// the actual function 
      mutable Gaudi::Math::Voigt m_voigt ;                      // the function 
      // ======================================================================      
    };    
    // ========================================================================
  } //                                        end of namespace Analysis::Models
  // ==========================================================================
} //                                                  end of namespace Analysis 
// ============================================================================
//                                                                      The END 
// ============================================================================
#endif // ANALYSIS_MODELS_H
// ============================================================================
