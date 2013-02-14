// $Id: JetTagTruthNew.h,v 1.1 2011-07-27 12:12:12 aphan Exp $


//-----------------------------------------------------------------------------
//
// Helper class to classify jets using truth for jet tagging
//
//-----------------------------------------------------------------------------

#ifndef LOKIJETTAGTRUTHNEW_H
#define LOKIJETTAGTRUTHNEW_H 1

// ============================================================================
// Includes

// Gaudi
#include "GaudiAlg/GaudiTool.h"
#include "GaudiKernel/ToolFactory.h"

// Kernel
#include "Kernel/IJetMaker.h"
#include "Kernel/IJets2Jets.h"
#include "Kernel/IJetTagTool.h"

// LoKi
#include "LoKi/LoKi.h"
#include "LoKi/ILoKiSvc.h"

#include "LoKi/AlgoMC.h"
#include "LoKi/GenExtract.h"
#include "LoKi/GenParticleCuts.h"

// Relations
#include "Relations/Relation2D.h"

// Event
#include "Event/HepMCEvent.h"


class IParticle2MCAssociator;
// ============================================================================
// Declaration
namespace LoKi
{
  class JetTagTruthNew
    : public virtual IJetTagTool
    , public GaudiTool
  {
    // friend factory
    friend class ToolFactory<LoKi::JetTagTruthNew>;

    public:

      // standard initialise
      StatusCode initialize();
      // standard finalise
      StatusCode finalize();

      // Method that calculates the truth tag of a jet
      //   -1 - not found, 1 - light jet, 4 - charm jet, 5 - bottom jet
      bool calculateJetProperty
      (
        const LHCb::Particle *jet,
	std::map <std::string,double > &jetTruth
      );

    protected:

      // standard constructor
      JetTagTruthNew
      (
        const std::string &type,
        const std::string &name,
        const IInterface *parent
      );

      // standard destructor
      ~JetTagTruthNew();

    private:

      // disable default constructor
      JetTagTruthNew();
      // disable copy constructor
      JetTagTruthNew(const JetTagTruthNew&);
      // disable assignement operator
      JetTagTruthNew& operator=(const JetTagTruthNew&);


      IParticle2MCAssociator* m_p2mcAssoc;
      std::string m_p2mcAssocType;

      IDVAlgorithm* m_dva;


      std::vector<double> TruthBWeight(const LHCb::Particle* jet);
      std::vector<double> TruthCWeight(const LHCb::Particle* jet);
      const LHCb::MCParticle* Bancestor(const LHCb::MCParticle* mcpart);
      const LHCb::MCParticle* Cancestor(const LHCb::MCParticle* mcpart);
      bool MuonDecay(const LHCb::MCParticle* mcpart);


   // minimum weight for matching
      double m_matchMinWeight;
      // maximum delta r for matching
      double m_matchMaxDeltaR;

      // table linking visible particle jets and reconstructed jets
      std::string m_jetMatchTableHepMC2Jets;
      // table linking particles and genparticles
      std::string m_jetMatchTablePart2GenPart;




  };
}

#endif
