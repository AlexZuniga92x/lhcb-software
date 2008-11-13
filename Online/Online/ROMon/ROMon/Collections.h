// $Id: Collections.h,v 1.6 2008-11-13 12:13:32 frankb Exp $
//====================================================================
//  ROMon
//--------------------------------------------------------------------
//
//  Package    : ROMon
//
//  Description: Structure of basic items published by the readout
//               monitoring.
//
//  Author     : M.Frank
//  Created    : 29/1/2008
//
//====================================================================
// $Header: /afs/cern.ch/project/cvs/reps/lhcb/Online/ROMon/ROMon/Collections.h,v 1.6 2008-11-13 12:13:32 frankb Exp $
#ifndef ROMON_COLLECTIONS_H
#define ROMON_COLLECTIONS_H 1

#include <climits>
#include <iomanip>
#include <iostream>
#include "ROMon/Collections.h"
#include "RTL/Pack.h"

/*
 *   ROMon namespace declaration
 */
namespace ROMon {

  /**@class FixItems ROMon.h ROMon/ROMon.h
   *
   * Class to store fixed length items in a flat memory area.
   *
   * @author M.Frank
   */
  template <typename T> PACK_DATA(class) FixItems {
  public:
    typedef T        item_type;
    typedef T*       pointer_type;
    typedef T*       iterator;
    typedef const T* const_iterator;
    typedef T        value_type;
    typedef T&       reference;
    typedef const T& const_reference;

    /// Number of fixed size elements in the container
    int  m_size;

  public:    // Public data accessors
    /// Reset object and return iterator to first element
    iterator reset()
      {  m_size = 0;  return begin();                  }
    /// Return number of elements in the object
    int size()  const
      {  return m_size;                                }
    /// Length of the object in bytes
    int length() const
      {  return sizeof(int)+size()*sizeof(T);          }
    /// Length of the object's payload in bytes
    int data_length() const
      {  return size()*sizeof(T);                      }
    /// Acces to opaque data buffer 
    char* data() const                       
      {  return ((char*)&m_size)+sizeof(m_size);       }
    /// Iterator implementation: begin of object iteration
    pointer_type begin()  const
      {  return (pointer_type)data();                  }
    /// Iterator implementation: end of object iteration
    const_iterator end() const 
      {  return begin()+size();                        }
    /// Iterator implementation: Next element of iteration
    iterator next(iterator prev) const
      {  return ++prev;                                }
    /// Iterator implementation: Next element of iteration
    const_iterator next(const_iterator prev) const
      {  return ++prev;                                }
    /// Add new element to container and return pointer to next object
    pointer_type add(pointer_type /* prev */)      
      {  return begin()+(++m_size);                    }
  };

  /**@class VarItems ROMon.h ROMon/ROMon.h
   *
   * Class to store variable length items in a flat memory area.
   *
   * @author M.Frank
   */
  template <typename T> PACK_DATA(class) VarItems {
  public:
    typedef T        item_type;
    typedef T*       pointer_type;
    typedef T*       iterator;
    typedef const T* const_iterator;
    typedef T        value_type;
    typedef T&       reference;
    typedef const T& const_reference;

    /// Number of variable size elements in the container
    int  m_size;
    /// Length of the data payload
    int  m_length;

  public:    // Public data accessors
    /// Reset object and return iterator to first element
    iterator reset()
      {  m_size = m_length = 0;  return begin();                 }
    /// Return number of elements in the object
    int size()  const
      {  return m_size;                                          }
    /// Length of the object in bytes
    int length() const
      {  return 2*sizeof(int)+m_length;                          }
    /// Length of the object's payload in bytes
    int data_length() const
      {  return m_length;                                        }
    /// Acces to opaque data buffer 
    char* data() const
      {  return ((char*)&m_length)+sizeof(int);                  }
    /// Iterator implementation: begin of object iteration
    pointer_type begin()  const
      {  return (pointer_type)data();                            }

    /// Iterator implementation: end of object iteration
    const_iterator end()   const
      {  return (pointer_type)(data()+data_length());            }

    /// Iterator implementation: Next element of iteration
    const_iterator next(const_iterator prev) const
      {  return (const_iterator)(((char*)prev)+prev->length());}

    /// Iterator implementation: Next element of iteration
    iterator next(iterator prev) 
      {  return (iterator)(((char*)prev)+prev->length());}

    /// Add new element to container and return pointer to next object
    pointer_type add(pointer_type prev)            {
      pointer_type t;
      m_size++;
      m_length += prev->length();
      t = (pointer_type)(data()+data_length());
      return t->reset();
    }
  };


  typedef std::pair<time_t,unsigned int> TimeStamp;

  template <typename T> TimeStamp _firstUpdate(const T& nodes) {
    bool has_nodes = false;
    TimeStamp t(INT_MAX,999);
    for(typename T::const_iterator n=nodes.begin(); n!=nodes.end(); n=nodes.next(n))  {
      has_nodes = true;
      if ( (*n).time < t.first ) {
        t.first = (*n).time;
        t.second = (*n).millitm;
      }
      else if ( (*n).time == t.first && (*n).millitm < t.second ) {
        t.second = (*n).millitm;
      }
    }
    return has_nodes ? t : TimeStamp(0,0);
  }

  template <typename T> TimeStamp _lastUpdate(const T& nodes) {
    bool has_nodes = false;
    TimeStamp t(0,0);
    for(typename T::const_iterator n=nodes.begin(); n!=nodes.end(); n=nodes.next(n))  {
      has_nodes = true;
      if ( (*n).time > t.first ) {
        t.first = (*n).time;
        t.second = (*n).millitm;
      }
      else if ( (*n).time == t.first && (*n).millitm > t.second ) {
        t.second = (*n).millitm;
      }
    }
    return has_nodes ? t : TimeStamp(0,0);
  }

}

template<typename T> inline 
std::ostream& operator<<(std::ostream& os, const ROMon::FixItems<T>& items) {
  os << "Start address:" << (void*)&items
     << " Num Elements:" << std::setw(6)  << std::right << items.size()
     << " - "            << std::setw(6)  << std::left << (items.end()-items.begin())
     << " Length:"       << std::setw(4)  << std::right << items.length()
     << " DataLen:"      << std::setw(4)  << std::right << items.data_length()
     << " Item size: "   << std::setw(4)  << std::right << sizeof(T)
     << " Begin: "       << (void*)items.begin()
     << " End: "         << (void*)items.end();
  if ( items.begin() != items.end() ) os << std::endl;
  for (const T* p=items.begin(); p!=items.end(); ++p) os << *p;
  return os;
}

template<typename T> inline 
std::ostream& operator<<(std::ostream& os, const ROMon::VarItems<T>& items) {
  os << "Start address:" << (void*)&items
     << " Num Elements:" << std::setw(6)  << std::right << items.size()
     << " - "            << std::setw(6)  << std::left << (items.end()-items.begin())
     << " Length:"       << std::setw(4)  << std::right << items.length()
     << " DataLen:"      << std::setw(4)  << std::right << items.data_length()
     << " Item size: "   << std::setw(4)  << std::right << sizeof(T)
     << " Begin: "       << (void*)items.begin()
     << " End: "         << (void*)items.end();
  if ( items.begin() != items.end() ) os << std::endl;
  for (const T* p=items.begin(); p!=items.end(); p=items.next(p)) os << *p;
  return os;
}

#include "RTL/Unpack.h"

#endif /* ROMON_COLLECTIONS_H */

