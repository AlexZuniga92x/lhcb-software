
// $Id: MuonIDFOI.h,v 1.6 2005-06-15 06:26:39 pkoppenb Exp $

#ifndef MUONIDFOI_H 
#define MUONIDFOI_H 1

// Include files
// from STL

#include <string>

// from Gaudi

#include "GaudiAlg/GaudiAlgorithm.h"
#include "MuonDet/MuonBasicGeometry.h"

// forward declaration of tools

class IMuonTileXYZTool;
class IMuonGeometryTool;

//forward declaration of MuonID and TrStoredTrack from Event classes

class MuonID;
class TrStoredTrack;

/** @class MuonIDFOI MuonIDFOI.h
 *  
 *  This is an Algorithm to create MuonID objects starting from tracks and
 *  using the hits in the muon system
 *
 *  @author David Hutchcroft
 *  @date   02/05/2002
 *  
 *  updated by jtmn, miriam 22/04/2003
 *  
 */

class MuonIDFOI : public GaudiAlgorithm {
public:

  /// Standard constructor

  MuonIDFOI( const std::string& name, ISvcLocator* pSvcLocator );

  virtual ~MuonIDFOI( ); ///< Destructor

  virtual StatusCode initialize();    ///< Algorithm initialization
  virtual StatusCode execute   ();    ///< Algorithm execution

protected:

private:

  /// Do the identification
  // StatusCode doID(MuonID * pMuid);

  /// check the track is in the p and angle acceptance
  StatusCode preSelection(MuonID * pMuid, bool &passed);

  /// Fill the local vector of coord positions
  StatusCode fillCoordVectors();

  /// Empty the coord vectors
  void clearCoordVectors();

  /// Set the MuonCoords used in this ID
  // StatusCode setCoords(MuonID *pMuid);

  /// Extract the momentum and extrapolate the track to each station
  StatusCode trackExtrapolate(TrStoredTrack *pTrack);

  /// return the FOI in x in a station and region for momentum (in MeV/c)
  double foiX(const int &station, const int &region, const double &p, const double &dx);
  /// return the FOI in y in a station and region for momentum (in MeV/c)
  double foiY(const int &station, const int &region, const double &p, const double &dy);
  /// function to calculate the dll_probability , a copy of DLL algorithm
  // double landau_root(double x, double mpv, double sigma);
  
  
  /// clear track based local variables
  void resetTrackLocals();

  // Properties

  /// TES path of the tracks to analyse
  std::string m_TrStoredTracksPath;

  /// TES path to output the MuonIDs to
  std::string m_MuonIDsPath;

  /// Preselection momentum (no attempt to ID below this)
  std::vector<double> m_PreSelMomentum;

  /// Momentum ranges: different treatement of M4/M5 in each
  std::vector<double> m_MomentumCuts; // vector of momentum ranges

  int m_MakeNoShared;
	    
  // function that defines the field of interest size
  // formula is p(1) + p(2)*momentum + p(3)*exp(-p(4)*momentum)
  std::vector< double >     m_xfoiParam1;
  std::vector< double >     m_xfoiParam2;
  std::vector< double >     m_xfoiParam3;
  std::vector< double >     m_xfoiParam4;
  std::vector< double >     m_yfoiParam1;
  std::vector< double >     m_yfoiParam2;
  std::vector< double >     m_yfoiParam3;
  std::vector< double >     m_yfoiParam4;

  // Number of stations
  int m_NStation;
  // Number of regions
  int m_NRegion;
  //Names of the station
  std::vector<std::string> m_stationNames;

  // local array of pad sizes in mm
  // all std::vectors here are indexed: [station * m_NRegion + region]
  std::vector<double> m_padSizeX;
  std::vector<double> m_padSizeY;

  // local array of region sizes
  std::vector<double> m_regionInnerX; // inner edge in abs(x)
  std::vector<double> m_regionOuterX; // outer edge in abs(x) 
                            
  std::vector<double> m_regionInnerY; // inner edge in abs(y)
  std::vector<double> m_regionOuterY; // outer edge in abs(y) 
  
  // These are indexed [station]
  std::vector<double> m_stationZ; // station position
  
  // local track parameters: momentum and linear extrapolation to each station
  double m_Momentum; // in MeV/c
  double m_trackSlopeX,m_trackSlopeY;
  std::vector<double> m_trackX; // position of track in x(mm) in each station
  std::vector<double> m_trackY; // position of track in y(mm) in each station  

  // store X and Y of hits
  std::vector<double> m_CoordX,m_CoordDX,m_CoordY,m_CoordDY;
  // int m_xMatchStation; // first station to calculate slope (M2)
  
  // MuonTileID to ZYX tool
  IMuonTileXYZTool *m_iTileTool;
  IMuonGeometryTool *m_iGeomTool;

  // OK nasty optimisation here, store x,dx,y,dy of each coord to test against
  // track extrapolation
  class coordExtent_{
  public:
    coordExtent_(double x, double dx, double y, double dy, MuonCoord *pCoord) :
      m_x(x), m_dx(dx), m_y(y), m_dy(dy), m_pCoord(pCoord)  {};

    double m_x;
    double m_dx;
    double m_y;
    double m_dy;

    MuonCoord *m_pCoord;

  };

  // vector of positions of coords (innner vector coords, 
  // outer is [station* m_NRegion + region ]
  std::vector<std::vector<coordExtent_> > m_coordPos; 

};
#endif // MUONIDFOI_H
