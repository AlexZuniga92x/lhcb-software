// $Id: ExampleAnalysisTask.h,v 1.2 2008-04-30 13:28:54 ggiacomo Exp $
#ifndef OMALIB_EXAMPLEANALYSISTASK_H
#define OMALIB_EXAMPLEANALYSISTASK_H 1
#include "OMAlib/AnalysisTask.h"
#include <string>

class ExampleAnalysisTask : public AnalysisTask 
{
 public:
  ExampleAnalysisTask(const std::string& name, 
                      ISvcLocator* pSvcLocator ) :
    AnalysisTask( name, pSvcLocator ) {}
  virtual ~ExampleAnalysisTask ();
  virtual StatusCode initialize();
  virtual StatusCode analyze(std::string& SaveSet,
                             std::string Task);
};

#endif // OMALIB_EXAMPLEANALYSISTASK_H
