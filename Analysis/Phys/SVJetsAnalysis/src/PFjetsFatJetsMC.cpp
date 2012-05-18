//Created by Murilo Rangel

// ============================================================================
// LoKi
// ============================================================================
#include "LoKi/LoKi.h"
#include "LoKi/LoKiAlgoMC.h"
#include "LoKi/Algo.h"
#include "LoKi/CoreCuts.h"
#include "LoKi/GenTypes.h"
#include "LoKi/GenTypes.h"
#include "LoKi/GenParticleCuts.h"
#include "LoKi/Vertices.h"
#include "LoKi/VertexCast.h"
#include "LoKi/Particles.h"
#include "LoKi/ParticleCuts.h"
#include "LoKi/VertexCuts.h"
#include "LoKi/ParticleContextCuts.h"
#include "Kernel/IParticle2MCAssociator.h"
#include "Kernel/DVAlgorithm.h"
#include <iostream>
#include "stdio.h"
#include "stdlib.h"

#include "JetUtils.h"

#include <Kernel/IJetMaker.h>

using namespace LHCb;
using namespace LoKi;
using namespace LoKi::GenTypes;
using namespace LoKi::Types;
using namespace LoKi::Cuts;
using namespace LoKi::Particles;
using namespace LoKi::Vertices;

LOKI_MCALGORITHM(PFJetsFatJetsMC)
{

	const IJetMaker* jetMaker = tool<IJetMaker> ( "LoKi::FastJetMaker");//FastJetMaker",this ) ;                                                                          
	IJetMaker::Jets   mcjets,hepgenjets,vbpvhepgenjets,mcjetsinLHCb; 

  Tuple tuple_stdpfjets = nTuple("StdPFJets","StdPFJets");
  //Tuple tuple_mcpfjets = nTuple("MCPFJets","MCPFJets");

  const double MaxRdistance = 0.6;
  unsigned int jetcounter = 0;

  LoKi::GenTypes::GRange GFinal		= gselect( "GFinal" , (1 == GSTATUS || 999 == GSTATUS) && GABSID != 13);
  LoKi::GenTypes::GRange GMuons		= gselect( "GMuons" , (1 == GSTATUS || 999 == GSTATUS) && GABSID == 13);

  LoKi::GenTypes::GRange GPartons	= gselect( "GPartons" , (4 == GABSID || 2 == GABSID || 1 == GABSID || 3 == GABSID || 21 == GABSID || 5 == GABSID || 6 == GABSID ) && GPT > 2000); 

  //LoKi::GenTypes::GRange GVBON		= gselect( "GVBON" , GABSID == 1000022 || 24 == GABSID || 23 == GABSID );
  LoKi::GenTypes::GRange GVB		= gselect( "GVB" , GABSID == 25);//1000022 || 24 == GABSID || 23 == GABSID );

  MCRange mcparticles = mcselect("mcparticles", MCNENDVERTICES == 0);

  LHCb::Particle::Range rstdjets	= get<LHCb::Particle::Range>("Phys/StdJets/Particles");
  LHCb::Particle::Range fatjets	= get<LHCb::Particle::Range>("Phys/FatJets/Particles");
  LHCb::Particle::Range stdloosemuons	= get<LHCb::Particle::Range>("Phys/StdLooseMuons/Particles");

  if(fatjets.size() < 1) return StatusCode::SUCCESS ;

  LHCb::RecVertex::Range pvs = primaryVertices();
  tuple_stdpfjets->column("NRecPVs",pvs.size());

  tuple_stdpfjets->farray("stdloosemuon_px",PX,
				"stdloosemuon_py",PY,
				"stdloosemuon_pz",PZ,
				"stdloosemuon_e",E,stdloosemuons.begin(),stdloosemuons.end(),"NMaxjets",10000);

  tuple_stdpfjets->farray("fatjet_px",PX,
				"fatjet_py",PY,
				"fatjet_pz",PZ,
				"fatjet_e",E,fatjets.begin(),fatjets.end(),"NMaxjets",10000);

  //tuple_stdpfjets->farray("stdjet_pt",PT,rstdjets.begin(),rstdjets.end(),"NMaxjets",10000);
  //tuple_stdpfjets->farray("stdjet_p",P,rstdjets.begin(),rstdjets.end(),"NMaxjets",10000);
  tuple_stdpfjets->farray("stdjet_px",PX,
				"stdjet_py",PY,
				"stdjet_pz",PZ,
				"stdjet_e",E,rstdjets.begin(),rstdjets.end(),"NMaxjets",10000);
  tuple_stdpfjets->farray("stdjet_pdgid",ID,rstdjets.begin(),rstdjets.end(),"NMaxjets",10000);
  //tuple_stdpfjets->farray("stdjet_eta",ETA,rstdjets.begin(),rstdjets.end(),"NMaxjets",10000);
  //tuple_stdpfjets->farray("stdjet_phi",PHI,rstdjets.begin(),rstdjets.end(),"NMaxjets",10000);
  tuple_stdpfjets->farray("stdjet_ndaughters",NDAUGHTERS,rstdjets.begin(),rstdjets.end(),"NMaxjets",10000);

  tuple_stdpfjets->column("NumberOfstdjets",rstdjets.size());
   
  std::vector<const LHCb::Particle *> jetdaughters;
  std::vector<unsigned int> jetcounterdaughters;

  std::vector<int> Ngendaughters;
  std::vector<float> trackchi2ofdaughters;
  std::vector<const HepMC::GenParticle *> jetgendaughters;
  std::vector<const LHCb::Particle *> genjets;
  std::vector<unsigned int> jetcountergenjetsdaughters;

  std::vector<const HepMC::GenParticle *> jetqgmothers;
  std::vector<unsigned int> jetcounterjetqgmothers;
  std::vector<int> numberofqgmothers;
  std::vector<float> stdjet_bestpvz;
  //std::vector<const LHCb::Particle *> genjets;

  jetcounter = 0;
  DVAlgorithm* dvAlg = 0;
// The tool service doesn't understand typedefs, so we have to pass this long string
const std::string finderType = "GenericParticle2PVRelator__p2PVWithIPChi2_OfflineDistanceCalculatorName_";

// get it from the tool service.
const IRelatedPVFinder* pFinder = tool<IRelatedPVFinder>(finderType,this);

// get a particle
// get some primary vertices
  LHCb::RecVertex::Range m_vertices;
//  const LHCb::RecVertex::Container* pVertices = ....;
    m_vertices = get<LHCb::RecVertex::Range>(LHCb::RecVertexLocation::Primary);
// get the best PV for the particle. See the IRelatedPVFinder doxygen for full interface.
  IRelatedPVFinder* m_pvRelator ;
  m_pvRelator = tool<IRelatedPVFinder>("GenericParticle2PVRelator__p2PVWithIPChi2_OfflineDistanceCalculatorName_/P2PVWithIPChi2", this);

  std::vector<int> number_clusters;
  std::vector<float> clusters_e;

  for (LHCb::Particle::Range::iterator ijet = rstdjets.begin() ; ijet != rstdjets.end() ; ++ijet)
  {
	  const LHCb::Particle * jet = * ijet;
//	  const LHCb::VertexBase *vtxjet = dvAlg->bestPV(jet);
/*
	  const Particle2Vertex::LightWTable bestPVTable = 
	  m_pvRelator->relatedPVs(jet, 
                            LHCb::VertexBase::ConstVector(m_vertices.begin(), 
                                                          m_vertices.end()));
  const Particle2Vertex::LightWTable::Range range = bestPVTable.relations();
*/
	  const LHCb::VertexBase *bestPV = m_pvRelator->relatedPV(jet,m_vertices);
	  if(bestPV!=NULL)
	  stdjet_bestpvz.push_back(bestPV->position().Z());
          else stdjet_bestpvz.push_back(-9999.);
//	  if(bestPV!=NULL)
//	  info() << "jet " << jetcounter << " " <<  bestPV->position().Z() << endreq;

	  std::vector<const LHCb::Particle *> daughtersvector = jet->daughtersVector();
	  jetcounter++;

	   int numberofgendaughters = 0;
	   LHCb::Particle * GenJet = new LHCb::Particle() ;//98 = jetid
	   LorentzVector * genjet4vector = new LorentzVector(0,0,0,0);


	   int nqgmothers = 0;

	  for (std::vector<const LHCb::Particle *>::iterator idaughter = daughtersvector.begin() ; idaughter != daughtersvector.end() ; ++idaughter){

		  const LHCb::Particle * daughter = *idaughter;

		  jetdaughters.push_back(daughter);
		  jetcounterdaughters.push_back(jetcounter);

		  if (daughter == NULL)continue;// ?????

		  if (daughter->proto() != NULL){
			  if (daughter->proto()->track() != NULL){
			  //info() << daughter->proto() << endmsg; 
			  //info() << daughter->proto()->track() << endmsg; 
			  //info() << daughter->proto()->track()->chi2() << endmsg; 
			  trackchi2ofdaughters.push_back(daughter->proto()->track()->chi2());
			  }
			  else trackchi2ofdaughters.push_back(-10000);
		  }
		  else trackchi2ofdaughters.push_back(-10000);

		  //else trackchi2ofdaughters.push_back(-10000);


		  if(daughter->proto() == NULL) {number_clusters.push_back(0);continue;}
		  if(daughter->proto()->calo().size() < 1) {number_clusters.push_back(0);continue;}


		  const SmartRefVector < LHCb::CaloHypo > daughtercalo = daughter->proto()->calo();

		  //for (SmartRefVector<LHCb::CaloHypo>::const_iterator icalodaughter = daughter->proto()->calo().begin() ; icalodaughter != daughter->proto()->calo().end() ; ++icalodaughter){
		  const LHCb::CaloHypo *calodaughter = * (daughter->proto()->calo().begin());//* icalodaughter;

		  number_clusters.push_back(calodaughter->clusters().size());

		  SmartRefVector<LHCb::CaloCluster>::const_iterator icluster = calodaughter->clusters().begin();

		  for (; icluster != calodaughter->clusters().end() ; ++icluster){
			  const LHCb::CaloCluster *cluster = * icluster;
			  clusters_e.push_back(cluster->e());

		  }
		  //break;// only the first Hypo 
		  //}
	  }

	  for (LoKi::GenTypes::GRange::iterator igen = GFinal.begin() ; igen != GFinal.end() ; ++igen)
	   {

	           const HepMC::GenParticle * gen = * igen;

		   double deltaPhi = std::abs(jet->momentum().phi() - gen->momentum().phi());
		   if(deltaPhi > std::acos(-1)) deltaPhi = 2*std::acos(-1) - deltaPhi;
		   double deltaEta = jet->momentum().eta() - gen->momentum().eta(); 

	           if (std::sqrt( std::pow(deltaPhi,2) + std::pow(deltaEta,2)) < MaxRdistance)
	           {
	        	   jetgendaughters.push_back(gen);
			   jetcountergenjetsdaughters.push_back(jetcounter);
	        	   numberofgendaughters++;
			   *genjet4vector += (LorentzVector(gen->momentum().px(),gen->momentum().py(),gen->momentum().pz(),gen->momentum().e()));
	           }


	   }
	   for (LoKi::GenTypes::GRange::iterator iquark = GPartons.begin() ; iquark != GPartons.end() ; ++iquark)
	   {

	           const HepMC::GenParticle * quark = * iquark;

		   double deltaPhi = std::abs(jet->momentum().phi() - quark->momentum().phi());
		   if(deltaPhi > std::acos(-1)) deltaPhi = 2*std::acos(-1) - deltaPhi;
		   double deltaEta = jet->momentum().eta() - quark->momentum().eta(); 

	           if (std::sqrt( std::pow(deltaPhi,2) + std::pow(deltaEta,2)) < MaxRdistance)
	           {
			   jetcounterjetqgmothers.push_back(jetcounter);
			   nqgmothers++;
	        	   jetqgmothers.push_back(quark);
	           }


	   }

	   numberofqgmothers.push_back(nqgmothers);

	   GenJet->setMomentum(*genjet4vector);
	   Ngendaughters.push_back(numberofgendaughters);
	   genjets.push_back(GenJet);
  }
  

  //tuple_stdpfjets->farray("jetdaughters_pt",PT,jetdaughters.begin(),jetdaughters.end(),"NMaxdaughters",10000);
  //tuple_stdpfjets->farray("jetdaughters_p",P,jetdaughters.begin(),jetdaughters.end(),"NMaxdaughters",10000);
  tuple_stdpfjets->farray("jetdaughters_trackchi2",trackchi2ofdaughters,"NMaxdaughters",10000);
  tuple_stdpfjets->farray("jetdaughters_px",PX,
				"jetdaughters_py",PY,
				"jetdaughters_pz",PZ,
				"jetdaughters_e",E,jetdaughters.begin(),jetdaughters.end(),"NMaxdaughters",10000);
  //tuple_stdpfjets->farray("jetdaughters_jetid",jetcounterdaughters,"NMaxdaughters",10000);
  tuple_stdpfjets->farray("jetdaughters_pdgid",ID,
			  "jetdaughters_info",INFO4JET,jetdaughters.begin(),jetdaughters.end(),"NMaxdaughters",10000);
  tuple_stdpfjets->farray("stdjet_nclusters",number_clusters,"NMaxjets",10000);
  tuple_stdpfjets->farray("stdjet_clusters_e",clusters_e,"NMaxjets",10000);
  //tuple_stdpfjets->farray("jetdaughters_eta",ETA,jetdaughters.begin(),jetdaughters.end(),"NMaxdaughters",10000);
  //tuple_stdpfjets->farray("jetdaughters_phi",PHI,jetdaughters.begin(),jetdaughters.end(),"NMaxdaughters",10000);

  tuple_stdpfjets->farray("stdjet_ngendaughters",Ngendaughters,"NMaxjets",10000);
  tuple_stdpfjets->farray("stdjet_nqgmothers",numberofqgmothers,"NMaxjets",10000);
  tuple_stdpfjets->farray("stdjet_bestpvz",stdjet_bestpvz,"NMaxjets",10000);

  //tuple_stdpfjets->farray("jetgendaughters_jetid",jetcountergenjetsdaughters,"NMaxgendaughters",10000);
  //tuple_stdpfjets->farray("jetgendaughters_pt",GPT,jetgendaughters.begin(),jetgendaughters.end(),"NMaxgendaughters",10000);
  //tuple_stdpfjets->farray("jetgendaughters_p",GP,jetgendaughters.begin(),jetgendaughters.end(),"NMaxgendaughters",10000);
  tuple_stdpfjets->farray("jetgendaughters_px",GPX,
			  "jetgendaughters_py",GPY,
			  "jetgendaughters_pz",GPZ,
			  "jetgendaughters_e",GE,jetgendaughters.begin(),jetgendaughters.end(),"NMaxgendaughters",10000);
  tuple_stdpfjets->farray("jetgendaughters_pdgid",GID,jetgendaughters.begin(),jetgendaughters.end(),"NMaxgendaughters",10000);
  //tuple_stdpfjets->farray("jetgendaughters_eta",GETA,jetgendaughters.begin(),jetgendaughters.end(),"NMaxgendaughters",10000);
  //tuple_stdpfjets->farray("jetgendaughters_phi",GPHI,jetgendaughters.begin(),jetgendaughters.end(),"NMaxgendaughters",10000);

  //tuple_stdpfjets->farray("genjet_p",PT,genjets.begin(),genjets.end(),"NMaxgenjets",10000);
  //tuple_stdpfjets->farray("genjet_pt",P,genjets.begin(),genjets.end(),"NMaxgenjets",10000);
  tuple_stdpfjets->farray("genjet_px",PX,
			  "genjet_py",PY,
			  "genjet_pz",PZ,
			  "genjet_e",E,genjets.begin(),genjets.end(),"NMaxgenjets",10000);
  //tuple_stdpfjets->farray("genjet_eta",ETA,genjets.begin(),genjets.end(),"NMaxgenjets",10000);
  //tuple_stdpfjets->farray("genjet_phi",PHI,genjets.begin(),genjets.end(),"NMaxgenjets",10000);

  //tuple_stdpfjets->farray("jetqgmothers_pt",GPT,jetqgmothers.begin(),jetqgmothers.end(),"NMaxqgmothers",10000);
  //tuple_stdpfjets->farray("jetqgmothers_p",GP,jetqgmothers.begin(),jetqgmothers.end(),"NMaxqgmothers",10000);
  tuple_stdpfjets->farray("jetqgmothers_px",GPX,
			  "jetqgmothers_py",GPY,
			  "jetqgmothers_pz",GPZ,
			  "jetqgmothers_e",GE,jetqgmothers.begin(),jetqgmothers.end(),"NMaxqgmothers",10000);
  //tuple_stdpfjets->farray("jetqgmothers_jetid",jetcounterjetqgmothers,"NMaxqgmothers",10000);
  tuple_stdpfjets->farray("jetqgmothers_pdgid",GID,jetqgmothers.begin(),jetqgmothers.end(),"NMaxqgmothers",10000);
  //tuple_stdpfjets->farray("jetqgmothers_eta",GETA,jetqgmothers.begin(),jetqgmothers.end(),"NMaxqgmothers",10000);
  //tuple_stdpfjets->farray("jetqgmothers_phi",GPHI,jetqgmothers.begin(),jetqgmothers.end(),"NMaxqgmothers",10000);

  //std::vector<const LHCb::Particle *> mcp4jets;
  //for (MCRange::iterator imcp = mcparticles.begin() ; imcp != mcparticles.end() ; ++imcp)
  //  {
  //          const LHCb::MCParticle * mcp = * imcp;
  //          LHCb::Particle * part = new LHCb::Particle();
  //          part->setMomentum(mcp->momentum());
  //          mcp4jets.push_back(part);
  //  }

  //StatusCode sc = jetMaker -> makeJets ( mcp4jets , mcjets ) ;                                                                                    
  //tuple_mcpfjets->farray("mcjet_pt",PT,mcjets.begin(),mcjets.end(),"NMaxmcjets",10000);
  //tuple_mcpfjets->farray("mcjet_p",P,mcjets.begin(),mcjets.end(),"NMaxmcjets",10000);
  //tuple_mcpfjets->farray("mcjet_px",PX,mcjets.begin(),mcjets.end(),"NMaxmcjets",10000);
  //tuple_mcpfjets->farray("mcjet_py",PY,mcjets.begin(),mcjets.end(),"NMaxmcjets",10000);
  //tuple_mcpfjets->farray("mcjet_pz",PZ,mcjets.begin(),mcjets.end(),"NMaxmcjets",10000);
  //tuple_mcpfjets->farray("mcjet_e",E,mcjets.begin(),mcjets.end(),"NMaxmcjets",10000);
  //tuple_mcpfjets->farray("mcjet_pdgid",ID,mcjets.begin(),mcjets.end(),"NMaxmcjets",10000);
  //tuple_mcpfjets->farray("mcjet_eta",ETA,mcjets.begin(),mcjets.end(),"NMaxmcjets",10000);
  //tuple_mcpfjets->farray("mcjet_phi",PHI,mcjets.begin(),mcjets.end(),"NMaxmcjets",10000);


//  std::vector<const LHCb::Particle *> genp4jets;
//  for (LoKi::GenTypes::GRange::iterator igenp = GFinal.begin() ; igenp != GFinal.end() ; ++igenp)
//    {
//            const HepMC::GenParticle * genp = * igenp;
//	    LHCb::Particle * part = new LHCb::Particle();
//	    LorentzVector gen4vectortmp(genp->momentum().px(),genp->momentum().py(),genp->momentum().pz(),genp->momentum().e());
//	    part->setMomentum(gen4vectortmp);
//	    part->setParticleID(LHCb::ParticleID(genp->pdg_id()));
//	    genp4jets.push_back(part);
//    }
//
//  StatusCode sc2 = jetMaker -> makeJets ( genp4jets , hepgenjets ) ;                                                                                    
//  //tuple_stdpfjets->farray("hepgenjet_pt",PT,hepgenjets.begin(),hepgenjets.end(),"NMaxhepgenjets",10000);
//  //tuple_stdpfjets->farray("hepgenjet_p",P,hepgenjets.begin(),hepgenjets.end(),"NMaxhepgenjets",10000);
//  tuple_stdpfjets->farray("hepgenjet_px",PX,
//			  "hepgenjet_py",PY,
//			  "hepgenjet_pz",PZ,
//			  "hepgenjet_e",E,hepgenjets.begin(),hepgenjets.end(),"NMaxhepgenjets",10000);
//  tuple_stdpfjets->farray("hepgenjet_pdgid",ID,
//			  "hepgenjet_ndaughters",NDAUGHTERS,hepgenjets.begin(),hepgenjets.end(),"NMaxhepgenjets",10000);

  //tuple_stdpfjets->farray("hepgenjet_eta",ETA,hepgenjets.begin(),hepgenjets.end(),"NMaxhepgenjets",10000);
  //tuple_stdpfjets->farray("hepgenjet_phi",PHI,hepgenjets.begin(),hepgenjets.end(),"NMaxhepgenjets",10000);

  HepMC::GenVertex *GV;
  LHCb::Vertex *PV = new LHCb::Vertex();
  HepMC::GenEvent *VBosonEvent = NULL;


  for(LoKi::GenTypes::GRange::iterator ik1 = GVB.begin(); GVB.end() != ik1 ; ++ik1 ) {
	  const HepMC::GenParticle *Vgen = *ik1;

	  if(ik1==GVB.begin()) {
		  GV = Vgen->production_vertex(); VBosonEvent = Vgen->parent_event();
		  const Gaudi::XYZPoint xyz(GV->point3d().x(),GV->point3d().y(),GV->point3d().z());
		  PV->setPosition(xyz);
		  tuple_stdpfjets->column("V_px",Vgen->momentum().px());
		  tuple_stdpfjets->column("V_py",Vgen->momentum().py());
		  tuple_stdpfjets->column("V_pz",Vgen->momentum().pz());
		  tuple_stdpfjets->column("V_e",Vgen->momentum().e());
		  tuple_stdpfjets->column("V_vtxz",GV->point3d().z());
//		  info() << "V_vtxz = " << GV->point3d().z() << endreq;

		  //std::vector<const HepMC::GenParticle *> Vdaughters;

		  //for (std::vector < HepMC::GenParticle * > ::const_iterator igendaughter = Vgen->end_vertex ()->particles_out_const_begin(); igendaughter != Vgen->end_vertex ()->particles_out_const_end(); igendaughter++){

		//	  const HepMC::GenParticle * gendaughter = *igendaughter;
		//	  Vdaughters.push_back(gendaughter);

		  //}

		  //tuple_stdpfjets->farray("V_daughters_px",GPX,Vdaughters.begin(),Vdaughters.end(),"NMaxVdaughters",10000);
		  HepMC::GenVertex *final = Vgen->end_vertex();
		  while(final->particles_out_size ()==1) final = (*(final->particles_out_const_begin()))->end_vertex(); 


		  tuple_stdpfjets->farray("V_daughters_px",GPX,
				  "V_daughters_py",GPY,
				  "V_daughters_pz",GPZ,
				  "V_daughters_e",GE, 
				  final->particles_out_const_begin(),final->particles_out_const_end(),"NMaxVdaughters",10000);

		  tuple_stdpfjets->farray("V_daughters_pdgid",GID,
				  final->particles_out_const_begin(),final->particles_out_const_end(),"NMaxVdaughters",10000);

	  }

	  break;
  }

  std::vector<const HepMC::GenParticle *> genparts_from_pv_vboson;
  std::vector<const HepMC::GenParticle *> genmuons_from_pv_vboson;

  for(LoKi::GenTypes::GRange::iterator ik1 = GFinal.begin(); GFinal.end() != ik1 ; ++ik1 ) {
    const HepMC::GenParticle *gen = *ik1;
    if(gen->momentum().pz()<0) continue;
    if(VBosonEvent==NULL) continue;
    if(VBosonEvent->signal_process_id() == gen->parent_event()->signal_process_id()) {

            genparts_from_pv_vboson.push_back(gen);

    }
  }

  for(LoKi::GenTypes::GRange::iterator ik1 = GMuons.begin(); GMuons.end() != ik1 ; ++ik1 ) {
    const HepMC::GenParticle *gen = *ik1;
    if(gen->momentum().pz()<0) continue;
    if(VBosonEvent==NULL) continue;
    if(VBosonEvent->signal_process_id() == gen->parent_event()->signal_process_id()) {

            genmuons_from_pv_vboson.push_back(gen);

    }
  }


  std::vector<const LHCb::Particle *> vbpvgenp4jets;
  for (std::vector<const HepMC::GenParticle *>::iterator igenp = genparts_from_pv_vboson.begin() ; igenp != genparts_from_pv_vboson.end() ; ++igenp)
    {
            const HepMC::GenParticle * genp = * igenp;
	    LHCb::Particle * part = new LHCb::Particle();
	    LorentzVector gen4vectortmp(genp->momentum().px(),genp->momentum().py(),genp->momentum().pz(),genp->momentum().e());
	    part->setMomentum(gen4vectortmp);
	    part->setParticleID(LHCb::ParticleID(genp->pdg_id()));
	    vbpvgenp4jets.push_back(part);
    }

  StatusCode sc3 = jetMaker -> makeJets ( vbpvgenp4jets , vbpvhepgenjets ) ;                                                                       

  //tuple_stdpfjets->farray("vbpvhepgenjet_pt",PT,vbpvhepgenjets.begin(),vbpvhepgenjets.end(),"NMaxvbpvhepgenjets",10000);
  //tuple_stdpfjets->farray("vbpvhepgenjet_p",P,vbpvhepgenjets.begin(),vbpvhepgenjets.end(),"NMaxvbpvhepgenjets",10000);
  tuple_stdpfjets->farray("vbpvhepgenjet_px",PX,
				  "vbpvhepgenjet_py",PY,
				  "vbpvhepgenjet_pz",PZ,
				  "vbpvhepgenjet_e",E,vbpvhepgenjets.begin(),vbpvhepgenjets.end(),"NMaxvbpvhepgenjets",10000);
  tuple_stdpfjets->farray("vbpvhepgenjet_pdgid",ID,
				  "vbpvhepgenjet_ndaughters",NDAUGHTERS,vbpvhepgenjets.begin(),vbpvhepgenjets.end(),"NMaxvbpvhepgenjets",10000);



  //tuple_stdpfjets->farray("vbpvhepgenjet_eta",ETA,vbpvhepgenjets.begin(),vbpvhepgenjets.end(),"NMaxvbpvhepgenjets",10000);
  //tuple_stdpfjets->farray("vbpvhepgenjet_phi",PHI,vbpvhepgenjets.begin(),vbpvhepgenjets.end(),"NMaxvbpvhepgenjets",10000);

  jetcounter = 0;
  std::vector<const LHCb::Particle *> vbpvjetdaughters;
  std::vector<int> vbpvjetcounterdaughters;
  std::vector<const HepMC::GenParticle *> vbpvjetpartons;
  std::vector<int> vbpvjetcounterpartons;


  for (std::vector<Particle *>::iterator ijet = vbpvhepgenjets.begin() ; ijet != vbpvhepgenjets.end() ; ++ijet)
  {
	  const LHCb::Particle * jet = * ijet;
	  std::vector<const LHCb::Particle *> daughtersvector = jet->daughtersVector();
	  jetcounter++;
	  LHCb::Particle * GenJet = new LHCb::Particle() ;//98 = jetid
	  int quarkcounter = 0;


	  for (LoKi::GenTypes::GRange::iterator iquark = GPartons.begin() ; iquark != GPartons.end() ; ++iquark)
	  {

		  const HepMC::GenParticle * quark = * iquark;

		  double deltaPhi = std::abs(jet->momentum().phi() - quark->momentum().phi());
		  if(deltaPhi > std::acos(-1)) deltaPhi = 2*std::acos(-1) - deltaPhi;
		  double deltaEta = jet->momentum().eta() - quark->momentum().eta(); 

		  if (std::sqrt( std::pow(deltaPhi,2) + std::pow(deltaEta,2)) < MaxRdistance)
		  {
			  vbpvjetpartons.push_back(quark);
			  quarkcounter++;
		  }


	  }

	  vbpvjetcounterpartons.push_back(quarkcounter);

	  for (std::vector<const LHCb::Particle *>::iterator idaughter = daughtersvector.begin() ; idaughter != daughtersvector.end() ; ++idaughter){

		  const LHCb::Particle * daughter = *idaughter;
		  vbpvjetdaughters.push_back(daughter);
		  vbpvjetcounterdaughters.push_back(jetcounter);

	  }
  }

  //tuple_stdpfjets->farray("vbpvjetdaughters_pt",PT,vbpvjetdaughters.begin(),vbpvjetdaughters.end(),"NMaxvbpvjetdaughters",10000);
  //tuple_stdpfjets->farray("vbpvjetdaughters_p",P,vbpvjetdaughters.begin(),vbpvjetdaughters.end(),"NMaxvbpvjetdaughters",10000);
  tuple_stdpfjets->farray("vbpvjetdaughters_px",PX,
				  "vbpvjetdaughters_py",PY,
				  "vbpvjetdaughters_pz",PZ,
				  "vbpvjetdaughters_e",E,vbpvjetdaughters.begin(),vbpvjetdaughters.end(),"NMaxvbpvjetdaughters",10000);

  tuple_stdpfjets->farray("vbpvjetpartons_px", GPX,
				  "vbpvjetpartons_py",GPY,
				  "vbpvjetpartons_pz",GPZ,
				  "vbpvjetpartons_e",GE,vbpvjetpartons.begin(),vbpvjetpartons.end(),"NMaxvbpvjetpartons",10000);

  tuple_stdpfjets->farray("vbpvhepgenjet_pdgid",GID,vbpvjetpartons.begin(),vbpvjetpartons.end(),"NMaxvbpvjetpartons",10000);

  tuple_stdpfjets->farray("vbpvhepgenjet_npartons",vbpvjetcounterpartons,"NMaxvbpvhepgenjets",10000);

  tuple_stdpfjets->farray("vbpvjetdaughters_pdgid",ID,
				  "vbpvjetdaughters_ndaughters",NDAUGHTERS,vbpvjetdaughters.begin(),vbpvjetdaughters.end(),"NMaxvbpvjetdaughters",10000);
  //tuple_stdpfjets->farray("vbpvjetdaughters_eta",ETA,vbpvjetdaughters.begin(),vbpvjetdaughters.end(),"NMaxvbpvjetdaughters",10000);
  //tuple_stdpfjets->farray("vbpvjetdaughters_phi",PHI,vbpvjetdaughters.begin(),vbpvjetdaughters.end(),"NMaxvbpvjetdaughters",10000);
  //tuple_stdpfjets->farray("vbpvjetdaughters_jetid",vbpvjetcounterdaughters,"NMaxvbpvjetdaughters",10000);

  //tuple_mcpfjets->write();
  tuple_stdpfjets->write();


  return StatusCode::SUCCESS ;

}
