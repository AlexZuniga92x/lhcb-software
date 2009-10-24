// $Id: VeloSimTell1Algorithm.h,v 1.2 2009-10-24 13:48:23 szumlat Exp $
#ifndef VELOSIMTELL1ALGORITHM_H 
#define VELOSIMTELL1ALGORITHM_H 1

// Include files
// from Gaudi
#include "GaudiAlg/GaudiAlgorithm.h"

// form TELL1 Kernel
#include "Tell1Kernel/VeloDecodeCore.h"
#include "Tell1Kernel/VeloDecodeConf.h"
#include "Tell1Kernel/VeloTell1Core.h"

// from TELL1 event
#include "VeloEvent/VeloTELL1Data.h"
#include "VeloEvent/VeloProcessInfo.h"

// DAQ Event
#include "Event/RawEvent.h"
#include "Event/RawBank.h"

// VeloDet
#include "VeloDet/DeVelo.h"

// boost
#include <boost/lexical_cast.hpp>
#include <boost/assign/std/vector.hpp>

/** @class VeloSimTell1Algorithm VeloSimTell1Algorithm.h
 *  
 *  base class for any TELL1 algorithm
 *
 *  @author Tomasz Szumlak
 *  @date   2009-08-02
 */

class VeloSimTell1Algorithm : public GaudiAlgorithm {
public: 

  /// Standard constructor
  VeloSimTell1Algorithm( const std::string& name, ISvcLocator* pSvcLocator );

  virtual ~VeloSimTell1Algorithm( ); ///< Destructor

  virtual StatusCode initialize();    ///< Algorithm initialization
  virtual StatusCode execute   ();    ///< Algorithm execution
  virtual StatusCode finalize  ();    ///< Algorithm finalization

protected:
  
  virtual StatusCode getData();
  virtual StatusCode INIT();
  virtual StatusCode writeData();
  virtual StatusCode cloneData();
  virtual void setInputDataLoc(const std::string inValue);
  virtual void setOutputDataLoc(const std::string inValue);
  virtual void setTELL1Process(const unsigned int inValue);
  virtual void setAlgoName(const std::string inValue);
  virtual const std::string algoName() const;
  virtual void setAlgoType(const std::string inValue);
  virtual const std::string algoType() const;
  virtual LHCb::VeloTELL1Datas* inputData() const;
  virtual void outputData(LHCb::VeloTELL1Data* inData);
  virtual LHCb::VeloTELL1Datas* outputData();
  virtual void setIsEnable(bool flag);
  virtual bool isEnable() const;
  virtual unsigned int convergenceLimit() const;
  virtual bool isInitialized() const;
  virtual StatusCode readDataFromFile(const char* fileName,
                                      VeloTELL1::sdataVec& inVec);
  virtual StatusCode writeDataToFile(const char* fileName,
                                     VeloTELL1::sdataVec& inVec);
  virtual StatusCode inputStream(LHCb::VeloTELL1Datas* inVec) const;
  virtual void prepareData(bool insertAtEnd);
  virtual void prepareData(VeloTELL1::sdataVec& inVec) const;
  virtual DeVelo* deVelo() const;
  virtual void propagateReorderingInfo(const LHCb::VeloTELL1Data* inObj,
                                       LHCb::VeloTELL1Data* outObj);
  virtual VeloTELL1::AListPair getSrcIdList() const;
  virtual void setSrcIdList(std::vector<unsigned int> inVec);
  virtual StatusCode i_cacheSrcIdList();
  virtual StatusCode i_cacheConditions();

  unsigned int m_evtNumber;
  bool m_validationRun;
  unsigned int m_dbConfig;
  std::string m_inputDataLoc;
  std::string m_outputDataLoc;
  std::string m_algorithmName;
  std::string m_condPath;
  std::vector<unsigned int> m_srcIdList;  /// list of the TELL1 numbers
  bool m_forceEnable;
  VeloTELL1::sdataVec m_dataBuffer;
  
private:

  LHCb::RawEvent* m_rawEvent;
  std::string m_rawEventLoc;
  DeVelo* m_detElement;
  LHCb::VeloTELL1Datas* m_inputData;
  LHCb::VeloTELL1Datas* m_outputData;
  VeloProcessInfos* m_procInfos;
  std::string m_procInfosLoc;
  std::string m_algorithmType;
  unsigned int m_tell1Process;
  bool m_isEnable;
  unsigned int m_convergenceLimit;
  bool m_isInitialized;

};
#endif // VELOSIMTELL1ALGORITHM_H
