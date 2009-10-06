#ifndef HLT_HLTUTILS_H 
#define HLT_HLTUTILS_H 1

#include "GaudiKernel/Point3DTypes.h"
#include "GaudiKernel/Vector3DTypes.h"

//#include "Event/Track.h"
#include "Event/RecVertex.h"

#include <algorithm>
#include <functional>

//forward declarations
namespace LHCb {
  class Track;
};


namespace Hlt {

  /* merge two tracks (adds info, ids, states)
   *
   */
  void TrackMerge(const LHCb::Track& track, LHCb::Track& otrack);
  
  template <class T>
  void MergeInfo(const T& input, T& output) 
  {
    const GaudiUtils::VectorMap<int,double>& info = input.extraInfo();
    GaudiUtils::VectorMap<int,double>::const_iterator it = info.begin();
    for (; it != info.end(); ++it)
      output.addInfo(it->first,it->second);
  }

  /* It fills the vertex using the 2 tracks
   *
   */
  class VertexCreator 
  {
  public:
    explicit VertexCreator() {}
    void operator() (const LHCb::Track& track1,
                     const LHCb::Track& track2,
                     LHCb::RecVertex& ver) const;
  };
  
  /* Functor: to order tracks by decresing order of PT
   *
   */
  class SortTrackByPt {
  public:
    bool operator() (const LHCb::Track* lhs, 
                     const LHCb::Track* rhs ) const {
        double ptl = lhs->pt();
        double ptr = rhs->pt();
        return (ptl == ptr) ? (lhs->key() > rhs->key())
                            : (ptl > ptr) ;
}
  };
}


namespace HltUtils
{

  //! return the impact parameter vector
  double rImpactParameter(const LHCb::RecVertex& vertex,
                          const LHCb::Track& track);
  
  //! return the impact parameter vector
  double impactParameter(const LHCb::RecVertex& vertex,
                         const LHCb::Track& track);

  double invariantMass(const LHCb::Track&, const LHCb::Track&,
                      double, double);
 

  //! return the RecVertex made with this 2 tracks
  //! WARNING NOTE: the caller get ownership of the LHCb::RecVertex and 
  //                the responsability of deleting it!
  LHCb::RecVertex* newRecVertex(const LHCb::Track& track1,
                                    const LHCb::Track& track2);

  //! return the closest distance between the 2 tracks (first State)
  Gaudi::XYZVector closestDistance(const LHCb::Track& track1, 
                          const LHCb::Track& track2);

  double deltaEta(const LHCb::Track& track1, const LHCb::Track& track2);

  double deltaPhi(const LHCb::Track& track1, const LHCb::Track& track2);

  double deltaAngle(const LHCb::Track& track1, const LHCb::Track& track2);

  //! retun the closest point between the 2 tracks (first State)
  Gaudi::XYZPoint closestPoint(const LHCb::Track& track1,
                      const LHCb::Track& track2);

  double FC(const LHCb::RecVertex& svertex, 
            const LHCb::RecVertex& pvertex );

  double VertexMinPT(const LHCb::RecVertex& vertex);

  double VertexMaxPT(const LHCb::RecVertex& vertex);

  double matchIDsFraction(const LHCb::Track& trackreference, 
                          const LHCb::Track& track2);

  double vertexMatchIDsFraction(const LHCb::RecVertex& vreference, 
                                const LHCb::RecVertex& vertex);

  inline bool matchIDs(const LHCb::Track& treference, 
                       const LHCb::Track& track) {
    return (matchIDsFraction(treference,track) > 0.70);    
  }
  
  double closestDistanceMod(const LHCb::Track& track1,
                            const LHCb::Track& track2);


  bool doShareM3(const LHCb::Track& track0, const LHCb::Track& track1);
  

  double vertexMatchIDsFraction(const LHCb::RecVertex& vreference, 
                                const LHCb::RecVertex& vertex);
  
//   bool matchIDs(const LHCb::Track& track1, const LHCb::Track& track2,
//                 double minf = 0.75) {
//     int n = ELoop::count(track1.lhcbIDs(),track2.lhcbIDs());
//     int n0 = track1.lhcbIDs().size();
//     double f = (n0>0?(1.*n)/(1.*n0):0);
//     return f>minf;
//   }

  double IPError(const LHCb::Track& track);

  //------------------------------------------------------------

  //! compute the impact parameter vector
  Gaudi::XYZVector impactParameterVector(const Gaudi::XYZPoint& vertex,
                                const Gaudi::XYZPoint& point,
                                const Gaudi::XYZVector& direction);

  //! return the modules of the impact parameter (signed)
  double impactParameter(const Gaudi::XYZPoint& vertex,
                         const Gaudi::XYZPoint& point,
                         const Gaudi::XYZVector& direction);
  
  //! return points in rays 
  bool closestPoints(const Gaudi::XYZPoint& ori1, const Gaudi::XYZVector& dir1,
                     const Gaudi::XYZPoint& ori2, const Gaudi::XYZVector& dir2,
                     Gaudi::XYZPoint& close1, Gaudi::XYZPoint& close2);

  //! old parameterization of Pt error
  double impactParameterError(double pt);
  
  
  //------------------------------------------------------------
  
  bool doShareM3(const LHCb::Track& track0, const LHCb::Track& track1);

  // calocellid tools
	std::vector<LHCb::CaloCellID> get3x3CellIDs( const LHCb::CaloCellID& centercell );	
	
	std::vector<LHCb::CaloCellID> get2x2CellIDs( const LHCb::CaloCellID& bottomleftcell );
	
	bool matchCellIDs( const std::vector<LHCb::CaloCellID>& oncells, 
					   const std::vector<LHCb::CaloCellID>& offcells );
	

};

#endif 
