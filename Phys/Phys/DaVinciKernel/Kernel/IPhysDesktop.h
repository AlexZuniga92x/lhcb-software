// $Id: IPhysDesktop.h,v 1.13 2006-08-17 16:40:59 jpalac Exp $
#ifndef DAVINCIKERNEL_IPHYSDESKTOP_H 
#define DAVINCIKERNEL_IPHYSDESKTOP_H 1

// Include files
#include "GaudiKernel/IAlgTool.h"
#include "Event/RecVertex.h"
#include "Event/Particle.h"
#include "Kernel/Particle2Vertex.h"
// Forward declarations
class StatusCode;

// Declaration of the interface ID ( interface id, major version, minor version)
static const InterfaceID IID_IPhysDesktop("IPhysDesktop", 1, 2);


/** @class IPhysDesktop IPhysDesktop.h Kernel/IPhysDesktop.h
 *  Interface for the Desk Top
 *  upon which the user keeps his particles and vertices
 *  
 *  @todo Method to create a table associating Particles and the PV that have been used
 *
 *  @author Sandra Amato
 *  @date   18/02/2002
 */
class IPhysDesktop : virtual public IAlgTool {

public:
  /// Retrieve interface ID
  static const InterfaceID& interfaceID() { return IID_IPhysDesktop; }

  /// Fill the particle and vertex containers. Called by DVAlgorithm::sysExecute()
  virtual StatusCode getEventInput() = 0;

  /// Retrieve the particles containers
  virtual const LHCb::Particle::ConstVector particles() = 0;

  /// Retrieve the PV from vertex container
  virtual const LHCb::RecVertex::ConstVector& primaryVertices() = 0;

  /// Retrieve the secondary vertices
  virtual const LHCb::Vertex::ConstVector secondaryVertices() = 0;

  /// Add the particles  to the Desktop
  virtual const LHCb::Particle* save( const LHCb::Particle* input=0 ) = 0;

  /// Add the vertices  to the Desktop
  virtual const LHCb::Vertex* save( const LHCb::Vertex* input=0 ) = 0;

  /// Save the particles to the TES
  virtual StatusCode saveDesktop() = 0;
 
  /// Save the particles to the TES (used by HLT)
  virtual StatusCode saveDesktop(const LHCb::Particle::ConstVector&,
                                 LHCb::Vertex::ConstVector& ) = 0;
 
  /// Save a vector of Particles
  /// If a particle is composite its descendents are also saved
  virtual StatusCode saveTrees( const LHCb::Particle::ConstVector& ) = 0;

  /// Save all Particles with a given particleID code
  virtual StatusCode saveTrees( int pid ) = 0;

  /// Clone all particles given by a list. This duplicates information 
  /// on the TES and should be used only when necessary. (Used by Filters)
  virtual StatusCode cloneTrees( const LHCb::Particle::ConstVector& ) = 0;

  /// Impose output location
  virtual void imposeOutputLocation(const std::string& outputLocationString) = 0;

  /// Get output location
  virtual std::string getOutputLocation() = 0 ;
  
  /// Find all particles & vertices in a tree. 
  // virtual void findAllTree( LHCb::Particle*, LHCb::Particle::ConstVector&, LHCb::Vertex::ConstVector& ) = 0;
  // virtual void findAllTree( LHCb::Vertex*, LHCb::Particle::ConstVector&, LHCb::Vertex::ConstVector& )= 0;

  /// Make sure the PhysDesktop has written out the container
  virtual StatusCode writeEmptyContainerIfNeeded() = 0 ;

  /// Get the vertex with the highest weight in the association
  /// between LHCb::Particle and LHCb::VertexBase
  virtual LHCb::Vertex* relatedVertex(const LHCb::Particle* part) const = 0;
  
  /// Establish a relation between an LHCb::Particle and an LHCb::VertexBase
  virtual void relate(const LHCb::Particle* part, 
                      const LHCb::VertexBase* vert,
                      Particle2Vertex::Weight weight) const = 0;

  /// Obtain the weight relating an LHCb::Particle and an LHCb::VertexBase
  virtual Particle2Vertex::Weight weight(const LHCb::Particle* part, 
                                         const LHCb::VertexBase* vert ) const = 0;
  
  /// Obtain a range of weighted LHCb::VertexBase related to an LHCb::Particle
  virtual Particle2Vertex::Range particle2Vertices(const LHCb::Particle* part ) const =0;
  
  
protected:

private:

};
#endif // DAVINCIKERNEL_IPHYSDESKTOP_H


