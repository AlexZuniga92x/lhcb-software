// $Id: SolidUnion.cpp,v 1.8 2003-05-16 13:59:14 ibelyaev Exp $ 
// ===========================================================================
// CVS tag $Name: not supported by cvs2svn $ 
// ===========================================================================
// $Log: not supported by cvs2svn $
// Revision 1.7  2002/05/11 18:25:48  ibelyaev
//  see $DETDESCROOT/doc/release.notes 11 May 2002
//
// ===========================================================================
/** STD & STL  */
#include <iostream> 
#include <string>
/** DetDesc */ 
#include "DetDesc/SolidUnion.h"
#include "DetDesc/SolidException.h"
#include "DetDesc/SolidFactory.h"
#include "DetDesc/Solid.h"
#include "DetDesc/SolidBox.h"

// ============================================================================
/** @file
 *
 *  implementation of class SolidUnion
 * 
 *  @author Vanya Belyaev Ivan.Belyaev@itep.ru
 *  @date   xx/xx/xxxx
 */
// ============================================================================

// ============================================================================
/** constructor 
 *  @param name name of the intersection
 *  @param first pointer to first/main solid 
 */
// ============================================================================
SolidUnion::SolidUnion( const std::string& name  , 
                        ISolid*            First )
  : SolidBoolean( name , First )
  , SolidBase   ( name         )
  , m_coverTop ( 0 ) 
{
  if( 0 == First ) 
    { throw SolidException(" SolidUnion:: ISolid* points to NULL!"); }
};
// ============================================================================

// ============================================================================
/** constructor 
 *  @param name name of the solid union 
 */
// ============================================================================
SolidUnion::SolidUnion( const std::string& Name )
  : SolidBoolean( Name )
  , SolidBase   ( Name )
  , m_coverTop( 0 ) 
{};
// ============================================================================

// ============================================================================
/// destructor 
// ============================================================================
SolidUnion::~SolidUnion()
{ 
  if( 0 != m_coverTop ) { delete m_coverTop ; }
};
// ============================================================================

// ============================================================================
/** - check for the given 3D-point. 
 *    Point coordinates are in the local reference 
 *    frame of the solid.   
 *  - implementation of ISolid absstract interface  
 *  @see ISolid 
 *  @param point point (in local reference system of the solid)
 *  @return true if the point is inside the solid
 */
// ============================================================================
bool SolidUnion::isInside     ( const HepPoint3D   & point ) const 
{ 
  /// check bounding box 
  if ( isOutBBox( point )         ) { return false ; }
  ///  is point inside the "main" volume?  
  if ( first()->isInside( point ) ) { return true  ; }
  /// find the first daughter in which the given point is placed   
  SolidUnion::SolidChildrens::const_iterator ci = 
    std::find_if( childBegin () , 
                  childEnd   () , 
                  Solid::IsInside( point ) ) ;
  ///
  return ( childEnd() == ci ? false : true );   
};

// ============================================================================
/** add child solid to the solid union
 *  @param solid pointer to child solid 
 *  @param mtrx  pointer to transformation 
 *  @return status code 
 */
// ============================================================================
StatusCode  SolidUnion::unite( ISolid*                solid    , 
                               const HepTransform3D*  mtrx     )
{  
  StatusCode sc = addChild( solid , mtrx ); 
  if( sc.isFailure() ) { return sc ; }
  return updateBP();
};

// ============================================================================
/** add child solid to the solid union
 *  @param solid pointer to child solid 
 *  @param position position  
 *  @return status code 
 */
// ============================================================================
StatusCode  SolidUnion::unite ( ISolid*               solid    , 
                                const HepPoint3D&     position , 
                                const HepRotation&    rotation )
{
  StatusCode sc = addChild( solid , position , rotation ); 
  if( sc.isFailure() ) { return sc ; }
  return updateBP();
};
// ============================================================================

// ============================================================================
/** create the cover top box 
 */
// ============================================================================
const ISolid* SolidUnion::coverTop() const 
{
  if( 0 != m_coverTop ) { return m_coverTop ; }
  
  const double x =  
    fabs( xMax() ) > fabs( xMin() ) ? fabs( xMax() ) : fabs( xMin() ) ;
  const double y =  
    fabs( yMax() ) > fabs( yMin() ) ? fabs( yMax() ) : fabs( yMin() ) ;
  const double z =  
    fabs( zMax() ) > fabs( zMin() ) ? fabs( zMax() ) : fabs( zMin() ) ;
  
  m_coverTop = new SolidBox ("CoverTop for " + name () , x , y, z  ) ;
  // 
  return m_coverTop;
};
// ============================================================================

// ============================================================================
/** update bonding parameters 
 *  @return status code 
 */
// ============================================================================
StatusCode SolidUnion::updateBP()
{
  if( childBegin() == childEnd() ) { return StatusCode::SUCCESS ; }
  // get last child 
  SolidChild* child = 
    *( childBegin() + ( childEnd() - childBegin() - 1 ) );
  // cast it!
  SolidBase* base = dynamic_cast<SolidBase*> (child);
  if( 0 == base ) { return StatusCode::FAILURE ; }
  //
  setXMin   ( base->xMin   () < xMin   () ? base->xMin   () : xMin   () );
  setXMax   ( base->xMax   () > xMax   () ? base->xMax   () : xMax   () );

  setYMin   ( base->yMin   () < yMin   () ? base->yMin   () : yMin   () );
  setYMax   ( base->yMax   () > yMax   () ? base->yMax   () : yMax   () );

  setZMin   ( base->zMin   () < zMin   () ? base->zMin   () : zMin   () );
  setZMax   ( base->zMax   () > zMax   () ? base->zMax   () : zMax   () );

  setRMax   ( base->rMax   () > rMax   () ? base->rMax   () : rMax   () );
  setRhoMax ( base->rhoMax () > rhoMax () ? base->rhoMax () : rhoMax () );
  //
  return checkBP();
};
// ============================================================================

// ============================================================================
// The END 
// ============================================================================

