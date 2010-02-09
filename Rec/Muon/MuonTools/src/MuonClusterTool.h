// $Id: MuonClusterTool.h,v 1.1 2010-02-09 16:02:24 asatta Exp $
#ifndef MUONCLUSTERTOOL_H 
#define MUONCLUSTERTOOL_H 1

// Include files
// from Gaudi
#include "GaudiAlg/GaudiTool.h"
#include "MuonDet/DeMuonDetector.h"
#include "Event/MuonCoord.h"
#include "Event/MuonCluster.h"
#include "MuonInterfaces/IMuonClusterTool.h"            // Interface


/** @class MuonClusterTool MuonClusterTool.h
 *  
 *
 *  @author Alessia Satta
 *  @date   2010-01-15
 */
class MuonClusterTool : public GaudiTool, virtual public IMuonClusterTool {
public: 
  /// Standard constructor
  MuonClusterTool( const std::string& type, 
                   const std::string& name,
                   const IInterface* parent);

  virtual ~MuonClusterTool( ); ///< Destructor
  StatusCode initialize();	
  virtual StatusCode doCluster(const std::string Input,const std::string Output);

protected:
  
private:
  StatusCode initClusterVector(LHCb::MuonCoords* coords);
  StatusCode mergeCluster();
  StatusCode mergeStation(int i);
  void ClearMemory();
  StatusCode SaveOutput(std::string output);
  
  bool detectCluster(LHCb::MuonCluster* one,LHCb::MuonCluster* two);
  bool isIncluded(int station,LHCb::MuonCluster* cluster)	;  
  std::vector<std::pair<LHCb::MuonCluster *,int> > m_inputclust[5];
  //  std::vector<std::pair<LHCb::MuonCluster *,int> > m_outputclust[5];
  std::vector<LHCb::MuonCluster * > m_finalclust[5];
  DeMuonDetector* m_muonDetector;  
  bool m_debug;


};
#endif // MUONCLUSTERTOOL_H
