// $Id: PVRelatorAlg.h,v 1.9 2010-03-02 14:04:12 jpalac Exp $
#ifndef PVRELATORALG_H 
#define PVRELATORALG_H 1

// Include files
// boost
 #include  <boost/type_traits/remove_pointer.hpp> 
// from Gaudi
#include "GaudiAlg/GaudiAlgorithm.h"

class IOnOffline;
class IRelatedPVFinder;

/** @class PVRelatorAlg PVRelatorAlg.h
 *  
 *
 *  A simple GaudiAlgorithm that takes as input the TES location of some
 *  LHCb::Particles, plus the TES location of some LHCb::RecVertices <b>or</b>
 *  the TES location of an LHCb::Particle->LHCb::VertexBase weighted relations 
 *  table(Particle2Vertex::WTable), and uses an IRelatedPVFinder to create sorted 
 *  Particle->PV relations or to re-sort the input relations if a relations table 
 *  has given as input.
 *  The sorting is done according to the logic of the implementation of the
 *  IRelatedPVFinder, which is determined by an IOnOfflineTool.
 *
 *  <b>Properties</b>
 *
 *  <b>ParticleInputLocation</b> : TES location of input LHCb::Particles. 
 *  Default: "". Must be set by user unless <b>P2PVRelationsInputLocation<b> is set
 *
 *  <b>PrimaryVertexInputLocation</b> : TES location of input LHCb::RecVertices. 
 *  Default: LHCb::RecVertexLocation::Primary
 *
 *  <b>P2PVRelationsInputLocation</b> : TES location of input Particle2Vertex::WTable. 
 *  Default: "".
 *
 *  <b>P2PVRelationsOutputLocation</b> : TES location of the output 
 *  Particle2Vertex::WTable with the newly sorted relations. Default "".
 *  Must be set by user.
 *
 *  Besides the properties that <b>must</b> be set, there are two extra 
 *  conditions: neither <b>ParticleInputLocation</b> nor <b>PrimaryVertexInputLocation<b> 
 *  can be set if <b>P2PVRelationsInputLocation<b> is set. This will result in 
 *  the initalize() method returning a StatusCode::FAILURE.
 *
 *  <b>Example</b>: Create a Particle2Vertex::WTable relating LHCb::Particles from
 *  "Phys/DC06selBd2Jpsi2MuMu_Kst2KPi/Particles" to default primary vertices and 
 *  put it in "Phys/DC06selBd2Jpsi2MuMu_Kst2KPi/P2DefaultPVRelations". Create 
 *  another Particle2Vertex::WTable relating LHCb::Particles to re-fitted PVs 
 *  starting from a Particle2Vertex::WTable in "Phys/DC06selBd2Jpsi2MuMu_Kst2KPi/P2ReFitPVRelations" 
 *  and put it in "Phys/DC06selBd2Jpsi2MuMu_Kst2KPi/P2ReFitPVSortedRelations"
 *
 *  @code
 *  from Configurables import PVRelatorAlg
 *  # Default PVs
 *  p2PV = PVRelatorAlg("P2StandardVertices")
 *  p2PV.ParticleInputLocation = "Phys/DC06selBd2Jpsi2MuMu_Kst2KPi/Particles"
 *  p2PV.P2PVRelationsOutputLocation = "Phys/DC06selBd2Jpsi2MuMu_Kst2KPi/P2DefaultPVRelations"
 *  # re-fitted PVs
 *  p2RefitPV = PVRelatorAlg("P2ReFitVertices")
 *  p2RefitPV.ParticleInputLocation = "Phys/DC06selBd2Jpsi2MuMu_Kst2KPi/Particles"
 *  p2RefitPV.P2PVRelationsInputLocation = "Phys/DC06selBd2Jpsi2MuMu_Kst2KPi/P2ReFitPVRelations"
 *  p2RefitPV.P2PVRelationsOutputLocation = "Phys/DC06selBd2Jpsi2MuMu_Kst2KPi/P2ReFitPVSortedRelations"
 *  # Add it to a selection sequence
 *  seq = GaudiSequencer('SeqDC06selBd2Jpsi2MuMu_Kst2KPi')
 *  seq.Members += [p2PV, P2RefitPV]
 *
 * 
 *  @endcode
 *
 *  @author Juan Palacios
 *  @date   2009-05-23
 */
class PVRelatorAlg : public GaudiAlgorithm {
public: 
  /// Standard constructor
  PVRelatorAlg( const std::string& name, ISvcLocator* pSvcLocator );

  virtual ~PVRelatorAlg( ); ///< Destructor

  virtual StatusCode initialize();    ///< Algorithm initialization
  virtual StatusCode execute   ();    ///< Algorithm execution
  virtual StatusCode finalize  ();    ///< Algorithm finalization

private:

  template <typename T>
  inline T* 
  i_get(const std::string& location) const
  {
    typedef typename boost::remove_pointer<typename T::value_type>::type ContainedType;
    const bool exists = 
      exist<ContainedType::Container>( location ) || exist<T>(location);
    return exists ? get<T>( location ) : 0 ;
  }

  Particle2Vertex::WTable* table() const;
  
  Particle2Vertex::WTable* tableFromTable() const;

private:

  std::string m_particleInputLocation;
  std::string m_PVInputLocation;
  std::string m_P2PVInputLocation;
  std::string m_P2PVOutputLocation;
  bool m_useTable;
  IOnOffline* m_OnOffline ; ///< context switch tool. To be deprecated.
  IRelatedPVFinder* m_pvRelator ; ///< Tool that relates the Particle to a PV


};
#endif // PVRELATORALG_H
