#ifndef DECODEPILEUPDATA_H 
#define DECODEPILEUPDATA_H 1

// Include files
// from Gaudi
#include "GaudiAlg/GaudiAlgorithm.h"

// from TELL1 Kernel:
#include "Tell1Kernel/VeloDecodeConf.h" 
#include "Tell1Kernel/PUTell1Core.h"

/** @class DecodePileUpData DecodePileUpData.h
 *  
 *
 *  @author Serena Oggero
 *  @date   2009-06-19
 */

// -----------------------------------
namespace LHCb
{
  class RawEvent;
  class RawBank;
}
using namespace LHCb;
// -----------------------------------

namespace PuTell1
{
  enum BankTypes{
    LOPU_NZS = 57
  };
}

class DecodePileUpData : public GaudiAlgorithm {
public: 

  DecodePileUpData( const std::string& name, ISvcLocator* pSvcLocator ); /// Standard constructor
  virtual ~DecodePileUpData(); /// Destructor
  virtual StatusCode initialize();    ///< Algorithm initialization
  virtual StatusCode execute   ();    ///< Algorithm execution
  virtual StatusCode finalize  ();    ///< Algorithm finalization

  StatusCode getRawEvent();
  StatusCode decode();
  StatusCode getPileUpBank();
  StatusCode writePUBanks(LHCb::VeloClusters *, LHCb::VeloClusters *);
  StatusCode decodePileUpBinary( const std::vector<LHCb::RawBank*> & banks );
  StatusCode decodePileUpBinaryNZS( const std::vector<LHCb::RawBank*> & banks );
  StatusCode findPileUpHitsBee( PuTell1::dataObject, int, int, LHCb::VeloClusters * );
  StatusCode findPileUpHitsBeeNZS( PuTell1::dataObject, int, int, LHCb::VeloClusters * );
 
  int SensorId( int );
  std::string binary ( unsigned int );
  void inizializePUcontainer( PuTell1::DataTable PUcontainerBee );
  void Fill( unsigned int wordIt, int word_Tot, unsigned int* data_Ptr, int step, PuTell1::DataTable PUcontainerBee );
 
protected:

private:

  LHCb::RawEvent*       m_rawEvent;    
  std::string		m_rawEventLoc;   
  const std::string	m_PUClusterLocation;  
  const std::string	m_PUClusterNZSLocation;        
  bool          	m_isNonZeroSupp;
  PuTell1::DataTable	m_PUcontainerBee_NZS;
  PuTell1::DataTable	m_PUcontainerBee;
};
#endif // DecodePileUpData_H
