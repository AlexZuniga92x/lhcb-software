// $Id: PVSeed3DTool.cpp,v 1.1 2008-06-11 19:28:24 witekma Exp $
// Include files 

// from Gaudi
#include "GaudiKernel/ToolFactory.h" 

// local
#include "PVSeed3DTool.h"

//-----------------------------------------------------------------------------
// Implementation file for class : PVSeed3DTool
//
// 2008-04-20 : Mariusz Witek
//-----------------------------------------------------------------------------

// Declaration of the Tool Factory
DECLARE_TOOL_FACTORY( PVSeed3DTool );


class seedPoint 
{
public:
  EPoint position;
  EPoint error;
  int multiplicity;
};

class seedTrack 
{
public:
  const Track* lbtrack;
  int nclose;
  int used;
};

class closeNode 
{
public:
  seedTrack* seed_track;
  double distance;
  EPoint closest_point;
  int take;
};

bool  trackcomp( const seedTrack &first, const seedTrack &second ) {
    return first.nclose > second.nclose;
    //    return first->distance < second->distance;
}

//=============================================================================
// Standard constructor, initializes variables
//=============================================================================
PVSeed3DTool::PVSeed3DTool( const std::string& type,
                            const std::string& name,
                            const IInterface* parent )
  : GaudiTool ( type, name , parent )
{
  declareInterface<IPVSeeding>(this);

}
//=============================================================================
// Destructor 
//=============================================================================
PVSeed3DTool::~PVSeed3DTool() {} 


//=============================================================================
// getSeeds
//=============================================================================
void PVSeed3DTool::getSeeds(std::vector<const LHCb::Track*>& inputTracks, std::vector<Gaudi::XYZPoint>& seeds) {

  if(inputTracks.size() < 3 ) return; 

  std::vector<seedTrack> seed_tracks;
  seed_tracks.reserve(inputTracks.size());

  std::vector<const LHCb::Track*>::iterator it;
  for ( it = inputTracks.begin(); it != inputTracks.end(); it++ ) {
    const LHCb::Track* ptr = (*it);
    seedTrack seedtr;
    seedtr.used=0;
    seedtr.nclose=0;
    seedtr.lbtrack = ptr;
    seed_tracks.push_back(seedtr);
  }

  cout << " seed_tracks.size  " << seed_tracks.size() << endl; 
  cout << " inputTracks.size  " << inputTracks.size() << endl; 

  std::vector<seedTrack>::iterator its1,its2;
  for(its1 = seed_tracks.begin(); its1 != seed_tracks.end()--; its1++) {       
     for(its2 = its1+1; its2 != seed_tracks.end(); its2++) {
        const Track* lbtr1 = its1->lbtrack;
        const Track* lbtr2 = its2->lbtrack;

        EPoint closestPoint;
        double distance;
        bool ok = xPointParameters(*lbtr1, *lbtr2, distance, closestPoint);
        if (ok && distance < 1.) {
	  its1->nclose++;
	  its2->nclose++;
        }
     }
  }
  
  std::sort(seed_tracks.begin(), seed_tracks.end(), trackcomp);

  for(its1 = seed_tracks.begin(); its1 != seed_tracks.end(); its1++) {

    if(its1->nclose<4) break;
    if ( its1->used > 0 ) continue;

    std::vector<closeNode> close_nodes;

    for(its2 = seed_tracks.begin(); its2 != seed_tracks.end(); its2++) {
      if ( its2->used > 0 || its1 == its2 ) continue;
       EPoint closest_point;
       double distance;
       const Track* lbtr1 = its1->lbtrack;
       const Track* lbtr2 = its2->lbtrack;
       bool ok = xPointParameters(*lbtr1, *lbtr2, distance, closest_point);
       double costh = thetaTracks(*lbtr1, *lbtr2);
       if (ok && distance < 1. && costh<0.999) {
         closeNode closetr; 
         closetr.take           = 1;
         closetr.seed_track    =  &(*its2); 
         closetr.distance      = distance;
         closetr.closest_point = closest_point;
         close_nodes.push_back( closetr );
       }
    }  // its2

    seedPoint mean_point;
    seedPoint mean_point_w;
    bool OK = simpleMean(close_nodes, mean_point);
    if ( OK ) {
      its1->used=1; // base track
      int multi = 1;
      std::vector<closeNode>::iterator it;
      for ( it = close_nodes.begin(); it != close_nodes.end(); it++ ) { 
        if ( it->take > 0 ) {
          it->seed_track->used = 1;
          multi++;
	}
      }   
      if ( multi < 4 ) continue;
      seedTrack* base_track = &(*its1); 
      wMean(close_nodes, base_track, mean_point_w);
      seeds.push_back(mean_point_w.position);
      
      cout << " xyz seed multi  " 
          << mean_point_w.position.X()  << " " 
          << mean_point_w.position.Y()  << " " 
          << mean_point_w.position.Z()  << " | " 
          << mean_point_w.error.X()  << " " 
          << mean_point_w.error.Y()  << " " 
          << mean_point_w.error.Z()  << " | " 
          << mean_point_w.multiplicity  << " " 
          << endl;
      
      

    }

  } // its1     

}

//=============================================================================
// closestPoints
//=============================================================================
bool PVSeed3DTool::closestPoints(const EPoint& ori1, const EVector& dir1,
                             const EPoint& ori2, const EVector& dir2,
                             EPoint& close1, EPoint& close2) {
  
  // Calculate the point between two tracks
  // (closest distance to both tracks)
  // code from Paul Bourke, 
  // http://astronomy.swin.edu.au/~pbourke/geometry/lineline3d/
 
  double eps(1.e-6);
  
  close1 = EPoint(0.,0.,0.);
  close2 = EPoint(0.,0.,0.);
  
  EVector udir1 = dir1.unit();
  EVector udir2 = dir2.unit();
  
  EPoint t1b = ori1;
  EPoint t2b = ori2;
  
  EVector v0 = ori1 - ori2;
  EVector v1 = udir1;
  if (fabs(v1.x())  < eps && fabs(v1.y())  < eps && fabs(v1.z())  < eps)
    return false;
  EVector v2 = udir2;
  if (fabs(v2.x())  < eps && fabs(v2.y())  < eps && fabs(v2.z())  < eps)
    return false;
  
  double d02 = v0.Dot(v2);
  double d21 = v2.Dot(v1);
  double d01 = v0.Dot(v1);
  double d22 = v2.Dot(v2);
  double d11 = v1.Dot(v1);
  
  double denom = d11 * d22 - d21 * d21;
  if (fabs(denom) < eps) return false;
  double numer = d02 * d21 - d01 * d22;
  
  double mu1 = numer / denom;
  double mu2 = (d02 + d21 * mu1) / d22;
  
  close1 = t1b + mu1 * v1;  
  close2 = t2b + mu2 * v2;
  
  return true;
}

//=============================================================================
// thetaTracks
//=============================================================================
double PVSeed3DTool::thetaTracks(const Track& track1, 
                                 const Track& track2) {
  const State& state1 = track1.firstState();
  const State& state2 = track2.firstState();
  EVector dir1(state1.tx(),state1.ty(),1.);
  EVector dir2(state2.tx(),state2.ty(),1.);
  EVector udir1 = dir1.unit();
  EVector udir2 = dir2.unit();
  double ct = udir1.Dot(udir2);
  return ct;  
}

//=============================================================================
// closestDistance
//=============================================================================
bool PVSeed3DTool::xPointParameters(const Track& track1, const Track& track2,
                       double & distance, EPoint & closestPoint) {

  distance = 1.e10;
  const State& state1 = track1.firstState();
  const State& state2 = track2.firstState();
  EVector dis(0.,0.,1.e6);
  EPoint pos1(0.,0.,0.);
  EPoint pos2(0.,0.,0.);
  EVector dir1(state1.tx(),state1.ty(),1.);
  EVector dir2(state2.tx(),state2.ty(),1.);
  EPoint  ori1(state1.x(),state1.y(),state1.z());
  EPoint  ori2(state2.x(),state2.y(),state2.z());
  bool ok = closestPoints(ori1,dir1,ori2,dir2,pos1,pos2);
  if (!ok) return ok;
  closestPoint.SetXYZ(0.5*(pos1.x()+pos2.x()), 0.5*(pos1.y()+pos2.y()), 0.5*(pos1.z()+pos2.z()));
  distance = sqrt((pos1 - pos2).Mag2());
  return ok;;
}

//=============================================================================
// weighedMean
//=============================================================================
void PVSeed3DTool::wMean(std::vector<closeNode> & close_nodes, seedTrack* base_track, 
                               seedPoint & pseed) {

   pseed.position.SetXYZ(0., 0., 0.); 
   pseed.error.SetXYZ(0., 0., 0.); 
   pseed.multiplicity = close_nodes.size(); 
   if ( close_nodes.size() < 2 ) return;

   double sum_wx = 0.;
   double sum_wy = 0.;
   double sum_wz = 0.;
   double sum_wxx =0.;
   double sum_wyy =0.;
   double sum_wzz =0.;

   std::vector<closeNode>::iterator it;
   for ( it = close_nodes.begin(); it != close_nodes.end(); it++ ) { 
     if ( it->take == 0 ) continue;

     double errxy2 = 0.1*0.1;
     const Track* lbtr1 = base_track->lbtrack;
     const Track* lbtr2 = it->seed_track->lbtrack;
     double costh = thetaTracks(*lbtr1, *lbtr2);
     double c2 = costh*costh;
     double ctanth2 = c2/(1.-c2);
     double errz2 = 2.*ctanth2*errxy2;

     double wx = 1./ errxy2;      
     double wy = 1./ errxy2;      
     double wz = 1./ errz2;      
     sum_wx += wx;
     sum_wy += wy;
     sum_wz += wz;
     sum_wxx += wx*it->closest_point.X();
     sum_wyy += wy*it->closest_point.Y();
     sum_wzz += wz*it->closest_point.Z();       
   }

   double x = sum_wxx/sum_wx;
   double y = sum_wyy/sum_wy;
   double z = sum_wzz/sum_wz;

   double ex = sqrt(1./sum_wx);
   double ey = sqrt(1./sum_wy);
   double ez = sqrt(1./sum_wz);

   pseed.position.SetXYZ(x,y,z);
   pseed.error.SetXYZ(ex,ey,ez);
   pseed.multiplicity = close_nodes.size();
   /*

      cout << " www mean  " 
          << x  << " " 
          << y  << " " 
          << z  << "  err " 
          << ex  << " " 
          << ey  << " " 
          << ez  << " " 
          << endl;
   */
   return;
}

//=============================================================================
// simpleMean
//=============================================================================
bool PVSeed3DTool::simpleMean(std::vector<closeNode> & close_nodes, seedPoint & pseed) {
  
   pseed.position.SetXYZ(0., 0., 0.); 
   pseed.error.SetXYZ(0., 0., 0.); 
   pseed.multiplicity = close_nodes.size(); 

   if ( close_nodes.size() < 2 ) return false;

   double x = 0.;
   double y = 0.;
   double z = 0.;

   double  spread2_max = 5.*5.;

   EPoint pmean;
   std::vector<closeNode>::iterator it;
   std::vector<closeNode>::iterator itmax;

   double dist2_max = 1000.*1000.;
   int ngood = 1000;
   while (dist2_max>spread2_max && ngood>1) {
     int ng = 0;
     x = 0.;
     y = 0.;
     z = 0.;
     for ( it = close_nodes.begin(); it != close_nodes.end(); it++ ) {         
        if ( it->take == 0 ) continue;
        ng++;
        x += it->closest_point.X();
        y += it->closest_point.Y();
        z += it->closest_point.Z();
     }
     if ( ng < 4 ) return false;
     x /= ng;
     y /= ng;
     z /= ng;
     pmean.SetXYZ(x,y,z);

     double d2max=0.;
     for ( it = close_nodes.begin(); it != close_nodes.end(); it++ ) { 
       if ( it->take == 0 ) continue;
       double dist2 = (pmean - it->closest_point).Mag2(); 
       if ( dist2>d2max ) {
         d2max = dist2;
         itmax = it;
       }
     }
   
     ngood = ng;
     if ( d2max > spread2_max ) {
       itmax->take = 0;
       ngood--;
     }   
     dist2_max = d2max;

   } // end while

   if ( ngood < 4 ) return false;
   pseed.position = pmean;
   pseed.multiplicity = ngood;
   return true;
}
	
//=============================================================================
