/*
 * IFitterFcn.h
 *
 *  Created on: Aug 12, 2014
 *      Author: beat
 */
#include "GaudiKernel/IAlgTool.h"
#ifndef IFITTERFCN_H_
#define IFITTERFCN_H_
namespace LHCb
{
  class IFitterFcn : virtual public IAlgTool
  {
    public:
      DeclareInterfaceID(LHCb::IFitterFcn,1,0);
      virtual void init()=0;
      virtual double analyze()=0;
      virtual void ReadParams()=0;
      virtual void PubResult(long reference)=0;
      virtual void i_run()=0;
//      virtual void setWorkerSvc(IDumAligWork *p)=0;
  };
}
#endif /* IFITTERFCN_H_ */
