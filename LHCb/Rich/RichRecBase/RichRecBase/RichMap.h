
// ============================================================================
// A fix for map problems on Windows.
// ============================================================================

#ifndef RICHRECKERNEL_RICHMAP_H
#define RICHRECKERNEL_RICHMAP_H 1

// Include files
#ifdef WIN32
#include "GaudiKernel/HashTable.h"   // GaudiKernel (only for Visual-C Win32)
#include "RichRecBase/RichHash.h"    // LHCbKernel  (only for Visual-C Win32)
#include <vector>
#else
#include <map>                       // STD & STL   (except for Visual-C Win32)
#endif

/** @class RichMap RichMap.h RichRecBase/RichMap.h
 *
 *  An auxillary class to hide underlying
 *  std::map implementation for Microsoft Visual-C++ (Win32)
 *
 *  Gratuitously copied from CaloMap !!
 *
 *  The public interface is minimal
 *
 *  @warning the implementation is platform dependent!!!
 *
 *  @author Chris Jones Christopher.Rob.Jones@cern.ch (original Vanya Belyaev)
 *  @date   19/11/2002
 */

template<class KEY, class VALUE>
class RichMap {

public:

  /// type of the key
  typedef  KEY                   Key      ;
  /// type of the mapped value
  typedef  VALUE                 Value    ;

#ifdef WIN32
  /** the  map itself
   *  @warning the actual type is platform-dependent!
   */
  typedef  HashTable<Key,Value,RichHash<Key> >  Map  ;
  typedef  std::vector<Key>                    Keys ;
#else
  /** the  map itself
   *  @warning the actual type is platform-dependent!
   */
  typedef  std::map<Key,Value>   Map      ;
#endif

  /** the type of iterator
   *  @warning the actual type is platform-dependent!
   */
  typedef  Map::iterator         iterator       ;

  /** the type of const_iterator
   *  @warning the actual type is platform-dependent!
   */
  typedef  Map::const_iterator   const_iterator ;

public:

  /// Standard constructor
  RichMap()
    : m_map  ()
#ifdef WIN32
    , m_keys ()
#endif
  {};

  /// destructor (virtual)
  virtual ~RichMap() { clear() ; };

  /** access to elements by the key    (non-const!)
   *  for Linux the implementation is trivial
   *
   *  @warning the implementation is platform-dependent!
   *
   *   - for Linux the implementation is trivial
   *   - for WIN32 some tricks are applied
   *
   *  @param key  key
   *  @return reference to the value
   */
  inline Value& operator[] ( const Key& key )
#ifdef WIN32
  {
    // find the value
    Map::value_type* value = m_map.find( key ) ;
    if( 0 != value ) { return value->second ; } // RETURN
    // insert the key
    m_map.insert( key , Value() );
    value = m_map.find( key ) ;
    return value->second ;
  };
#else
  { return m_map[key]; };
#endif

  /** erase the sequence from the map
   *  @warning implementation  is platform-dependent
   *  @param first the "begin" iterator for the sequence
   *  @param last  the "end"   iterator for the sequence
   */
  void erase( iterator first ,
              iterator last  )
  {
#ifndef WIN32
    m_map.erase( first , last );
#else
    if( !m_keys.empty() ) { m_keys.clear() ; }
    for( ; first != last ; ++first )
      { m_keys.push_back( first->first ); }
    for( Keys::const_iterator key = m_keys.begin() ;
         m_keys.end() != key ; ++key ) { m_map.remove( *key  ) ; }
    m_keys.clear();
#endif;
  };

  /** erase the sequence from the map
   *  @warning implementation  is platform-dependent
   *  @param it the "begin" iterator for the sequence
   *  @param last  the "end"   iterator for the sequence
   */
  void erase( iterator it )
  {
#ifndef WIN32
    m_map.erase  ( it        );
#else
    m_map.remove ( it->first );
#endif;
  };

  /** remove/erase element from the map (by key)
   *  @warning implementation  is platform-dependent
   *  @param key key of element to be removed
   *  @return true if element is removed
   */
  void erase( const Key& key )
  {
#ifndef WIN32
    m_map.erase  ( key ) ;
#else
    m_map.remove ( key ) ;
#endif
  };

  /// clear the content of all containers
  void clear() { m_map.clear() ; }

  /** get the size ( == number of elements )
   *  @return number of elements
   */
  unsigned int  size() const { return m_map.size() ; }

  /** iterator for sequential access to the content of the "map"
   *  @return begin-iterator (non-const version)
   */
  Map::iterator       begin ()       { return m_map.begin () ; }

  /** iterator for sequential access to the content of the "map"
   *  @return begin-iterator (const version)
   */
  Map::const_iterator begin () const { return m_map.begin () ; }

  /** iterator for sequential access to the content of the "map"
   *  @return end-iterator (non-const version)
   */
  Map::iterator       end   ()       { return m_map.end   () ; }

  /** iterator for sequential access to the content of the "map"
   *  @return end-iterator (const version)
   */
  Map::const_iterator end   () const { return m_map.end   () ; }

private:

  Map m_map                ;   ///< the underlying  MAP container

#ifdef WIN32
  std::vector<Key>  m_keys ; ///< auxillary small container ;
#endif

};

/// Output ostream a RichMap
template <class T1, class T2> inline
std::ostream& operator << ( std::ostream& s, const RichMap<T1,T2>& m )
{
  for ( RichMap<T1,T2>::const_iterator i = m.begin(); i != m.end(); i++ ) {
    s << "[" << i->first << ", " << i->second << "]";
  }
  return s;
}

/// Output serialize a RichMap
template <class T1, class T2> inline
StreamBuffer& operator << ( StreamBuffer& s, const RichMap<T1,T2>& m ) 
{
  s << m.size();
  for ( RichMap<T1,T2>::const_iterator i = m.begin(); i != m.end(); i++ ) {
    s << i->first << i->second;
  }
  return s;
}

/// Input serialize a RichMap
template <class T1, class T2> inline
StreamBuffer& operator >> ( StreamBuffer& s, RichMap<T1,T2>& m ) 
{
  long length;
  s >> length;
  T1 key;
  T2 value; 
  for ( int i = 0; i < length; i++ ) {
    s >> key;
    s >> value;
    m[key] = value;
  }
  return s;
}

#endif // RICHRECKERNEL_RICHMAP_H

