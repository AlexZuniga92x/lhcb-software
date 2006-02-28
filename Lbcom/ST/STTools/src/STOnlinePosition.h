// $Id: STOnlinePosition.h,v 1.4 2006-02-28 15:37:05 mneedham Exp $
#ifndef _STOnlinePosition_H
#define _STOnlinePosition_H

#include "GaudiAlg/GaudiTool.h"

// From LHCbKernel
#include "Kernel/ISTClusterPosition.h"

#include <vector>

/** @class STOnlinePosition STOnlinePosition.h
 *
 *  Tool for calculating offline cluster position
 *
 *  @author M.Needham
 *  @date   14/3/2002
 */

class STOnlinePosition: public GaudiTool, virtual public ISTClusterPosition {

public: 
   
  /// constructer
  STOnlinePosition(const std::string& type,
                      const std::string& name,
                      const IInterface* parent);

  /// destructer
  virtual ~STOnlinePosition();

  /// method
  virtual ISTClusterPosition::Info estimate(const LHCb::STCluster* aCluster) const;

  virtual ISTClusterPosition::Info estimate(const SmartRefVector<LHCb::STDigit>& digits) const;

  virtual double error(const unsigned int nStrips) const;

private:

  double STOnlinePosition::stripFraction(const double interStripPos) const;

  std::vector<double> m_ErrorVec;

};

#endif // _STOnlinePosition_H



