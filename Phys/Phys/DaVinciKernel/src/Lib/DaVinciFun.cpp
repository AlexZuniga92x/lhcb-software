// $Id: DaVinciFun.cpp,v 1.4 2009-11-05 10:19:04 jpalac Exp $
// Include files 
// Gaudi
#include "GaudiKernel/SmartIF.h"
// DaVinciInterfaces
#include "Kernel/ISetInputParticles.h"
// local
#include "Kernel/DaVinciFun.h"

//-----------------------------------------------------------------------------
// Implementation file for DaVinciFun
//
// 2009-08-02 : Juan Palacios
//-----------------------------------------------------------------------------
namespace DaVinci 
{
  
  const LHCb::VertexBase* bestVertexBase(const Particle2Vertex::Table::Range& range) 
  {
    if (range.empty()) return 0;
    return range.back().to();
  }

  const LHCb::RecVertex* bestRecVertex(const Particle2Vertex::Table::Range& range) 
  {
    if (range.empty()) return 0;
    return dynamic_cast<const LHCb::RecVertex*>( range.back().to() );
  }

  namespace Utils 
  {
    // ========================================================================
    /*  set the input particles for some component
     *  @param component the component inself
     *  @param input the intut data 
     *  @return status code 
     *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
     *  @date   2008-07-11
     */
    // ========================================================================
    StatusCode setInput 
    ( IInterface*                        component , 
      const LHCb::Particle::ConstVector& input     ) 
    {
      if ( 0 == component ) { return StatusCode ( 300 ) ; }        // RETURN 
      // get the interface 
      SmartIF<ISetInputParticles> alg ( component ) ;
      // valid ? 
      if ( !alg           ) { return StatusCode ( 301 ) ; }        // RETURN  
      // use the valid inyterface 
      return alg->setInput ( input ) ;                             // RETURN 
    }

    // ========================================================================
    template <class PREDICATE> 
    void findDecayTree(const LHCb::Particle*        head,
                       LHCb::Particle::ConstVector& particles,
                       LHCb::Vertex::ConstVector&   vertices,
                       PREDICATE& truncate)
    {

      if (truncate(head)) return;
      
      if ( particles.end() == std::find ( particles.begin() , 
                                          particles.end() , 
                                          head ) ) {
        particles.push_back ( head ) ; 
      }
      //
      if ( 0 != head->endVertex() 
           && vertices.end() == std::find ( vertices.end() , 
                                            vertices.end() , 
                                            head->endVertex() ) ) {
        vertices.push_back( head->endVertex() ); // save Vertex
      }
  
      // Loop on daughters
      SmartRefVector<LHCb::Particle>::const_iterator iDaughter = 
        head->daughters().begin(); 
      SmartRefVector<LHCb::Particle>::const_iterator iDaughterEnd = 
        head->daughters().end(); 
      for ( ; iDaughter!=iDaughterEnd; ++iDaughter){
        findDecayTree( *iDaughter, particles, vertices, truncate );
      }
  
      // loop over outgoing particles
      if ( 0 != head->endVertex() ) {
        SmartRefVector<LHCb::Particle>::const_iterator iPart = 
          head->endVertex()->outgoingParticles().begin();
        SmartRefVector<LHCb::Particle>::const_iterator iPartEnd = 
          head->endVertex()->outgoingParticles().end();
        for ( ; iPart!= iPartEnd; ++iPart){
          findDecayTree( *iPart, particles, vertices, truncate );
        }
      }
  
      return;

    }


  } // namespace Utils
  
} // namespace DaVinci 
