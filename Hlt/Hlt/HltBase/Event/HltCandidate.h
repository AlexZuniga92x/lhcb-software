// $Id: HltCandidate.h,v 1.8 2010-08-30 13:00:44 amazurov Exp $
// ============================================================================
#ifndef HltEvent_Candidate_H_
#define HltEvent_Candidate_H_ 1
// ============================================================================
// Include files
// ============================================================================
// STD & STL
// ============================================================================
#include <vector>
#include <ostream>
// #include <type_traits>
// ============================================================================
// GaudiKernel
// ============================================================================
#include "GaudiKernel/ContainedObject.h"
#include "GaudiKernel/ObjectVector.h"
#include "GaudiKernel/SmartRefVector.h"
#include "GaudiKernel/NamedRange.h"
#include "GaudiKernel/INamedInterface.h"
// ============================================================================
// HltBase
// ============================================================================
#include "Event/HltStage.h"
#include "Event/NamedEntry.h"
// ============================================================================
#ifdef _WIN32
// ============================================================================
// Avoid conflict of Windows macro with std::max
// ============================================================================
#ifndef NOMINMAX
#define NOMINMAX
#endif
#endif
// ============================================================================
// Allocators from  Gaudi
// ============================================================================
#include "GaudiKernel/boost_allocator.h"
// ============================================================================
namespace Hlt
{
// ==========================================================================
// Forward declarations
class Stage;
class Candidate;
// ==========================================================================
// Namespace for locations in TDS
namespace CandidateLocation
{
// =========================================================================
/** @var Default
 *  The defaull TES-location of HLT-candidates
 */
const std::string Default = "Hlt/Candidates";
// =========================================================================
} //                                   end of namespace Hlt::CandidateLocation
  // ===========================================================================
  /** @class Candidate Event/Candidate.h
*
* Shows mutation history of tracks, particles,...
*  (which are wrapped by Stage-objects)
* Store processing history: list of algorithms (Workers)
*
* @author alexander.mazurov@gmail.com
*
* @example Test.py
* @example Ex1Writer.cpp
* @example Ex1Updater.cpp
* @example Ex1Observer.cpp
*
* created Wed May 26 15:35:22 2010
*
*/
class GAUDI_API Candidate : public ContainedObject
{
  public:
    // ========================================================================
    /// typedef for std::vector of Candidate
    typedef std::vector<Candidate*> Vector;
    typedef std::vector<const Candidate*> ConstVector;
    /// typedef for ObjectVector of Candidate
    typedef ObjectVector<Candidate> Container;
    /// The container type for objects
    typedef SharedObjectsContainer<Hlt::Candidate> Selection;
    /// For uniform access to containers in TES (KeyedContainer,SharedContainer)
    typedef Gaudi::NamedRange_<ConstVector> Range;
    /// Phases
    typedef SmartRefVector<Hlt::Stage> Stages;
    /// Phases iterator (const)
    typedef SmartRefVector<Hlt::Stage>::const_iterator StagesConstIterator;
    /// Worker (Algorithm name)
    typedef Hlt::NamedEntry Worker;
    /// Workers (Algorithm names)
    typedef std::vector<Worker> Workers;
    /// Workers iterator (const)
    typedef Workers::const_iterator WorkersConstIterator;
    // ========================================================================
  public:
    // ========================================================================
    /// Destructor
    virtual ~Candidate();
    // ========================================================================
  public:
    // ========================================================================
    /// Retrieve pointer to class definition structure     (virtual)
    virtual const CLID& clID() const
    {
        return Hlt::Candidate::classID();
    }
    /// Retrieve pointer to class definition structure     (static)
    static const CLID& classID();
    // ========================================================================
  public: // standard accessors
    // ========================================================================
    /// Add worker
    void addToWorkers( const std::string& worker )
    {
        m_workers.push_back( worker );
    }
    // TODO: once we get rid of gccxml, drop version above...
    // void addToWorkers( std::string worker )
    // { m_workers.emplace_back( std::move(worker) ) ; }
    /// Add worker
    void addToWorkers( const INamedInterface* worker );
    /// Get the initial stage
    const Hlt::Stage* initiatorStage() const
    {
        return !m_stages.empty() ? m_stages.front().data() : 0;
    }
    /// Get the current stage
    const Hlt::Stage* currentStage() const
    {
        return !m_stages.empty() ? m_stages.back().data() : 0;
    }
    /// Last worker (algorithm name)
    const Worker& lastWorker() const;
    /// print
    virtual std::ostream& fillStream( std::ostream& s ) const;
    /// Retrieve const  list of workers
    const Workers& workers() const
    {
        return m_workers;
    }
    /// Retrieve (const)  Reference to the stages
    inline const SmartRefVector<Hlt::Stage>& stages() const
    {
        return m_stages;
    }
    /// Retrieve (non-const)  Reference to the stages
    inline SmartRefVector<Hlt::Stage>& stages()
    {
        return m_stages;
    }
    /// Update  Reference to the stages
    void setStages( const SmartRefVector<Hlt::Stage>& value );
    /// Add to (pointer) reference to the stages
    void addToStages( const Hlt::Stage* value );
    /// Remove from  reference to the stages
    void removeFromStages( const Hlt::Stage* value );
    /// Clear  Reference to the stages
    void clearStages()
    {
        m_stages.clear();
    }
    /// Has stage ?
    bool hasStage( const Hlt::Stage* stage ) const;
    // ========================================================================
  public: // standard
   // ========================================================================
   /** easy accessors to various  underlying objects
    *
    *  @code
    *
    *   const Hlt::Candidate* candidate = ... ;
    *
    *   // get the stage
    *  const Hlt::Stage* stage = candidate->get<Hlt::Stage>() ;
    *
    *   // get the L0diMuon
    *  const Hlt::L0Dimuon* dimuon = candidate->get<Hlt::Stage>() ;
    *
    *  @endcode
    *
    *  @param slot the slot to be used for data extraction
    *     - 0 corresponds to the current stage ,
    *     - negative value corresponds to initiator stage
    *     - positive value corresponds to step-back in history
    *  @return the obejct
    */
  private :
  /** get the stage from the Hlt Candidate -- public version is 'get<Stage>
   *  @param slot the slot to be used for data extraction 
   *     - 0 corresponds to the current stage , 
   *     - negative value corresponds to initiator stage 
   *     - positive value corresponds to step-back in history
   *  @return the stage 
   */
    const Hlt::Stage* stage( int slot ) const {
        return ( slot == 0 ) ? currentStage()      // current
             : ( slot <  0 ) ? initiatorStage()    // initiator
             : ( unsigned(slot) < m_stages.size() ) ? (m_stages.end()-slot-1)->data() // step-back
             : NULL ; // gccxml doesn't like nullptr..
    }
  public:
  // ===========================================================================
  /** get the underlying object from the stage.
   *  any type support by the stage can be 'got'.
   * 
   *  @param slot the slot to be used for data extraction 
   *     - 0 corresponds to the current stage , 
   *     - negative value corresponds to initiator stage 
   *     - positive value corresponds to step-back in history
   *  @return the track
   */
      template <typename T> const T* get( int slot = 0 ) const 
#ifdef __GCCXML__
          ;
#else
      {
        const Hlt::Stage* _stage = this->stage( slot ) ;
        return std::is_same<typename std::remove_cv<T>::type,Hlt::Stage>::value ? (const T*)_stage 
             : ( _stage ? _stage->get<T>() : 0 );                // RETURN 
      }
#endif
    // ========================================================================
  public: // python-friendly access
          // ========================================================================
          /** easy accessor to various  underlying objects through bypassin
     *  of stage-object
     *  @see Hlt::Stage::get_
     *  @see Hlt::Candidate::get
     *  @param slot the slot to be used for data extraction
     *     - 0 corresponds to the current stage ,
     *     - negative value corresponds to initiator stage
     *     - positive value corresponds to step-back in history
     *  @return the obejct
     */
    const ContainedObject *get_( int slot = 0 ) const;
    // ========================================================================
  public: // standard
          // ========================================================================
#ifndef GOD_NOALLOC
    /// operator new
    static void* operator new( size_t size )
    {
        return (
            sizeof( Candidate ) == size
                ? boost::singleton_pool<Candidate, sizeof( Candidate )>::malloc()
                : ::operator new( size ) );
    }
    /// placement operator new
    /// it is needed by libstdc++ 3.2.3 (e.g. in std::vector)
    /// it is not needed in libstdc++ >= 3.4
    static void* operator new( size_t size, void* pObj )
    {
        return ::operator new( size, pObj );
    }
    /// operator delete
    static void operator delete( void* p )
    {
        boost::singleton_pool<Candidate, sizeof( Candidate )>::is_from( p )
            ? boost::singleton_pool<Candidate, sizeof( Candidate )>::free( p )
            : ::operator delete( p );
    }
    /// placement operator delete
    /// not sure if really needed, but it does not harm
    static void operator delete( void* p, void* pObj )
    {
        ::operator delete( p, pObj );
    }
// ========================================================================
#endif
    // ========================================================================
  private: // the data
    // ========================================================================
    /// list of workers
    Workers m_workers; // list of workers
    /// list of stages
    SmartRefVector<Hlt::Stage> m_stages; // Reference to the stages
    // ========================================================================
}; // class Candidate
// ============================================================================
/// Definition of vector container type for Candidate
typedef Hlt::Candidate::Container Candidates;
// ============================================================================
} //                                                       end of namespace Hlt
// ============================================================================
namespace Gaudi
{
// ============================================================================
namespace Utils
{
// ============================================================================
/// print-out
GAUDI_API
std::ostream& toStream( const Hlt::Candidate* c, std::ostream& s );
/// print-out
GAUDI_API
std::ostream& toStream( const Hlt::Stage* c, std::ostream& s );
// ============================================================================
} //                                              end of namespace Gaudi::Utils
  // ==========================================================================
} //                                                     end of namespace Gaudi
// ============================================================================
// HltBase
// ============================================================================
#include "Event/HltStage.h"
// ============================================================================
namespace Hlt
{
// ============================================================================
// printout
inline std::ostream& operator<<( std::ostream& str, const Candidate& obj )
{
    return obj.fillStream( str );
}
// printout
inline std::ostream& operator<<( std::ostream& str, const Candidate* obj )
{
    return obj ? obj->fillStream( str ) : ( str << "<NULL>" );
}
// ============================================================================
} //                                                       end of namespace Hlt
// ============================================================================
namespace Hlt
{
// ==========================================================================
/** @struct CandidateType2Id
 *  Helper structure to map candidate type to ID
 *  @author Vanya BELYAEV Ivan.Belyaev@cern.ch
 *  @date 2010-11-21
 */
template <class TYPE>
struct CandidateType2Id;
// ==========================================================================
template <class TYPE>
struct CandidateType2Id<const TYPE>
{
    enum {
        value = CandidateType2Id<TYPE>::value
    };
};
template <class TYPE>
struct CandidateType2Id<TYPE*>
{
    enum {
        value = CandidateType2Id<TYPE>::value
    };
};
template <class TYPE>
struct CandidateType2Id<TYPE&>
{
    enum {
        value = CandidateType2Id<TYPE>::value
    };
};
template <class TYPE>
struct CandidateType2Id<SmartRef<TYPE> >
{
    enum {
        value = CandidateType2Id<TYPE>::value
    };
};
// ==========================================================================
template <>
struct CandidateType2Id<LHCb::L0MuonCandidate>
{
    enum {
        value = Hlt::Stage::L0Muon
    };
};
template <>
struct CandidateType2Id<LHCb::L0CaloCandidate>
{
    enum {
        value = Hlt::Stage::L0Calo
    };
};
template <>
struct CandidateType2Id<Hlt::L0DiMuonCandidate>
{
    enum {
        value = Hlt::Stage::L0DiMuon
    };
};
template <>
struct CandidateType2Id<LHCb::Track>
{
    enum {
        value = Hlt::Stage::HltTrack
    };
};
template <>
struct CandidateType2Id<LHCb::RecVertex>
{
    enum {
        value = Hlt::Stage::HltVertex
    };
};
template <>
struct CandidateType2Id<Hlt::MultiTrack>
{
    enum {
        value = Hlt::Stage::HltMultiTrack
    };
};
template <>
struct CandidateType2Id<Hlt::Stage>
{
    enum {
        value = Hlt::Stage::HltStage
    };
};
// ==========================================================================
/** @struct CandidateId2Type
 *  Helper structure to map candidate ID to Type
 *  @author Vanya BELYAEV Ivan.Belyaev@cern.ch
 *  @date 2010-11-21
 */
template <unsigned int>
struct CandidateId2Type;
//
template <>
struct CandidateId2Type<Hlt::Stage::L0Muon>
{
    typedef LHCb::L0MuonCandidate Type;
};
template <>
struct CandidateId2Type<Hlt::Stage::L0Calo>
{
    typedef LHCb::L0CaloCandidate Type;
};
template <>
struct CandidateId2Type<Hlt::Stage::L0DiMuon>
{
    typedef Hlt::L0DiMuonCandidate Type;
};
template <>
struct CandidateId2Type<Hlt::Stage::HltTrack>
{
    typedef LHCb::Track Type;
};
template <>
struct CandidateId2Type<Hlt::Stage::HltMultiTrack>
{
    typedef Hlt::MultiTrack Type;
};
template <>
struct CandidateId2Type<Hlt::Stage::HltStage>
{
    typedef Hlt::Stage Type;
};
// ============================================================================
} //                                                       end of namespace Hlt
// ============================================================================
//                                                                      the END
// ============================================================================
#endif /// HltEvent_Candidate_H
// ============================================================================
