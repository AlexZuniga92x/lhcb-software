#ifndef     GIGA_GIGATRAJECTORYPOINT_H
#define     GIGA_GIGATRAJECTORYPOINT_H 1 


#include "CLHEP/Vector/LorentzVector.h"

/// base class form G4 
#include "G4TrajectoryPoint.hh" 


/** @class GiGaTrajectoryPoint GiGaTrajectoryPoint.h GiGa/GiGaTrajectoryPoint.h

    Customisation of "standard" G4TrajectoryPoint to incorporate TimeOfFlight
    of the point. Overwise conversion to MCVertex class is impossible.  
    
    @author  Vanya Belyaev
    @date    22/02/2001
*/

class GiGaTrajectoryPoint: public G4TrajectoryPoint
{
  ///
public:
  ///
  GiGaTrajectoryPoint(); 
  GiGaTrajectoryPoint( const Hep3Vector& , const double&     );
  GiGaTrajectoryPoint( const HepLorentzVector&               );
  GiGaTrajectoryPoint(            const GiGaTrajectoryPoint& ); 
  virtual ~GiGaTrajectoryPoint();
  ///
  inline void* operator new(size_t);
  inline void  operator delete(void*);
  ///
  bool operator==( const GiGaTrajectoryPoint& right )
  { return (&right) == this ; } 
  ///
  inline const double&           time       () const        { return m_time ; } 
  inline void                    setTime    ( double Time ) { m_time = Time ; }  
  inline const double&           GetTime    () const        { return time() ; } 
  ///
  inline const HepLorentzVector  fourVector () const 
  { return HepLorentzVector( GetPosition() , GetTime() ) ; };  
  ///
private:
  ///
  double m_time; 
  ///
};
///
extern G4Allocator<GiGaTrajectoryPoint> s_GiGaTrajectoryPointAllocator;
///
inline void* GiGaTrajectoryPoint::operator new(size_t)
{ return (void *) s_GiGaTrajectoryPointAllocator.MallocSingle(); };
///
inline void GiGaTrajectoryPoint::operator delete(void *aTrajectoryPoint)
{ s_GiGaTrajectoryPointAllocator.FreeSingle((GiGaTrajectoryPoint *) aTrajectoryPoint); }
///


#endif  //  GIGA_GIGATRAJECTORYPOINT_H












