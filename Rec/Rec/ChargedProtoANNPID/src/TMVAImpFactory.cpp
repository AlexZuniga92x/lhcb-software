// Include files 

// local
#include "TMVAImpFactory.h"

// So the TMVA networks pick up the std:: functions
using namespace std;

namespace ANNGlobalPID
{

  /** @namespace MC12TuneV2 TMVAImpFactory.cpp
   *
   *  Namespace for C++ implementations of MC12TuneV2 TMVA networks.
   *
   *  @author Chris Jones  Christopher.Rob.Jones@cern.ch
   *  @date   2013-04-26
   */
  namespace MC12TuneV2
  {
    // Long tracks
#include "tmva/MC12TuneV2/GlobalPID_Electron_Long_TMVA.class.C"
#include "tmva/MC12TuneV2/GlobalPID_Muon_Long_TMVA.class.C"
#include "tmva/MC12TuneV2/GlobalPID_Pion_Long_TMVA.class.C"
#include "tmva/MC12TuneV2/GlobalPID_Kaon_Long_TMVA.class.C"
#include "tmva/MC12TuneV2/GlobalPID_Proton_Long_TMVA.class.C"
#include "tmva/MC12TuneV2/GlobalPID_Ghost_Long_TMVA.class.C"
    // Downstream
#include "tmva/MC12TuneV2/GlobalPID_Electron_Downstream_TMVA.class.C"
#include "tmva/MC12TuneV2/GlobalPID_Muon_Downstream_TMVA.class.C"
#include "tmva/MC12TuneV2/GlobalPID_Pion_Downstream_TMVA.class.C"
#include "tmva/MC12TuneV2/GlobalPID_Kaon_Downstream_TMVA.class.C"
#include "tmva/MC12TuneV2/GlobalPID_Proton_Downstream_TMVA.class.C"
#include "tmva/MC12TuneV2/GlobalPID_Ghost_Downstream_TMVA.class.C"
    // Upstream
#include "tmva/MC12TuneV2/GlobalPID_Electron_Upstream_TMVA.class.C"
#include "tmva/MC12TuneV2/GlobalPID_Muon_Upstream_TMVA.class.C"
#include "tmva/MC12TuneV2/GlobalPID_Pion_Upstream_TMVA.class.C"
#include "tmva/MC12TuneV2/GlobalPID_Kaon_Upstream_TMVA.class.C"
#include "tmva/MC12TuneV2/GlobalPID_Proton_Upstream_TMVA.class.C"
#include "tmva/MC12TuneV2/GlobalPID_Ghost_Upstream_TMVA.class.C"
  }

  /** @namespace MC12TuneV3 TMVAImpFactory.cpp
   *
   *  Namespace for C++ implementations of MC12TuneV3 TMVA networks.
   *
   *  @author Chris Jones  Christopher.Rob.Jones@cern.ch
   *  @date   2014-06-26
   */
  namespace MC12TuneV3
  {
    // Long tracks
#include "tmva/MC12TuneV3/GlobalPID_Electron_Long_TMVA.class.C"
#include "tmva/MC12TuneV3/GlobalPID_Muon_Long_TMVA.class.C"
#include "tmva/MC12TuneV3/GlobalPID_Pion_Long_TMVA.class.C"
#include "tmva/MC12TuneV3/GlobalPID_Kaon_Long_TMVA.class.C"
#include "tmva/MC12TuneV3/GlobalPID_Proton_Long_TMVA.class.C"
#include "tmva/MC12TuneV3/GlobalPID_Ghost_Long_TMVA.class.C"
    // Downstream
#include "tmva/MC12TuneV3/GlobalPID_Electron_Downstream_TMVA.class.C"
#include "tmva/MC12TuneV3/GlobalPID_Muon_Downstream_TMVA.class.C"
#include "tmva/MC12TuneV3/GlobalPID_Pion_Downstream_TMVA.class.C"
#include "tmva/MC12TuneV3/GlobalPID_Kaon_Downstream_TMVA.class.C"
#include "tmva/MC12TuneV3/GlobalPID_Proton_Downstream_TMVA.class.C"
#include "tmva/MC12TuneV3/GlobalPID_Ghost_Downstream_TMVA.class.C"
    // Upstream
#include "tmva/MC12TuneV3/GlobalPID_Electron_Upstream_TMVA.class.C"
#include "tmva/MC12TuneV3/GlobalPID_Muon_Upstream_TMVA.class.C"
#include "tmva/MC12TuneV3/GlobalPID_Pion_Upstream_TMVA.class.C"
#include "tmva/MC12TuneV3/GlobalPID_Kaon_Upstream_TMVA.class.C"
#include "tmva/MC12TuneV3/GlobalPID_Proton_Upstream_TMVA.class.C"
#include "tmva/MC12TuneV3/GlobalPID_Ghost_Upstream_TMVA.class.C"
  }

  // Standard constructor
  TMVAImpFactory::TMVAImpFactory()
  {
    // Add the MC12TuneV2 implementations to the factory
    {
      const std::string tune = "MC12TuneV2";
      // Long
      add<MC12TuneV2::ReadElectron_Long_TMVA>       ( tune, "electron", "Long" );
      add<MC12TuneV2::ReadMuon_Long_TMVA>           ( tune, "muon",     "Long" );
      add<MC12TuneV2::ReadPion_Long_TMVA>           ( tune, "pion",     "Long" );
      add<MC12TuneV2::ReadKaon_Long_TMVA>           ( tune, "kaon",     "Long" );
      add<MC12TuneV2::ReadProton_Long_TMVA>         ( tune, "proton",   "Long" );
      add<MC12TuneV2::ReadGhost_Long_TMVA>          ( tune, "ghost",    "Long" );
      // Downstream
      add<MC12TuneV2::ReadElectron_Downstream_TMVA> ( tune, "electron", "Downstream" );
      add<MC12TuneV2::ReadMuon_Downstream_TMVA>     ( tune, "muon",     "Downstream" );
      add<MC12TuneV2::ReadPion_Downstream_TMVA>     ( tune, "pion",     "Downstream" );
      add<MC12TuneV2::ReadKaon_Downstream_TMVA>     ( tune, "kaon",     "Downstream" );
      add<MC12TuneV2::ReadProton_Downstream_TMVA>   ( tune, "proton",   "Downstream" );
      add<MC12TuneV2::ReadGhost_Downstream_TMVA>    ( tune, "ghost",    "Downstream" );
      // Upstream
      add<MC12TuneV2::ReadElectron_Upstream_TMVA>   ( tune, "electron", "Upstream" );
      add<MC12TuneV2::ReadMuon_Upstream_TMVA>       ( tune, "muon",     "Upstream" );
      add<MC12TuneV2::ReadPion_Upstream_TMVA>       ( tune, "pion",     "Upstream" );
      add<MC12TuneV2::ReadKaon_Upstream_TMVA>       ( tune, "kaon",     "Upstream" );
      add<MC12TuneV2::ReadProton_Upstream_TMVA>     ( tune, "proton",   "Upstream" );
      add<MC12TuneV2::ReadGhost_Upstream_TMVA>      ( tune, "ghost",    "Upstream" );
    }
    // Add the MC12TuneV3 implementations to the factory
    {
      const std::string tune = "MC12TuneV3";
      // Long
      add<MC12TuneV3::ReadElectron_Long_TMVA>       ( tune, "electron", "Long" );
      add<MC12TuneV3::ReadMuon_Long_TMVA>           ( tune, "muon",     "Long" );
      add<MC12TuneV3::ReadPion_Long_TMVA>           ( tune, "pion",     "Long" );
      add<MC12TuneV3::ReadKaon_Long_TMVA>           ( tune, "kaon",     "Long" );
      add<MC12TuneV3::ReadProton_Long_TMVA>         ( tune, "proton",   "Long" );
      add<MC12TuneV3::ReadGhost_Long_TMVA>          ( tune, "ghost",    "Long" );
      // Downstream
      add<MC12TuneV3::ReadElectron_Downstream_TMVA> ( tune, "electron", "Downstream" );
      add<MC12TuneV3::ReadMuon_Downstream_TMVA>     ( tune, "muon",     "Downstream" );
      add<MC12TuneV3::ReadPion_Downstream_TMVA>     ( tune, "pion",     "Downstream" );
      add<MC12TuneV3::ReadKaon_Downstream_TMVA>     ( tune, "kaon",     "Downstream" );
      add<MC12TuneV3::ReadProton_Downstream_TMVA>   ( tune, "proton",   "Downstream" );
      add<MC12TuneV3::ReadGhost_Downstream_TMVA>    ( tune, "ghost",    "Downstream" );
      // Upstream
      add<MC12TuneV3::ReadElectron_Upstream_TMVA>   ( tune, "electron", "Upstream" );
      add<MC12TuneV3::ReadMuon_Upstream_TMVA>       ( tune, "muon",     "Upstream" );
      add<MC12TuneV3::ReadPion_Upstream_TMVA>       ( tune, "pion",     "Upstream" );
      add<MC12TuneV3::ReadKaon_Upstream_TMVA>       ( tune, "kaon",     "Upstream" );
      add<MC12TuneV3::ReadProton_Upstream_TMVA>     ( tune, "proton",   "Upstream" );
      add<MC12TuneV3::ReadGhost_Upstream_TMVA>      ( tune, "ghost",    "Upstream" );
    }
  }

  TMVAImpFactory::~TMVAImpFactory( )
  {
    for ( auto i : m_map ) { delete i.second; }
  }

  // Method to get a static instance of the factory
  const TMVAImpFactory& tmvaFactory()
  {
    static TMVAImpFactory factory;
    return factory;
  }

}

