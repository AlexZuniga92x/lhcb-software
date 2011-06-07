// $Id: HltAlgorithm.cpp,v 1.64 2010-08-18 09:51:11 graven Exp $
// Include files 

#include "Event/Particle.h"
#include "Event/Track.h"
#include "Event/RecVertex.h"
#include "HltBase/HltAlgorithm.h"
#include "boost/lambda/lambda.hpp"
#include "boost/lambda/bind.hpp"
#include "boost/foreach.hpp"

namespace bl = boost::lambda; 
//-----------------------------------------------------------------------------
// Implementation file for class : HltAlgorithm
//
// 2006-06-15 : Jose Angel Hernando Morata
//-----------------------------------------------------------------------------
namespace {
    class histoGuard {
    public:
        histoGuard(bool switchOff,HltAlgorithm& parent) : m_parent(parent), m_flag( parent.produceHistos() && switchOff) {
            if (m_flag) m_parent.setProduceHistos(false);
        }
        ~histoGuard() {
            if (m_flag) m_parent.setProduceHistos(true);
        }
    private:
        HltAlgorithm& m_parent;
        bool m_flag;
    };
}
// ============================================================================
HltAlgorithm::HltAlgorithm( const std::string& name,
                            ISvcLocator* pSvcLocator,
                            bool requireInputsToBeValid )
  : HltBaseAlg ( name , pSvcLocator )
  , m_requireInputsToBeValid(requireInputsToBeValid)
  , m_outputSelection(0)
  , m_outputHisto(0)
{
  declareProperty("RequirePositiveInputs", m_requireInputsToBeValid );
  declareProperty("HistogramUpdatePeriod" , m_histogramUpdatePeriod = 0 );

  //TODO: in init, declare these to mon svc...
  counter("#accept");
  counter("#candidates accepted");


  //TODO: since this is not applicable to all algorithms, remove from base...
  declareProperty("MinCandidates",m_minNCandidates = 1);
}

HltAlgorithm::~HltAlgorithm()
{
  delete m_outputSelection; m_outputSelection = 0;
} 

StatusCode HltAlgorithm::queryInterface 
(const InterfaceID& iid, void** ppvi ) 
{
  /// valid placeholder?
  if ( 0 == ppvi ) { return StatusCode::FAILURE ; }
  /// 
  if ( Hlt::IUnit::interfaceID() == iid ) { 
    *ppvi = static_cast<Hlt::IUnit*>( this ) ; 
    addRef() ;
    return StatusCode::SUCCESS ;                                      // RETURN 
  }
  // fall back to the base 
  return HltBaseAlg::queryInterface ( iid , ppvi ) ; 
}

StatusCode HltAlgorithm::sysInitialize() {
  // Bypass the initialization if the algorithm
  // has already been initialized.
  if ( Gaudi::StateMachine::INITIALIZED <= FSMState() ) return StatusCode::SUCCESS;
  // set up context such that tools can grab the algorithm...
  // kind of non-local, but kind of better (much more 
  // lightweight and less invasive) than the alternative.
  // Note that while GaudiAlgorithm registers the context svc
  // in sysExectute, it doesn't do so in sysInitialize...
  Gaudi::Utils::AlgContext sentry( contextSvc(), this );
  Hlt::IRegister::Lock lock(regSvc(),this);
  return HltBaseAlg::sysInitialize();
}

StatusCode HltAlgorithm::sysFinalize() {
  BOOST_FOREACH( CallBack* i, m_callbacks ) delete i ; 
  m_callbacks.clear();
  return HltBaseAlg::sysFinalize();
}


StatusCode HltAlgorithm::restart() {
  info() << "restart of " << name() << " requested " << endmsg;
  return StatusCode::SUCCESS;
}

StatusCode HltAlgorithm::sysExecute() {

  StatusCode sc = StatusCode::SUCCESS;

  // switch of histogramming for this event only if so requested
  histoGuard guard( m_histogramUpdatePeriod>0 && (counter("#accept").nEntries() % m_histogramUpdatePeriod !=0), *this );

  sc = beginExecute();
  if (sc.isFailure()) return sc;
  sc = HltBaseAlg::sysExecute();
  if (sc.isFailure()) return sc;
  sc = endExecute();

  return sc;

}

StatusCode HltAlgorithm::beginExecute() {
  if ( m_outputSelection == 0 ) {
      error() << " no output selection !!" << endmsg;
      return StatusCode::FAILURE;
  }

  setFilterPassed(false);

  // we always process callbacks first...
  BOOST_FOREACH( CallBack* i, m_callbacks ) { 
    StatusCode status = i->execute();
    if (!status) {

    }
  }
  
  // assert if not done properly...
  Assert(m_outputSelection->decision()==false 
      && m_outputSelection->processed()==false
      && m_outputSelection->error()==false," output already touched???");

  bool ok = verifyInput();
  
  return ok ? StatusCode::SUCCESS : StatusCode::FAILURE ;
}

StatusCode HltAlgorithm::endExecute() {
  //TODO: add flushbacks here...
  size_t n = m_outputSelection->size();
  counter("#candidates accepted") += n ; 
  // for non-counting triggers, this must be done explicity by hand!!!
  if (n>=m_minNCandidates) m_outputSelection->setDecision(true); 
  setDecision( m_outputSelection->decision() );

  if (produceHistos()) {
      for (IMap::const_iterator it = m_in.begin();
           it != m_in.end(); ++it) {
        fill(m_inputHistos[it->second->id()],it->second->size(),1.);
      }
      fill(m_outputHisto,m_outputSelection->size(),1.);
  }
  
  if (msgLevel(MSG::DEBUG)) debug() << " output candidates " << m_outputSelection->size() 
          << " decision " << m_outputSelection->decision() 
          << " filterpassed " << filterPassed() << endreq;
  return StatusCode::SUCCESS;
}



bool HltAlgorithm::verifyInput() 
{
  if (!m_requireInputsToBeValid) return true;
  bool ok = true;
  for (IMap::const_iterator i = m_in.begin(); i!= m_in.end();++i) {
    // propagate error status!
    if (i->second->error()) m_outputSelection->setError(true);
    ok = ok &&  i->second->decision() ;
    if (msgLevel(MSG::DEBUG)) 
      debug() << " input " << i->second->id()
              << " decision " << i->second->decision() 
              << " process status " << i->second->processed() 
              << " error status " << i->second->error() 
              << " candidates " << i->second->size() << endreq;
  }

  if (!ok) {
    warning() << endreq;
    warning() << endreq;
    warning() << " FIXME FIXME FIXME FIXME" << endreq;
    warning() << endreq;
    warning() << " Empty input or false input selection!" << endreq;
    warning() << " Most likely due to a misconfiguration" << endreq;
    for (IMap::const_iterator i = m_in.begin() ; i!=m_in.end(); ++i ) {
      warning() << " input selection " << i->second->id()
                << " decision " << i->second->decision()
                << " processed " << i->second->processed()
                << " error " << i->second->error()
                << " candidates " << i->second->size() << endreq;      
    }
    warning() << endreq;
    warning() << endreq;
    warning() << endreq;
    return StatusCode::FAILURE;
  }
  return ok;
}


void HltAlgorithm::setDecision(bool accept) {
  setFilterPassed(accept);
  // the next forces the 'processed' flag to be set -- even if in some 
  // cases 'accept' is obtained from m_outputSelection!!!
  // (basically, if we are of type TSelection<X>, and have no selected
  // candidates, we never explicitly set 'false', and hence would otherwise
  // not set 'processed'
  m_outputSelection->setDecision(accept);
  counter("#accept") += accept;
}


class cmp_by_id {
public:
    cmp_by_id(const Hlt::Selection& sel) : m_id( sel.id() ) {}
    template <typename T>
    bool operator()(const T& i) { return  i.second->id() == m_id; }
private:
    Gaudi::StringKey m_id;
};

// TODO: switch selection & declareInput around...
const Hlt::Selection* HltAlgorithm::declareInput(const Gaudi::StringKey& key, const Hlt::IUnit::Client&) const {
    return selection(key);
}

const Hlt::Selection* HltAlgorithm::selection(const Gaudi::StringKey& key, const Hlt::IUnit::Client&) const {
    return selection(key);
}

const Hlt::Selection* HltAlgorithm::selection(const Gaudi::StringKey& selname) const {
    Assert(!selname.empty()," selection(): no selection name");
    if (msgLevel(MSG::DEBUG)) debug() << " selection: " << selname << endreq;
    StatusCode sc = regSvc()->registerInput(selname,this);
    if (sc.isFailure()) {
      error() << " failed to register input " << selname << endreq;
      Assert(0," selection:, failed to register input!");
        
    }
    const Hlt::Selection* sel = hltSvc()->selection(selname,this);
    if (sel == 0 ) {
      error() << " failed to retrieve input " << selname << endreq;
      Assert(0," selection:, failed to retrieve input!");
    }
    if (std::find_if(m_in.begin(), m_in.end(), cmp_by_id(*sel))==m_in.end() ) {
      m_in.insert(std::make_pair(sel->id(),sel));
      if (produceHistos()) {
        bool ok = m_inputHistos.find(sel->id()) == m_inputHistos.end();
        Assert(ok, "selection(): already input selection "+sel->id().str());
        m_inputHistos[sel->id()] = const_cast<HltAlgorithm*>(this)->initializeHisto(sel->id().str());
      }
      if (msgLevel(MSG::DEBUG)) debug() << " Input selection " << sel->id() << endreq;
    }
    if (msgLevel(MSG::DEBUG)) debug() << " retrieved selection " << sel->id() << endreq;    
    return sel;
}

StatusCode HltAlgorithm::registerOutput(Hlt::Selection* sel, const Hlt::IUnit::Client& /*client*/) const{
        return registerOutput(sel);
}
  
StatusCode HltAlgorithm::registerOutput(Hlt::Selection* sel) const{
    if (m_outputSelection!=0) {
        error() << "attempt to register a 2nd output selection: " << sel->id() << " already have " << m_outputSelection->id() << endmsg;
        return StatusCode::FAILURE;
    }
    m_outputSelection = sel;
    std::vector<const Hlt::Selection*> sels;
    for (IMap::const_iterator i=m_in.begin();i!=m_in.end();++i) {
          sels.push_back(i->second);
    }
    sel->addInputSelectionIDs( sels.begin(), sels.end() );
    if (msgLevel(MSG::DEBUG)) debug() << " Output selection " << sel->id() << endreq;
    if (regSvc()->registerOutput(sel,this).isFailure()) {
       error() <<"Failed to add Selection" << sel->id() << endmsg; 
       return StatusCode::FAILURE;
    }
    m_outputHisto = const_cast<HltAlgorithm*>(this)->initializeHisto("Ncandidates",-0.5,99.5,100);
    if (msgLevel(MSG::DEBUG)) debug() << " registered selection " << sel->id() << " type " << sel->classID() << endreq;
    return StatusCode::SUCCESS;
}

StatusCode HltAlgorithm::registerTESInput(const Gaudi::StringKey& key, const Hlt::IUnit::Client&) const
{
    return registerTESInput(key);
}
StatusCode HltAlgorithm::registerTESInput(const Gaudi::StringKey& key) const 
{

  StatusCode sc = regSvc()->registerTESInput(key,this);
  if ( sc.isFailure() ) 
  { return Error ( "Unable to register INPUT TES location '" + 
                   key.str() + "'" , sc ) ; }

  LVct::const_iterator ifind = 
    std::find ( m_tes.begin() , m_tes.end() , key ) ;
  if ( m_tes.end () != ifind ) { return StatusCode::SUCCESS ;}        // RETURN 
  //
  m_tes.push_back ( key ) ;
  return StatusCode::SUCCESS ;
}

const DataObject*
HltAlgorithm::tes 
( const Hlt::IUnit::Client& /* client  */ , 
  const Hlt::IUnit::Key&       location   ) const 
{
  // check the location
  LVct::const_iterator ifind = std::find
    ( m_tes.begin() , m_tes.end  () , location ) ;
  //
  Assert ( m_tes.end() != ifind , 
           "tes: anuthorized access to TES -data" ) ;
  return hltSvc()->tes ( this , location ) ;
  //
}


Hlt::Selection* HltAlgorithm::retrieve  ( const Hlt::IUnit::Client& /* client */ , 
                                          const Gaudi::StringKey&    /* key    */ ) const 
{
      Error("retrieve(): not implemented ") ;
      return 0 ;
}
