// $Id: ICheckOverlap.h,v 1.3 2005-02-21 08:56:36 pkoppenb Exp $
#ifndef DAVINCIKERNEL_ICHECKOVERLAP_H 
#define DAVINCIKERNEL_ICHECKOVERLAP_H 1

// Include files
// from STL
#include <string>

// from Gaudi
#include "GaudiKernel/IAlgTool.h"

// Forward declarations
class Vertex;
class Particle;
typedef std::vector<const Particle*> ConstParticleVector ;

// Declaration of the interface ID ( interface id, major version, minor version)
static const InterfaceID IID_ICheckOverlap("ICheckOverlap", 2, 0);

/** @class ICheckOverlap ICheckOverlap.h
 *  
 *  Tool to check if more than one particle originate 
 *  from the same protoparticle.
 *
 *  @author Jose Helder Lopes
 *  @date   28/06/2002
 */

class ICheckOverlap : virtual public IAlgTool {
  
public:
  /// Retrieve interface ID
  static const InterfaceID& interfaceID() { return IID_ICheckOverlap; }
  
  
  /// Check for duplicate use of a protoparticle to produce particles.
  /// Argument: parts is a vector of pointers to particles.
  ///  Create an empty vector of pointers to protoparticles. 
  ///  Call the real checking method.
  
  virtual bool ICheckOverlap::foundOverlap( ConstParticleVector & parts  ) = 0;
  
  /// backward-compatible methods
  virtual bool ICheckOverlap::foundOverlap( ParticleVector & parts  ) = 0;
  /// backward-compatible methods
  virtual bool ICheckOverlap::foundOverlap( const ParticleVector & parts  ) = 0;
  

  /// Check for duplicate use of a protoparticle to produce particles.
  /// Arguments: particle1 up to particle4 are pointers to particles.
  ///  Create a ParticleVector and fill it with the input particles.
  ///  Create an empty vector of pointers to protoparticles. 
  ///  Call the real checking method.
  
  virtual bool foundOverlap( const Particle* particle1, 
                             const Particle* particle2 ) = 0;
  
  virtual bool foundOverlap( const Particle* particle1, 
                             const Particle* particle2, 
                             const Particle* particle3) = 0;
  
  virtual bool foundOverlap( const Particle* particle1, 
                             const Particle* particle2, 
                             const Particle* particle3, 
                             const Particle* particle4) = 0;

  /// Check for duplicate use of a protoparticle to produce particles.
  /// Continue a previous check using the contents of the vector of pointers
  /// to protoparticles.(Most intended for internal use by the recursive calls).
  /// Arguments: parts is a vector of pointer to particles. 
  ///            proto is a vector of pointers to protoparticles.
  ///  Real checking method. Scans the tree headed by parts. Add each
  ///  protoparticle to proto if it is a new one. Returns true otherwise.
  ///  If called directly by the user, it will continue a previous check, 
  ///  not start a new one!
  virtual bool foundOverlap( ConstParticleVector& parts,
                             std::vector<const ContainedObject* > 
                             & proto ) = 0;  
protected:
  
private:
  
};
#endif // DAVINCIKERNEL_ICHECKOVERLAP_H
