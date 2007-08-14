// $Id: ITExpectedHits.h,v 1.1.1.1 2007-08-14 13:50:47 jonrob Exp $
#ifndef _ITExpectedHits_H
#define _ITExpectedHits_H

#include <algorithm>

#include "GaudiAlg/GaudiTool.h"
#include "GaudiKernel/Plane3DTypes.h"
#include "GaudiKernel/Point3DTypes.h"

#include "TsaKernel/IITExpectedHits.h"
#include "TsaKernel/Parabola.h"
#include "TsaKernel/Line.h"
#include "TsaKernel/Line3D.h"
#include "TsaKernel/TsaConstants.h"

// Kernel
#include "Kernel/STChannelID.h"
#include "LHCbMath/GeomFun.h"

// Geometry
#include "STDet/DeSTDetector.h"
#include "STDet/DeSTLayer.h"
#include "STDet/DeSTSector.h"

namespace Tf
{
  namespace Tsa
  {

    /** @class ITExpectedHits ITExpectedHits.h
     * How many hits do we expect in in the OT
     * @author M. Needham
     * @date 30.05.2004
     */

    class ITExpectedHits: public GaudiTool,
                          virtual public IITExpectedHits {

    public:

      /// constructer
      ITExpectedHits(const std::string& type,
                     const std::string& name,
                     const IInterface* parent);

      // destructer
      virtual ~ITExpectedHits();

      /// init
      virtual StatusCode initialize();

      // execute method
      virtual StatusCode collect(const Tsa::Parabola& parab,
                                 const Tsa::Line& line,
                                 const LHCb::STChannelID& aChan,
                                 std::vector<IITExpectedHits::ITPair>& hits ,
                                 const unsigned int iSector) const;

    private:

      bool insideSector(const DeSTSector* sector,
                        const Tsa::Line3D& line) const;

      bool insideLayer(const DeSTLayer* sector,
                       const Tsa::Line3D& line) const;

      // Gaudi::XYZPoint intersection(const Tsa::Line3D& line, const DeSTSector* sector,
      //          const Gaudi::XYZPoint& aPoint) const;


      Gaudi::XYZPoint intersection(const Tsa::Line3D& line,
                                   const Gaudi::Plane3D& aPlane) const;

      DeSTDetector* m_tracker;

    };

  }
}

#endif // _ITExpectedHits_H
