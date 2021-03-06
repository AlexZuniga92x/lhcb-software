// $Id$
// ============================================================================
#ifndef LOKI_VERTICES1_H 
#define LOKI_VERTICES1_H 1
// ============================================================================
// Include files
// ============================================================================
// LoKiCore 
// ============================================================================
#include "LoKi/UniqueKeeper.h"
// ============================================================================
// LoKiPhys 
// ============================================================================
#include "LoKi/PhysTypes.h"
#include "LoKi/PhysRangeTypes.h"
#include "LoKi/VertexHolder.h"
// ============================================================================
// Event
// ============================================================================
#include "Event/RecVertex.h"
// ============================================================================
/** @file
 *
 *  This file is a part of LoKi project - 
 *    "C++ ToolKit  for Smart and Friendly Physics Analysis"
 *
 *  The package has been designed with the kind help from
 *  Galina PAKHLOVA and Sergey BARSUK.  Many bright ideas, 
 *  contributions and advices from G.Raven, J.van Tilburg, 
 *  A.Golutvin, P.Koppenburg have been used in the design.
 *
 *  By usage of this code one clearly states the disagreement 
 *  with the campain of Dr.O.Callot et al.: 
 *  ``No Vanya's lines are allowed in LHCb/Gaudi software.''
 *  
 *  @author Vanya BELYAEV ibelyaev@physics.syr.edu
 *  @date 2006-02-16 
 */
// ============================================================================
namespace LoKi 
{
  // ==========================================================================
  namespace  Vertices 
  {
    // ========================================================================
    /** @class VertexDistance
     *  evaluator of the distance
     *  between vertex and the 'vertex'
     *  @see LoKi::Cuts::VVDIST
     *  @see LoKi::Cuts::VVD
     *  @author Vanya Belyaev Ivan.Belyaev@itep.ru
     *  @date   2004-07-07
     */
    class GAUDI_API VertexDistance 
      : public LoKi::BasicFunctors<const LHCb::VertexBase*>::Function 
      , public LoKi::Vertices::VertexHolder 
    {
    public:
      // ======================================================================
      /// constructor 
      VertexDistance 
      ( const LHCb::VertexBase*  vertex ) ;
      /// constructor 
      VertexDistance 
      ( const LoKi::Point3D& vertex ) ;      
      /// constructor 
      VertexDistance 
      ( const LoKi::Vertices::VertexHolder& base ) ;
      /// destructor 
      virtual ~VertexDistance(){}
      /// clone method (mandatory!)
      virtual VertexDistance* clone() const 
      { return new VertexDistance(*this) ; }
      /** the only one essential method 
       *  @param v pointer to the vertex 
       *  @return the estimate of the distance between vertex and the 'vertex'
       */
      virtual result_type operator() ( argument v ) const 
      { return distance( v ) ; }
      /// OPTIONAL: the specific printout 
      virtual std::ostream& fillStream( std::ostream& s ) const 
      { return s << "VVDIST" ; }
      // ======================================================================
    public:
      // ======================================================================
      /** the only one essential method 
       *  @param v pointer to the vertex 
       *  @return the estimate of the distance between vertex and the 'vertex'
       */
      result_type         distance   ( argument v ) const ;
      /** the only one essential method 
       *  @param v pointer to the vertex 
       *  @return the estimate of the distance between vertex and the 'vertex'
       */
      result_type         chi2       ( argument v ) const ;
      // ======================================================================
    private:
      // ======================================================================
      /// default constructor is private 
      VertexDistance();
      // ======================================================================
    };
    // ========================================================================
    /** @class VertexSignedDistance
     *  evaluator of the distance
     *  between vertex and the 'vertex', signed 
     *  according (Zv-Z0)
     *  
     *  The actual evaluation is done by 
     *   LoKi::Vertices::VertexDistance 
     *  
     *  @see LoKi::Vertices::VertexDistance 
     *  @see LHCb::Vertex
     *  @see LoKi::Cuts::VVDSIGN
     *
     *  @author Vanya Belyaev Ivan.Belyaev@itep.ru
     *  @date   2004-07-07
     */
    class GAUDI_API VertexSignedDistance : public LoKi::Vertices::VertexDistance 
    {
    public:
      // ======================================================================
      /// constructor 
      VertexSignedDistance 
      ( const LHCb::VertexBase*  vertex ) 
        : LoKi::Vertices::VertexDistance( vertex ) {}
      /// constructor 
      VertexSignedDistance 
      ( const LoKi::Point3D& vertex )       
        : LoKi::Vertices::VertexDistance( vertex  ) {}
      /// constructor 
      VertexSignedDistance 
      ( const LoKi::Vertices::VertexHolder& base ) 
        : LoKi::Vertices::VertexDistance( base   ) {}
      /// destructor 
      virtual ~VertexSignedDistance() {}
      /// clone method (mandatory!)
      virtual VertexSignedDistance* clone() const 
      { return new VertexSignedDistance ( *this ) ; }
      /** the only one essential method 
       *  @param v pointer to the vertex 
       *  @return the estimate of the distance between vertex and the 'vertex'
       */
      virtual result_type operator() ( argument v ) const 
      { return signedDistance ( v ) ; }
      /// OPTIONAL: the specific printout 
      virtual std::ostream& fillStream( std::ostream& s ) const 
      { return s << "VVDSIGN" ; }
      /** the only one essential method 
       *  @param p pointer to the vertex 
       *  @return the estimate of the distance between vertex and the 'vertex'
       */
      result_type signedDistance   ( argument v ) const ;
      // ======================================================================
    private :
      // ======================================================================
      /// default constructor is private 
      VertexSignedDistance();
      // ======================================================================
    };
    // ========================================================================
    /** @class VertexDeltaZ 
     *  Trivial functor to evaluate the delta-Z with respect to "the vertex"
     *  @see LoKi::Cuts::VDZ 
     *  @author Vanya BELYAEV Ivan.Belyaev@inkhef.nl
     *  @date 2009-03-11
     */
    class GAUDI_API VertexDeltaZ : public LoKi::Vertices::VertexDistance 
    {
    public:
      // ======================================================================
      /// constructor 
      VertexDeltaZ 
      ( const LHCb::VertexBase*  vertex ) 
        : LoKi::Vertices::VertexDistance( vertex ) {}
      /// constructor 
      VertexDeltaZ 
      ( const LoKi::Point3D& vertex )       
        : LoKi::Vertices::VertexDistance( vertex  ) {}
      /// constructor 
      VertexDeltaZ  
      ( const LoKi::Vertices::VertexHolder& base ) 
        : LoKi::Vertices::VertexDistance( base   ) {}
      /// destructor 
      virtual ~VertexDeltaZ() {}
      /// clone method (mandatory!)
      virtual VertexDeltaZ* clone() const 
      { return new VertexDeltaZ ( *this ) ; }
      /** the only one essential method 
       *  @param v pointer to the vertex 
       *  @return the estimate of the distance between vertex and the 'vertex'
       */
      virtual result_type operator() ( argument v ) const 
      { return deltaZ ( v ) ; }
      /// OPTIONAL: the specific printout 
      virtual std::ostream& fillStream( std::ostream& s ) const 
      { return s << "VDZ" ; }
      /** the only one essential method 
       *  @param p pointer to the vertex 
       *  @return the estimate of the distance between vertex and the 'vertex'
       */
      result_type deltaZ( argument v ) const ;
      // ======================================================================
    private :
      // ======================================================================
      /// default constructor is private 
      VertexDeltaZ();                         // default constructor is private 
      // ======================================================================
    };
    // ========================================================================
    /** @class VertexAbsDeltaZ 
     *  Trivial functor to evaluate the abs(deltaZ) with respect to "the vertex"
     *  @see LoKi::Cuts::VABSDZ 
     *  @author Vanya BELYAEV Ivan.Belyaev@inkhef.nl
     *  @date 2009-03-11
     */
    class GAUDI_API VertexAbsDeltaZ : public LoKi::Vertices::VertexDeltaZ
    {
    public:
      // ======================================================================
      /// constructor 
      VertexAbsDeltaZ 
      ( const LHCb::VertexBase*  vertex ) 
        : LoKi::Vertices::VertexDeltaZ ( vertex ) {}
      /// constructor 
      VertexAbsDeltaZ 
      ( const LoKi::Point3D& vertex )       
        : LoKi::Vertices::VertexDeltaZ ( vertex  ) {}
      /// constructor 
      VertexAbsDeltaZ  
      ( const LoKi::Vertices::VertexHolder& base ) 
        : LoKi::Vertices::VertexDeltaZ ( base   ) {}
      /// destructor 
      virtual ~VertexAbsDeltaZ() {}
      /// clone method (mandatory!)
      virtual VertexAbsDeltaZ* clone() const 
      { return new VertexAbsDeltaZ ( *this ) ; }
      /** the only one essential method 
       *  @param v pointer to the vertex 
       *  @return the estimate of the distance between vertex and the 'vertex'
       */
      virtual result_type operator() ( argument v ) const 
      { return absDeltaZ ( v ) ; }
      /// OPTIONAL: the specific printout 
      virtual std::ostream& fillStream( std::ostream& s ) const 
      { return s << "VABSDZ" ; }
      /** the only one essential method 
       *  @param p pointer to the vertex 
       *  @return the estimate of the distance between vertex and the 'vertex'
       */
      result_type absDeltaZ ( argument v ) const ;
      // ======================================================================
    private :
      // ======================================================================
      /// default constructor is private 
      VertexAbsDeltaZ();                      // default constructor is private 
      // ======================================================================
    };
    // ========================================================================    
    /** @class VertexChi2Distance
     *  evaluator of the distance in chi2 units  
     *  between vertex and the 'vertex'
     *  @see LoKi::Cuts::VVDCHI2
     *  @author Vanya Belyaev Ivan.Belyaev@itep.ru
     *  @date   2004-07-07
     */
    class GAUDI_API VertexChi2Distance : public LoKi::Vertices::VertexDistance 
    {
    public:
      // ======================================================================
      /// constructor 
      VertexChi2Distance 
      ( const LHCb::VertexBase*  vertex ) 
        : LoKi::Vertices::VertexDistance ( vertex ) {}      
      /// constructor 
      VertexChi2Distance 
      ( const LoKi::Point3D& vertex ) 
        : LoKi::Vertices::VertexDistance ( vertex ) {}
      /// constructor 
      VertexChi2Distance 
      ( const LoKi::Vertices::VertexHolder& base ) 
        : LoKi::Vertices::VertexDistance ( base ) {}
      /// destructor 
      virtual ~VertexChi2Distance() {} ;
      /// clone method (mandatory!)
      virtual VertexChi2Distance* clone() const 
      { return new VertexChi2Distance ( *this ) ; }
      /** the only one essential method 
       *  @param v pointer to the vertex 
       *  @return the estimate of the distance between vertex and the 'vertex'
       */
      virtual result_type operator() ( argument v ) const 
      { return chi2 ( v ) ; }
      /// OPTIONAL: the specific printout 
      virtual std::ostream& fillStream ( std::ostream& s ) const 
      { return s << "VVDCHI2" ; }
      // ======================================================================
    private:
      // ======================================================================
      /// default constructor is private 
      VertexChi2Distance();
      // ======================================================================
    };
    // ========================================================================    
    /** @class MinVertexDistance
     *  class for evaluation of minimal GEOMETRY distance 
     *  between the vertex and other vertices 
     *  @see LoKi::Cuts::MINVVD
     *  @author Vanya BELYAEV Ivan.Belyaev@itep.ru
     *  @date   2004-07-08
     */
    class GAUDI_API MinVertexDistance 
      : public LoKi::Vertices::VertexDistance
      , public LoKi::UniqueKeeper<LHCb::VertexBase>
    {
    public:
      // ======================================================================
      /** constructor from container of vertices 
       *  @param vs container of vertices 
       */
      MinVertexDistance 
      ( const LHCb::VertexBase::ConstVector& vs ) ;
      /** constructor from container of vertices 
       *  @param vs container of vertices 
       */
      MinVertexDistance 
      ( const LHCb::Vertex::ConstVector& vs ) ;
      /** constructor from container of vertices 
       *  @param vs container of primary vertices 
       */
      MinVertexDistance 
      ( const LHCb::RecVertex::ConstVector& vs ) ;
      /** constructor from container of vertices 
       *  @param vs container of primary vertices 
       */
      MinVertexDistance 
      ( const LHCb::RecVertex::Container*   vs  ) ;
      /** constructor from container of vertices 
       *  @param vs container of primary vertices 
       */
      MinVertexDistance 
      ( const LoKi::PhysTypes::VRange& vs ) ;
      /** templated constructor from arbitrary sequence 
       *  of vertices and helper object 
       *  @param first begin iterator of arbitrary sequence of primary vertices
       *  @param last  end iterator of arbitrary sequence of primary vertices
       */
      template <class VERTEX>
      MinVertexDistance
      ( const LoKi::Keeper<VERTEX>& keeper ) 
        : LoKi::Vertices::VertexDistance ( LoKi::Point3D() ) 
        , LoKi::UniqueKeeper<LHCb::VertexBase>( keeper.begin() , keeper.end() )
      {}
      /** templated constructor from arbitrary sequence 
       *  of vertices and helper object 
       *  @param first begin iterator of arbitrary sequence of primary vertices
       *  @param last  end iterator of arbitrary sequence of primary vertices
       */
      template <class VERTEX>
      MinVertexDistance
      ( const LoKi::UniqueKeeper<VERTEX>& keeper ) 
        : LoKi::Vertices::VertexDistance ( LoKi::Point3D() ) 
        , LoKi::UniqueKeeper<LHCb::VertexBase>( keeper.begin() , keeper.end() )
      {}
      /** templated constructor from arbitrary sequence 
       *  of vertices and helper object 
       *  @param first begin iterator of arbitrary sequence of primary vertices
       *  @param last  end iterator of arbitrary sequence of primary vertices
       */
      template <class VERTEX>
      MinVertexDistance 
      ( VERTEX                          first , 
        VERTEX                          last  )
        : LoKi::Vertices::VertexDistance ( LoKi::Point3D() ) 
        , LoKi::UniqueKeeper<LHCb::VertexBase>( first , last  )
      {}
      /// MANDATORY: destructor 
      virtual ~MinVertexDistance() {}
      /// MANDATORY: clone method ("virtual constructor")
      virtual  MinVertexDistance* clone() const 
      { return new MinVertexDistance(*this) ; }
      /// MANDATORY: the only one essential method 
      virtual result_type operator() ( argument v ) const 
      { return minvd ( v ) ; }
      /// OPTIONAL: the specific printout 
      virtual std::ostream& fillStream( std::ostream& s ) const 
      { return s << "MINVVD" ; }
      // ======================================================================
    public:
      // ======================================================================
      result_type minvd     ( argument v ) const ;
      result_type minvdchi2 ( argument v ) const ;
      // ======================================================================
    private:
      // ======================================================================
      /// default constructor is disabled 
      MinVertexDistance () ;
      // ======================================================================
    };
    // ========================================================================
    /** @class MinVertexChi2Distance
     *  class for evaluation of minimal chi2 distance 
     *  between the vertex and other vertices 
     *  @see LoKi::Cuts::MINVVDCHI2 
     *  @author Vanya ELYAEV Ivan.Belyaev@itep.ru
     *  @date   2004-07-08
     */
    class GAUDI_API MinVertexChi2Distance 
      : public LoKi::Vertices::MinVertexDistance
    {
    public:
      // ======================================================================
      /** constructor from container of vertices 
       *  @param vs container of vertices 
       */
      MinVertexChi2Distance 
      ( const LHCb::VertexBase::ConstVector& vs ) ;
      /** constructor from container of vertices 
       *  @param vs container of vertices 
       */
      MinVertexChi2Distance 
      ( const LHCb::Vertex::ConstVector& vs ) ;
      /** constructor from container of vertices 
       *  @param vs container of primary vertices 
       */
      MinVertexChi2Distance 
      ( const LHCb::RecVertex::ConstVector& vs ) ;
      /** constructor from container of vertices 
       *  @param vs container of primary vertices 
       */
      MinVertexChi2Distance 
      ( const LHCb::RecVertex::Container*   vs  ) ;
      /** constructor from container of vertices 
       *  @param vs container of primary vertices 
       */
      MinVertexChi2Distance 
      ( const LoKi::PhysTypes::VRange& vs ) ;
      /** templated constructor from arbitrary sequence 
       *  of vertices and helper object 
       *  @param first begin iterator of arbitrary sequence of primary vertices
       *  @param last  end iterator of arbitrary sequence of primary vertices
       */
      template <class VERTEX>
      MinVertexChi2Distance 
      ( const LoKi::Keeper<VERTEX>& keeper )
        : LoKi::Vertices::MinVertexDistance ( keeper ) 
      {} 
      /** templated constructor from arbitrary sequence 
       *  of vertices and helper object 
       *  @param first begin iterator of arbitrary sequence of primary vertices
       *  @param last  end iterator of arbitrary sequence of primary vertices
       */
      template <class VERTEX>
      MinVertexChi2Distance 
      ( const LoKi::UniqueKeeper<VERTEX>& keeper )
        : LoKi::Vertices::MinVertexDistance ( keeper ) 
      {} 
      /** templated constructor from arbitrary sequence 
       *  of vertices and helper object 
       *  @param first begin iterator of arbitrary sequence of primary vertices
       *  @param last  end iterator of arbitrary sequence of primary vertices
       */
      template <class VERTEX>
      MinVertexChi2Distance 
      ( VERTEX                          first , 
        VERTEX                          last  )
        : LoKi::Vertices::MinVertexDistance ( first , last ) 
      {} 
      /// MANDATORY: destructor 
      virtual ~MinVertexChi2Distance() {}
      /// MANDATORY: clone method ("virtual constructor")
      virtual  MinVertexChi2Distance* clone() const 
      { return new MinVertexChi2Distance ( *this ) ; }
      /// MANDATORY: the only one essential method 
      virtual result_type operator() ( argument v ) const 
      { return minvdchi2 ( v ) ; }
      /// OPTIONAL: the specific printout 
      virtual std::ostream& fillStream ( std::ostream& s ) const 
      { return s << "MINVVDCHI2" ; }
      // ======================================================================
    private:
      // ======================================================================
      /// default constructor is disabled 
      MinVertexChi2Distance () ;
      // ======================================================================
    };
    // ========================================================================
  } //                                          end of namespace LoKi::Vertices 
  // ==========================================================================
} //                                                      end of namespace LoKi
// ============================================================================
//                                                                      The END 
// ============================================================================
#endif // LOKI_VERTICES1_H
// ============================================================================
