// $Id: ClhepToStream.h,v 1.5 2005-08-30 11:42:36 cattanem Exp $
/// ===========================================================================
/// CVS tag $Name: not supported by cvs2svn $
/// ===========================================================================
#ifndef     DETDESC_CLHEP_TO_STREAM_H
#define     DETDESC_CLHEP_TO_STREAM_H 1 


#include "GaudiKernel/StreamBuffer.h"

#include "CLHEP/Vector/ThreeVector.h"                
#include "CLHEP/Vector/Rotation.h"                
#include "CLHEP/Geometry/Transform3D.h" 

/**  @file ClhepToStream.h
 *
 *   small collection of output operators of some 
 *   CLHEP objects to StreamBuffer 
 *   
 *   @author Vanya Belyaev Ivan.Belyaev@itep.ru
 *   @date xx/xx/xxxx
 */

/** @defgroup StreamBuffer StreamBuffer serializers
 *  @{ 
 */
inline StreamBuffer& operator<<( StreamBuffer& sb , const Hep3Vector& Vect ) 
{ return sb << Vect.x() << Vect.y() << Vect.z() ; };
inline StreamBuffer& operator>>( StreamBuffer& sb ,       Hep3Vector& Vect ) 
{ 
  double  x , y, z ;
  sb >> x >> y >> z ; 
  ///
  Vect.setX( x ) ; 
  Vect.setY( y ) ; 
  Vect.setZ( z ) ; 
  ///
  return sb; 
};

inline StreamBuffer& operator<<( StreamBuffer& sb , const HepRotation& Rot ) 
{
  Hep3Vector Axis  ; double     Angle ;
  Rot.getAngleAxis( Angle, Axis );
  return sb << Angle << Axis ; 
};
inline StreamBuffer& operator>>( StreamBuffer& sb ,       HepRotation& Rot ) 
{
  Hep3Vector Axis  ; double     Angle ;
  sb >> Angle >> Axis ; 
  Rot = HepRotation().rotate( Angle , Axis );
  return sb; 
};

inline StreamBuffer& operator<<( StreamBuffer& sb , const HepTransform3D& Tr ) 
{ return sb << Tr.getTranslation() << Tr.getRotation() ; };
inline StreamBuffer& operator>>( StreamBuffer& sb ,       HepTransform3D& Tr ) 
{ 
  HepRotation rot;
  Hep3Vector  pos;
  sb >> pos >> rot ; 
  Tr.setIdentity() ; 
  Tr = HepTransform3D( rot , pos ); 
  return sb; 
};
/** @} */ // End of StreamBuffer group

/// ===========================================================================
#endif  ///<  DETDESC_CLHEP_TO_STREAM_H
/// ===========================================================================
