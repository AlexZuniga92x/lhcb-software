// $Id: Tracks.h,v 1.7 2010-06-02 16:51:01 ibelyaev Exp $
// ============================================================================
#ifndef LOKI_TRACKS_H 
#define LOKI_TRACKS_H 1
// =======================================================================
// Include files
// ============================================================================
// Event 
// ============================================================================
#include "Event/Track.h"
// ============================================================================
// Track Interfaces 
// ============================================================================
#include "TrackInterfaces/ITrackSelector.h"
// ============================================================================
// LoKi
// ============================================================================
#include "LoKi/ExtraInfo.h"
#include "LoKi/TrackTypes.h"
#include "LoKi/Interface.h"
// ============================================================================
namespace LoKi
{
  // ==========================================================================
  /** @namespace LoKi::Tracks Tracks.h LoKi/Tracks.h
   *  Namespace with few basic "track"-functors 
   *  @author Vanya BELYAEV ibelyaev@physics.syr.edu
   *  @date   2007-06-08
   */
  namespace Tracks
  {
    // ========================================================================
    /** @class Key
     *  simple evaluator of the "key"
     *  @see LoKi::Cuts::TrKEY
     *  @author Vanya BELYAEV ibelyaev@physics.syr.edu
     *  @date   2007-06-08
     */  
    class Key : public LoKi::BasicFunctors<LHCb::Track>::Function
    {
    public:
      // ======================================================================
      /// MANDATORY: virtual destructor  
      virtual ~Key () {}
      /// MANDATORY: clone method ("virtual constructor")  
      virtual  Key* clone() const { return new Key(*this) ; }
      /// mandatory: the only one essential method 
      virtual result_type operator() ( argument t ) const { return t.key() ; }
      /// OPTIONAL: the nice printout 
      virtual std::ostream& fillStream( std::ostream& s ) const 
      { return s << "TrKEY" ; }
      // ======================================================================
    } ;
    // ========================================================================
    /** @class InTES
     *  Simple predicate which check is teh obejct registered in TES
     *  @see LoKi::Cuts::TrINTES
     *  @author Vanya BELYAEV ibelyaev@physics.syr.edu
     *  @date   2007-06-08
     */  
    class InTES : public LoKi::BasicFunctors<LHCb::Track>::Predicate
    {
    public:
      // ======================================================================
      /// MANDATORY: virtual destrcutor  
      virtual ~InTES () {}
      /// MANDATORY: clone method ("virtual constructor")  
      virtual  InTES* clone() const { return new InTES(*this) ; }
      /// mandatory: the only one essential method 
      virtual result_type operator() ( argument t ) const
      { return 0 != t.parent() ; }
      /// OPTIONAL: the nice printout 
      virtual std::ostream& fillStream( std::ostream& s ) const 
      { return s << "TrINTES" ; }
      // ======================================================================
    } ;
    // ========================================================================
    /** @class Charge
     *  simple evaluator of the charge of the track 
     *  @see LoKi::Cuts::TrQ
     *  @author Vanya BELYAEV ibelyaev@physiocs.syr.edu
     *  @date   2007-06-08
     */  
    class Charge : public LoKi::BasicFunctors<LHCb::Track>::Function
    {
    public:
      // ======================================================================
      /// MANDATORY: virtual destrcutor  
      virtual ~Charge () {}
      /// MANDATORY: clone method ("virtual constructor")  
      virtual  Charge* clone() const { return new Charge(*this) ; }
      /// mandatory: the only one essential method 
      virtual result_type operator() ( argument t ) const { return t.charge() ; }
      /// OPTIONAL: the nice printout 
      virtual std::ostream& fillStream( std::ostream& s ) const 
      { return s << "TrQ" ; }
      // ======================================================================
    } ;
    // ========================================================================
    /** @class TransverseMomentum 
     *  simple evaluator of transverse momentum of the track
     *  @see LoKi::Cuts::TrPT
     *  @author Vanya BELYAEV ibelyaev@physiocs.syr.edu
     *  @date   2007-06-08
     */  
    class TransverseMomentum : public LoKi::BasicFunctors<LHCb::Track>::Function
    {
    public:
      // ======================================================================
      /// MANDATORY: virtual destrcutor  
      virtual ~TransverseMomentum() {}
      /// MANDATORY: clone method ("virtual constructor")  
      virtual  TransverseMomentum* clone() const 
      { return new TransverseMomentum(*this) ; }
      /// mandatory: the only one essential method 
      virtual result_type operator() ( argument t ) const { return t.pt() ; }
      /// OPTIONAL: the nice printout 
      virtual std::ostream& fillStream( std::ostream& s ) const 
      { return s << "TrPT" ; }
      // ======================================================================
    } ;
    // ========================================================================
    /** @class Momentum 
     *  @see LoKi::Cuts::TrP 
     *  simple evaluator of momentum of the track
     *  @author Vanya BELYAEV ibelyaev@physiocs.syr.edu
     *  @date   2007-06-08
     */  
    class Momentum : public LoKi::BasicFunctors<LHCb::Track>::Function
    {
    public:
      // ======================================================================
      /// MANDATORY: virtual destrcutor  
      virtual ~Momentum() {}
      /// MANDATORY: clone method ("virtual constructor")  
      virtual  Momentum* clone() const { return new Momentum(*this) ; }
      /// mandatory: the only one essential method 
      virtual result_type operator() ( argument t ) const { return t.p() ; }
      /// OPTIONAL: the nice printout 
      virtual std::ostream& fillStream( std::ostream& s ) const 
      { return s << "TrP" ; }
      // ======================================================================
    } ;
    // ========================================================================
    /** @class CheckFlag 
     *  @see LoKi::Cuts::TrISFLAG
     *  @see LoKi::Cuts::TrBACKWARD
     *  @see LoKi::Cuts::TrINVALID
     *  @see LoKi::Cuts::TrCLONE
     *  @see LoKi::Cuts::TrUSED
     *  @see LoKi::Cuts::TrIPSELECTED
     *  @see LoKi::Cuts::TrPIDSELECTED
     *  @see LoKi::Cuts::TrSELECTED
     *  @see LoKi::Cuts::TrL0CANDIDATE
     *  @simple predicate to check the flag 
     *  @author Vanya BELYAEV ibelyaev@physiocs.syr.edu
     *  @date   2007-06-08
     */  
    class CheckFlag : public LoKi::BasicFunctors<LHCb::Track>::Predicate 
    {
    public:
      // ======================================================================
      /// constructor form the flag ; 
      CheckFlag ( LHCb::Track::Flags flag ) ;
      /// copy constructor 
      CheckFlag ( const CheckFlag& right  ) ;
      /// MANDATORY: virtual destrcutor  
      virtual ~CheckFlag() {}
      /// MANDATORY: clone method ("virtual constructor")  
      virtual  CheckFlag* clone() const { return new CheckFlag (*this) ; }
      /// MANDATORY: the only one essential method 
      virtual result_type operator() ( argument t ) const 
      { return t.checkFlag( m_flag ) ; }
      /// OPTIONAL: the nice printout 
      virtual std::ostream& fillStream( std::ostream& s ) const ;
      // ======================================================================
    private:
      // ======================================================================
      // the default constructor is disabled
      CheckFlag() ; ///< no default contructor
      // ======================================================================
    private:
      // ======================================================================
      // the flag to be checked:
      LHCb::Track::Flags m_flag ; ///< the flag to be checked:
      // ======================================================================
    } ;    
    // ========================================================================
    /** @class Selector
     *  Simple class to use 
     *  @see ITrackSelectorInterface 
     *  @see LoKi:Cuts::TrSELECTOR
     *  @see LoKi:Cuts::TrFILTER
     *  @author Vanya BELYAEV ibelayev@physics.syr.edu
     *  @date 2007-06-10
     */
    class Selector : public LoKi::BasicFunctors<LHCb::Track>::Predicate
    {
    public :
      // ======================================================================
      /// constructor form the tool 
      Selector (                       ITrackSelector*  tool ) ;
      /// constructor form the tool 
      Selector ( const LoKi::Interface<ITrackSelector>& tool ) ;
      /// copy constructor 
      Selector ( const Selector& );
      /// MANDATORY: virtual destructor 
      virtual ~Selector() {}
      /// MANDATORY: clone method ("virtual constructor")  
      virtual  Selector* clone() const { return new Selector(*this) ; }
      /// MANDATORY: the only one essential method 
      virtual result_type operator() ( argument t ) const ;
      /// OPTIONAL: the nice printout 
      virtual std::ostream& fillStream( std::ostream& s ) const 
      { return s << "TrSELECTOR" ; }
      // ======================================================================
    public:
      // ======================================================================
      /// conversion operator to the tool
      operator const LoKi::Interface<ITrackSelector>&() const { return m_tool ;}    
      // ======================================================================
    private:
      // ======================================================================
      // the default constructor is disabled 
      Selector() ; ///< the default constructor is disabled 
      // ======================================================================
    private:
      // ======================================================================
      /// the tool itself 
      LoKi::Interface<ITrackSelector> m_tool ;            // the tool itself 
      // ======================================================================
    } ;
    // ========================================================================
    /** @class HasInfo
     *  Trivial predicate which evaluates LHCb::Track::hasInfo
     *  function
     *
     *  It relies on the method LoKi::Info::hasInfo
     *
     *  @see LHCb::Track
     *  @see LoKi::Cuts::TrHASINFO 
     *  @see LoKi::ExtraInfo::CheckInfo
     *  @see LoKi::ExtraInfo::hasInfo
     *
     *  @author Vanya BELYAEV ibelyaev@physics.syr.edu
     *  @date 2006-02-15
     */
    class HasInfo : public LoKi::ExtraInfo::CheckInfo<LHCb::Track>
    {
    public:
      // ======================================================================
      /** constructor from "info"
       *  @param key info index/mark/key
       */
      HasInfo ( const int key ) ;
      /// copy constructor 
      HasInfo ( const HasInfo& right ) ;
      /// destructor 
      virtual ~HasInfo(){} ;
      /// clone method (mandatory!)
      virtual HasInfo* clone() const { return new HasInfo(*this) ; }
      /// the specific printout 
      virtual std::ostream& fillStream( std::ostream& s ) const ;
      // ======================================================================
    private:
      // ======================================================================
      // the default constructor is disabled 
      HasInfo();
      // ======================================================================
    };
    // ========================================================================    
    /** @class Info
     *  Trivial function which evaluates LHCb::Track::info
     *  
     *  It relies on the method LoKi::ExtraInfo::info 
     *
     *  @see LHCb::Track
     *  @see LoKi::Cuts::TrINFO 
     *  @see LoKi::ExtraInfo::GeInfo
     *  @see LoKi::ExtraInfo::info
     *
     *  @author Vanya BELYAEV ibelyaev@physics.syr.edu
     *  @date 2006-02-15
     */
    class Info : public LoKi::ExtraInfo::GetInfo<LHCb::Track>
    {
    public:
      // ======================================================================
      /** constructor from "info"
       *  @param key info index/mark/key
       *  @param def default value for missing key/invalid object 
       */
      Info ( const int    key , const double def ) ;
      /// copy constructor 
      Info ( const Info& right ) ;
      /// destructor 
      virtual ~Info(){};
      /// clone method (mandatory!)
      virtual Info* clone() const { return new Info(*this); }
      /// the specific printout 
      virtual std::ostream& fillStream( std::ostream& s ) const ;
      // ======================================================================
    private:
      // ======================================================================
      // the default constructor is disabled 
      Info();
      // ======================================================================
    };
    // ========================================================================    
    /** @class SmartInfo
     *  Trivial function which:
     *    - checks the presence of informnation in LHCb::Track::extraInfo
     *    - if the information present, it returns it 
     *    - for missing infomation, use function to evaluate it 
     *    - (optionally) fill th emissing field
     *  
     *  @see LHCb::Track
     *  @see LoKi::Cuts::TrSINFO 
     *  @see LoKi::ExtraInfo::GetSmartInfo
     *  @see LoKi::ExtraInfo::info
     *  @see LoKi::ExtraInfo::hasInfo
     *  @see LoKi::ExtraInfo::setInfo
     *
     *  @author Vanya BELYAEV ibelyaev@physics.syr.edu
     *  @date 2006-02-15
     */
    class SmartInfo : public LoKi::ExtraInfo::GetSmartInfo<LHCb::Track>
    {
    public:
      // ======================================================================
      /** constructor from fuction, key and update-flag
       *  @param index the key in LHCb::Track::extraInfo table 
       *  @param fun functionto be evaluated for missing keys 
       *  @param update the flag to allow the insert of mnissing information
       */
      SmartInfo
      ( const int                                         index          , 
        const LoKi::BasicFunctors<LHCb::Track>::Function& fun            , 
        const bool                                        update = false ) ;
      /// destructor 
      virtual ~SmartInfo(){};
      /// clone method (mandatory!)
      virtual  SmartInfo* clone() const { return new SmartInfo(*this); }
      /// the specific printout 
      virtual std::ostream& fillStream( std::ostream& s ) const ;
      // ======================================================================
    private:
      // ======================================================================
      // the default constructor is disabled 
      SmartInfo();
      // ======================================================================
    };
    // ========================================================================
    /** @class Chi2
     *  simple evaluator of the LHcb::Track::chi2
     *  @see LoKi::Cuts::TrCHI2
     *  @author Vanya BELYAEV ibelyaev@physics.syr.edu
     *  @date   2007-06-08
     */  
    class Chi2 : public LoKi::BasicFunctors<LHCb::Track>::Function
    {
    public:
      // ======================================================================
      /// MANDATORY: virtual destrcutor  
      virtual ~Chi2 () {}
      /// MANDATORY: clone method ("virtual constructor")  
      virtual  Chi2* clone() const { return new Chi2(*this) ; }
      /// mandatory: the only one essential method 
      virtual result_type operator() ( argument t ) const { return t.chi2() ; }
      /// OPTIONAL: the nice printout 
      virtual std::ostream& fillStream( std::ostream& s ) const 
      { return s << "TrCHI2" ; }
      // ======================================================================
    } ;
    // ========================================================================
    /** @class ProbChi2
     *  simple evaluator of the LHCb::Track::probChi2
     *  @see LoKi::Cuts::TrPROBCHI2
     *  @author Vanya BELYAEV ibelyaev@physics.syr.edu
     *  @date   2007-06-08
     */  
    class ProbChi2 : public LoKi::BasicFunctors<LHCb::Track>::Function
    {
    public:
      // ======================================================================
      /// MANDATORY: virtual destrcutor  
      virtual ~ProbChi2 () {}
      /// MANDATORY: clone method ("virtual constructor")  
      virtual  ProbChi2* clone() const { return new ProbChi2(*this) ; }
      /// mandatory: the only one essential method 
      virtual result_type operator() ( argument t ) const { return t.probChi2() ; }
      /// OPTIONAL: the nice printout 
      virtual std::ostream& fillStream( std::ostream& s ) const 
      { return s << "TrPROBCHI2" ; }
      // ======================================================================
    } ;
    // ========================================================================
    /** @class HasStateAt 
     *  Simple predicate which evaluates LHCb::Track::HasStateAt
     *  @see LoKi::Cuts::TrHASSTATE
     *  @author Vanya BELYAEV ibelyaev@physics.syr.edu
     *  @date   2007-06-08
     */  
    class HasStateAt : public LoKi::BasicFunctors<LHCb::Track>::Predicate
    {
    public:
      // ======================================================================
      HasStateAt ( const LHCb::State::Location& loc )
        : LoKi::BasicFunctors<LHCb::Track>::Predicate () 
        , m_loc ( loc ) {}
      /// MANDATORY: virtual destructor 
      virtual ~HasStateAt() {}  ;
      /// MANDATORY: clone method ("virtual constructor")
      virtual  HasStateAt* clone() const { return new HasStateAt(*this) ; }
      /// mandatory: the only one essential method 
      virtual result_type operator() ( argument t ) const 
      { return t.hasStateAt( m_loc ) ; }
      /// OPTIONAL: the nice printout 
      virtual std::ostream& fillStream( std::ostream& s ) const 
      { return s << "TrHASSTATE(" << loc () << ")" ; }
      // ======================================================================
    public:
      // ======================================================================
      /// get the location 
      int loc () const { return m_loc ; }
      // ======================================================================
    private:
      // ======================================================================
      // defautl constructor is disabled 
      HasStateAt() ; ///< no default constructor 
      // ======================================================================
    private:
      // ======================================================================
      /// the location 
      LHCb::State::Location m_loc ;                             // the location 
      // ======================================================================
    } ;  
    // ========================================================================
    /** @class IsOnTrack
     *  Somple predicate whcih evaluates LHCb::Track::isOnTrack
     *  @see LoKi::Cuts::TrISONTRACK
     *  @author Vanya BELYAEV ibelyaev@physics.syr.edu
     *  @date   2007-06-08
     */  
    class IsOnTrack : public LoKi::BasicFunctors<LHCb::Track>::Predicate
    {
    public:
      // ======================================================================
      IsOnTrack ( const LHCb::LHCbID& id ) 
        : LoKi::BasicFunctors<LHCb::Track>::Predicate () 
        , m_id ( id ) {}
      /// MANDATORY: virtual destructor 
      virtual ~IsOnTrack () {} ;
      /// MANDATORY: clone method ("virtual constructor")
      virtual  IsOnTrack* clone() const { return new IsOnTrack(*this) ; }
      /// mandatory: the only one essential method 
      virtual result_type operator() ( argument t ) const 
      { return t.isOnTrack ( m_id ) ; }
      /// OPTIONAL: the nice printout 
      virtual std::ostream& fillStream( std::ostream& s ) const 
      { return s << "TrIsOnTRACK(LHCb.LHCbID(" << m_id.lhcbID() << "))" ; }
      // ======================================================================
    private:
      // ======================================================================
      // the default constructor is disabled 
      IsOnTrack() ; ///< no default consructor 
      // ======================================================================
    private:
      // ======================================================================
      LHCb::LHCbID m_id ;
      // ======================================================================
    } ;
    // ========================================================================
    /** @class Type
     *  simple evaluator of the LHCb::Track::type
     *  @see LHCb::Track::type 
     *  @see LHCb::Track::Types
     *  @see LoKi::Cuts::TrTYPE
     *  @author Vanya BELYAEV ibelyaev@physics.syr.edu
     *  @date   2007-06-08
     */  
    class Type : public LoKi::BasicFunctors<LHCb::Track>::Function
    {
    public:
      // ======================================================================
      /// MANDATORY: virtual destrcutor  
      virtual ~Type() {} 
      /// MANDATORY: clone method ("virtual constructor")  
      virtual  Type* clone() const { return new Type(*this) ; }
      /// mandatory: the only one essential method 
      virtual result_type operator() ( argument t ) const { return t.type() ; }
      /// OPTIONAL: the nice printout 
      virtual std::ostream& fillStream( std::ostream& s ) const 
      { return s << "TrTYPE" ; }
      // ======================================================================
    } ;
    // ========================================================================
    /** @class StateZ 
     *  check Z-positon for the given state 
     *  @see LoKi::Cuts::TrSTATEZ 
     *  @see LoKi::Cuts::TrFIRSTHITZ
     *  @see LHCb::State::Location
     *  @author Vanya Belyaev@nikhef.nl
     *  @date 2010-06-02
     */
    class StateZ : public LoKi::BasicFunctors<LHCb::Track>::Function
    {
    public:
      // ======================================================================
      /// constructor with the state indicator 
      StateZ ( const LHCb::State::Location location = LHCb::State::FirstMeasurement ) ;
      /// constructor with the state indicator & bad value  
      StateZ ( const LHCb::State::Location location , 
               const double       bad      ) ;
      /// MANDATORY: virtual destructor 
      virtual ~StateZ() ;
      /// MANDATORY: clone method ("virtual constructor")
      virtual  StateZ* clone () const ;
      /// MANDATORY: the only one essential method 
      virtual result_type operator() ( argument t ) const ;
      /// OPTIONAL: nice printout 
      virtual std::ostream& fillStream( std::ostream& s ) const ;
      // ======================================================================
    public:
      // ======================================================================
      /// get the string representation of the state 
      const std::string& state() const ; // the string representation 
      // ======================================================================
    private:
      // ======================================================================
      /// the state itself 
      LHCb::State::Location m_state ;                       // the state itself 
      /// the bad value 
      double       m_bad   ;                                //    the bad value
      /// the state 
      mutable std::string m__state ;                        //        the state 
      // ======================================================================
    } ;
    // ========================================================================
  } //                                            end of namespace LoKi::Tracks
  // ==========================================================================
} //                                                      end of namespace LoKi 
// ============================================================================
// The END 
// ============================================================================
#endif // LOKI_TRACKS_H
// ============================================================================
