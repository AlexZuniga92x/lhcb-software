// $Id: HltSelection.h,v 1.4 2008-07-30 13:33:16 graven Exp $
#ifndef HLTBASE_HLTSELECTION_H 
#define HLTBASE_HLTSELECTION_H 1

#include <vector>
#include <boost/utility.hpp>
#include "GaudiKernel/DataObject.h"
#include "GaudiKernel/ContainedObject.h"
#include "HltBase/stringKey.h"

namespace LHCb 
{
  class Track;
  class RecVertex;
}

namespace Hlt 
{

  template <typename T> class TSelection;

  class Selection : public ContainedObject, public DataObject, private boost::noncopyable {
  public:
    Selection(const stringKey& id) : m_id(id), m_processed(false) {}
    virtual ~Selection() {}

    const stringKey& id() const {return m_id;}
    const std::vector<stringKey>& inputSelectionsIDs() const {return m_inputSelectionsIDs;}    
    
    template <typename I> // I is assumed to be iterator over a range of Selection*
    void addInputSelectionIDs(I i, I end) {
        while (i!=end) m_inputSelectionsIDs.push_back( (*i++)->id() );
    }

    void setDecision(bool value) {m_decision = value; m_processed = true;}
    bool decision() const {return m_decision;}
    bool processed() const {return m_processed;}

    virtual CLID classID() const {return DataObject::clID();}
    virtual size_t ncandidates() const {return 0;}
    virtual void clean() {m_decision = false; m_processed = false;}    

    template <typename T>
    TSelection<T>* down_cast() { return T::classID()==classID() 
                                   ?  dynamic_cast<TSelection<T>*>(this)
                                   : (TSelection<T>*)0 ; }

  private:
    std::vector<stringKey> m_inputSelectionsIDs;
    stringKey m_id;
    bool m_decision; // accept / reject
    bool m_processed;    // did we actually set the decision?
  };
  
  template <typename T>
  class TSelection : public Selection {
  public:
    typedef T                                     candidate_type;
    typedef std::vector<T*>                       container_type;

    TSelection(const stringKey& id) : Selection(id) {}
    virtual ~TSelection() ;

    CLID classID() const { return T::classID(); }
    size_t ncandidates() const  {return m_candidates.size();}
    void clean();

    // forward container functionality..
    typedef typename container_type::iterator        iterator;
    typedef typename container_type::value_type      value_type;
    typedef typename container_type::size_type       size_type;
    typedef typename container_type::const_iterator  const_iterator;
    typedef typename container_type::const_reference const_reference;
    typedef typename container_type::reference       reference;
    bool empty() const { return m_candidates.empty(); }
    size_type size() const { return m_candidates.size(); }
    reference front() { return m_candidates.front(); }
    const_reference front() const { return m_candidates.front(); }
    const_iterator begin() const { return m_candidates.begin(); }
    const_iterator end() const { return m_candidates.end(); }
    iterator begin() { return m_candidates.begin(); }
    iterator end()   { return m_candidates.end(); }
    void push_back(T* t) { m_candidates.push_back(t); }
    template <typename ITER> void insert(iterator i, ITER begin, ITER end) { m_candidates.insert(i,begin,end); }
    iterator erase(iterator begin, iterator end)  { return m_candidates.erase(begin,end); }
  private:
    container_type m_candidates; // we do NOT own these...
  };
  
  typedef TSelection<LHCb::Track>      TrackSelection;
  typedef TSelection<LHCb::RecVertex> VertexSelection;
}
#endif 
