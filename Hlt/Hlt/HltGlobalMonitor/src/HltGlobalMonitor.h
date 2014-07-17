#ifndef HLTGLOBALMONITOR_H
#define HLTGLOBALMONITOR_H 1

#include "Event/ODIN.h"
#include "Event/HltDecReports.h"

#include "boost/function.hpp"

#include "GaudiKernel/IIncidentListener.h"
#include "GaudiKernel/StringKey.h"
#include "GaudiKernel/HistoDef.h"
#include "GaudiKernel/ObjectContainerBase.h"
#include "HltBase/HltBaseAlg.h"

#include "TProfile.h"
#include "GaudiUtils/Aida2ROOT.h"

class Condition;

/** @class HltGlobalMonitor HltGlobalMonitor.h
 *
 *  functionality:
 *        monitor the global Hlt results
 *        make histograms with Hlt diagnostics info
 *
 *  @author Arthur Maciel
 *  @date   2003-03-06
 */
class HltGlobalMonitor : public HltBaseAlg, virtual public IIncidentListener
{
  public:
    /// Standard constructor
    HltGlobalMonitor( const std::string& name, ISvcLocator* pSvcLocator );

    virtual ~HltGlobalMonitor(); ///< Destructor

    virtual StatusCode initialize(); ///< Algorithm initialization
    virtual StatusCode execute();    ///< Algorithm execution
    void handle( const Incident& );

    /// update the condition
    StatusCode updateCondition();

  private:
    size_t rawEvtLength( const LHCb::RawEvent* evt );
    size_t rawEvtLength( const std::vector<LHCb::RawBank*>& banks );
    void monitorODIN( const LHCb::ODIN*, const LHCb::HltDecReports*,  const LHCb::HltDecReports* );
    void monitorHLT( const LHCb::ODIN*, const LHCb::HltDecReports*,  const LHCb::HltDecReports* );
    void monitorVertices();
    void monitorTrends();
    void monitorResolverpositions();

    template <typename T>
    T* fetch( const std::string& location, bool warn = true )
    {
        if (location.empty()) return nullptr;
        T* t = this->getIfExists<T>( location );
        if ( !t && warn && this->msgLevel( MSG::WARNING ) ) {
            Warning( " could not retrieve " + location, StatusCode::SUCCESS, 10 );
        }
        return t;
    }

    typedef std::map<std::string, std::vector<std::string>> DecToGroupType;
    DecToGroupType m_DecToGroup1;
    DecToGroupType m_DecToGroup2;

    // index in m_hlt1Alleys, m_hlt1AlleyRates , followed by binnumber inside alley
    std::map<Gaudi::StringKey, std::pair<unsigned, unsigned>> m_hlt1Line2AlleyBin;
    std::map<Gaudi::StringKey, std::pair<unsigned, unsigned>> m_hlt2Line2AlleyBin;
    std::vector<AIDA::IHistogram1D*> m_hlt1Alleys;
    std::vector<AIDA::IHistogram1D*> m_hlt2Alleys;
    std::vector<StatEntity*> m_hlt1AlleyRates;
    std::vector<StatEntity*> m_hlt2AlleyRates;

    std::string m_Hlt1DecReportsLocation;
    std::string m_Hlt2DecReportsLocation;
    std::string m_ODINLocation;
    std::string m_hlt1Decision;
    std::string m_hlt2Decision;
    std::string m_rawEventLocation;

    AIDA::IHistogram1D* m_odin = nullptr;
    AIDA::IHistogram1D* m_odinHLT1 = nullptr;
    AIDA::IHistogram1D* m_odinHLT2 = nullptr;

    AIDA::IHistogram1D* m_hlt1Alley = nullptr;
    AIDA::IHistogram1D* m_hlt2Alley = nullptr;

    AIDA::IProfile1D* m_hltVirtTime = nullptr;
    AIDA::IHistogram1D* m_hltTime = nullptr;
    AIDA::IProfile1D* m_hltEventsTime = nullptr;
    AIDA::IHistogram1D* m_tasks = nullptr;

    AIDA::IHistogram2D* m_hlt1AlleysCorrelations = nullptr;
    AIDA::IHistogram2D* m_hlt2AlleysCorrelations = nullptr;

    AIDA::IProfile1D* m_hltTimeVsEvtSize = nullptr;

    // AIDA::IHistogram1D* m_resolvxr50 = nullptr;
    // AIDA::IHistogram1D* m_resolvxr5 = nullptr;
    // AIDA::IHistogram1D* m_resolvxl50 = nullptr;
    // AIDA::IHistogram1D* m_resolvxl5 = nullptr;
    AIDA::IProfile1D* m_resolvxle = nullptr;
    AIDA::IProfile1D* m_resolvxre = nullptr;
    // AIDA::IHistogram1D* m_resolvy50 = nullptr;
    // AIDA::IHistogram1D* m_resolvy5 = nullptr;
    AIDA::IProfile1D* m_resolvye = nullptr;
    AIDA::IProfile1D* m_magnetcurrent = nullptr;
    AIDA::IProfile1D* m_magnetpolarity = nullptr;
    AIDA::IProfile1D* m_magnetstate = nullptr;
    AIDA::IProfile1D* m_rich1pressure = nullptr;
    AIDA::IProfile1D* m_rich1temperature = nullptr;
    AIDA::IProfile1D* m_rich2pressure = nullptr;
    AIDA::IProfile1D* m_rich2temperature = nullptr;
    AIDA::IProfile1D* m_lhcnbofbunches = nullptr;
    AIDA::IProfile1D* m_lumipars = nullptr;


    // Monitor 3D vertex filter
    Condition* m_veloCondition = nullptr;
    Condition* m_magnetCondition = nullptr;
    Condition* m_rich1Condition = nullptr;
    Condition* m_rich2Condition = nullptr;
    Condition* m_lhcfillingschemeCondition = nullptr;
    Condition* m_lumiparsCondition = nullptr;
    /*Condition* m_runinfoCondition = nullptr;
    Condition* m_runparsCondition = nullptr;*/

    typedef std::vector<AIDA::IHistogram2D*> HistoVector;
    typedef std::map<std::string, HistoVector> HistoMap;
    HistoMap m_vertexHistos;
    double m_beamSpotX = 0;
    double m_beamSpotY = 0;
    double m_xRC = 0;
    double m_xLA = 0;
    double m_Y = 0;

    double m_magnetCurrent = 0;
    double m_magnetPolarity = 0;
    double m_magnetState = 0;

    double m_rich1Temperature = 0;
    double m_rich1Presssure = 0;
    double m_rich2Temperature = 0;
    double m_rich2Presssure = 0;

    int m_lhcNCollidingBunches = 0;

    std::vector<double> m_lumiparvalues;

    typedef std::map<std::string, std::string> LocationMap;
    LocationMap m_vertexLocations;

    class histopair
    {
      public:
        histopair( GaudiHistoAlg& parent, std::string loc,
                   const Gaudi::Histo1DDef& def, const char* yaxislabel = 0 );
        void fill( double x, double y )
        {
            m_histo->fill( x );
            m_profile->fill( x, y );
        }
        void operator()( double CPUtime )
        {
            int n = m_parent->exist<ObjectContainerBase>( m_loc )
                        ? (int)m_parent->get<ObjectContainerBase>( m_loc )
                              ->numberOfObjects()
                        : -1;
            fill( n, CPUtime );
        }

      private:
        GaudiHistoAlg* m_parent;
        AIDA::IHistogram1D* m_histo;
        AIDA::IProfile1D* m_profile;
        std::string m_loc;
    };

    std::map<std::string, Gaudi::Histo1DDef> m_correlateCPU;
    std::vector<histopair> m_CPUcorrelations;

    double m_currentTime = 0; // seconds since start of clock
    double m_timeSize = 0;
    double m_timeInterval = 0;
    longlong m_startClock = 0; // in microseconds
    longlong m_startEvent = 0; // in microseconds
    long m_virtmem = 0;
    int m_gpstimesec = 0;
};
#endif // HLTGLOBALMONITOR_H
