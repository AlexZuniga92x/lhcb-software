#include "Kernel/SelectionLine.h"
#include "GaudiKernel/AlgFactory.h" 
#include "GaudiKernel/VectorMap.h" 
#include "HltBase/IHltData.h"
#include "HltBase/IHltInspector.h"
#include "HltBase/IHltRegister.h"
#include "HltBase/HltSelection.h"

namespace Hlt {
    class Line : public ::Selection::Line {
    public:
        Line( const std::string& name, ISvcLocator* pSvcLocator );

        StatusCode initialize();
        unsigned int numberOfCandidates() const;
        unsigned int numberOfCandidates(const Algorithm*) const;
        std::pair<std::string,unsigned> id() const;

    private:
        void SetupSelections();
        std::pair<std::string,unsigned> SetupID();
        Hlt::IData& dataSvc() const;
        Hlt::IInspector& inspectionSvc() const;
        Hlt::IRegister& regSvc() const;

        mutable Hlt::IData* m_hltSvc;
        mutable Hlt::IInspector* m_inspectionSvc;
        mutable Hlt::IRegister* m_regSvc;
        const Hlt::Selection *m_selection;

        std::pair<std::string,unsigned> m_id;
        typedef GaudiUtils::VectorMap<Gaudi::StringKey,const Hlt::Selection*> Selection_t;
        Selection_t m_selections;


    };

};

//-----------------------------------------------------------------------------
// Implementation file for class : HltL0CaloCandidates
// 2008-12-05 : Gerhard Raven
//-----------------------------------------------------------------------------

// Declaration of the Algorithm Factory
DECLARE_NAMESPACE_ALGORITHM_FACTORY( Hlt, Line );



Hlt::Line::Line( const std::string& name, ISvcLocator* pSvcLocator )
    : ::Selection::Line(name,pSvcLocator)
    , m_hltSvc(0)
    , m_inspectionSvc(0)
    , m_selection(0)
{

}

StatusCode Hlt::Line::initialize() {
  StatusCode sc = ::Selection::Line::initialize();
  SetupSelections();
  m_id = SetupID();
  return sc;
}


Hlt::IRegister& Hlt::Line::regSvc() const {
  if (m_regSvc == 0) {
    m_regSvc = svc<Hlt::IRegister>("Hlt::Service");
    Assert( m_regSvc != 0, " no Hlt::Service??");
  }
  return *m_regSvc;
}

Hlt::IData& Hlt::Line::dataSvc() const {
  if (m_hltSvc == 0) {
    m_hltSvc = svc<Hlt::IData>("Hlt::Service");
    Assert( m_hltSvc != 0, " no Hlt::Service??");
  }
  return *m_hltSvc;
}
              
Hlt::IInspector& Hlt::Line::inspectionSvc() const {
  if (m_inspectionSvc == 0) {
    m_inspectionSvc = svc<Hlt::IInspector>("Hlt::Service");
    Assert( m_inspectionSvc != 0, " no Hlt::Service??");
  }
  return *m_inspectionSvc;
}

unsigned int Hlt::Line::numberOfCandidates() const {
  return m_selection != 0 ? m_selection->size() : 0 ;
}

unsigned int Hlt::Line::numberOfCandidates(const Algorithm* algorithm) const{
  Selection_t::const_iterator i = m_selections.find(algorithm->name());
  return i != m_selections.end() ? i->second->size() : 0 ;
}

void Hlt::Line::SetupSelections() {
  Hlt::IRegister::Lock lock(&regSvc(),this);
  Gaudi::StringKey key(decisionName());
  if ( inspectionSvc().hasSelection( key )  ) {
     lock->registerInput(key,this);
     m_selection = dataSvc().selection(key,this);
  }
  std::vector<const Algorithm*>  algos = algorithms();
  for( std::vector<const Algorithm*>::const_iterator i = algos.begin(); i!= algos.end();++i) {
     Hlt::IInspector::SelList selections;
     if (inspectionSvc().outputs( *i,selections)==0) continue;
     // just pick up the last one (don't know what to do if more than one anyway!)
     m_selections.insert(std::make_pair(Gaudi::StringKey((*i)->name()), selections.back()));
  }
}

std::pair<std::string,unsigned> Hlt::Line::id() const {
    return m_id;
}

std::pair<std::string,unsigned> Hlt::Line::SetupID() {
  // TODO: do this during initialize, 'lock' updates after initialize...
  static const std::string major1("Hlt1SelectionID");
  static const std::string major2("Hlt2SelectionID");
  const std::string* major = 0;
  if (decisionName().find("Hlt1")!=std::string::npos) {
    major =  &major1;
  } else if (decisionName().find("Hlt2")!=std::string::npos) {
    major = &major2;
  } else {
    error() << " Could not find Hlt1/Hlt2 in decision " << decisionName() << " ???? " << endmsg;
  }
  boost::optional<IANNSvc::minor_value_type> key = annSvc().value(*major,decisionName());

  if (!key) {
    warning() << " DecisionName=" << decisionName() << " not known under major " << *major << endmsg;
    return std::make_pair(std::string(),0);
  }
  return *key;
}
