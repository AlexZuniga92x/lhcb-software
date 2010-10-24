// $Id$
// ============================================================================
#ifndef LOKI_TRSOURCES_H 
#define LOKI_TRSOURCES_H 1
// ============================================================================
// Include files
// ============================================================================
// GaudiKernel
// ============================================================================
#include "GaudiKernel/IDataProviderSvc.h"
// ============================================================================
// TrackEvent/RecEvent
// ============================================================================
#include "Event/Track.h"
// ============================================================================
// LoKi
// ============================================================================
#include "LoKi/BasicFunctors.h"
#include "LoKi/Interface.h"
#include "LoKi/TrackTypes.h"
// ============================================================================
namespace LoKi
{
  // ==========================================================================
  namespace Tracks
  {
    // ========================================================================
    /** @class SourceTES 
     * 
     *  @see LoKi::Cuts::TrSOURCE
     *  simple "source for the tracks from TES "
     *  @author Vanya BELYAEV ibelyav@physics.syr.edu
     *  @date 2006-12-07
     */
    class SourceTES 
      : public LoKi::BasicFunctors<const LHCb::Track*>::Source 
    { 
      // ======================================================================
      typedef LoKi::BasicFunctors<const LHCb::Track*>::Source _Source;
      // ======================================================================
    public:
      // ======================================================================
      /// constructor from the service, TES location and cuts 
      SourceTES 
      ( const std::string&              path                    , 
        IDataProviderSvc*               svc  = 0                , 
        const LoKi::TrackTypes::TrCuts& cuts = 
        LoKi::BasicFunctors<const LHCb::Track*>::BooleanConstant(true) ) ; 
      /// constructor from the service, TES location and cuts 
      SourceTES 
      ( const std::vector<std::string>& path                    , 
        IDataProviderSvc*               svc  = 0                , 
        const LoKi::TrackTypes::TrCuts& cuts = 
        LoKi::BasicFunctors<const LHCb::Track*>::BooleanConstant(true) ) ; 
      /// constructor from the service, TES location and cuts 
      SourceTES
      ( const std::string&              path     , 
        const LoKi::TrackTypes::TrCuts& cuts     ,  
        IDataProviderSvc*               svc  = 0 ) ;
      /// constructor from the service, TES location and cuts 
      SourceTES
      ( const std::vector<std::string>& path     , 
        const LoKi::TrackTypes::TrCuts& cuts     ,  
        IDataProviderSvc*               svc  = 0 ) ;
      /// constructor from the service, TES location and cuts 
      SourceTES 
      ( const LoKi::TrackTypes::TrCuts& cuts        ,  
        const std::string&              path        ,
        IDataProviderSvc*               svc  = 0    ) ;
      /// constructor from the service, TES location and cuts 
      SourceTES 
      ( const LoKi::TrackTypes::TrCuts& cuts        ,  
        const std::vector<std::string>& path        ,
        IDataProviderSvc*               svc  = 0    ) ;
      /// constructor from the service, TES location and cuts 
      SourceTES 
      ( const LoKi::TrackTypes::TrCuts& cuts  ,  
        IDataProviderSvc*               svc   ,  
        const std::string&              path  ) ;
      /// constructor from the service, TES location and cuts 
      SourceTES 
      ( const LoKi::TrackTypes::TrCuts& cuts  ,  
        IDataProviderSvc*               svc   ,  
        const std::vector<std::string>& path  ) ;
      /// copy constructor
      SourceTES ( const SourceTES& right ) ;
      /// MANDATORY: virtual destructor 
      virtual ~SourceTES() ;
      /// MANDATORY: clone method ("virtual constructor")
      virtual  SourceTES* clone() const  ;
      /// MANDATORY: the only essential method:
      virtual result_type operator() ( /* argument */ ) const ;
      /// OPTIONAL: the nice printout
      virtual std::ostream& fillStream ( std::ostream& o ) const ;
      // ======================================================================
    private:
      // ======================================================================
      /// the default constructor is disabled 
      SourceTES() ;                      // the default constructor is disabled 
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
      /// get the cuts :
      const LoKi::TrackTypes::TrCuts& cut() const { return m_cut ; }
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
    public:
      // ======================================================================
      /// get the tracks from the certain  TES location 
      LHCb::Track::Range get 
      ( const std::string& location        , 
        const bool         exc      = true ) const ;
      // ======================================================================
    private:
      // ======================================================================
      /// TES locations of Tracks 
      std::vector<std::string>  m_path ;             // TES locations of tracks 
      /// data provider service 
      mutable LoKi::Interface<IDataProviderSvc>  m_dataSvc ; // the service
      /// 'on-flight' filter
      LoKi::TrackTypes::TrCut   m_cut ;              //      'on-flight' filter
      // ======================================================================
    } ;
    // ========================================================================
    /** @class TESCounter 
     *  simple functor to count number of 'good'-objects form TES 
     *  @author Vanya BELYAEV Ivan.BElyaev@nikhef.nl
     *  @see LoKi::Cuts::TrNUM
     *  @date   2010-10-24
     */
    class TESCounter 
      : public LoKi::Functor<void,double>
    {
    public:
      // ======================================================================
      /// constructor from the service, TES location and cuts 
      TESCounter 
      ( const std::string&              path                    , 
        const LoKi::TrackTypes::TrCuts& cuts = 
        LoKi::BasicFunctors<const LHCb::Track*>::BooleanConstant(true) ) ; 
      /// constructor from the service, TES location and cuts 
      TESCounter
      ( const std::vector<std::string>& path                    , 
        const LoKi::TrackTypes::TrCuts& cuts = 
        LoKi::BasicFunctors<const LHCb::Track*>::BooleanConstant(true) ) ; 
      /// MANDATORY: virtual destructor 
      virtual ~TESCounter() ;
      /// MANDATORY: clone method ("virtual constructor")
      virtual  TESCounter* clone() const ;
      /// MANDATORY: the only essential method:
      virtual result_type operator() ( /* argument */ ) const ;
      /// OPTIONAL: the nice printout
      virtual std::ostream& fillStream ( std::ostream& o ) const ;
      // ======================================================================
    private:
      // ======================================================================
      /// the default constructor is disabled 
      TESCounter () ;                    // the default constructor is disabled 
      // ======================================================================
    private:
      // ======================================================================
      /// the actual source 
      SourceTES m_source ;                                 // the actual source 
      // ======================================================================
    } ;
    // ========================================================================
  } //                                            end of namespace LoKi::Tracks
  // ==========================================================================
} //                                                      end of namespace LoKi
// ============================================================================
//                                                                      The END 
// ============================================================================
#endif // LOKI_TRSOURCES_H
// ============================================================================
