// $Id: ProtoParticles.h,v 1.1.1.1 2010-05-27 19:17:38 ibelyaev Exp $
// ============================================================================
#ifndef LOKI_PROTOPARTICLES_H 
#define LOKI_PROTOPARTICLES_H 1
// ============================================================================
// Include files
// ============================================================================
// GaudiKernel
// ============================================================================
#include "GaudiKernel/IDataProviderSvc.h"
// ============================================================================
// DaVinciInterafces 
// ============================================================================
#include "Kernel/IProtoParticleFilter.h"
// ============================================================================
// LoKi
// ============================================================================
#include "LoKi/apply.h"
#include "LoKi/Filters.h"
#include "LoKi/Streamers.h"
#include "LoKi/PPTypes.h"
#include "LoKi/Constants.h"
// ============================================================================
namespace LoKi 
{
  // ===========================================================================
  /** @namespace LoKi::ProtoParticles LoKi/ProtoParticles.h
   *  
   *  Helper namespaxe to keep all protoparticle functors
   *  @author Vanya Belyaev Ivan.Belyaev@nikhef.nl
   *  @date   2010-05-26
   */
  namespace ProtoParticles 
  {
    // ========================================================================
    /** @class Filter 
     *  Simple wrapper to pick up certain ProtoParticle Filter 
     *  @see IProtoParticleFilter 
     *  @see LoKi::Cuts::PP_FILTER 
     *  @author Vanya Belyaev Ivan.Belyaev@nikhef.nl
     *  @date   2010-05-26
     */
    class Filter : public LoKi::PPTypes::PPCuts 
    {
    public:
      // ======================================================================
      /// constructor from the filter 
      Filter ( const IProtoParticleFilter* fltr ) ;
      /// cosntructro from tool name 
      Filter ( const std::string&          fltr ) ;
      /// MANDATORY: virtual destructor 
      virtual ~Filter() ;
      /// MANDATORY: clone method ("virtual consructor")
      virtual  Filter* clone() const ;
      /// MANDATORY: the only one essential methdo 
      virtual  result_type operator() ( argument p ) const ;
      /// OPTIONAL: the nice printtout 
      virtual  std::ostream& fillStream ( std::ostream& s ) const ;
      // ======================================================================
    private: 
      // ======================================================================
      /// the default constructor is disabled 
      Filter() ; //                         the default constructor is disabled
      // ======================================================================
    private: 
      // ======================================================================
      /// the filter 
      LoKi::Interface<IProtoParticleFilter>  m_filter ; // the filter 
      // ======================================================================
    };
    // ========================================================================
    /** @class HasInfo
     *  Simple check for "Extra-Info"
     *  @see LHCb::ProtoParticle::hasInfo
     *  @see LoKi::Cuts::PP_HASINFO
     *  @see LoKi::Cuts::PPHASINFO
     *  @author Vanya Belyaev Ivan.BElyaev@nikhef.nl
     *  @date   2010-05-26
     */
    class HasInfo : public LoKi::PPTypes::PPCuts 
    {
    public:
      // ======================================================================
      /// contructor from the index 
      HasInfo ( const int index ) ;
      /// MANDATORY: virtual destructor
      virtual ~HasInfo() ;
      /// MANDATORY: clone method ("virtual consructor")
      virtual  HasInfo* clone() const ;
      /// MANDATORY: the only one essential methdo 
      virtual  result_type operator() ( argument p ) const ;
      /// OPTIONAL: the nice printtout 
      virtual  std::ostream& fillStream ( std::ostream& s ) const ;
      // ======================================================================
    private :
      // ======================================================================
      /// the default constructor is disabled 
      HasInfo () ;                       // the default constructor is disabled 
      // ======================================================================
    private :
      // ======================================================================
      /// the index 
      int m_index ;                                                // the index 
      // ======================================================================
    } ;
    // ========================================================================
    /** @class Info
     *  Simple extractor for "Extra-Info"
     *  @see LHCb::ProtoParticle::extraInfo
     *  @see LoKi::Cuts::PP_INFO
     *  @see LoKi::Cuts::PPINFO
     *  @author Vanya Belyaev Ivan.BElyaev@nikhef.nl
     *  @date   2010-05-26
     */
    class Info : public LoKi::PPTypes::PPFunc 
    {
    public:
      // ======================================================================
      /** constructor from "info"
       *  @param key info index/mark/key
       *  @param def default valeu for info 
       *  @param bad bad value to be retured for invalid protoparticle 
       */
      Info 
      ( const int    key , 
        const double def , 
        const double bad ) ;
      /** constructor from "info"
       *  @param key info index/mark/key
       *  @param def default value for info 
       */
      Info 
      ( const int    key , 
        const double def ) ;
      /// MANDATORY: virtual destructor
      virtual ~Info() ;
      /// MANDATORY: clone method ("virtual consructor")
      virtual  Info* clone() const ;
      /// MANDATORY: the only one essential methdo 
      virtual  result_type operator() ( argument p ) const ;
      /// OPTIONAL: the nice printtout 
      virtual  std::ostream& fillStream ( std::ostream& s ) const ;
      // ======================================================================
    private :
      // ======================================================================
      /// the default constructor is disabled 
      Info () ;                       // the default constructor is disabled 
      // ======================================================================
    private :
      // ======================================================================
      /// the index 
      int    m_key ;                                               // the index 
      /// default value for missing information 
      double m_def ;                   // default value for missing information 
      /// value to be returned for invalid protoparticle 
      double m_bad ;          // value to be returned for invalid protoparticle 
      // ======================================================================
    } ;
    // ========================================================================
    /** @class HasRichPID 
     *  Simple check for validity of RichPID link
     *  @see LHCb::ProtoParticle::richPID 
     *  @see LoKi::Cuts::PP_HASRICHPID 
     *  @see LoKi::Cuts::PPHASRICH 
     *  @author Vanya Belyaev Ivan.BElyaev@nikhef.nl
     *  @date   2010-05-26
     */
    class HasRichPID : public LoKi::PPTypes::PPCuts 
    {
    public:
      // ======================================================================
      /// MANDATORY: virtual destructor
      virtual ~HasRichPID() ;
      /// MANDATORY: clone method ("virtual consructor")
      virtual  HasRichPID* clone() const ;
      /// MANDATORY: the only one essential methdo 
      virtual  result_type operator() ( argument p ) const ;
      /// OPTIONAL: the nice printtout 
      virtual  std::ostream& fillStream ( std::ostream& s ) const ;
      // ======================================================================
    } ;
    // ========================================================================
    /** @class HasMuonPID 
     *  Simple check for validity of MuonPID link
     *  @see LHCb::ProtoParticle::muonPID 
     *  @see LoKi::Cuts::PP_HASMUONPID 
     *  @see LoKi::Cuts::PPHASMUON 
     *  @author Vanya Belyaev Ivan.BElyaev@nikhef.nl
     *  @date   2010-05-26
     */
    class HasMuonPID : public LoKi::PPTypes::PPCuts 
    {
    public:
      // ======================================================================
      /// MANDATORY: virtual destructor
      virtual ~HasMuonPID() ;
      /// MANDATORY: clone method ("virtual consructor")
      virtual  HasMuonPID* clone() const ;
      /// MANDATORY: the only one essential methdo 
      virtual  result_type operator() ( argument p ) const ;
      /// OPTIONAL: the nice printtout 
      virtual  std::ostream& fillStream ( std::ostream& s ) const ;
      // ======================================================================
    } ;
    // ========================================================================
    /** @class HasTrack
     *  Simple check for validity of Track
     *  @see LHCb::ProtoParticle::track
     *  @see LoKi::Cuts::PP_HASTRACK 
     *  @see LoKi::Cuts::HASTRACK 
     *  @author Vanya Belyaev Ivan.BElyaev@nikhef.nl
     *  @date   2010-05-26
     */
    class HasTrack : public LoKi::PPTypes::PPCuts 
    {
    public:
      // ======================================================================
      /// MANDATORY: virtual destructor
      virtual ~HasTrack () ;
      /// MANDATORY: clone method ("virtual consructor")
      virtual  HasTrack* clone() const ;
      /// MANDATORY: the only one essential methdo 
      virtual  result_type operator() ( argument p ) const ;
      /// OPTIONAL: the nice printtout 
      virtual  std::ostream& fillStream ( std::ostream& s ) const ;
      // ======================================================================
    } ;
    // ========================================================================
    /** @class Charged
     *  Simple check for validity of Trac
     *  @see LHCb::ProtoParticle::track
     *  @see LoKi::Cuts::PP_CHARGED
     *  @see LoKi::Cuts::PP_HASTRACK 
     *  @author Vanya Belyaev Ivan.BElyaev@nikhef.nl
     *  @date   2010-05-26
     */
    class Charged : public HasTrack
    {
    public:
      // ======================================================================
      /// MANDATORY: virtual destructor
      virtual ~Charged () ;
      /// MANDATORY: clone method ("virtual consructor")
      virtual  Charged* clone() const ;
      /// OPTIONAL: the nice printtout 
      virtual  std::ostream& fillStream ( std::ostream& s ) const ;
      // ======================================================================
    } ;
    // ========================================================================
    /** @class Neutral
     *  @see LoKi::Cuts::PP_NEUTRAL
     *  @author Vanya Belyaev Ivan.BElyaev@nikhef.nl
     *  @date   2010-05-26
     */
    class Neutral : public Charged 
    {
    public:
      // ======================================================================
      /// MANDATORY: virtual destructor
      virtual ~Neutral () ;
      /// MANDATORY: clone method ("virtual consructor")
      virtual  Neutral* clone() const ;
      /// MANDATORY: the only one essential methdo 
      virtual  result_type operator() ( argument p ) const ;
      /// OPTIONAL: the nice printtout 
      virtual  std::ostream& fillStream ( std::ostream& s ) const ;
      // ======================================================================
    } ;
    // ========================================================================
    /** @class Calos
     *  get number of calo-hypos 
     *  @see LHCb::ProtoParticle::calos
     *  @see LoKi::Cuts::PP_NCALOS 
     *  @author Vanya Belyaev Ivan.BElyaev@nikhef.nl
     *  @date   2010-05-26
     */
    class Calos : public LoKi::PPTypes::PPFunc
    {
    public:
      // ======================================================================
      /// MANDATORY: virtual destructor
      virtual ~Calos () ;
      /// MANDATORY: clone method ("virtual consructor")
      virtual  Calos* clone() const ;
      /// MANDATORY: the only one essential methdo 
      virtual  result_type operator() ( argument p ) const ;
      /// OPTIONAL: the nice printtout 
      virtual  std::ostream& fillStream ( std::ostream& s ) const ;
      // ======================================================================
    } ;
    // ========================================================================
    /** @class HasAerogel
     *  Simple check for Rich-aerogel
     *  @see LHCb::ProtoParticle::richPID
     *  @see LHCb::RichPID::usedAerogel
     *  @see LoKi::Cuts::PP_HASAEROGEL 
     *  @author Vanya Belyaev Ivan.BElyaev@nikhef.nl
     *  @date   2010-05-26
     */
    class HasAerogel : public LoKi::PPTypes::PPCuts 
    {
    public:
      // ======================================================================
      /// MANDATORY: virtual destructor
      virtual ~HasAerogel () ;
      /// MANDATORY: clone method ("virtual consructor")
      virtual  HasAerogel* clone() const ;
      /// MANDATORY: the only one essential methdo 
      virtual  result_type operator() ( argument p ) const ;
      /// OPTIONAL: the nice printtout 
      virtual  std::ostream& fillStream ( std::ostream& s ) const ;
      // ======================================================================
    } ;
    // ========================================================================
    /** @class HasRich1Gas
     *  Simple check for Rich-aerogel
     *  @see LHCb::ProtoParticle::richPID
     *  @see LHCb::RichPID::usedRich1Gas
     *  @see LoKi::Cuts::PP_HASRICH1GAS
     *  @author Vanya Belyaev Ivan.BElyaev@nikhef.nl
     *  @date   2010-05-26
     */
    class HasRich1Gas : public LoKi::PPTypes::PPCuts 
    {
    public:
      // ======================================================================
      /// MANDATORY: virtual destructor
      virtual ~HasRich1Gas () ;
      /// MANDATORY: clone method ("virtual consructor")
      virtual  HasRich1Gas* clone() const ;
      /// MANDATORY: the only one essential methdo 
      virtual  result_type operator() ( argument p ) const ;
      /// OPTIONAL: the nice printtout 
      virtual  std::ostream& fillStream ( std::ostream& s ) const ;
      // ======================================================================
    } ;
    // ========================================================================
    /** @class HasRich2Gas
     *  Simple check for Rich-aerogel
     *  @see LHCb::ProtoParticle::richPID
     *  @see LHCb::RichPID::usedRich2Gas
     *  @see LoKi::Cuts::PP_HASRICH2GAS
     *  @author Vanya Belyaev Ivan.BElyaev@nikhef.nl
     *  @date   2010-05-26
     */
    class HasRich2Gas : public LoKi::PPTypes::PPCuts 
    {
    public:
      // ======================================================================
      /// MANDATORY: virtual destructor
      virtual ~HasRich2Gas () ;
      /// MANDATORY: clone method ("virtual consructor")
      virtual  HasRich2Gas* clone() const ;
      /// MANDATORY: the only one essential methdo 
      virtual  result_type operator() ( argument p ) const ;
      /// OPTIONAL: the nice printtout 
      virtual  std::ostream& fillStream ( std::ostream& s ) const ;
      // ======================================================================
    } ;
    // ========================================================================
    /** @class HasDetector 
     *  check for "detector"
     *  @author Vanya Belyaev Ivan.BElyaev@nikhef.nl
     *  @date   2010-05-26
     */
    class HasDetector : public LoKi::PPTypes::PPCuts 
    {
    public  :
      // ======================================================================
      enum Detector
        {
          // ==================================================================
          //  General DLL information flags
          // ==================================================================
          /// Requires RICH DLL information
          RICH            ,              // Requires RICH DLL information
          /// Requires CALO DLL information
          CALO            ,              // Requires CALO DLL information
          /// Requires MUON DLL information
          MUON            ,              // Requires MUON DLL information
          // ==================================================================
          // Rich detector flags
          // ==================================================================
          /// Must have RICH1 aerogel information
          RICH_AEROGEL    ,     // Must have RICH1 aerogel information
          /// Must have RICH1 gas information
          RICH_RICH1GAS   ,    // Must have RICH1 gas information
          /// Must have RICH2 gas information
          RICH_RICH2GAS   ,    // Must have RICH2 gas information
          // ==================================================================
          // calo detector flags
          // ==================================================================
          /// Must be in the CALO SPD  acceptance
          CALO_SPDACC     ,       // Must be in the CALO SPD acceptance
          /// Must be in the CALO PRS  acceptance
          CALO_PRSACC     ,       // Must be in the CALO PRS acceptance
          /// Must be in the CALO ECAL acceptance
          CALO_ECALACC    ,       // Must be in the CALO ECAL acceptance
          /// Must be in the CALO HCAL acceptance
          CALO_HCALACC    ,       // Must be in the CALO HCAL acceptance
          /// Must be in the CALO BREM acceptance
          CALO_BREMACC    ,       // Must be in the CALO BREM acceptance
          /// Must be in the CALO acceptance 
          CALO_CALOACC    ,       // Must be in the CALO acceptance
          // ==================================================================
          // has information 
          // ==================================================================
          CALO_SPDINFO     ,       // has SPD  information ? 
          CALO_PRSINFO     ,       // has Prs  information ?
          CALO_ECALINFO    ,       // has Ecal information ?
          CALO_HCALINFO    ,       // has Hcal information ?
          CALO_BREMINFO    ,       // has Brem information ? 
          CALO_CALOINFO    ,       // has Calo information ? 
        };
      // ======================================================================
    public  :
      // ======================================================================
      /// constructor from the detector 
      HasDetector ( const Detector     det ) ;
      /// constructor from the detector 
      HasDetector ( const std::string& det ) ;
      /// MANDATORY: virtual destructor 
      virtual ~HasDetector() ;
      /// MANDATORY: clone method ("virtual constructor")
      virtual  HasDetector* clone() const ;
      /// MANDATORY: the only one essential methdo 
      virtual  result_type operator() ( argument p ) const ;
      /// OPTIONAL: the nice printtout 
      virtual  std::ostream& fillStream ( std::ostream& s ) const ;
      // ======================================================================
    private :
      // ======================================================================
      /// the default constructor is disabled 
      HasDetector () ;
      // ======================================================================
    public:
      // ======================================================================
      bool hasRichDLL       ( const LHCb::ProtoParticle* p ) const ;
      bool hasMuonInfo      ( const LHCb::ProtoParticle* p ) const ;
      bool hasCaloDLL       ( const LHCb::ProtoParticle* p ) const ;
      //
      bool hasCaloInfo      ( const LHCb::ProtoParticle* p ) const ;
      //
      bool hasSpdInfo       ( const LHCb::ProtoParticle* p ) const ;
      bool hasPrsInfo       ( const LHCb::ProtoParticle* p ) const ;
      bool hasEcalInfo      ( const LHCb::ProtoParticle* p ) const ;
      bool hasHcalInfo      ( const LHCb::ProtoParticle* p ) const ;
      bool hasBremInfo      ( const LHCb::ProtoParticle* p ) const ;
      // ======================================================================
      bool hasRichAerogel   ( const LHCb::ProtoParticle* p ) const ;
      bool hasRich1Gas      ( const LHCb::ProtoParticle* p ) const ;
      bool hasRich2Gas      ( const LHCb::ProtoParticle* p ) const ;
      // ======================================================================
      bool inSpdAcceptance  ( const LHCb::ProtoParticle* p ) const ;
      bool inPrsAcceptance  ( const LHCb::ProtoParticle* p ) const ;
      bool inEcalAcceptance ( const LHCb::ProtoParticle* p ) const ;
      bool inHcalAcceptance ( const LHCb::ProtoParticle* p ) const ;
      bool inBremAcceptance ( const LHCb::ProtoParticle* p ) const ;
      bool inCaloAcceptance ( const LHCb::ProtoParticle* p ) const ;
      // ======================================================================
    protected:
      // ======================================================================
      /// get the detector 
      Detector det() const { return m_det ; }               // get the detector 
      // ======================================================================
    private :
      // ======================================================================
      /// the detector 
      Detector m_det ;                                          // the detector 
      // ======================================================================
    };
    // =========================================================================
    /** @class OnlyDetector 
     *  check for "detector"
     *  @author Vanya Belyaev Ivan.BElyaev@nikhef.nl
     *  @date   2010-05-26
     */
    class OnlyDetector : public HasDetector
    { 
    public  :
      // ======================================================================
      /// constructor from the detector 
      OnlyDetector ( const Detector     det ) ;
      /// constructor from the detector 
      OnlyDetector ( const std::string& det ) ;
      /// MANDATORY: virtual destructor 
      virtual ~OnlyDetector() ;
      /// MANDATORY: clone method ("virtual constructor")
      virtual  OnlyDetector* clone() const ;
      /// MANDATORY: the only one essential methdo 
      virtual  result_type operator() ( argument p ) const ;
      /// OPTIONAL: the nice printtout 
      virtual  std::ostream& fillStream ( std::ostream& s ) const ;
      // ======================================================================
    private :
      // ======================================================================
      /// the default constructor is disabled 
      OnlyDetector () ;
      // ======================================================================
    };
    // ========================================================================
    /** @clas IsMuon 
     *  @see LoKi::Cuts::PP_ISMUON 
     *  @author Vanya Belyaev Ivan.BElyaev@nikhef.nl
     *  @date   2010-05-26
     */
    class IsMuon : public LoKi::PPTypes::PPCuts 
    {
    public:
      // ======================================================================
      /// MANDATORY: virtual destructor 
      virtual ~IsMuon () ;
      /// MANDATORY: clone method ("virtual constructor")
      virtual  IsMuon* clone() const ;
      /// MANDATORY: the only one essential method
      virtual  result_type operator() ( argument p ) const ;
      /// OPTIONAL: the nice printtout 
      virtual  std::ostream& fillStream ( std::ostream& s ) const ;
      // ======================================================================
    } ;
    // ========================================================================
    /** @clas IsLooseMuon 
     *  @see LoKi::Cuts::PP_ISLOOSEMUON 
     *  @author Vanya Belyaev Ivan.BElyaev@nikhef.nl
     *  @date   2010-05-26
     */
    class IsLooseMuon : public IsMuon
    {
    public:
      // ======================================================================
      /// MANDATORY: virtual destructor 
      virtual ~IsLooseMuon () ;
      /// MANDATORY: clone method ("virtual constructor")
      virtual  IsLooseMuon* clone() const ;
      /// MANDATORY: the only one essential method
      virtual  result_type operator() ( argument p ) const ;
      /// OPTIONAL: the nice printtout 
      virtual  std::ostream& fillStream ( std::ostream& s ) const ;
      // ======================================================================
    } ;
    // ========================================================================
    /** @class SourceTES
     *  "source of protoparticles"
     *  @see LoKi::Cuts::PPSOURCE
     *  @see LHCb::ProtoParticle
     *  @author Vanya Belyaev Ivan.BElyaev@nikhef.nl
     *  @date   2010-05-26
     */
    class SourceTES : public LoKi::PPTypes::PPSources
    {
    public:
      // ======================================================================
      /// constructor from the service, TES location and cuts 
      SourceTES 
      ( const std::string&           path                                 , 
        IDataProviderSvc*            svc  = 0                             , 
        const LoKi::PPTypes::PPCuts& cuts = 
        LoKi::BasicFunctors<const LHCb::ProtoParticle*>::BooleanConstant(true) ) ; 
      /// constructor from the service, TES location and cuts 
      SourceTES 
      ( const std::vector<std::string>& path                                , 
        IDataProviderSvc*               svc  = 0                            , 
        const LoKi::PPTypes::PPCuts&    cuts = 
        LoKi::BasicFunctors<const LHCb::ProtoParticle*>::BooleanConstant(true) ) ; 
      /// constructor from the service, TES location and cuts 
      SourceTES
      ( const std::string&           path     , 
        const LoKi::PPTypes::PPCuts& cuts     ,  
        IDataProviderSvc*            svc  = 0 ) ;
      /// constructor from the service, TES location and cuts 
      SourceTES
      ( const std::vector<std::string>& path     , 
        const LoKi::PPTypes::PPCuts&    cuts     ,  
        IDataProviderSvc*               svc  = 0 ) ;
      /// constructor from the service, TES location and cuts 
      SourceTES 
      ( const LoKi::PPTypes::PPCuts& cuts        ,  
        const std::string&           path        ,
        IDataProviderSvc*            svc  = 0    ) ;
      //a/ constructor from the service, TES location and cuts 
      SourceTES 
      ( const LoKi::PPTypes::PPCuts&    cuts        ,  
        const std::vector<std::string>& path        ,
        IDataProviderSvc*               svc  = 0    ) ;
      /// constructor from the service, TES location and cuts 
      SourceTES 
      ( const LoKi::PPTypes::PPCuts& cuts  ,  
        IDataProviderSvc*            svc   ,  
        const std::string&           path  ) ;
      /// constructor from the service, TES location and cuts 
      SourceTES 
      ( const LoKi::PPTypes::PPCuts&    cuts  ,  
        IDataProviderSvc*               svc   ,  
        const std::vector<std::string>& path  ) ;
      /// MANDATORY: virtual destructor 
      virtual ~SourceTES() ;
      /// MANDATORY: clone method ("virtual constructor")
      virtual  SourceTES* clone() const ;
      /// MANDATORY: the only essential method:
      virtual result_type operator() ( /* argument */ )  const ;
      /// OPTIONAL: the nice printout
      virtual std::ostream& fillStream ( std::ostream& o ) const ;
      // ======================================================================
    private:
      // ======================================================================
      /// the default constructor is disabled 
      SourceTES() ; // the default constructor is disabled 
      // ======================================================================
    public:
      // ======================================================================
      /// get the path 
      const std::vector<std::string>& paths() const { return m_path ; }
      /// get the service
      const LoKi::Interface<IDataProviderSvc>& dataSvc() const 
      { return m_dataSvc ; }      
      // ======================================================================
    public:
      // ======================================================================
      /// set the  paths
      void setPaths  ( const std::vector<std::string>& value ) { m_path = value ; }
      /// set the  paths
      void addToPath ( const std::string& value ) { m_path.push_back( value ) ; }
      /// set the  service 
      void setDataSvc ( const                 IDataProviderSvc*  value ) 
      { m_dataSvc = value ; }
      /// set the  service 
      void setDataSvc ( const LoKi::Interface<IDataProviderSvc>& value ) 
      { m_dataSvc = value ; }
      // ======================================================================
    protected:
      // ======================================================================
      /// get the protoparticles from the certain  TES location 
      std::size_t get
      ( const std::string&                location , 
        LHCb::ProtoParticle::ConstVector& output   ) const ;
      // ======================================================================
    private:
      // ======================================================================
      /// TES locations of Particles 
      std::vector<std::string>  m_path ; // TES locations of ProtoParticles 
      /// data provider service 
      mutable LoKi::Interface<IDataProviderSvc>  m_dataSvc ;
      /// 'on-flight' filter
      LoKi::PPTypes::PPCut m_cut ; // 'on-flight' filter
      // ======================================================================
    } ;
    // ======================================================================
    /** @class TrackCut 
     *  adapter for "track-cut"
     *  @see LoKi::Cuts::PP_TRCUT 
     *  
     *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
     *  @date 2010-05-26
     */
    class TrackCut : public LoKi::PPTypes::PPCuts 
    {
    public:
      // ======================================================================
      /// contructor from predicate
      TrackCut ( const LoKi::BasicFunctors<LHCb::Track>::Predicate& cut ) ;
      /// MANDATORY: virtual destructor 
      virtual ~TrackCut () ;
      /// MANDATORY: clone method ("virtual constructor")
      virtual  TrackCut* clone () const ;
      /// MANDATORY: the only one essential method
      virtual  result_type operator() ( argument p ) const ;
      /// OPTIONAL: the nice printout 
      virtual  std::ostream& fillStream ( std::ostream& s ) const ;
      // ======================================================================
    private:
      // ======================================================================
      /// default constructor is disabled 
      TrackCut () ;                          // default constructor is disabled
      // ======================================================================
    private:
      // ======================================================================      
      /// the predicate 
      LoKi::Assignable<LoKi::BasicFunctors<LHCb::Track>::Predicate>::Type m_cut ;
      // ======================================================================
    } ;
    // ======================================================================
    /** @class TrackFun
     *  adapter for "track-fun"
     *  @see LoKi::Cuts::PP_TRFUN 
     *  
     *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
     *  @date 2010-05-26
     */
    class TrackFun : public LoKi::PPTypes::PPFunc
    {
    public:
      // ======================================================================
      /// contructor from functor and 'bad'-value 
      TrackFun 
      ( const LoKi::BasicFunctors<LHCb::Track>::Function& cut , 
        const double                                      bad ) ;
      /// contructor from functor 
      TrackFun 
      ( const LoKi::BasicFunctors<LHCb::Track>::Function& cut ) ;
      /// MANDATORY: virtual destructor 
      virtual ~TrackFun () ;
      /// MANDATORY: clone method ("virtual constructor")
      virtual  TrackFun* clone () const ;
      /// MANDATORY: the only one essential method
      virtual  result_type operator() ( argument p ) const ;
      /// OPTIONAL: the nice printout 
      virtual  std::ostream& fillStream ( std::ostream& s ) const ;
      // ======================================================================
    private:
      // ======================================================================
      /// default constructor is disabled 
      TrackFun () ;                          // default constructor is disabled
      // ======================================================================
    private:
      // ======================================================================      
      /// the predicate 
      LoKi::Assignable<LoKi::BasicFunctors<LHCb::Track>::Function>::Type m_fun ;
      /// "bad"-value 
      double m_bad ; // "bad"-value 
      // ======================================================================
    } ;
    // ======================================================================
  } //                                    end of namespace LoKi::ProtoParticles
  // ==========================================================================
  namespace Functors 
  {
    // ========================================================================
    // the specific printout
    // ========================================================================
    template <>
    std::ostream& 
    Empty<const LHCb::ProtoParticle*>::fillStream ( std::ostream& s ) const ;
    // ========================================================================
    // the specific printpout
    // ========================================================================
    template <>
    std::ostream& 
    Size<const LHCb::ProtoParticle*>::fillStream ( std::ostream& s ) const ; 
    // ========================================================================
  } //                                          end of namespace LoKi::Functors
  // ==========================================================================
  // the specific printpout
  // ==========================================================================
  template <>
  std::ostream& 
  Valid<const LHCb::ProtoParticle*>::fillStream ( std::ostream& s ) const ;
  // ==========================================================================
  namespace Cuts 
  {
    // ========================================================================
    /** @typedef PP_FILTER 
     *  simple functor to allow usage of IParticleFilter
     *
     *  @code
     * 
     *     const IParticleFilter* fltr = ... ;
     * 
     *     const PP_FILTER filter = PP_FILTER ( fltr ) ;
     *  
     *     const LHCb::ProtoParticle* pp = ... ;
     *  
     *     const bool OK = filter ( pp ) ;
     *
     *  @endcode 
     *
     *  @code
     * 
     *     const std::string& fltr = ... ;
     * 
     *     const PP_FILTER filter = PP_FILTER ( fltr ) ;
     *  
     *     const LHCb::ProtoParticle* pp = ... ;
     *  
     *     const bool OK = filter ( pp ) ;
     *
     *  @endcode 
     *
     *  @see IParticleFilter 
     *  @see LoKi::ProtoParticles::Filter 
     *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
     *  @date 2010-05-26
     */
    typedef LoKi::ProtoParticles::Filter                            PP_FILTER ;
    // ========================================================================
    /** @typedef PP_HASINFO 
     *  simple predicate to check the presence of 'extra-info'
     *
     *  @code
     * 
     *     const PP_HASINFO has = PP_HADINFO ( LHCb::ProtoParticle::RichDLLp ) ;
     *  
     *     const LHCb::ProtoParticle* pp = ... ;
     *  
     *     const bool OK =  has  ( pp ) ;
     *
     *  @endcode 
     *
     *  @see LHCb::ProtoParticle::hasInfo 
     *  @see LoKi::ProtoParticleS::HasInfo
     *  @see LoKi::Cuts::PPHASINFO
     *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
     *  @date 2010-05-26
     */
    typedef LoKi::ProtoParticles::HasInfo                          PP_HASINFO ;
    // ========================================================================
    /** @typedef PP_INFO 
     *  simple function to access  'extra-info'
     *
     *  @code
     * 
     *     const PP_INFO dllp = PP_HADINFO ( LHCb::ProtoParticle::RichDLLp , -1000) ;
     *  
     *     const LHCb::ProtoParticle* pp = ... ;
     *  
     *     const double result  =  dllp  ( pp ) ;
     *
     *  @endcode 
     *
     *  @see LHCb::ProtoParticle::info 
     *  @see LoKi::ProtoParticles::Info
     *  @see LoKi::Cuts::PPINFO
     *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
     *  @date 2010-05-26
     */
    typedef LoKi::ProtoParticles::Info                                PP_INFO ;
    // ========================================================================
    /** @var PP_HASRICHPID 
     *  simple check for presence of RichPID
     *
     *  @code
     *  
     *     const LHCb::ProtoParticle* pp = ... ;
     *  
     *     const bool ok = PP_HASRICHPID ( pp ) 
     *
     *  @endcode 
     *
     *  @see LHCb::ProtoParticle::richPID 
     *  @see LoKi::ProtoParticles::HasRichPID 
     *  @see LOKi::Cuts:PPHASRICH 
     *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
     *  @date 2010-05-26
     */
    const LoKi::ProtoParticles::HasRichPID                      PP_HASRICHPID ;
    // ========================================================================
    /** @var PP_HASMUONPID 
     *  simple check for presence of MuonPID
     *
     *  @code
     *  
     *     const LHCb::ProtoParticle* pp = ... ;
     *  
     *     const bool ok = PP_HASMUONPID ( pp ) 
     *
     *  @endcode 
     *
     *  @see LHCb::ProtoParticle::muonPID 
     *  @see LoKi::ProtoParticles::HasMuonPID 
     *  @see LOKi::Cuts:PPHASMUON 
     *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
     *  @date 2010-05-26
     */
    const LoKi::ProtoParticles::HasMuonPID                      PP_HASMUONPID ;
    // ========================================================================
    /** @var PP_HASTRACK
     *  simple check for presence of track
     *
     *  @code
     *  
     *     const LHCb::ProtoParticle* pp = ... ;
     *  
     *     const bool ok = PP_HASTRACK ( pp ) 
     *
     *  @endcode 
     *
     *  @see LHCb::ProtoParticle::track 
     *  @see LoKi::ProtoParticles::HasTrack 
     *  @see LOKi::Cuts:HASTRACK 
     *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
     *  @date 2010-05-26
     */
    const LoKi::ProtoParticles::HasTrack                          PP_HASTRACK ;
    // ========================================================================
    /** @var PP_NCALOS
     *  number of calohypos 
     *
     *  @code
     *  
     *     const LHCb::ProtoParticle* pp = ... ;
     *  
     *     const bool ok = 0 < PP_NCALOS ( pp ) 
     *
     *  @endcode 
     *
     *  @see LHCb::ProtoParticle::calos
     *  @see LoKi::ProtoParticles::Calos
     *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
     *  @date 2010-05-26
     */
    const LoKi::ProtoParticles::Calos                               PP_NCALOS ;
    // ========================================================================
    /** @var PP_HASAEROGEL
     *  simple check for aerogel information
     *
     *  @code
     *  
     *     const LHCb::ProtoParticle* pp = ... ;
     *  
     *     const bool ok = PP_HASAEROGEL ( pp ) 
     *
     *  @endcode 
     *
     *  @see LHCb::ProtoParticle::richPID 
     *  @see LHCb::RichPID::usedAerogel
     *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
     *  @date 2010-05-26
     */
    const LoKi::ProtoParticles::HasAerogel                      PP_HASAEROGEL ;
    // ========================================================================
    /** @var PP_HASRICH1GAS
     *  simple check for aerogel information
     *
     *  @code
     *  
     *     const LHCb::ProtoParticle* pp = ... ;
     *  
     *     const bool ok = PP_HASRICH1GAS ( pp ) 
     *
     *  @endcode 
     *
     *  @see LHCb::ProtoParticle::richPID 
     *  @see LHCb::RichPID::usedRich1Gas
     *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
     *  @date 2010-05-26
     */
    const LoKi::ProtoParticles::HasRich1Gas                    PP_HASRICH1GAS ;
    // ========================================================================
    /** @var PP_HASRICH2GAS
     *  simple check for aerogel information
     *
     *  @code
     *  
     *     const LHCb::ProtoParticle* pp = ... ;
     *  
     *     const bool ok = PP_HASRICH2GAS ( pp ) 
     *
     *  @endcode 
     *
     *  @see LHCb::ProtoParticle::richPID 
     *  @see LHCb::RichPID::usedRich2Gas
     *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
     *  @date 2010-05-26
     */
    const LoKi::ProtoParticles::HasRich2Gas                    PP_HASRICH2GAS ;
    // ========================================================================
    /** @var PP_HASCALOSPD 
     *  @see LHCb::ProtoParticle::hasInfo
     *  @see LHCb::ProtoParticle::InAccSPD 
     *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
     *  @date 2010-05-26
     */
    const LoKi::ProtoParticles::HasInfo PP_HASCALOSPD ( LHCb::ProtoParticle::InAccSpd ) ;
    // ========================================================================
    /** @var PP_HASCALOPRS
     *  @see LHCb::ProtoParticle::hasInfo
     *  @see LHCb::ProtoParticle::InAccPRS
     *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
     *  @date 2010-05-26
     */
    const LoKi::ProtoParticles::HasInfo PP_HASCALOPRS ( LHCb::ProtoParticle::InAccPrs ) ;
    // ========================================================================
    /** @var PP_HASCALOECAL
     *  @see LHCb::ProtoParticle::hasInfo
     *  @see LHCb::ProtoParticle::InAccECAL
     *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
     *  @date 2010-05-26
     */
    const LoKi::ProtoParticles::HasInfo PP_HASCALOECAL( LHCb::ProtoParticle::InAccEcal ) ;
    // ========================================================================
    /** @var PP_HASCALOHCAL
     *  @see LHCb::ProtoParticle::hasInfo
     *  @see LHCb::ProtoParticle::InAccHCAL
     *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
     *  @date 2010-05-26
     */
    const LoKi::ProtoParticles::HasInfo PP_HASCALOHCAL ( LHCb::ProtoParticle::InAccHcal ) ;
    // ========================================================================
    /** @var PP_HASCALOBREM
     *  @see LHCb::ProtoParticle::hasInfo
     *  @see LHCb::ProtoParticle::InAccBREM
     *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
     *  @date 2010-05-26
     */
    const LoKi::ProtoParticles::HasInfo PP_HASCALOBREM ( LHCb::ProtoParticle::InAccBrem ) ;
    // ========================================================================
    /** @var PP_HASMUONINFO
     *  @see LHCb::ProtoParticle::hasInfo
     *  @see LHCb::ProtoParticle::InMuonAcc
     *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
     *  @date 2010-05-26
     */
    const LoKi::ProtoParticles::HasInfo PP_HASMUONINFO ( LHCb::ProtoParticle::InAccMuon ) ;
    // ========================================================================
    /** @var PP_HASMUONDLL
     *  @see LHCb::ProtoParticle::hasInfo
     *  @see LHCb::ProtoParticle::MuonPIDStatus
     *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
     *  @date 2010-05-26
     */
    const LoKi::ProtoParticles::HasInfo PP_HASMUONDLL ( LHCb::ProtoParticle::MuonPIDStatus ) ;
    // ========================================================================
    /** @var PP_HASRICHDLL
     *  @see LHCb::ProtoParticle::hasInfo
     *  @see LHCb::ProtoParticle::RichPIDStatus
     *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
     *  @date 2010-05-26
     */
    const LoKi::ProtoParticles::HasInfo PP_HASRICHDLL ( LHCb::ProtoParticle::RichPIDStatus ) ;
    // ======================================================================== 
    /// @var PP_RichDLLe     
    const LoKi::ProtoParticles::Info PP_RichDLLe  ( LHCb::ProtoParticle::RichDLLe  , -1000 ) ;
    // ========================================================================
    /// @var PP_RichDLLmu     
    const LoKi::ProtoParticles::Info PP_RichDLLmu ( LHCb::ProtoParticle::RichDLLmu , -1000 ) ;
    // ========================================================================
    /// @var PP_RichDLLpi     
    const LoKi::ProtoParticles::Info PP_RichDLLpi ( LHCb::ProtoParticle::RichDLLpi , -1000 ) ;
    // ========================================================================
    /// @var PP_RichDLLk     
    const LoKi::ProtoParticles::Info PP_RichDLLk  ( LHCb::ProtoParticle::RichDLLk  , -1000 ) ;
    // ========================================================================
    /// @var PP_RichDLLp    
    const LoKi::ProtoParticles::Info PP_RichDLLp  ( LHCb::ProtoParticle::RichDLLp  , -1000 ) ;
    // ========================================================================
    /// @var PP_RichDLLbt     
    const LoKi::ProtoParticles::Info PP_RichDLLbt ( LHCb::ProtoParticle::RichDLLbt , -1000 ) ;
    // ========================================================================
    /// @var PP_MuonMuLL     
    const LoKi::ProtoParticles::Info PP_MuonMuLL  ( LHCb::ProtoParticle::MuonMuLL  , -1000 ) ;
    // ========================================================================
    /// @var PP_MuonBkgLL     
    const LoKi::ProtoParticles::Info PP_MuonBkgLL ( LHCb::ProtoParticle::MuonBkgLL , -1000 ) ;
    // ========================================================================
    /// @var PP_MuonNShared
    const LoKi::ProtoParticles::Info PP_MuonNShared ( LHCb::ProtoParticle::MuonNShared , -1 ) ;
    // ========================================================================
    /// @var PP_MuonPIDStatus
    const LoKi::ProtoParticles::Info PP_MuonPIDStatus ( LHCb::ProtoParticle::MuonPIDStatus , 0 , 0 ) ;
    // ========================================================================
    /// @var PP_InAccMuon
    const LoKi::ProtoParticles::Info PP_InAccMuon ( LHCb::ProtoParticle::InAccMuon , false , false ) ;
    // ========================================================================
    /// @var PP_InAccSpd
    const LoKi::ProtoParticles::Info PP_InAccSpd  ( LHCb::ProtoParticle::InAccSpd  , false , false ) ;
    // ========================================================================
    /// @var PP_InAccPrs
    const LoKi::ProtoParticles::Info PP_InAccPrs  ( LHCb::ProtoParticle::InAccPrs  , false , false ) ;
    // ========================================================================
    /// @var PP_InAccEcal
    const LoKi::ProtoParticles::Info PP_InAccEcal ( LHCb::ProtoParticle::InAccEcal , false , false ) ;
    // ========================================================================
    /// @var PP_InAccHcal
    const LoKi::ProtoParticles::Info PP_InAccHcal ( LHCb::ProtoParticle::InAccHcal , false , false ) ;
    // ========================================================================
    /// @var PP_InAccBrem
    const LoKi::ProtoParticles::Info PP_InAccBrem ( LHCb::ProtoParticle::InAccBrem , false , false ) ;
    // ========================================================================
    /// @var PP_CaloTrMatch
    const LoKi::ProtoParticles::Info PP_CaloTrMatch ( LHCb::ProtoParticle::CaloTrMatch , -1000 ) ;
    // ========================================================================
    /// @var PP_CaloElectronMatch
    const LoKi::ProtoParticles::Info PP_CaloElectronMatch ( LHCb::ProtoParticle::CaloElectronMatch , -1000 ) ;
    // ========================================================================
    /// @var PP_CaloBremMatch
    const LoKi::ProtoParticles::Info PP_CaloBremMatch ( LHCb::ProtoParticle::CaloBremMatch , -1000 ) ;
    // ========================================================================
    /// @var PP_CaloChargedSpd
    const LoKi::ProtoParticles::Info PP_CaloChargedSpd ( LHCb::ProtoParticle::CaloChargedSpd , -1 ) ;
    // ========================================================================
    /// @var PP_CaloChargedPrs
    const LoKi::ProtoParticles::Info PP_CaloChargedPrs  ( LHCb::ProtoParticle::CaloChargedPrs  , -1.e+10 ) ;
    // ========================================================================
    /// @var PP_CaloChargedEcal
    const LoKi::ProtoParticles::Info PP_CaloChargedEcal ( LHCb::ProtoParticle::CaloChargedEcal , -1.e+10 ) ;
    // ========================================================================
    /// @var PP_CaloDepositID 
    const LoKi::ProtoParticles::Info PP_CaloDepositID   ( LHCb::ProtoParticle::CaloDepositID   , -1.e+10 ) ;
    // ========================================================================
    /// @var PP_ShowerShape  
    const LoKi::ProtoParticles::Info PP_ShowerShape     ( LHCb::ProtoParticle::ShowerShape     , -1.e+10 ) ;
    // ========================================================================
    /// @var PP_ClusterMass
    const LoKi::ProtoParticles::Info PP_ClusterMass     ( LHCb::ProtoParticle::ClusterMass     , -1.e+10 ) ;
    // ========================================================================
    /// @var PP_CaloNeutralSpd
    const LoKi::ProtoParticles::Info PP_CaloNeutralSpd  ( LHCb::ProtoParticle::CaloNeutralSpd  , -1      ) ;
    // ========================================================================
    /// @var PP_CaloNeutralPrs
    const LoKi::ProtoParticles::Info PP_CaloNeutralPrs  ( LHCb::ProtoParticle::CaloNeutralPrs  , -1.e+10 ) ;
    // ========================================================================
    /// @var PP_CaloNeutralEcal
    const LoKi::ProtoParticles::Info PP_CaloNeutralEcal ( LHCb::ProtoParticle::CaloNeutralEcal , -1.e+10 ) ;
    // ========================================================================
    /// @var PP_CaloSpdE
    const LoKi::ProtoParticles::Info PP_CaloSpdE        ( LHCb::ProtoParticle::CaloSpdE , -1.e+10 ) ;
    // ========================================================================
    /// @var PP_CaloPrsE
    const LoKi::ProtoParticles::Info PP_CaloPrsE        ( LHCb::ProtoParticle::CaloPrsE , -1.e+10 ) ;
    // ========================================================================
    /// @var PP_CaloEcalE
    const LoKi::ProtoParticles::Info PP_CaloEcalE      ( LHCb::ProtoParticle::CaloEcalE , -1.e+10 ) ;
    // ========================================================================
    /// @var PP_CaloHcalE
    const LoKi::ProtoParticles::Info PP_CaloHcalE      ( LHCb::ProtoParticle::CaloHcalE , -1.e+10 ) ;
    // ========================================================================
    /// @var PP_CaloEcalChi2
    const LoKi::ProtoParticles::Info PP_CaloEcalChi2   ( LHCb::ProtoParticle::CaloEcalChi2 , -1.e+10 ) ;
    // ========================================================================
    /// @var PP_CaloBremChi2
    const LoKi::ProtoParticles::Info PP_CaloBremChi2   ( LHCb::ProtoParticle::CaloBremChi2 , -1.e+10 ) ;
    // ========================================================================
    /// @var PP_CaloClusChi2
    const LoKi::ProtoParticles::Info PP_CaloClusChi2   ( LHCb::ProtoParticle::CaloClusChi2 , -1.e+10 ) ;
    // ========================================================================
    /// @var PP_CaloTrajectoryL 
    const LoKi::ProtoParticles::Info PP_CaloTrajectoryL ( LHCb::ProtoParticle::CaloTrajectoryL , -1.e+10 ) ;
    // ========================================================================
    /// @var PP_EcalPIDe
    const LoKi::ProtoParticles::Info PP_EcalPIDe  ( LHCb::ProtoParticle::EcalPIDe  , -1.e+10 ) ;
    // ========================================================================
    /// @var PP_PrsPIDe
    const LoKi::ProtoParticles::Info PP_PrsPIDe   ( LHCb::ProtoParticle::PrsPIDe   , -1.e+10 ) ;
    // ========================================================================
    /// @var PP_HcalPIDe
    const LoKi::ProtoParticles::Info PP_HcalPIDe  ( LHCb::ProtoParticle::HcalPIDe  , -1.e+10 ) ;
    // ========================================================================
    /// @var PP_EcalPIDmu
    const LoKi::ProtoParticles::Info PP_EcalPIDmu ( LHCb::ProtoParticle::EcalPIDmu , -1.e+10 ) ;
    // ========================================================================
    /// @var PP_HcalPIDmu
    const LoKi::ProtoParticles::Info PP_HcalPIDmu ( LHCb::ProtoParticle::HcalPIDmu , -1.e+10 ) ;
    // ========================================================================
    /// @var PP_PhotonID 
    const LoKi::ProtoParticles::Info PP_PhotonID   ( LHCb::ProtoParticle::PhotonID , -1.e+10 ) ;
    // ========================================================================
    /// @var PP_VeloCharge 
    const LoKi::ProtoParticles::Info PP_VeloCharge ( LHCb::ProtoParticle::VeloCharge  , -1.e+10 ) ;
    // ========================================================================
    /// @var PP_TrackChi2PerDof
    const LoKi::ProtoParticles::Info PP_TrackChi2PerDof ( LHCb::ProtoParticle::TrackChi2PerDof , -1.e+10 ) ;
    // ========================================================================
    /// @var PP_TrackNumDof
    const LoKi::ProtoParticles::Info PP_TrackNumDof ( LHCb::ProtoParticle::TrackNumDof , -1 ) ;
    // ========================================================================
    /// @var PP_TrackType
    const LoKi::ProtoParticles::Info PP_TrackType   ( LHCb::ProtoParticle::TrackType     , 
                                                      LHCb::Track::TypeUnknown           , 
                                                      LHCb::Track::TypeUnknown           ) ;
    // ========================================================================
    /// @var PP_TrackHistory
    const LoKi::ProtoParticles::Info PP_TrackHistory ( LHCb::ProtoParticle::TrackHistory , 
                                                       LHCb::Track::HistoryUnknown       , 
                                                       LHCb::Track::HistoryUnknown       ) ;
    // ========================================================================
    /// @var PP_TrackP
    const LoKi::ProtoParticles::Info PP_TrackP  ( LHCb::ProtoParticle::TrackP  , -1.e+10 ) ;
    // ========================================================================
    /// @var PP_TrackPt
    const LoKi::ProtoParticles::Info PP_TrackPt ( LHCb::ProtoParticle::TrackPt , -1.e+10 ) ;
    // ========================================================================
    /// @var PP_CombDLLe 
    const LoKi::ProtoParticles::Info PP_CombDLLe  ( LHCb::ProtoParticle::CombDLLe  , -1000 ) ;
    // ========================================================================
    /// @var PP_CombDLLmu
    const LoKi::ProtoParticles::Info PP_CombDLLmu ( LHCb::ProtoParticle::CombDLLmu , -1000 ) ;
    // ========================================================================
    /// @var PP_CombDLLpi
    const LoKi::ProtoParticles::Info PP_CombDLLpi ( LHCb::ProtoParticle::CombDLLpi , -1000 ) ;
    // ========================================================================
    /// @var PP_CombDLLk
    const LoKi::ProtoParticles::Info PP_CombDLLk  ( LHCb::ProtoParticle::CombDLLk  , -1000 ) ;
    // ========================================================================
    /// @var PP_CombDLLp
    const LoKi::ProtoParticles::Info PP_CombDLLp  ( LHCb::ProtoParticle::CombDLLp  , -1000 ) ;
    // ========================================================================
    /// @var PP_ProbNNe
    const LoKi::ProtoParticles::Info PP_ProbNNe   ( LHCb::ProtoParticle::ProbNNe   , -1000 ) ;
    // ========================================================================
    /// @var PP_ProbNNmu
    const LoKi::ProtoParticles::Info PP_ProbNNmu  ( LHCb::ProtoParticle::ProbNNmu  , -1000 ) ;
    // ========================================================================
    /** @var PP_ISMUON
     *  check 'is-muon'
     *  @see LHCb::MuonPID::IsMuon 
     *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
     *  @date 2010-05-27
     */
    const LoKi::ProtoParticles::IsMuon                              PP_ISMUON ;
    // ========================================================================
    /** @var PP_ISLOOSEMUON
     *  check 'is-loose-muon'
     *  @see LHCb::MuonPID::IsMuonLoose 
     *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
     *  @date 2010-05-27
     */
    const LoKi::ProtoParticles::IsLooseMuon                    PP_ISLOOSEMUON ;
    // ========================================================================
    /** @var PP_ISMUONLOOSE
     *  check 'is-loose-muon'
     *  @see LHCb::MuonPID::IsMuonLoose 
     *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
     *  @date 2010-05-27
     */
    const LoKi::ProtoParticles::IsLooseMuon                    PP_ISMUONLOOSE ;
    // ========================================================================
    /** @typedef PP_SOURCE 
     *  Simple source of protoparticles 
     *  @see LoKi:ProtoParticles::SourceTES
     *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
     *  @date 2010-05-27
     */
    typedef LoKi::ProtoParticles::SourceTES                         PP_SOURCE ;
    // ========================================================================
    /** @typedef PP_TRCUT 
     *  adapter for track-predicate 
     *
     *  @code
     * 
     *     const PP_TRCUT cut = PP_TRCUT ( TrP > 10 * GeV ) ;
     *  
     *     const LHCb::ProtoParticle* pp  = .. ;
     * 
     *     const bool fast = cut ( pp ) ;
     *
     *  @endcode
     *
     *  @see LoKi:ProtoParticles::TrackCut 
     *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
     *  @date 2010-05-27
     */
    typedef LoKi::ProtoParticles::TrackCut                           PP_TRCUT ;
    // ========================================================================
    /** @typedef PP_TRFUN
     *  adapter for track-fucntion
     *
     *  @code
     * 
     *     const PP_TRFUN fun = PP_TRFUN ( TrP ) ;
     *  
     *     const LHCb::ProtoParticle* pp  = .. ;
     * 
     *     const bool fast = 10 * GeV < fun ( pp ) ;
     *
     *  @endcode
     *
     *  @see LoKi:ProtoParticles::TrackFun
     *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
     *  @date 2010-05-27
     */
    typedef LoKi::ProtoParticles::TrackFun                           PP_TRFUN ;
    // ========================================================================
  } //                                              end of namespace LoKi::Cuts 
  // ==========================================================================
} //                                                      end of namespace LoKi 
// ============================================================================
// The END
// ============================================================================
#endif // LOKI_PROTOPARTICLES_H
// ============================================================================
