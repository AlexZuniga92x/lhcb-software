#ifndef DISPLVERTICES_VELOEVENTSHAPECUTSS20p3_H
#define DISPLVERTICES_VELOEVENTSHAPECUTSS20p3_H 1

#include "GaudiAlg/GaudiHistoAlg.h"
#include "VeloDet/DeVelo.h"

/** @class VeloEventShapeCutsS20p3 VeloEventShapeCutsS20p3.h DisplVertices/VeloEventShapeCutsS20p3.h
 *
 * Global event cuts to reject events with high Velo track (and vertex) ghost rates, Stripping20p3 version
 *
 * @author Wouter Hulsbergen
 * @author Pieter David
 * @date   2013-12-29
 */
class VeloEventShapeCutsS20p3 : public GaudiHistoAlg {
public:
  /// Standard constructor
  VeloEventShapeCutsS20p3( const std::string& name, ISvcLocator* pSvcLocator );

  virtual ~VeloEventShapeCutsS20p3();           ///< Destructor

  virtual StatusCode initialize();              ///< Algorithm initialization
  virtual StatusCode execute   ();              ///< Algorithm execution
  virtual StatusCode finalize  ();              ///< Algorithm finalization
private:
  double m_maxVeloRatio;
  double m_maxPhiVectorSize;
  DeVelo* m_velo;

  double calculateVeloRatio() const;
  double calculatePhiVectorSize() const;

  // monitoring
  AIDA::IHistogram1D* m_hVeloRatio;
  AIDA::IHistogram1D* m_hPhiVectorSize;
};

#include "Kernel/IRelatedInfoTool.h"
#include "GaudiAlg/GaudiTool.h"
#include "Event/RelatedInfoMap.h"
class AddVeloEventShapeS21 : public GaudiTool, virtual public IRelatedInfoTool {
public:
  AddVeloEventShapeS21( const std::string& type, const std::string& name, const IInterface* parent );
  virtual ~AddVeloEventShapeS21();

  virtual StatusCode initialize();
  virtual StatusCode calculateRelatedInfo( const LHCb::Particle* top, const LHCb::Particle* part );
  virtual LHCb::RelatedInfoMap* getInfo(void);

private:
  DeVelo* m_velo;

  // cache
  double m_phiVectorSize;

  LHCb::RelatedInfoMap m_map;
};

#endif // DISPLVERTICES_VELOEVENTSHAPECUTSS20p3_H
