// $Id: MakeResonances.cpp,v 1.24 2007-02-17 12:51:01 pkoppenb Exp $

#include <algorithm>

// from Gaudi
#include "GaudiKernel/AlgFactory.h" 

// from DaVinci
#include "Kernel/IDecodeSimpleDecayString.h"
#include "Kernel/IFilterCriterion.h"
#include "Kernel/IPlotTool.h"
#include "Kernel/ICheckOverlap.h"
#include "Kernel/IParticleDescendants.h"
// local
#include "MakeResonances.h"
// #define PKDEBUG
// #define PKDEBUG2

//-----------------------------------------------------------------------------
// Implementation file for class : MakeResonances
//
// 2004-11-29 : Patrick KOPPENBURG
//-----------------------------------------------------------------------------

// Declaration of the Algorithm Factory
DECLARE_ALGORITHM_FACTORY( MakeResonances );

//=============================================================================
// Standard constructor, initializes variables
//=============================================================================
MakeResonances::MakeResonances( const std::string& name,
                                ISvcLocator* pSvcLocator)
  : 
  DVAlgorithm ( name , pSvcLocator ),
  m_daughterFilter(),
  m_motherFilter(),
  m_daughterPlots(),
  m_motherPlots(),
  m_checkOverlap(),
  m_particleDescendants(),
  m_decayDescriptors(),
  m_nEvents(0),
  m_nAccepted(0),
  m_nCandidates(0),
  m_decays()
{
  declareProperty( "DaughterFilterName" , m_daughterFilterName = "DaughterFilter" );
  declareProperty( "MotherFilterName" , m_motherFilterName = "MotherFilter");
  declareProperty( "Window", m_massWindow = 1000000. );  
  declareProperty( "UpperWindow", m_upperMassWindow = 1000000. );  
  declareProperty( "LowerWindow", m_lowerMassWindow = 1000000. );  
  declareProperty( "MinMomentum", m_minMomentum = -1. );  
  declareProperty( "MinPt", m_minPt = -1. );
  declareProperty( "KillOverlap", m_killOverlap = true );  
  declareProperty( "DaughterPlotTool", m_daughterPlotTool = "RecursivePlotTool/DaughterPlots" );  
  declareProperty( "MotherPlotTool", m_motherPlotTool = "RecursivePlotTool/MotherPlots" );  
  declareProperty( "DaughterPlotsPath", m_daughterPlotsPath = "" );  
  declareProperty( "MotherPlotsPath", m_motherPlotsPath = "" );  
  declareProperty( "DecayDescriptors", m_decayDescriptors );  
  //  declareProperty( "MakePlots" , m_makePlots = false) ;
  declareProperty( "MotherToNGammas" , m_motherToNGammas = false) ;
  declareProperty( "OutputLocation" , m_outputLocation = "" ) ;
  setProperty ( "HistoProduce", "0" ) ; // overwrites GaudiHistoAlg.cpp
}
//=============================================================================
// Destructor
//=============================================================================
MakeResonances::~MakeResonances() {}; 

//#############################################################################
// Initialization
//#############################################################################
StatusCode MakeResonances::initialize() {

  StatusCode sc = DVAlgorithm::initialize();
  if (!sc) return sc;

  debug() << "==> Initialize" << endmsg;
  info() << "Mass Cuts are " << endmsg;
  info() << ">>>   Upper Mass Window  " 
         << std::min(m_massWindow,m_upperMassWindow) << endmsg;
  info() << ">>>   Lower Mass Window  " 
         << std::min(m_massWindow,m_lowerMassWindow) << endmsg;
  if ( m_minPt > 0.) info() << ">>>   Minimum Pt  " << m_minPt << endmsg;
  if ( m_minMomentum > 0.) info() << ">>>   Minimum P  " 
                                  << m_minMomentum << endmsg;

  m_daughterFilter = tool<IFilterCriterion>("ByPIDFilterCriterion",m_daughterFilterName , this ) ;
  m_motherFilter = tool<IFilterCriterion>("ByPIDFilterCriterion", m_motherFilterName, this ) ;
  if ( m_killOverlap ){
    m_checkOverlap = checkOverlap() ;
    m_particleDescendants = descendants();
    if ( !m_checkOverlap || ! m_particleDescendants ){
      err() << "Cannot retrieve CheckOverlap or ParticleDescendants Tools" 
            << endmsg ;
      return StatusCode::FAILURE ;    
    }
    verbose() << "Got overlap and particle descendants tools" << endmsg ;
  }
  // histogramming 

  if ( produceHistos() ){
    if ( m_daughterPlotTool != "none" ){
      m_daughterPlots = tool<IPlotTool>(m_daughterPlotTool,this);
      if( !m_daughterPlots ) {
        err() << "Unable to get " << m_daughterPlotTool << endmsg;
        return StatusCode::FAILURE;
      }
      if (m_daughterPlotsPath == "") m_daughterPlotsPath = "I"+name();
      else m_daughterPlots->setPath(m_daughterPlotsPath);
      info() << "Daughter plots will be in " << m_daughterPlotsPath << endmsg ;
    }
    m_motherPlots = tool<IPlotTool>(m_motherPlotTool,this);
    if ( m_motherPlotTool != "none" ){
      if( !m_motherPlots ) {
        err() << "Unable to get " << m_motherPlotTool << endmsg;
        return StatusCode::FAILURE;
      }
      if (m_motherPlotsPath == "") m_motherPlotsPath = "O"+name();
      else m_motherPlots->setPath(m_motherPlotsPath);
      info() << "Mother plots will be in " << m_motherPlotsPath << endmsg ;
    }    
  }

  if(m_motherToNGammas){
    info() << "Will make mother to n gammas" << endmsg;
  }

  if(m_motherToNGammas){
    if ( m_vertexFitNames.end() == m_vertexFitNames.find("ParticleAdder") ) 
    {
      m_vertexFitNames["ParticleAdder"] = "ParticleAdder" ; /// @todo Write ParticleAdder tool
      IVertexFit* tmp = vertexFitter("ParticleAdder") ;
      if (NULL==tmp){
        err() << "Could not get ParticleAdder" <<endmsg ;
        return StatusCode::FAILURE;
      }
    }
  }

  sc = createDecays();
  if ( sc.isFailure()) return sc ;

  return StatusCode::SUCCESS;
};
//=============================================================================
// Create decays, Stolen from CombineParticles
//=============================================================================
StatusCode MakeResonances::createDecays(){
  // get string decoder
  IDecodeSimpleDecayString* dsds = tool<IDecodeSimpleDecayString>("DecodeSimpleDecayString",this);
  if ( !dsds ) return StatusCode::FAILURE ;
  
  if ( m_decayDescriptors.empty() ){
    debug() << "No decay descriptors array defined. Pushing back " << getDecayDescriptor() << endmsg ;
    m_decayDescriptors.push_back(getDecayDescriptor());
  }
  debug() << "Decay descriptors are " << m_decayDescriptors << endmsg ;

  for ( std::vector<std::string>::const_iterator dd = m_decayDescriptors.begin() ;
        dd != m_decayDescriptors.end() ; ++dd ){
    
    debug() << " setting up " << *dd << endmsg;

    // initialize string decode
    StatusCode sc = dsds->setDescriptor(*dd);
    if ( sc.isFailure()) return sc ;  

    std::string mother;
    strings daughters;
    debug() << "Getting strings for " << dsds->getDescriptor() << endmsg;
    sc = dsds->getStrings(mother, daughters);
    if (sc.isFailure()) return sc;  
    std::sort(daughters.begin(),daughters.end()); // helps a lot, and used to avoid duplication of mothers in cc
    debug() << "Sorted daughters to " << daughters << endmsg ;
    sc = createDecay(mother, daughters);
    if (sc.isFailure()) return sc;
    
    // once again for cc if needed
    if ( dsds->is_cc() ){
      // LF : avoid duplication of mothers when using []cc
      strings daughtersBeforecc = daughters; // daughters have been sorted
      std::string motherBeforecc = mother ;
      debug() << "Sorted before cc daughters to " << daughtersBeforecc << endmsg;
      
      debug() << "Setting up cc for " << dsds->getDescriptor() << endmsg;
      sc = dsds->getStrings_cc(mother, daughters);
      if (sc.isFailure()) return sc;
      std::sort(daughters.begin(),daughters.end()); // needed to compare daughters for original and cc
      debug() << "Sorted cc daughters to " << daughters << endmsg ;

      if(daughtersBeforecc == daughters){
        warning() << "You have chosen two charged-conjugated modes with identical final states: " 
                  << m_decayDescriptors << endmsg ;
        warning() << "All final state particle will be duplicated as " << motherBeforecc << " and " << mother << endmsg ;
      }
      sc = createDecay(mother, daughters);
      if (sc.isFailure()) return sc;
      
    } else verbose() << dsds->getDescriptor() << " is not a cc mode" << endmsg ;
  }
  
  return StatusCode::SUCCESS;
}
//=============================================================================
// Create decays
//=============================================================================
StatusCode MakeResonances::createDecay(const std::string& mother, 
                                       const strings& daughters){
  info() << "Creating " << mother << " -> " << daughters << endmsg;
  
  // mother
  verbose() << "Found ParticlePropertySvc " << ppSvc() << endmsg ;
  ParticleProperty* pmother = ppSvc()->find(mother);
  if (NULL==pmother){
    err() << "Cannot find particle property for mother " << mother << endmsg ;
    return StatusCode::FAILURE;
  } else verbose() << "Found ParticleProperty " << pmother << endmsg ;
  int pid = pmother->pdgID() ;
  verbose() << "Found pid of " << mother << " = " << pid << endmsg ;
  if (!consideredPID(pid)) m_allPids.push_back(pid) ;
  
  //daughters
  std::vector<int> dpid ;
  for (strings::const_iterator d=daughters.begin() ; d!=daughters.end() ; ++d){
  // mother
    ParticleProperty* pd = ppSvc()->find(*d);
    if (!pd){
      err() << "Cannot find particle property for daughter " << *d << endmsg ;
      return StatusCode::FAILURE;
    }
    verbose() << "Found pid of " << *d << " = " << (pd->pdgID()) << endmsg ;
    dpid.push_back(pd->pdgID()) ;
    
    // add to list of all daughter PIDs
    if (!consideredPID(pd->pdgID())) m_allPids.push_back(pd->pdgID()) ;
  }
  verbose() << "Pushed back " << dpid.size() << " daughters" << endmsg ;

  double mass = pmother->mass();
  double minmass = mass - std::min(m_massWindow,m_lowerMassWindow);
  double maxmass = mass + std::min(m_massWindow,m_upperMassWindow);
  verbose() << "Mass cuts are " << minmass << " " << maxmass << endmsg ;

  // decay container
  Decay dk;
  StatusCode sc = dk.initialize(pid,dpid,minmass,maxmass,m_minMomentum,m_minPt,m_checkOverlap, m_particleDescendants);
  if (!sc) {
    err() << "Cannot initialize Decay object with " << pid << " " << dpid << endmsg ;
    return sc;
  }
  m_decays.push_back(dk);
  info() << "Initialized decay # " << m_decays.size() << endmsg ;
  
  return StatusCode::SUCCESS;
}
//#############################################################################
// Execution
//#############################################################################
StatusCode MakeResonances::execute() {

  debug() << "==> Execute" << endmsg;
  ++m_nEvents ;

  setFilterPassed(false);   // Mandatory. Set to true if event is accepted.
  LHCb::Particle::ConstVector Daughters, Resonances ;
  StatusCode sc = applyFilter(desktop()->particles(),Daughters,m_daughterFilter);
  if (!sc) {
    err() << "Unable to filter daughters" << endmsg;
    return StatusCode::FAILURE ;  
  }
  if (Daughters.empty()) {
    debug() << "No daughters found at all" << endmsg ;
    return StatusCode::SUCCESS;
  }
  sc = makePlots(Daughters,m_daughterPlots);
  if (!sc) return sc;

  // The LOOP ///
  for ( Decays::iterator d = m_decays.begin() ; d != m_decays.end() ; ++d ){
    verbose() << "New Decay loop" << endmsg ;
    if (!d->fillPidParticles(Daughters)){
      debug() << "Not all necessary particles found for decay" << endmsg ;
      continue ;
    }
    sc = applyDecay(*d,Resonances); // make the resonances
    if (!sc) return sc;
  }
  debug() << "Found " << Resonances.size() << " candidates" << endmsg ;
  // filter
  LHCb::Particle::ConstVector Final ;
  sc = applyFilter(Resonances,Final,m_motherFilter);
  if (!sc) {
    err() << "Unable to filter mothers" << endmsg;
    return StatusCode::FAILURE ;  
  }  
  sc = makePlots(Final,m_motherPlots);
  if (!sc) return sc;
  debug() << "Saving " << Final.size() << " candidates" << endmsg ;
  sc = desktop()->saveTrees(Final);
  if (!sc) {
    err() << "Unable to save mothers" << endmsg;
    return StatusCode::FAILURE ;  
  }  
  if ( !Final.empty()){
    setFilterPassed(true);   // Mandatory. Set to true if event is accepted.
    ++m_nAccepted;
    m_nCandidates+=Final.size();
  }
  
  return StatusCode::SUCCESS;
};
//=============================================================================
// Filter particles
//=============================================================================
StatusCode MakeResonances::applyFilter(const LHCb::Particle::ConstVector& vIn, 
                                       LHCb::Particle::ConstVector& vOut, 
                                       IFilterCriterion* fc) const{
  if (fc==NULL) { // not possible yet
    debug() << "Null filter criterion" << endmsg ;
    vOut = vIn ;
    return StatusCode::SUCCESS;
  }
  ///@todo write some Print function for LHCb::Particle.
  for ( LHCb::Particle::ConstVector::const_iterator p = vIn.begin() ; 
        p!=vIn.end(); ++p){
    if ( consideredPID((*p)->particleID().pid() )){
      if (fc->isSatisfied(*p)) {
        vOut.push_back(*p);
        debug() << "Particle " << (*p)->key() 
                << " ID=" << (*p)->particleID().pid() 
                << " with momentum " << (*p)->momentum() 
                << " m=" << (*p)->measuredMass() 
                << " passes cuts" << endmsg ;
      } else {    
        verbose() << "Particle "  << (*p)->key() 
                  << " ID=" << (*p)->particleID().pid() 
                  << " with momentum " 
                  << (*p)->momentum() << " m=" << (*p)->measuredMass() 
                  << " fails cuts" << endmsg ;
      }
    } else {
      verbose() << "Particle "  << (*p)->key() 
                << " ID=" << (*p)->particleID().pid() 
                << " with momentum " << (*p)->momentum() 
                << " m=" << (*p)->measuredMass() 
                << " is discarded" << endmsg ;
    }
    
  }
  return StatusCode::SUCCESS;
}
//=============================================================================
// Check if Particle needs to be considered
//=============================================================================
inline bool MakeResonances::consideredPID(const int& pid)const{
  verbose() << "consideredPID " << pid << endmsg ;
  bool out = false ;// not in list
  for ( std::vector<int>::const_iterator ap = m_allPids.begin() ; ap != m_allPids.end() ; ++ap ){
    if ( *ap == pid ) {
      out = true; // in list
      break ;
    }
  }
  verbose() << "consideredPID " << pid << " " << out << endmsg ;
  return out ; 
}
//=============================================================================
// Apply one decay
//=============================================================================
StatusCode MakeResonances::applyDecay(Decay& d, LHCb::Particle::ConstVector& Resonances){
  verbose() << "In applyDecay" << endmsg ;
  LHCb::Particle::ConstVector DaughterVector ;
  bool inloop = d.getFirstCandidates(DaughterVector); // get first daughter vector
  while (inloop){ 
    verbose() << "In while loop " << DaughterVector.size() << endmsg ;
    // Find invariant mass
    Gaudi::XYZTVector sum4(0,0,0,0) ;
    for ( LHCb::Particle::ConstVector::const_iterator p = DaughterVector.begin() ; 
          p!=DaughterVector.end() ; ++p){
      verbose() << "Particle " << (*p)->key() << " ID=" 
                << (*p)->particleID().pid() << " with momentum " << 
        (*p)->momentum() << " m=" << (*p)->measuredMass() << endmsg ;
      sum4 += (*p)->momentum() ;
    }
    verbose() << " -> momentum " << sum4 << " m=" << sum4.M() << endmsg ;
    if (d.goodFourMomentum(sum4)) {
      verbose() << "Found a candidate with mass " << sum4.M() << endmsg ;
      // LF
      // vertex fit or make mother to n gammas!
      const LHCb::Particle* Mother = 
        makeMother(DaughterVector,d.getMotherPid());
      if (0==Mother){
        if(m_motherToNGammas) Warning("Something failed making mother to n gammas",StatusCode::SUCCESS,1);
        else Warning("Something failed in vertex fitting",StatusCode::SUCCESS,1);
      }
      else {
        verbose() << "Getting mother " << Mother->particleID().pid()
                  << " " << Mother->momentum() << endmsg ;
        Resonances.push_back(Mother);
      }
    } // mass cut
    inloop = d.getNextCandidates(DaughterVector);
  }
  debug() << "Found " << Resonances.size() << " candidates with PID " 
          << d.getMotherPid().pid() << endmsg ;
  return StatusCode::SUCCESS ;  
}

//=============================================================================
//  Make Mother Vertex fit
//=============================================================================
const LHCb::Particle* MakeResonances::makeMother(const LHCb::Particle::ConstVector& Daughters,
                                                 const LHCb::ParticleID& motherPid){
  verbose() << "Will make particle with PID " << motherPid.pid() << endmsg ;

  StatusCode sc = StatusCode::SUCCESS;
  LHCb::Vertex CandidateVertex(Gaudi::XYZPoint(0.,0.,0.)) ;
  LHCb::Particle Candidate(motherPid);

  // LF
  if(m_motherToNGammas){
    sc = vertexFitter("ParticleAdder")->fit(Daughters,Candidate,CandidateVertex); // no fit
 } else{
    sc = vertexFitter()->fit(Daughters,Candidate,CandidateVertex);
    if (!sc){
      Warning("Failed to fit vertex",StatusCode::SUCCESS,1);
      return 0;
    }
    debug() << "Fit vertex at " << CandidateVertex.position()
            << " with chi^2 " << CandidateVertex.chi2() << endmsg;
    // may add a chi^2 cut here
  } 
  
  verbose() << "Calling desktop()->save(const LHCb::Particle*)" << endmsg ;
  return desktop()->save(&Candidate);

//   if (!Mother){
//     err() << "Cannot create particle with pid " << motherPid.pid() << endmsg ;
//     return StatusCode::FAILURE;
//   }
  
//   debug() << "Made Particle " << Mother->particleID().pid() << " with momentum "  
//           << Mother->momentum() << " m=" << Mother->measuredMass() << endmsg ;
//   return StatusCode::SUCCESS;
};
//#############################################################################
// Plotting
//#############################################################################
StatusCode MakeResonances::makePlots(const LHCb::Particle::ConstVector& PV,IPlotTool* PT) {

  if (!produceHistos()) return StatusCode::SUCCESS;
  if (!PT) return StatusCode::SUCCESS;
  debug() << "Plotting " << endmsg;

  return PT->fillPlots(PV) ;
}
//#############################################################################
// Finalization
//#############################################################################
StatusCode MakeResonances::finalize() {

  debug() << "==> Finalize" << endmsg;
  info() << "Found " << m_nCandidates << " candidates in " << m_nAccepted << " accepted events among " 
         << m_nEvents << " events" << endmsg ;

  return DVAlgorithm::finalize() ;
}
//#############################################################################
// Decay class
//#############################################################################
StatusCode MakeResonances::Decay::initialize(const int& pid, 
                                             const std::vector<int>& pids,
                                             const double& m1, 
                                             const double& m2, 
                                             const double& mp, 
                                             const double& mpt, 
                                             ICheckOverlap* co,
                                             IParticleDescendants* pd){ 
  m_motherPid = LHCb::ParticleID(pid) ;

  for ( std::vector<int>::const_iterator p = pids.begin() ; p != pids.end() ; ++p){
    // check for identical PIDs
    PidParticles PP(*p);
    for ( std::vector<PidParticles>::const_iterator p2 = m_pidParticles.begin() ; 
          p2 != m_pidParticles.end() ; ++p2){
      if ( (*p)==(*p2).getPid() ){
#ifdef PKDEBUG
        std::cout << "   initialize Found identical PIDs " << (*p) << " and " << (*p2).getPid() 
                  << " -> check for ordering" << std::endl ;
#endif       
        m_checkOrder = true ;       
        PP.setSamePid(true);
      }
    }
    m_pidParticles.push_back(PP);
  }  
  m_minMass = m1;
  m_maxMass = m2;
  m_minP    = mp;
  m_minPt   = mpt;
  m_checkP = (( mpt>0.) || (mp>0.));
  m_checkOverlap = co ;
  m_particleDescendants = pd;
  
  return StatusCode::SUCCESS ;
}
//=============================================================================
// Fill PidParticles at each event
//=============================================================================
bool MakeResonances::Decay::fillPidParticles(const LHCb::Particle::ConstVector& PV){

  for ( std::vector<PidParticles>::iterator pp=m_pidParticles.begin() ;
        pp != m_pidParticles.end() ; ++pp ){    
    if (!pp->fillParticles(PV)){ // fill from vector
#ifdef PKDEBUG
      std::cout << "   fillPidParticles: No particles with ID " << pp->getPid() << " found" << std::endl ;
#endif
      return false ;
    }
  }
  return true ;
}

//=============================================================================
// Get first set of daughters candidates 
//=============================================================================
bool MakeResonances::Decay::getFirstCandidates(LHCb::Particle::ConstVector& DaughterVector){
  return getCandidates(DaughterVector);
}
//=============================================================================
// Get next set of daughters candidates 
//=============================================================================
bool MakeResonances::Decay::getNextCandidates(LHCb::Particle::ConstVector& DaughterVector){
  if (!iterate()) return false; // first iterate to next iterators
  return getCandidates(DaughterVector);
}
//=============================================================================
// Get set of daughters candidates 
//=============================================================================
bool MakeResonances::Decay::getCandidates(LHCb::Particle::ConstVector& DaughterVector){
  bool refusedcombination = true ;
  while ( refusedcombination ){    
    DaughterVector.clear();
    refusedcombination = false ;
    for ( std::vector<PidParticles>::const_iterator pp = m_pidParticles.begin() ;
          pp != m_pidParticles.end() ; ++pp ){
      const LHCb::Particle* P = pp->getCandidate() ;
      if (!P) { 
        std::cout << "MakeResonances::Decay::getNextCandidates : This should never happen (1)" << std::endl;
        return false ;
      } else {
        if ( m_checkOrder && pp->samePid() ) {  
          if (*(DaughterVector.rbegin())) {
            if ( (*(DaughterVector.rbegin()))->key() >= P->key()){ // in case of same PID I want one key to be greater
#ifdef PKDEBUG
              std::cout << "   getCandidates skipping combination with keys " 
                        << (*(DaughterVector.rbegin()))->key() << " and " <<  P->key() << std::endl ;
#endif
              refusedcombination = true ;
              break ;
            }
          } else {
            std::cout << "MakeResonances::Decay::getNextCandidates : This should never happen (2)" << std::endl;
            return false ;
          } 
        }
#ifdef PKDEBUG
        std::cout << "MakeResonances::Decay::getCandidates : adding particle\n" 
                  << *P << std::endl;
#endif 
        DaughterVector.push_back(P);
      } // there is a particle
    } // pp loop
    
    if ( (!refusedcombination) && ( m_checkOverlap )) {
#ifdef PKDEBUG
      std::cout << "calling foundOverlap with "
                << DaughterVector.size() << " particles" << std::endl;
        for ( LHCb::Particle::ConstVector::const_iterator c = DaughterVector.begin() ; 
              c !=  DaughterVector.end() ; ++c ){
          std::cout << (*c)->particleID().pid() << " " << (*c)->momentum()  << std::endl;
        }
        
#endif
      if ( foundOverlap(DaughterVector) ){
#ifdef PKDEBUG
        std::cout << "   getCandidates Found Overlap -> iterating " << std::endl ;
#endif
        refusedcombination = true ;
      }
    }
    if (refusedcombination){
      if (!iterate()) return false; // iterate if possible
    }
    
  } // while
#ifdef PKDEBUG
  std::cout << "   getCandidates returns vector : "  ;
    for ( LHCb::Particle::ConstVector::const_iterator p = DaughterVector.begin() ;
          p != DaughterVector.end() ; ++p ){ std::cout << (*p)->key() << " " ; }
    std::cout << std::endl ;
#endif
  return true; // it's OK
}
//=============================================================================
// Found Overlap in vector?
//=============================================================================
bool MakeResonances::Decay::foundOverlap(const LHCb::Particle::ConstVector& DaughterVector){

  LHCb::Particle::ConstVector Children = DaughterVector ;


#ifdef PKDEBUG2
  std::cout << "   foundOverlap Starting replaceResonanceByDaughters " 
            << Children.size() 
            << std::endl ;
#endif

  replaceResonanceByDaughters( Children );

#ifdef PKDEBUG
      std::cout << "Decay::foundOverlap with "
                << Children.size() << " particles before CheckOverlap" << std::endl;
        for ( LHCb::Particle::ConstVector::const_iterator c = Children.begin() ; 
              c !=  Children.end() ; ++c ){
          std::cout << (*c)->particleID().pid() << " " << (*c)->momentum()  << std::endl;
        }
        
#endif

  bool found = m_checkOverlap->foundOverlap( Children );

#ifdef PKDEBUG
      std::cout << "Decay::foundOverlap with "
                << Children.size() << " particles after CheckOverlap" 
                << std::endl;
        for ( LHCb::Particle::ConstVector::const_iterator c = Children.begin() ; 
              c !=  Children.end() ; ++c ){
          std::cout << (*c)->particleID().pid() << " " << (*c)->momentum()  
                    << std::endl;
        }
        
#endif

  return found ;
}
//=============================================================================
// Replace resonance by daughters in vector
//=============================================================================
void MakeResonances::Decay::replaceResonanceByDaughters(LHCb::Particle::ConstVector& Children){

  LHCb::Particle::ConstVector stableParticles;
  
  for ( LHCb::Particle::ConstVector::iterator c = Children.begin() ; 
        c !=  Children.end() ; ++c ){
#ifdef PKDEBUG
      std::cout << " replaceResonanceByDaughters testing particle"
                << (*c)->key() << " " 
                << (*c)->particleID().pid() << " "
                <<  (*c)->momentum() << std::endl ;
#endif
    if ( (*c)->isBasicParticle() ) {
#ifdef PKDEBUG
      std::cout << "replaceResonanceByDaughters keeping stable particle! "
                << std::endl ;
#endif
      stableParticles.push_back(*c);
    } else {
      const LHCb::Particle::ConstVector finalStates = 
        m_particleDescendants->finalStates(*c);
      if ( finalStates.empty() ) {
        stableParticles.push_back(*c);
      } else {
        for (LHCb::Particle::ConstVector::const_iterator f = finalStates.begin();
             f != finalStates.end(); ++f) {
#ifdef PKDEBUG
          std::cout << "   replaceResonanceByDaughters Pushing back " 
                    << (*f)->key() << " " 
                    << (*f)->particleID().pid() << " "
                    <<  (*f)->momentum() << std::endl ;
#endif 
          stableParticles.push_back(*f);
        }
      }
    }
  }

  Children.clear();
  Children = stableParticles;
  
}

//===========================================================================  
  
    


// #ifdef PKDEBUG
//     std::cout << "   replaceResonanceByDaughters Erasing     " 
//               << (*c)->particleID().pid() << " "
//               <<  (*c)->momentum() 
//               <<  " addr " << *c 
//               << " key " << (*c)->key() << std::endl ;
// #endif
//     Children.erase(c);
// #ifdef PKDEBUG
//     std::cout << "Will push back " << daughters.size() << " daughters"<< std::endl;
// #endif
//     for ( LHCb::Particle::ConstVector::const_iterator d = daughters.begin() ; 
//           d !=  daughters.end() ; ++d ) {
// #ifdef PKDEBUG
//       std::cout << "replaceResonanceByDaughters pushing back" << *(*d) << std::endl;
      
// #endif
//       Children.push_back(*d);
// #ifdef PKDEBUG
//       std::cout << "   replaceResonanceByDaughters Pushed back " 
//                 << (*d)->key() << " " 
//                 << (*d)->particleID().pid() << " "
//                 <<  (*d)->momentum() << std::endl ;
// #endif 
//       return true ;
//     }
//   }
//   return false;
// }
//=============================================================================
// iterate to next combination
//=============================================================================
bool MakeResonances::Decay::iterate(){
  // now iterate to the next combination
  bool didbreak = false ;
  for ( std::vector<PidParticles>::reverse_iterator pp = m_pidParticles.rbegin() ;
        pp != m_pidParticles.rend() ; ++pp ){ // reverse iterator to first increment vector n 
    if (pp->iterate()) {
      didbreak = true ;
      break ; // if I can go to the next in this vector I can exit the loop
    }
#ifdef PKDEBUG
    std::cout << "   iterate could not iterate for pid " << pp->getPid() << std::endl ;     
#endif
  }
#ifdef PKDEBUG
  std::cout << "   iterate returns " << didbreak << std::endl ;
#endif
  return didbreak ; // If I did not break, then I'm at the end of the loop
}
//#############################################################################
// PidParticles class
//#############################################################################
bool MakeResonances::Decay::PidParticles::fillParticles(const LHCb::Particle::ConstVector& PV){
  
  m_particles.clear();
  for ( LHCb::Particle::ConstVector::const_iterator p=PV.begin(); p!=PV.end(); ++p){
    if ( m_pid==(*p)->particleID().pid() ) m_particles.push_back(*p) ;
  }
#ifdef PKDEBUG
  std::cout << "PidParticles::fillParticles Found " <<  m_particles.size() << " particles with pid "
            << m_pid << " : " ;
  for ( LHCb::Particle::ConstVector::const_iterator p = m_particles.begin(); p!=m_particles.end();++p) {
    std::cout << (*p)->key() << " " ;
  }
  std::cout << std::endl ;
#endif
  m_iterator = m_particles.begin();
  return ( !m_particles.empty() );
  
}
//=============================================================================
// go to next
//=============================================================================
bool MakeResonances::Decay::PidParticles::iterate(){
  ++m_iterator ;
  if ( m_iterator==m_particles.end() ) {
#ifdef PKDEBUG
    std::cout << "   PidParticles::iterate Reached end of loop for pid " << m_pid << std::endl ;
#endif
    m_iterator=m_particles.begin() ;
    return false ;
  } else {
#ifdef PKDEBUG
    std::cout << "   PidParticles::iterate Iterating for pid " << m_pid << std::endl ;    
#endif
    return true ; 
  }
  
}
