// $Id: PVReFitterAlg.h,v 1.15 2010-06-21 12:11:36 jpalac Exp $
#ifndef PVREFITTERALG_H 
#define PVREFITTERALG_H 1

// Include files
// from Gaudi
#include "GaudiAlg/GaudiAlgorithm.h"

// From LHCb
#include "Kernel/Particle2Vertex.h"

class IPVReffitter;
class IPVOfflineTool;
class ILifetimeFitter;
class IOnOffline;

/** @class PVReFitterAlg PVReFitterAlg.h
 *  
 * Simple GaudiAlgorithm that takes some particles, some primary 
 * vertices, and refits the vertices excluding the tracks coming from the 
 * particle's decay. It also creates a relations table associating each 
 * particle to its corresponding new vertex.
 *
 * <b>Properties</b>:
 *
 * <b>IPVOfflineTool</b>  : Implemenmtation of IPVOfflineTool. Default: PVOfflineTool.
 *
 * <b>IPVReFitter</b>     : Implemenmtation of IPVRefitter. Default: AdaptivePVReFitter.
 *
 * <b>UseIPVOfflineTool</b> : bool, apply IPVOfflineTool::reDoSinglePV method. Default: false.
 *
 * <b>UseIPVReFitter</b> : bool, apply IPVReFitter::remove method. Default: true.
 *
 * <b>ParticleInputLocation</b>: TES location of the particles who's related primary vertices will be re-fitted. Default "".
 *
 * <b>PrimaryVertexInputLocation</b>: TES location of the LHCb::RecVertices to be
 * re-fitted. Default "". By default, location is obtained from an IOnOffline tol.
 *
 * The algorithm iterates over the LHCb::Particles in ParticleInputLocation,
 * and the LHCb::RecVertices in PrimaryVertexLocation, re-fits clones of the
 * vertices excluding any 
 * tracks that originate from the particle, and creates a relations table
 * connecting the particle to the re-fitted vertices. The re-fitted vertices 
 * are stored  in a KeyedContainer<LHCb::RecVertex>,
 * which is placed in <location>/<instance name>ReFittedPVs. The relations table 
 * is placed in <location>/<instance name>P2PVRelations. <location> 
 * is obtained by stripping away "/Particles" from <b> ParticleInputLocation</b>
 *
 * The re-fitting itself is a sequence of
 * IPVOfflineTool::reDoSinglePV (if UseIPVOfflineTool==true), 
 * and IPVReFitter::remove (if UseIPVReFitter == true).
 * <b>Beware</b>: If there is more than one candidate per event, there can be 
 * more than one refitted PV for each original one. Hence users are required
 * to access the re-fitted vertices on a particle by particle basis, using
 * the relations table, and not the full container of re-fitted vertices.
 *
 *
 *  @author Juan PALACIOS
 *  @date   2008-06-25
 */
class PVReFitterAlg : public GaudiAlgorithm {
public: 
  /// Standard constructor
  PVReFitterAlg( const std::string& name, ISvcLocator* pSvcLocator );

  virtual ~PVReFitterAlg( ); ///< Destructor

  virtual StatusCode initialize();    ///< Algorithm initialization
  virtual StatusCode execute   ();    ///< Algorithm execution
  virtual StatusCode finalize  ();    ///< Algorithm finalization

private:

  LHCb::RecVertex* refitVertex(const LHCb::RecVertex* v,
                               const LHCb::Particle* p  ) const;
  
  void getTracks(const LHCb::Particle* p,
                 LHCb::Track::ConstVector& tracks) const;


  void removeEnding(std::string& a, const std::string& ending);

private:

  IPVOfflineTool* m_pvOfflineTool;
  IPVReFitter* m_pvReFitter;

  IOnOffline* m_onOfflineTool;

  std::string m_pvOfflinetoolType;
  std::string m_pvReFitterType;
  bool m_useIPVOfflineTool;
  bool m_useIPVReFitter;
  std::string m_particleInputLocation;
  std::string m_PVInputLocation;
  std::string m_particle2VertexRelationsOutputLocation;
  std::string m_vertexOutputLocation;
  std::string m_outputLocation;

  Particle2Vertex::Table m_p2VtxTable;
  
};
#endif // PVREFITTERALG_H
