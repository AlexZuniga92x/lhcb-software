// $Id$
// ============================================================================
#ifndef LOKI_HLTSTAGES_H 
#define LOKI_HLTSTAGES_H 1
// ============================================================================
// $URL$
// ============================================================================
// Include files
// ============================================================================
// HltBsse 
// ============================================================================
#include "Event/HltStage.h"
// ============================================================================
// LoKi
// ============================================================================
#include "LoKi/BasicFunctors.h"
#include "LoKi/TrackTypes.h"
// ============================================================================
// Boost
// ============================================================================
#include "boost/regex.hpp"
// ============================================================================
/** @file  LoKi/HltStages.h
 *
 *  Hlt-Stages functors 
 * 
 *  This file is part of LoKi project: 
 *   ``C++ ToolKit for Smart and Friendly Physics Analysis''
 * 
 *  By usage of this code one clearly states the disagreement 
 *  with the campain of Dr.O.Callot et al.: 
 *  ``No Vanya's lines are allowed in LHCb/Gaudi software.''
 *
 *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
 *  @date 2010-08-01
 *
 *                    $Revision$
 *  Last Modification $Date$ 
 *                 by $Author$ 
 */
// ============================================================================
namespace LoKi 
{
  // ==========================================================================
  /** @namespace LoKi::Stages
   *  collection of functors that deal with Hlt::Stage objects 
   *  @see Hlt::Stage
   *  @author Vanya Belyaev Ivan.BElyaev@nikhef.nl
   *  @date 2010-08-01
   */
  namespace Stages
  {
    // ========================================================================
    /** @class IsTrack
     *  trivial predicate to check if the stage is a track
     *  @author Vanya BELAYEV Ivan.Belyaev@nikhef.nl
     *  @see LoKi::Cuts::TS_TRACK 
     *  @date 2010-08-01
     */
    class IsTrack 
      : public LoKi::BasicFunctors<const Hlt::Stage*>::Predicate
    { 
    public:
      // ======================================================================
      /// MANDATORY: virtual destructor 
      virtual ~IsTrack() ;
      /// MANDATORY: clone method ("virtual constructor") 
      virtual  IsTrack* clone() const ;
      /// MANDATORY: the only one essential method 
      virtual  result_type operator() ( argument a ) const ;
      /// OPTIONAL: the ince printout 
      virtual  std::ostream& fillStream ( std::ostream& s ) const ;
      // ======================================================================
    };      
    // ========================================================================
    /** @class IsL0Muon
     *  trivial predicate to check if the stage is a L0-muon stage 
     *  @author Vanya BELAYEV Ivan.Belyaev@nikhef.nl
     *  @see LoKi::Cuts::TS_L0MUON 
     *  @date 2010-08-01
     */
    class IsL0Muon : public IsTrack
    { 
    public:
      // ======================================================================
      /// MANDATORY: virtual destructor 
      virtual ~IsL0Muon () ;
      /// MANDATORY: clone method ("virtual constructor") 
      virtual  IsL0Muon* clone() const ;
      /// MANDATORY: the only one essential method 
      virtual  result_type operator() ( argument a ) const ;
      /// OPTIONAL: the ince printout 
      virtual  std::ostream& fillStream ( std::ostream& s ) const ;
      // ======================================================================
    };      
    // ========================================================================
    /** @class IsL0DiMuon
     *  trivial predicate to check if the stage is a L0-dimuon candidate 
     *  @author Vanya BELAYEV Ivan.Belyaev@nikhef.nl
     *  @see LoKi::Cuts::TS_L0DIMUON 
     *  @date 2010-08-01
     */
    class IsL0DiMuon : public IsL0Muon
    { 
    public:
      // =======================================================================
      /// MANDATORY: virtual destructor 
      virtual ~IsL0DiMuon () ;
      /// MANDATORY: clone method ("virtual constructor") 
      virtual  IsL0DiMuon* clone() const ;
      /// MANDATORY: the only one essential method 
      virtual  result_type operator() ( argument a ) const ;
      /// OPTIONAL: the ince printout 
      virtual  std::ostream& fillStream ( std::ostream& s ) const ;
      // ======================================================================
    };      
    // ========================================================================
    /** @class IsL0Calo
     *  trivial predicate to check if the stage is a L0-calo candidate 
     *  @author Vanya BELAYEV Ivan.Belyaev@nikhef.nl
     *  @see LoKi::Cuts::TS_L0CALO
     *  @date 2010-08-01
     */
    class IsL0Calo : public IsL0DiMuon
    { 
    public:
      // ======================================================================
      /// MANDATORY: virtual destructor 
      virtual ~IsL0Calo () ;
      /// MANDATORY: clone method ("virtual constructor") 
      virtual  IsL0Calo* clone() const ;
      /// MANDATORY: the only one essential method 
      virtual  result_type operator() ( argument a ) const ;
      /// OPTIONAL: the ince printout 
      virtual  std::ostream& fillStream ( std::ostream& s ) const ;
      // ======================================================================
    };      
    // ========================================================================
    /** @class IsVertex
     *  trivial predicate to check if the stage is a RecVertex
     *  @author Vanya BELAYEV Ivan.Belyaev@nikhef.nl
     *  @see LoKi::Cuts::TS_VERTEX
     *  @date 2010-08-01
     */
    class IsVertex : public IsL0Calo
    { 
    public:
      // ======================================================================
      /// MANDATORY: virtual destructor 
      virtual ~IsVertex () ;
      /// MANDATORY: clone method ("virtual constructor") 
      virtual  IsVertex* clone() const ;
      /// MANDATORY: the only one essential method 
      virtual  result_type operator() ( argument a ) const ;
      /// OPTIONAL: the ince printout 
      virtual  std::ostream& fillStream ( std::ostream& s ) const ;
      // ======================================================================
    };      
    // ========================================================================
    /** @class IsMultiTrack
     *  trivial predicate to check if the stage is a Multitrack
     *  @author Vanya BELAYEV Ivan.Belyaev@nikhef.nl
     *  @see LoKi::Cuts::TS_MULTITRACK
     *  @date 2010-08-01
     */
    class IsMultiTrack : public IsVertex
    { 
    public:
      // ======================================================================
      /// MANDATORY: virtual destructor 
      virtual ~IsMultiTrack () ;
      /// MANDATORY: clone method ("virtual constructor") 
      virtual  IsMultiTrack* clone() const ;
      /// MANDATORY: the only one essential method 
      virtual  result_type operator() ( argument a ) const ;
      /// OPTIONAL: the ince printout 
      virtual  std::ostream& fillStream ( std::ostream& s ) const ;
      // ======================================================================
    };
    // ========================================================================
    /** @class IsStage
     *  trivial predicate to check if the stage is a Stage
     *  @author Vanya BELAYEV Ivan.Belyaev@nikhef.nl
     *  @see LoKi::Cuts::TS_STAGE
     *  @date 2010-08-01
     */
    class IsStage : public IsMultiTrack
    { 
    public:
      // ======================================================================
      /// MANDATORY: virtual destructor 
      virtual ~IsStage () ;
      /// MANDATORY: clone method ("virtual constructor") 
      virtual  IsStage* clone() const ;
      /// MANDATORY: the only one essential method 
      virtual  result_type operator() ( argument a ) const ;
      /// OPTIONAL: the ince printout 
      virtual  std::ostream& fillStream ( std::ostream& s ) const ;
      // ======================================================================
    };
    // ========================================================================
    /** @class IsCandidate
     *  trivial predicate to check if the stage is a Candidate
     *  @author Vanya BELAYEV Ivan.Belyaev@nikhef.nl
     *  @see LoKi::Cuts::TS_CANDIDATE
     *  @date 2010-08-01
     */
    class IsCandidate : public IsStage
    { 
    public:
      // ======================================================================
      /// MANDATORY: virtual destructor 
      virtual ~IsCandidate () ;
      /// MANDATORY: clone method ("virtual constructor") 
      virtual  IsCandidate* clone() const ;
      /// MANDATORY: the only one essential method 
      virtual  result_type operator() ( argument a ) const ;
      /// OPTIONAL: the ince printout 
      virtual  std::ostream& fillStream ( std::ostream& s ) const ;
      // ======================================================================
    };
    // ========================================================================
    /** @class Type
     *  trivial function to get stage type 
     *  @author Vanya BELAYEV Ivan.Belyaev@nikhef.nl
     *  @see LoKi::Cuts::TS_TYPE 
     *  @date 2010-08-01
     */
    class Type 
      : public LoKi::BasicFunctors<const Hlt::Stage*>::Function
    { 
    public:
      // ======================================================================
      /// MANDATORY: virtual destructor 
      virtual ~Type () ;
      /// MANDATORY: clone method ("virtual constructor") 
      virtual  Type* clone() const ;
      /// MANDATORY: the only one essential method 
      virtual  result_type operator() ( argument a ) const ;
      /// OPTIONAL: the ince printout 
      virtual  std::ostream& fillStream ( std::ostream& s ) const ;
      // ======================================================================
    };      
    // ========================================================================
    /** @class TrFun
     *  trivial adaptor for track-stage  
     *  @author Vanya BELAYEV Ivan.Belyaev@nikhef.nl
     *  @see LoKi::Cuts::TS_TrFUN
     *  @date 2010-08-01
     */
    class TrFun 
      : public LoKi::BasicFunctors<const Hlt::Stage*>::Function
    { 
    public:
      // ======================================================================
      /// constructor 
      TrFun ( const LoKi::TrackTypes::TrFunc& fun , 
              const double                    bad ) ;      
      /// constructor 
      TrFun ( const LoKi::TrackTypes::TrFunc& fun ) ;
      /// MANDATORY: virtual destructor 
      virtual ~TrFun() ;
      /// MANDATORY: clone method ("virtual constructor") 
      virtual  TrFun* clone() const ;
      /// MANDATORY: the only one essential method 
      virtual  result_type operator() ( argument a ) const ;
      /// OPTIONAL: the ince printout 
      virtual  std::ostream& fillStream ( std::ostream& s ) const ;
      // ======================================================================
    private:
      // ======================================================================
      /// the default constructor is disabled 
      TrFun() ;                          // the default constructor is disabled 
      // ======================================================================
    private:
      // ======================================================================
      LoKi::TrackTypes::TrFun m_fun ;
      double                  m_bad ;
      // ======================================================================
    };      
    // ========================================================================
    /** @class TrCut
     *  trivial adaptor for track-stage  
     *  @author Vanya BELAYEV Ivan.Belyaev@nikhef.nl
     *  @see LoKi::Cuts::TS_TrCUT
     *  @date 2010-08-01
     */
    class TrCut
      : public LoKi::BasicFunctors<const Hlt::Stage*>::Predicate
    { 
    public:
      // ======================================================================
      /// consttuctor 
      TrCut ( const LoKi::TrackTypes::TrCuts& fun , 
              const bool                      bad ) ;      
      /// constructor 
      TrCut ( const LoKi::TrackTypes::TrCuts& fun ) ;
      /// MANDATORY: virtual destructor 
      virtual ~TrCut () ;
      /// MANDATORY: clone method ("virtual constructor") 
      virtual  TrCut* clone() const ;
      /// MANDATORY: the only one essential method 
      virtual  result_type operator() ( argument a ) const ;
      /// OPTIONAL: the ince printout 
      virtual  std::ostream& fillStream ( std::ostream& s ) const ;
      // ======================================================================
    private:
      // ======================================================================
      /// the default constructor is disabled 
      TrCut() ;                          // the default constructor is disabled 
      // ======================================================================
    private:
      // ======================================================================
      LoKi::TrackTypes::TrCut m_fun ;
      bool                    m_bad ;
      // ======================================================================
    };      
    // ========================================================================
    /** @class Locked
     *  trivial predicate to check Hlt::Stage::locked 
     *  @author Vanya BELAYEV Ivan.Belyaev@nikhef.nl
     *  @see LoKi::Cuts::TS_LOCKED
     *  @date 2010-08-01
     */
    class Locked
      : public LoKi::BasicFunctors<const Hlt::Stage*>::Predicate
    {
    public:
      // ======================================================================
      /// MANDATORY: virtual descructor 
      virtual ~Locked() ;
      /// MANDATORY: clone method ("virtual destructor")
      virtual  Locked* clone() const ;
      /// MANDATORY: the only one essential method 
      virtual  result_type operator() ( argument a ) const ;
      /// OPTIONAL: the ince printout 
      virtual  std::ostream& fillStream ( std::ostream& s ) const ;
      // ======================================================================
    } ;
    // ========================================================================
    /** @class History 
     *  simple predicate to check the presence of algorithm in history 
     *  @see LoKi::Cuts::TS_HISTORY 
     *  @author Vanya BELAYEV Ivan.Belyaev@nikhef.nl
     *  @date 2010-08-02
     */     
    class History 
      : public LoKi::BasicFunctors<const Hlt::Stage*>::Predicate
    {
    public:
      // ======================================================================
      /// constructor from the algorithm name
      History ( const std::string& alg ) ;
      /// MANDATORY: virtual descructor 
      virtual ~History() ;
      /// MANDATORY: clone method ("virtual destructor")
      virtual  History* clone() const ;
      /// MANDATORY: the only one essential method 
      virtual  result_type operator() ( argument a ) const ;
      /// OPTIONAL: the ince printout 
      virtual  std::ostream& fillStream ( std::ostream& s ) const ;
      // ======================================================================
    private:
      // ======================================================================
      /// the default constructor is disabled 
      History () ;                       // the default constructor is disabled 
      // ======================================================================
    protected:
      // ======================================================================
      /// the algorithm to be checked 
      std::string m_algorithm ;                  // the algorithm to be checked 
      // ======================================================================
    } ;
    // ========================================================================
    /** @class HistorySub 
     *  simple predicate to check the presence of algorithm in history 
     *  @see LoKi::Cuts::TS_HISTORY_SUB 
     *  @author Vanya BELAYEV Ivan.Belyaev@nikhef.nl
     *  @date 2010-08-02
     */     
    class HistorySub : public History 
    {
    public:
      // ======================================================================
      /// constructor from the algorithm name substring
      HistorySub ( const std::string& alg ) ;
      /// MANDATORY: virtual descructor 
      virtual ~HistorySub () ;
      /// MANDATORY: clone method ("virtual destructor")
      virtual  HistorySub* clone() const ;
      /// MANDATORY: the only one essential method 
      virtual  result_type operator() ( argument a ) const ;
      /// OPTIONAL: the ince printout 
      virtual  std::ostream& fillStream ( std::ostream& s ) const ;
      // ======================================================================
    private:
      // ======================================================================
      /// the default constructor is disabled 
      HistorySub () ;                    // the default constructor is disabled 
      // ======================================================================
    } ;
    // ========================================================================
    /** @class HistoryRegex 
     *  simple predicate to check the presence of algorithm in history 
     *  @see LoKi::Cuts::TS_HISTORY_RE
     *  @author Vanya BELAYEV Ivan.Belyaev@nikhef.nl
     *  @date 2010-08-02
     */     
    class HistoryRegex : public HistorySub
    {
    public:
      // ======================================================================
      /// constructor from the algorithm name pattern
      HistoryRegex ( const std::string& alg ) ;
      /// MANDATORY: virtual descructor 
      virtual ~HistoryRegex () ;
      /// MANDATORY: clone method ("virtual destructor")
      virtual  HistoryRegex* clone() const ;
      /// MANDATORY: the only one essential method 
      virtual  result_type operator() ( argument a ) const ;
      /// OPTIONAL: the ince printout 
      virtual  std::ostream& fillStream ( std::ostream& s ) const ;
      // ======================================================================
    private:
      // ======================================================================
      /// the default constructor is disabled 
      HistoryRegex () ;                  // the default constructor is disabled
      // ======================================================================
    private:
      // ======================================================================
      /// the regular expression 
      boost::regex  m_expression ;                    // the regular expression 
      // ======================================================================
    } ;
    // ========================================================================
    /** @class Cut_
     *  Helper class for implementation of various filters 
     *  @thanks Alexander Mazurov 
     *  @author Vanya BELYAEV Ivan.BElyaev@cern.ch
     *  @date 2010-11-21
     */
    template <class TYPE>
    class Cut_ : public LoKi::BasicFunctors<const Hlt::Stage*>::Predicate
    {
    public:
      // ==================================================================
      /// constructor from the predicate and the fake integer argument 
      Cut_
      ( const typename LoKi::BasicFunctors<const TYPE*>::Predicate& cut , 
        const int /* a */ )
        : LoKi::BasicFunctors<const Hlt::Stage*>::Predicate ()
        , m_cut  ( cut  )
      {}
      /// MANDATORY: virtual destructor
      virtual ~Cut_() {}
      /// MANDATORY: clone method ("virtual constructor")
      virtual  Cut_* clone() const { return new  Cut_ ( *this ) ; }
      /// MANDATORY: the only one essential method
      virtual      typename LoKi::BasicFunctors<const Hlt::Stage*>::Predicate::result_type 
      operator() ( typename LoKi::BasicFunctors<const Hlt::Stage*>::Predicate::argument a )  const 
      { return filterStage ( a ) ; }
      /// OPTIONAL: the nice printout
      virtual std::ostream& fillStream ( std::ostream& s ) const
      { return s << m_cut ; }    
      // ======================================================================
    public:
      // ======================================================================
      /** perform the actual evaluation 
       *  @attention predicate is appleid only for VALID stages
       *  @param stage (INPUT) the input stage 
       *  @return result of predicate evaluation for VALID stages
       */
      bool filterStage ( const Hlt::Stage* stage ) const 
      {
        //
        if ( 0 == stage ) 
        {
          Error ( "Invalid Stage, return false" ) ;
          return false ;                                         // RETURN 
        }
        // get the object from the stage 
        const TYPE* obj = stage->get<TYPE>() ;
        // use the actual predicate for VALID stage :
        return ( 0 != obj ) && m_cut.fun ( obj ) ;  
      } 
      // ======================================================================
    private:
      // ======================================================================
      /// default constructor is disabled
      Cut_ () ;                              // default constructor is disabled
      // ======================================================================
    private:
      // ======================================================================
      LoKi::FunctorFromFunctor<const TYPE*, bool> m_cut;
      // ======================================================================
    };
    // ========================================================================
    /// helper function to create the predicate 
    template <class TYPE>
    inline Cut_<TYPE> cut_ 
    ( const LoKi::Functor<const TYPE*,bool>& cut , const int fake )
    { return Cut_<TYPE> ( cut , fake ) ; }
    // ========================================================================
    /** @class Fun_
     *  Helper class for implementation
     *  @thanks Alexander Mazurov 
     *  @author Vanya BELYAEV Ivan.BElyaev@cern.ch
     *  @date 2010-11-21
     */
    template <class TYPE>
    class Fun_ : public LoKi::BasicFunctors<const Hlt::Stage*>::Function
    {
    public:
      // ==================================================================
      /// constructor from the function and "bad"-value 
      Fun_
      ( const typename LoKi::BasicFunctors<const TYPE*>::Function& fun , 
        const double                                               bad )
        : LoKi::BasicFunctors<const Hlt::Stage*>::Function ()
        , m_fun  ( fun )
        , m_bad  ( bad )
      {}
      /// MANDATORY: virtual destructor
      virtual ~Fun_() {}
      /// MANDATORY: clone method ("virtual constructor")
      virtual  Fun_* clone() const { return new  Fun_ ( *this ) ; }
      /// MANDATORY: the only one essential method
      virtual      typename LoKi::BasicFunctors<const Hlt::Stage*>::Function::result_type 
      operator() ( typename LoKi::BasicFunctors<const Hlt::Stage*>::Function::argument a )  const 
      { return evalStage ( a ) ; }
      /// OPTIONAL: the nice printout
      virtual std::ostream& fillStream ( std::ostream& s ) const
      { return s << m_fun ; }    
      // ======================================================================
    public:
      // ======================================================================
      /** perform the actual evaluation 
       *  @attention function is applied only for VALID stages
       *  @param stage (INPUT) the input stage 
       *  @return result of predicate evaluation for VALID stages
       */
      double evalStage ( const Hlt::Stage* stage ) const 
      {
        //
        if ( 0 == stage ) 
        {
          Error ( "Invalid Stage, return 'bad'" ) ;
          return m_bad ;                                         // RETURN 
        }
        // get the object from the stage 
        const TYPE* obj = stage->get<TYPE>() ;
        // use the actual predicate for VALID stage :
        return ( 0 != obj ) ? m_fun.fun ( obj ) : m_bad ;
      } 
      // ======================================================================
    private:
      // ======================================================================
      /// default constructor is disabled
      Fun_ () ;                              // default constructor is disabled
      // ======================================================================
    private:
      // ======================================================================
      /// the function 
      LoKi::FunctorFromFunctor<const TYPE*, double> m_fun ; // the function 
      /// bad-value 
      double                                        m_bad ; // bad-value 
      // ======================================================================
    };
    // ========================================================================
    /// helper function to create the function 
    template <class TYPE>
    inline Fun_<TYPE> fun_ 
    ( const LoKi::Functor<const TYPE*,double>& fun , const double bad )
    { return Fun_<TYPE> ( fun , bad ) ; }
    // ========================================================================
  } //                                            end of namespace LoKi::Stages 
  // ==========================================================================
  namespace Cuts 
  {
    // ========================================================================
    /** @var TS_TRACK 
     *  trivial predicate to check the type of Hlt::Stage 
     *  @see Hlt::Stage 
     *  @see Hlt::Stage::is<LHCb::Track>
     *  @see LHCb::Track
     *  @author Vanya BELAYEV Ivan.Belyaev@nikhef.nl
     *  @date 2010-08-02
     */
    const LoKi::Stages::IsTrack                                      TS_TRACK ;
    // ========================================================================
    /** @var TS_L0MUON 
     *  trivial predicate to check the type of Hlt::Stage 
     *  @see Hlt::Stage 
     *  @see Hlt::Stage::is<LHCb::L0MuonCandidate>
     *  @see LHCb::L0MuonCandidate
     *  @author Vanya BELAYEV Ivan.Belyaev@nikhef.nl
     *  @date 2010-08-02
     */
    const LoKi::Stages::IsL0Muon                                    TS_L0MUON ;    
    // ========================================================================
    /** @var TS_LDI0MUON 
     *  trivial predicate to check the type of Hlt::Stage 
     *  @see Hlt::Stage 
     *  @see Hlt::Stage::is<Hlt::L0DiMuonCandidate>
     *  @author Vanya BELAYEV Ivan.Belyaev@nikhef.nl
     *  @date 2010-08-02
     */
    const LoKi::Stages::IsL0DiMuon                                TS_L0DIMUON ;    
    // ========================================================================
    /** @var TS_L0CALO
     *  trivial predicate to check the type of Hlt::Stage 
     *  @see Hlt::Stage 
     *  @see Hlt::Stage::is<LHCb::L0CaloCandidate>
     *  @see LHCb::L0CaloCandidate
     *  @author Vanya BELAYEV Ivan.Belyaev@nikhef.nl
     *  @date 2010-08-02
     */
    const LoKi::Stages::IsL0Calo                                    TS_L0CALO ;
    // ========================================================================
    /** @var TS_VERTEX
     *  trivial predicate to check the type of Hlt::Stage 
     *  @see Hlt::Stage 
     *  @see Hlt::Stage::is<LHCb::RecVertex>
     *  @see LHCb::RecVertex
     *  @author Vanya BELAYEV Ivan.Belyaev@nikhef.nl
     *  @date 2010-08-02
     */
    const LoKi::Stages::IsVertex                                    TS_VERTEX ;
    // ========================================================================
    /** @var TS_MULTITRACK
     *  trivial predicate to check the type of Hlt::Stage 
     *  @see Hlt::Stage 
     *  @see Hlt::Stage::is<Hlt::MultiTrack>
     *  @author Vanya BELAYEV Ivan.Belyaev@nikhef.nl
     *  @date 2010-08-02
     */
    const LoKi::Stages::IsMultiTrack                            TS_MULTITRACK ;
    // ========================================================================
    /** @var TS_STAGE
     *  trivial predicate to check the type of Hlt::Stage 
     *  @see Hlt::Stage 
     *  @see Hlt::Stage::is<Hlt::Stage>
     *  @author Vanya BELAYEV Ivan.Belyaev@nikhef.nl
     *  @date 2010-08-02
     */
    const LoKi::Stages::IsStage                                      TS_STAGE ;
    // ========================================================================
    /** @var TS_CANDIDATE
     *  trivial predicate to check the type of Hlt::Stage 
     *  @see Hlt::Stage 
     *  @see Hlt::Stage::is<Hlt::Candidate>
     *  @author Vanya BELAYEV Ivan.Belyaev@nikhef.nl
     *  @date 2010-08-02
     */
    const LoKi::Stages::IsCandidate                              TS_CANDIDATE ;
    // ========================================================================
    /** @var TS_TYPE
     *  trivial functor to check the type of Hlt::Stage 
     *  @see Hlt::Stage 
     *  @see Hlt::Stage::stageType
     *  @author Vanya BELAYEV Ivan.Belyaev@nikhef.nl
     *  @date 2010-08-02
     */
    const LoKi::Stages::Type                                          TS_TYPE ;
    // ========================================================================
    /** @typedef TS_TrFUN
     *  trivial adapter for Track-function 
     *
     *  @code
     *  
     *   const TS_TrFUN pt ( TrPT ) ;
     *
     *  @endcode 
     *
     *  @see Hlt::Stage 
     *  @see LHCb::Track
     *  @author Vanya BELAYEV Ivan.Belyaev@nikhef.nl
     *  @date 2010-08-02
     */
    typedef LoKi::Stages::TrFun                                      TS_TrFUN ;
    // ========================================================================
    /** @typedef TS_TrCUT
     *  trivial adapter for Track-predicate 
     *
     *  @code
     *  
     *   const TS_TrCUT ok ( TrPT > 500 * MeV  ) ;
     *
     *  @endcode 
     *
     *  @see Hlt::Stage 
     *  @see LHCb::Track
     *  @author Vanya BELAYEV Ivan.Belyaev@nikhef.nl
     *  @date 2010-08-02
     */
    typedef LoKi::Stages::TrCut                                      TS_TrCUT ;
    // ========================================================================
    /** @var TS_LOCKED
     *  trivial predicate to check if Hlt::Stage is locked 
     *  @see Hlt::Stage
     *  @see Hlt::Stage::locked 
     *  @author Vanya BELAYEV Ivan.Belyaev@nikhef.nl
     *  @date 2010-08-02
     */
    const LoKi::Stages::Locked                                      TS_LOCKED ;
    // ========================================================================
    /** @typedef TS_HISTORY
     *  trivial predicate to checkcertain algorithm in history 
     *
     *  @code
     *  
     *   const TS_HISTORY ok ( "MyAlgorithm" ) ;
     *
     *  @endcode 
     *
     *  @see Hlt::Stage 
     *  @see LHCb::Track
     *  @author Vanya BELAYEV Ivan.Belyaev@nikhef.nl
     *  @date 2010-08-02
     */
    typedef LoKi::Stages::History                                  TS_HISTORY ;
    // ========================================================================
    /** @typedef TS_HISTORY_SUB
     *  trivial predicate to check certain sub-string algorithm in history 
     *
     *  @code
     *  
     *   const TS_HISTORY_SUB ok ( "DiMuon" ) ;
     *
     *  @endcode 
     *
     *  @see Hlt::Stage 
     *  @see LHCb::Track
     *  @author Vanya BELAYEV Ivan.Belyaev@nikhef.nl
     *  @date 2010-08-02
     */
    typedef LoKi::Stages::HistorySub                           TS_HISTORY_SUB ;
    // ========================================================================
    /** @typedef TS_HISTORY_RE
     *  trivial predicate to check certain sub-string algorithm in history 
     *
     *  @code
     *  
     *   const TS_HISTORY_RE ok ( "Hlt1.*MuonDecision" ) ;
     *
     *  @endcode 
     *
     *  @see Hlt::Stage 
     *  @see LHCb::Track
     *  @author Vanya BELAYEV Ivan.Belyaev@nikhef.nl
     *  @date 2010-08-02
     */
    typedef LoKi::Stages::HistoryRegex                          TS_HISTORY_RE ;
    // ========================================================================
  } //                                              end of namespace LoKi::Cuts 
  // ==========================================================================
} //                                                      end of namespace LoKi
// ============================================================================
// The END 
// ============================================================================
#endif // LOKI_HLTSTAGES_H
// ============================================================================
