/// ===========================================================================
/// CVS tag $Name: not supported by cvs2svn $ 
/// ===========================================================================
/// $Log: not supported by cvs2svn $
/// Revision 1.7  2001/08/09 16:47:58  ibelyaev
/// update in interfaces and redesign of solids
/// 
/// ===========================================================================
#ifndef     DETDESC_SOLIDCONS_H
#define     DETDESC_SOLIDCONS_H 1   
/// STD and STL 
#include <cmath> 
#include <iostream>
/// CLHEP 
#include "CLHEP/Geometry/Point3D.h"
#include "CLHEP/Geometry/Vector3D.h"
#include "CLHEP/Units/PhysicalConstants.h"
/// DetDesc 
#include "DetDesc/SolidBase.h" 
/// forward declarations 
template <class TYPE>
class SolidFactory;

/** @class SolidCons SolidCons.h DetDesc/SolidCons.h
 *
 *  A simple implementation segment of conical tube.
 *  The shape corresponds to Geant4 shape G4Cons 
 *  and to Geant3 shape CONS   
 *  
 *  @author Vanya Belyaev 
 */

class SolidCons: public virtual SolidBase
{
  /// friend factory for instantiation 
  friend class SolidFactory<SolidCons>;

public:
  
  /** standard constructor
   *  @param name                  name of conical tube segment
   *  @param ZHalfLength           half-length 
   *  @param OuterRadiusAtMinusZ   outer radius at minus Z 
   *  @param OuterRadiusAtPlusZ    outer radius at plus  Z 
   *  @param InnerRadiusAtMinusZ   inner radius at minus Z 
   *  @param InnerRadiusAtPlusZ    inner radius at plus  Z 
   *  @param StartPhiAngle         start phi angle 
   *  @param DeltaPhiAngle         delta phi angle 
   *  @param CoverModel            covering model 
   *  @exception SolidException    wrong parameters range 
   */
  SolidCons( const std::string& name                                  ,
             const double       ZHalfLength                           , 
             const double       OuterRadiusAtMinusZ                   ,
             const double       OuterRadiusAtPlusZ                    ,
             const double       InnerRadiusAtMinusZ  =   0.0          , 
             const double       InnerRadiusAtPlusZ   =   0.0          , 
             const double       StartPhiAngle        =   0.0 * degree , 
             const double       DeltaPhiAngle        = 360.0 * degree ,
             const int          CoverModel           =   0            );

  /// destructor 
  virtual ~SolidCons();
  
  /** - retrieve the conical tube segment  type 
   *  - implementation of ISolid abstract interface 
   *  @see ISolid 
   *  return box type
   */
  inline std::string typeName () const { return "SolidCons"; };
  
  /** - check for the given 3D-point. 
   *    Point coordinated are in the local reference 
   *    frame of the solid.   
   *  - implementation of ISolid absstract interface  
   *  @see ISolid 
   *  @param point point (in local reference system of the solid)
   *  @return true if the point is inside the solid
   */
  bool isInside (  const HepPoint3D& point ) const ;
  
  /** -# retrieve the pointer to "simplified" solid - "cover"
   *    - for Model = 0 
   *        -# the cover for the general conical tube segment is 
   *           the conical tube 
   *        -# the cover for the conical tube is conical cylinder 
   *        -# the cover for the conical cylinder is TRD 
   *    - for Model != 0 
   *        -# the cover for conical tube serment is conical cylinder segment 
   *        -# the cover for conical cylinder segment is conical cylinder 
   *        -# the cover for conical cylinder is TRD 
   *  -# implementation of ISolid abstract interface 
   *  @see ISolid 
   *  @return pointer to "simplified" solid - "cover"
   */
  const ISolid* cover () const ;
  
  /** - printout to STD/STL stream    
   *  - implementation  of ISolid abstract interface 
   *  - reimplementation of SolidBase::printOut( std::ostream& )
   *  @see SolidBase 
   *  @see ISolid 
   *  @param stream STD/STL stream
   *  @return reference to the stream 
   */
  virtual std::ostream&  printOut ( std::ostream& os = std::cout )    const;
  
  /** - printout to Gaudi MsgStream stream    
   *  - implementation  of ISolid abstract interface 
   *  - reimplementation of SolidBase::printOut( MsgStream& )
   *  @see SolidBase 
   *  @see ISolid 
   *  @param stream STD/STL stream
   *  @return reference to the stream 
   */
  virtual MsgStream&     printOut ( MsgStream&                   )    const; 
  
  /** - calculate the intersection points("ticks") of the solid objects 
   *    with given line. 
   *  -# Line is parametrized with parameter \a t :
   *     \f$ \vec{x}(t) = \vec{p} + t \times \vec{v} \f$ 
   *      - \f$ \vec{p} \f$ is a point on the line 
   *      - \f$ \vec{v} \f$ is a vector along the line  
   *  -# \a tick is just a value of parameter \a t, at which the
   *    intersection of the solid and the line occurs
   *  -# both  \a Point  (\f$\vec{p}\f$) and \a Vector  
   *    (\f$\vec{v}\f$) are defined in local reference system 
   *   of the solid 
   *  - implementation of ISolid abstract interface  
   *  @see ISolid 
   *  @param Point initial point for the line
   *  @param Vector vector along the line
   *  @param ticks output container of "Ticks"
   *  @return the number of intersection points
   */
  virtual unsigned int 
  intersectionTicks ( const HepPoint3D & Point  ,  
                      const HepVector3D& Vector ,  
                      ISolid::Ticks    & ticks  ) const ;
  
  /** - serialization for reading
   *  - implementation of ISerialize abstract interface 
   *  - reimplementation of SolidBase::serialize 
   *  @see ISerialize 
   *  @see ISolid  
   *  @see SolidBase   
   *  @param      s               reference to stream buffer
   *  @exception  SolidException  wrong parameters range 
   *  @return reference to stream buffer
   */
  StreamBuffer& serialize( StreamBuffer& sb )       ; 
  
  /** - serialization for writing
   *  - implementation of ISerialize abstract interface 
   *  - reimplementation of SolidBase::serialize 
   *  @see ISerialize 
   *  @see ISolid  
   *  @see SolidBase   
   *  @param s reference to stream buffer
   *  @return reference to stream buffer
   */
  StreamBuffer& serialize( StreamBuffer& sb ) const ; 

  /** inner radius at minus Z  
   *  @return inner radius at minus Z  
   */
  inline double  innerRadiusAtMinusZ () const 
  { return m_cons_innerRadiusMinusZ ; };
  
  /** outer radius at minus Z  
   *  @return outer radius at minus Z  
   */
  inline double  outerRadiusAtMinusZ () const 
  { return m_cons_outerRadiusMinusZ ; };
  
  /** inner radius at plus Z  
   *  @return inner radius at plus Z  
   */
  inline double  innerRadiusAtPlusZ  () const 
  { return m_cons_innerRadiusPlusZ  ; };
  
  /** outer radius at plus Z  
   *  @return outer radius at plus Z  
   */
  inline double  outerRadiusAtPlusZ  () const 
  { return m_cons_outerRadiusPlusZ  ; };
  
  /** half length 
   *  @return half length
   */ 
  inline double  zHalfLength         () const 
  { return m_cons_zHalfLength       ; };

  /** start phi angle   
   *  @return  start phi angle   
   */
  inline double  startPhiAngle       () const 
  { return m_cons_startPhiAngle     ; }; 

  /** delta phi   
   *  @return delta phi   
   */
  inline double  deltaPhiAngle       () const
  { return m_cons_deltaPhiAngle     ; }; 

  /** inner diameter at minus Z  
   *  @return inner diameter at minus Z  
   */
  inline double  innerDiameterAtMinusZ () const 
  { return m_cons_innerRadiusMinusZ * 2 ; };
  
  /** outer radius at minus Z  
   *  @return outer radius at minus Z  
   */
  inline double  outerDiameterAtMinusZ () const 
  { return m_cons_outerRadiusMinusZ * 2 ; };
  
  /**inner radius at plus Z  
   * @return inner radius at plus Z  
   */
  inline double  innerDiameterAtPlusZ  () const 
  { return m_cons_innerRadiusPlusZ  * 2 ; };
  
  /** outer radius at plus Z  
   *  @returnouter radius at plus Z  
   */
  inline double  outerDiameterAtPlusZ  () const 
  { return m_cons_outerRadiusPlusZ  * 2 ; };
  
  /** full length 
   *  @return full length 
   */
  inline double  zLength               () const 
  { return m_cons_zHalfLength       * 2 ; };
  
  /** end phi angle   
   *  @return end phi angle   
   */
  inline double  endPhiAngle           () const 
  { return m_cons_startPhiAngle + m_cons_deltaPhiAngle ; }; 
  
protected:

  /** inner radius at given z  
   *  @param z z position 
   *  @return inner ragius at given z 
   */
  inline double iR_z( double z ) const
  {  
    ///
    const double a = 
      ( innerRadiusAtPlusZ () - innerRadiusAtMinusZ () ) / zHalfLength ();
    const double b = 
      ( innerRadiusAtPlusZ () + innerRadiusAtMinusZ () )  ; 
    ///
    return 0.5 * ( a * z  + b );
  };
  
  /** outer radius at given z  
   *  @param z z position 
   *  @return outer ragius at given z 
   */
  inline double oR_z( double z ) const
  {
    ///  
    const double a = 
      ( outerRadiusAtPlusZ () - outerRadiusAtMinusZ () ) / zHalfLength ();
    const double b = 
      ( outerRadiusAtPlusZ () + outerRadiusAtMinusZ () )  ; 
    //
    return 0.5*(a*z+b);
  };

protected:
  
  /** default protected  coinstructor 
   *  @param name name of conical tube segment  
   */
  SolidCons( const std::string& Name = "Anonymous CONS") ;
  
private:

  SolidCons           ( const SolidCons & );  ///< no copy-constructor 
  SolidCons& operator=( const SolidCons & );  ///< no assignment 

private:

  double                  m_cons_zHalfLength       ;
  double                  m_cons_outerRadiusMinusZ ;
  double                  m_cons_outerRadiusPlusZ  ;
  double                  m_cons_innerRadiusMinusZ ;
  double                  m_cons_innerRadiusPlusZ  ;
  double                  m_cons_startPhiAngle     ;
  double                  m_cons_deltaPhiAngle     ;
  ///
  int                     m_cons_coverModel;
};

/// ===========================================================================
#endif ///<  DETDESC_SOLIDCONS_H
/// ===========================================================================















