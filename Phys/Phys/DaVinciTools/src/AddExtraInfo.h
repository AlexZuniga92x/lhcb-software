#ifndef ADDEXTRAINFO_H 
#define ADDEXTRAINFO_H 1

#include "Kernel/DVAlgorithm.h"

/** @class AddExtraInfo AddExtraInfo.h 
 *
 *  Algorithm to add Particle isolation variables 
 *  calculated by ConeVariables tool to particle extra info
 *
 *  @author Anton Poluektov
 *  @date   19/02/2012
 */

class AddExtraInfo : public DVAlgorithm {

public: 
  /// Standard constructor
  AddExtraInfo( const std::string& name, ISvcLocator* pSvcLocator );
  virtual ~AddExtraInfo( ); ///< Destructor
  virtual StatusCode initialize();    ///< Algorithm initialization
  virtual StatusCode execute   ();    ///< Algorithm execution
  virtual StatusCode finalize  ();    ///< Algorithm finalization

private: 
   
  std::vector<std::string> m_toolNames;
  
  std::list<IExtraInfoTool*> m_tools;

};

//=======================================================================//
#endif // AddExtraInfo_H
