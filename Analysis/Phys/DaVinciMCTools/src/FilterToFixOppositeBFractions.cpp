// $Id: FilterToFixOppositeBFractions.cpp,v 1.6 2007-03-01 12:57:55 sposs Exp $
// Include files
#include <sstream>
#include "GaudiKernel/MsgStream.h"


// from Gaudi
#include "GaudiKernel/DeclareFactoryEntries.h"

// local
#include "FilterToFixOppositeBFractions.h"

//-----------------------------------------------------------------------------
// Implementation file for class : FilterToFixOppositeBFractions
//
// This Filter is created to fix the Bug2 observed on DC06 data
// and reported by Patrick Robbe on the 13 feb 2007.
// This bug affects the B composition of the opposite side with respect to the
// signal B. This filter will remove specific fraction of events where 
// abs(BsigID) equal abs(BoppoID) depending on events type.
// It checks for signal B, using HepMC, looks for opposite B using the MC
// originVertex() method. The case with more than 2 B's coming from the same
// vertex is not handled properly for the time being.
// To use it, add in your job option:
// ApplicationMgr.DLLs += { "DaVinciMCTools" };
// and create a sequence with all the algorithms that have to be processed 
// each events. The FilterToFixOppositeBFractions has to be the first in the
// list. Example :
// ApplicationMgr.DLLs += { "DaVinciMCTools" };
// ApplicationMgr.TopAlg += { "GaudiSequencer/MySeq" };
// MySeq.Members += { "FilterToFixOppositeBFractions" };
// FilterToFixOppositeBFractions.ActivateCorrection = true;
//
// 2007-02-22 : Stephane Poss
//-----------------------------------------------------------------------------
using namespace LHCb ;
// Declaration of the Algorithm Factory
DECLARE_ALGORITHM_FACTORY( FilterToFixOppositeBFractions );


//=============================================================================
// Standard constructor, initializes variables
//=============================================================================
FilterToFixOppositeBFractions::FilterToFixOppositeBFractions( const std::string& name,
                                                              ISvcLocator* pSvcLocator)
  : GaudiAlgorithm ( name , pSvcLocator )
{
  declareProperty("ActivateCorrection",   m_activate=false);
}
//=============================================================================
// Destructor
//=============================================================================
FilterToFixOppositeBFractions::~FilterToFixOppositeBFractions() {}

//=============================================================================
// Initialization
//=============================================================================
StatusCode FilterToFixOppositeBFractions::initialize() {
  //=== The following two lines should be commented for DC04 algorithms ! ===
  StatusCode sc = GaudiAlgorithm::initialize();
  if ( sc.isFailure() ) return sc;

  if(!m_activate) return StatusCode::SUCCESS;

  info() << "This filter Will correct opposite B composition"<< endmsg;
  m_coutevt=m_coutevtAC=0.0;
  m_coutSameB=m_coutBd=m_coutBs=m_coutBu=m_coutOt=m_rejected=0;
  m_coutBdAC=m_coutBsAC=m_coutBuAC=m_coutOtAC=0;
  m_print=false;
  m_printevt=false;

  //initialize table
  //      Event type                               gx    fx 
  m_evtGxFx[11144001] = std::pair<double,double>(0.4998,0.406);// Bd_JpsiKst,mm=DecProdCut
  m_evtGxFx[11874001] = std::pair<double,double>(0.4954,0.406);// Bd_Dstmunu,Kpi=cocktail,D0muInAcc
  m_evtGxFx[12143001] = std::pair<double,double>(0.5026,0.406);// Bu_JpsiK,mm=DecProdCut
  m_evtGxFx[12463011] = std::pair<double,double>(0.4879,0.406);// Bu_D0h,Kpi=cocktail,D0InAcc
  m_evtGxFx[12873001] = std::pair<double,double>(0.4990,0.406);// Bu_D0munu,Kpi=cocktail,D0muInAcc
  m_evtGxFx[13264001] = std::pair<double,double>(0.1372,0.099);// Bs_Dspi=DecProdCut
  m_evtGxFx[13774001] = std::pair<double,double>(0.1404,0.099);// Bs_Dsmunu=cocktail,DsmuInAcc
  m_evtGxFx[11102003] = std::pair<double,double>(0.5654,0.406);// Bd_K+pi-=CPV,DecProdCut
  m_evtGxFx[11102013] = std::pair<double,double>(0.5683,0.406);// Bd_pi+pi-=CPV,DecProdCut
  m_evtGxFx[11144103] = std::pair<double,double>(0.7044,0.406);// Bd_JpsiKS,mm=CPV,DecProdCut
  

  return StatusCode::SUCCESS;
}

//=============================================================================
// Main execution
//=============================================================================
StatusCode FilterToFixOppositeBFractions::execute() {

  setFilterPassed( false );

  //execute code only if filter is activated
  if(!m_activate){
    setFilterPassed( true );
    return StatusCode::SUCCESS;
  }
  
  m_coutevt = m_coutevt+1.0;

  GenHeader* gene = get<GenHeader> (GenHeaderLocation::Default);

  ///Look at Gauss version used to produce evt.
  if(gene->applicationVersion()=="v25r7"){
    Warning("Gauss version v25r7, correction activated",
            StatusCode::SUCCESS,1);
    /// Look at event type :
    /// proceed only if evt type corresponds to buggy type
    m_ievt=gene->evType();
    std::map<int, std::pair<double,double> >::iterator iET = m_evtGxFx.find(m_ievt);
    
    if(iET != m_evtGxFx.end()){
      std::ostringstream t;
      t<<m_ievt;      
      Warning("Event type " + t.str() + 
              " corresponds to buggy type : Correction activated !"
              ,StatusCode::SUCCESS,1);

      //retrieve HepMC event
      LHCb::HepMCEvents* hepVect = get<LHCb::HepMCEvents>(HepMCEventLocation::Default );

      //Retrieve Rec header to have Event and Run numbers
      RecHeader* evt = get<RecHeader> (RecHeaderLocation::Default);

      //Retrieve MC parts
      MCParticles* mcpart = get<MCParticles>(MCParticleLocation::Default);

      int evtn = evt->evtNumber();
      int runn = evt->runNumber();
      debug()<<"Event ="<<evtn<<"    Run="<<runn<<endreq;

      //Look for B forced to decay
      const MCParticle *BS = 0, *BO = 0;
      for( std::vector<LHCb::HepMCEvent*>::iterator q=hepVect->begin();
           q!=hepVect->end(); ++q ) {
        for ( HepMC::GenEvent::particle_iterator p  =
                (*q)->pGenEvt()->particles_begin();
              p != (*q)->pGenEvt()->particles_end();
              ++p ) {
          if( (*p)->status() != 889 ) continue;
          BS = associatedofHEP(*p);
          if(BS) break;
        }
      }
      debug()<<"Signal B ID = "<<BS->particleID().pid()<<endreq;
      //retrieve opposite B using the same origin vertex
      MCParticles::const_iterator imc;
      for ( imc = mcpart->begin(); imc != mcpart->end(); imc++ ) {
        if( (*imc) != BS ) {
          if((*imc)->particleID().hasBottom()) {
            if((*imc)->originVertex()){
              if((*imc)->originVertex()==BS->originVertex()) {
                BO = (*imc);
              }
            }
            else{
              info()<<"Origin vertex of a B inexistant"<<endreq;
            }
          }
        }
      }
      if(!BO){
        info()<<"Opposite B not found !!"<<endreq;
        setFilterPassed(false);
        return StatusCode::SUCCESS;
      }
      debug()<<"B Opposite ID = "<<BO->particleID().pid()<<endreq;
      if(BO->particleID().abspid()==531) m_coutBs++;
      if(BO->particleID().abspid()==521) m_coutBu++;
      if(BO->particleID().abspid()==511) m_coutBd++;
      if(BO->particleID().abspid()!=531 &&
         BO->particleID().abspid()!=521 &&
         BO->particleID().abspid()!=511) m_coutOt++;
      //remove event acording to a certain prob if BS and BO are identical.
      if(BS->particleID().abspid()==BO->particleID().abspid()){
        m_coutSameB++;
        int a = evtn%50;
        double gx = iET->second.first;
        double fx = iET->second.second;
        if(a < 50.*(1-((1/gx)-1)/((1/fx)-1))){
          m_rejected++;
          debug()<<"Event rejected !!"<<endreq;
          setFilterPassed(false);
          return StatusCode::SUCCESS;
        }
      }
      debug()<<"Event accepted !!"<<endreq;
      m_coutevtAC = m_coutevtAC+1.0;
      if(BO->particleID().abspid()==531) m_coutBsAC++;
      if(BO->particleID().abspid()==521) m_coutBuAC++;
      if(BO->particleID().abspid()==511) m_coutBdAC++;
      if(BO->particleID().abspid()!=531 &&
         BO->particleID().abspid()!=521 &&
         BO->particleID().abspid()!=511) m_coutOtAC++;
    }
  }

  setFilterPassed(true);   // Mandatory. Set to true if event is accepted.
  return StatusCode::SUCCESS;
}

//=============================================================================
//  Finalize
//=============================================================================
StatusCode FilterToFixOppositeBFractions::finalize() {

  if(!m_activate) return GaudiAlgorithm::finalize();

  m_BdBC=percent(m_coutBd,m_coutevt);
  m_BsBC=percent(m_coutBs,m_coutevt);
  m_BuBC=percent(m_coutBu,m_coutevt);
  m_OtBC=percent(m_coutOt,m_coutevt);

  m_BdAC=percent(m_coutBdAC,m_coutevtAC);
  m_BsAC=percent(m_coutBsAC,m_coutevtAC);
  m_BuAC=percent(m_coutBuAC,m_coutevtAC);
  m_OtAC=percent(m_coutOtAC,m_coutevtAC);

  m_errBdBC=errorp(m_coutBd,m_coutevt);
  m_errBsBC=errorp(m_coutBs,m_coutevt);
  m_errBuBC=errorp(m_coutBu,m_coutevt);
  m_errOtBC=errorp(m_coutOt,m_coutevt);

  m_errBdAC=errorp(m_coutBdAC,m_coutevtAC);
  m_errBsAC=errorp(m_coutBsAC,m_coutevtAC);
  m_errBuAC=errorp(m_coutBuAC,m_coutevtAC);
  m_errOtAC=errorp(m_coutOtAC,m_coutevtAC);


  info() << "==> Finalize" << endmsg;

  info() << "   Event Type was :"<<m_ievt<<endreq;
  info() << "   Total evts Before any corrections= " <<m_coutevt<<endreq;
  info() << " Fraction where opposite B ID = Signal B ID : "
         <<r(percent(m_coutSameB,m_coutevt))<<" +/- "
         <<r(errorp(m_coutSameB,m_coutevt))<<" %"
         <<endreq;
  info() << "        Total Bu oppo = "
         <<m_coutBu<<"/"<<m_coutevt<<" = "
         <<r(m_BuBC)<<" +/- "
         <<r(m_errBuBC)<<" %"<<endreq;
  info() << "        Total Bd oppo = "
         <<m_coutBd<<"/"<<m_coutevt<<" = "
         <<r(m_BdBC)<<" +/- "
         <<r(m_errBdBC)<<" %"<<endreq;
  info() << "        Total Bs oppo = "
         <<m_coutBs<<"/"<<m_coutevt<<" = "
         <<r(m_BsBC)<<" +/- "
         <<r(m_errBsBC)<<" %"<<endreq;
  info() << "        Total Ot oppo = "
         <<m_coutOt<<"/"<<m_coutevt<<" = "
         <<r(m_OtBC)<<" +/- "
         <<r(m_errOtBC)<<" %"<<endreq;
  info() << endreq;
  info() << "        Total rejected = "
         <<r(percent(m_rejected,m_coutevt))<<" +/- "
         <<r(errorp(m_rejected,m_coutevt))<<" %"<<endreq;
  info() << "   Total evts After Correction = "
         <<m_coutevtAC<<endreq;
  info() << "        Total Bu oppo AC= "
         <<m_coutBuAC<<"/"<<m_coutevtAC<<" = "
         <<r(m_BuAC)<<" +/- "
         <<r(m_errBuAC)<<" %"<<endreq;
  info() << "        Total Bd oppo AC= "
         <<m_coutBdAC<<"/"<<m_coutevtAC<<" = "
         <<r(m_BdAC)<<" +/- "
         <<r(m_errBdAC)<<" %"<<endreq;
  info() << "        Total Bs oppo AC= "
         <<m_coutBsAC<<"/"<<m_coutevtAC<<" = "
         <<r(m_BsAC)<<" +/- "
         <<r(m_errBsAC)<<" %"<<endreq;
  info() << "        Total Ot oppo AC= "
         <<m_coutOtAC<<"/"<<m_coutevtAC<<" = "
         <<r(m_OtAC)<<" +/- "
         <<r(m_errOtAC)<<" %"<<endreq;


  return GaudiAlgorithm::finalize(); //=== For DC04, return StatusCode::SUCCESS;
}

//=============================================================================
MCParticle* FilterToFixOppositeBFractions::associatedofHEP(HepMC::GenParticle* hepmcp) {

  MCParticles* mcpart = get<MCParticles>(MCParticleLocation::Default);
  int mid = hepmcp->pdg_id();
  double mothmom = hepmcp->momentum().vect().mag();
  double moththeta = hepmcp->momentum().vect().theta();
  MCParticles::const_iterator imc;
  for ( imc = mcpart->begin(); imc != mcpart->end(); ++imc ) {
    if( mid == (*imc)->particleID().pid() ) {
      if( fabs(mothmom - (*imc)->momentum().P())< 0.1){
        if( fabs(moththeta -(*imc)->momentum().Theta())< 0.0001){
          return (*imc);
        }
      }
    }
  }
  return 0;
}
//=============================================================================
double FilterToFixOppositeBFractions::percent(int a, double b)
{
  return 100.*a/b;
}
//=============================================================================
double FilterToFixOppositeBFractions::errorp(int a, double b)
{
  double err = 100.*sqrt((a/b)*(1-(a/b))/b);
  return err;
}
//=============================================================================
double FilterToFixOppositeBFractions::r(double x)
{
  return round(x * 100.)/100.;
}
