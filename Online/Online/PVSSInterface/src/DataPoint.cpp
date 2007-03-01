// $Header: /afs/cern.ch/project/cvs/reps/lhcb/Online/PVSSInterface/src/DataPoint.cpp,v 1.1 2007-03-01 10:39:50 frankb Exp $
//  ====================================================================
//  DataPoint.cpp
//  --------------------------------------------------------------------
//
//  Author    : Markus Frank
//
//  ====================================================================
// $ID: $
#ifdef _WIN32
  // Disable warning C4250: 'const float' : forcing value to bool 'true' or 'false' (performance warning)
  #pragma warning ( disable : 4800 )
#endif

// Framework include files
#include "PVSS/DevType.h"
#include "PVSS/DataPoint.h"
#include "PVSS/Internals.h"
#include "PVSS/DevTypeElement.h"
#include "PVSSManager/DataPointIO.h"
#include <algorithm>
#include <strstream>
#include <stdexcept>
#include <memory>
#include <vector>

// PVSS namespace usage
using namespace PVSS;

namespace {
  DpIdentifier s_nullDP(0);
  struct _Init {
    _Init()  {  pvss_setup_null_dp(&s_nullDP,sizeof(s_nullDP));    }
  };
  _Init s_init;
}

template <> int DataValue<bool>::type_id()                         { return DevTypeElement::BIT;       }
template <> int DataValue<char>::type_id()                         { return DevTypeElement::CHAR;      }
template <> int DataValue<unsigned char>::type_id()                { return DevTypeElement::CHAR;      }
template <> int DataValue<short>::type_id()                        { return DevTypeElement::NOELEMENT; }
template <> int DataValue<unsigned short>::type_id()               { return DevTypeElement::NOELEMENT; }
template <> int DataValue<int>::type_id()                          { return DevTypeElement::INT;       }
template <> int DataValue<unsigned int>::type_id()                 { return DevTypeElement::INT;       }
template <> int DataValue<long>::type_id()                         { return DevTypeElement::NOELEMENT; }
template <> int DataValue<unsigned long>::type_id()                { return DevTypeElement::NOELEMENT; }
template <> int DataValue<float>::type_id()                        { return DevTypeElement::FLOAT;     }
template <> int DataValue<double>::type_id()                       { return DevTypeElement::NOELEMENT; }
//template <> int DataValue<time_t>::type_id()                       { return DevTypeElement::TIME;    }
template <> int DataValue<std::string>::type_id()                  { return DevTypeElement::TEXT;      }
template <> int DataValue<DpIdentifier>::type_id()                 { return DevTypeElement::DPID;      }
template <> int DataValue<DPTime>::type_id()                       { return DevTypeElement::TIME;      }
template <> int DataValue<DPRef>::type_id()                        { return DevTypeElement::NOELEMENT; }
template <> int DataValue<std::vector<bool> >::type_id()           { return DevTypeElement::DYNBIT;    }
template <> int DataValue<std::vector<char> >::type_id()           { return DevTypeElement::DYNCHAR;   }
template <> int DataValue<std::vector<unsigned char> >::type_id()  { return DevTypeElement::DYNCHAR;   }
template <> int DataValue<std::vector<short> >::type_id()          { return DevTypeElement::NOELEMENT; }
template <> int DataValue<std::vector<unsigned short> >::type_id() { return DevTypeElement::NOELEMENT; }
template <> int DataValue<std::vector<int> >::type_id()            { return DevTypeElement::DYNINT;    }
template <> int DataValue<std::vector<unsigned int> >::type_id()   { return DevTypeElement::DYNINT;    }
template <> int DataValue<std::vector<long> >::type_id()           { return DevTypeElement::NOELEMENT; }
template <> int DataValue<std::vector<unsigned long> >::type_id()  { return DevTypeElement::NOELEMENT; }
template <> int DataValue<std::vector<float> >::type_id()          { return DevTypeElement::DYNFLOAT;  }
template <> int DataValue<std::vector<double> >::type_id()         { return DevTypeElement::NOELEMENT; }
//template <> int DataValue<std::vector<time_t> >::type_id()         { return DevTypeElement::DYNTIME;   }
template <> int DataValue<std::vector<std::string> >::type_id()    { return DevTypeElement::DYNTEXT;   }
template <> int DataValue<std::vector<DpIdentifier> >::type_id()   { return DevTypeElement::DYNDPID;   }
template <> int DataValue<std::vector<DPTime> >::type_id()         { return DevTypeElement::DYNTIME;   }
template <> int DataValue<std::vector<DPRef> >::type_id()          { return DevTypeElement::NOELEMENT; }

namespace PVSS {
  static Value* invalidValue() 
  { throw std::runtime_error("Invalid type for data conversion.");                        }
  template <typename T> static inline DataValue<T>& holder(Value* v) 
  { return *(DataValue<T>*)(v->type()==DataValue<T>::type_id() ? v : invalidValue());     }
  template <typename T,typename Q> struct ConvertValue 
  { static Q get(Value* v)    { return holder<T>(v).value<Q>();  }                        };
  template <typename T> struct ConvertRef 
  { static T& ref(Value* v)   { return holder<T>(v).data();  }                            };
  template <typename T> T default_value()                { return T();                    }
  template <> std::string  default_value<std::string>()  { return std::string("");        }
  template <> DpIdentifier default_value<DpIdentifier>() { return DpIdentifier(s_nullDP); }
  static void copy_string(std::string& t,const char* s)  { t = s;                         }
  template <typename T> static void insert_vector(std::vector<T>& v, const T& o)  { v.push_back(o);  }
  static void insert_string(std::vector<std::string>& t,const char* s) { t.push_back(s);   }

  /// Allocate data buffer
  static PVSS::Value* createValue(int typ)   {
    switch(typ)  {
    case DevTypeElement::DYNDPID:
      return new DataValue<std::vector<DpIdentifier> >(typ);
    case DevTypeElement::DYNCHAR:
      return new DataValue<std::vector<char> >(typ);
    case DevTypeElement::DYNINT:
      //return new DataValue<std::vector<short> >(typ);
    case DevTypeElement::DYNUINT:
      return new DataValue<std::vector<int> >(typ);
    case DevTypeElement::DYNFLOAT:
      return new DataValue<std::vector<float> >(typ);
    case DevTypeElement::DYNTEXT:
      return new DataValue<std::vector<std::string> >(typ);
    case DevTypeElement::DYNTIME:
      return new DataValue<std::vector<time_t> >(typ);
    case DevTypeElement::DYNBIT:
      return new DataValue<std::vector<bool> >(typ);
    case DevTypeElement::DPID:
      return new DataValue<DpIdentifier>(typ,s_nullDP);
    case DevTypeElement::CHAR:
      return new DataValue<char>(typ);
    case DevTypeElement::INT:
      //return new DataValue<short>(typ);
    case DevTypeElement::UINT:
      return new DataValue<int>(typ);
    case DevTypeElement::FLOAT:
      return new DataValue<float>(typ);
    case DevTypeElement::TEXT:
      return new DataValue<std::string>(typ);
    case DevTypeElement::TIME:
      return new DataValue<time_t>(typ);
    case DevTypeElement::BIT:
      return new DataValue<bool>(typ);
    case DevTypeElement::BIT32:
    default:
      DataPoint::invalidValue(typeid(void));
    }
    return 0;
  }
}

// Default constructor
DataPoint::DataPoint(ControlsManager* m) 
: m_id(s_nullDP), m_valid(0), m_mgr(m), m_val(0)
{
}

// Initializing constructor
DataPoint::DataPoint(ControlsManager* m, const std::string& nam) 
: m_id(s_nullDP), m_valid(0), m_name(nam), m_mgr(m), m_val(0)
{
}

/// Initializing constructor
DataPoint::DataPoint(ControlsManager* m, const DpIdentifier& dpid)  
: m_id(dpid), m_valid(1), m_name(), m_mgr(m), m_val(0) 
{
  char* nam = 0;
  if ( !pvss_lookup_name(m_id,nam) )  {
    m_valid = 0;
    m_id = s_nullDP;
    return;
  }
  m_name = nam;
}

// Copy constructor
DataPoint::DataPoint(const DataPoint& c) 
: m_id(c.m_id), m_valid(0), m_name(c.m_name), m_mgr(c.m_mgr), m_val(0)
{
}

// Standard destructor
DataPoint::~DataPoint()   {
  if ( m_val ) delete m_val;
}

/// load datapoint identifier
const DpIdentifier& DataPoint::load() const  {
  DataPoint* p = const_cast<DataPoint*>(this);
  if ( !pvss_lookup_dpid(m_name.c_str(),p->m_id) )    {
    throw std::invalid_argument("The datapoint:"+m_name+" does not exist!");
  }
  p->m_valid = true;
  return m_id;
}

// Assignment
DataPoint& DataPoint::operator=(const DataPoint& c)  {
  if ( this != &c ) {
    m_mgr  = c.manager();
    m_name = c.name();
    m_id   = c.m_id;
    m_val  = 0;
  }
  return *this;
}

// Equivalence check
bool DataPoint::operator==(const DataPoint& c) const    {
  return (this == &c) ? true : m_name==c.m_name && m_id == c.m_id;
}

// Operator <
bool DataPoint::operator<(const DataPoint& c) const    {
  if (this != &c) {
    if      ( m_name < c.m_name ) return true;
    else if ( m_name > c.m_name ) return false;
    else if ( m_id   < c.m_id   ) return true;
  }
  return false;
}

void DataPoint::invalidValue(const std::type_info& tid)  {
  throw std::runtime_error(std::string("Invalid value data:")+tid.name());
}
void DataPoint::invalidConversion(const std::type_info& tid)  {
  throw std::runtime_error(std::string("Invalid type conversion to ")+tid.name());
}

/// Construct name of online value
std::string DataPoint::online(const std::string& dp)  {
  return dp.find(".")==std::string::npos ? dp+".:_online.._value" : dp+":_online.._value";
}

/// Construct name of online value
std::string DataPoint::original(const std::string& dp)  {
  return dp.find(".")==std::string::npos ? dp+".:_original.._value" : dp+":_original.._value";
}

/// Extract name of datapoint from online/original name
std::string DataPoint::dpname(const DpIdentifier& dpid)    {
  char* nam = "";
  if ( !pvss_lookup_name(*(DpIdentifier*)&dpid,nam) )  {
    return "";
  }
  return nam;
}

/// Extract name of datapoint from online/original name
std::string DataPoint::dpname(const std::string& dp)    {
  std::string::size_type id1 = dp.find(":");
  std::string::size_type id2 = dp.rfind(":");
  if ( id1 == std::string::npos && id2 == std::string::npos )
    return dp;
  else if ( id2 > id1 )
    return dp.substr(0,id2);
  else if ( id1 == id2 )  {
    std::string s = dp.substr(id1,3);
    if( s == ":_o")
      return dp.substr(0,id1);
  }
  return dp;
}

/// Extract system name of datapoint from online/original name
std::string DataPoint::sysname(const std::string& dp)   {
  std::string::size_type id1 = dp.find(":");
  std::string::size_type id2 = dp.rfind(":",id1+1);
  if ( id1 == std::string::npos && id2 == std::string::npos )
    return "";
  else if ( id2 == std::string::npos )
    return dp.substr(0,id1);
  return dp.substr(0,id1);
}

template <typename T> T DataPoint::data()  {
  if ( m_val )  {
    switch(m_val->type())  {
      case DevTypeElement::FLOAT: return ConvertValue<float,T>::get(m_val);
      case DevTypeElement::CHAR:  return ConvertValue<char,T>::get(m_val);
      case DevTypeElement::INT:   return ConvertValue<int,T>::get(m_val);
      case DevTypeElement::UINT:  return ConvertValue<int,T>::get(m_val);
      case DevTypeElement::TIME:  return ConvertValue<time_t,T>::get(m_val);
      case DevTypeElement::BIT:   return ConvertValue<bool,T>::get(m_val);
      default:                    break;
    }
    invalidConversion(typeid(T));
  }
  invalidValue(typeid(T));
  return default_value<T>();
}

template <typename T> const T DataPoint::data()  const {
  if ( m_val )  {
    switch(m_val->type())  {
      case DevTypeElement::FLOAT: return ConvertValue<float,T>::get(m_val);
      case DevTypeElement::CHAR:  return ConvertValue<char,T>::get(m_val);
      case DevTypeElement::INT:   return ConvertValue<int,T>::get(m_val);
      case DevTypeElement::UINT:  return ConvertValue<int,T>::get(m_val);
      case DevTypeElement::TIME:  return ConvertValue<time_t,T>::get(m_val);
      case DevTypeElement::BIT:   return ConvertValue<bool,T>::get(m_val);
      default:                    break;
    }
    invalidConversion(typeid(T));
  }
  invalidValue(typeid(T));
  return default_value<T>();
}

template <> const std::string DataPoint::data<std::string>() const  {
  if ( m_val )  {
    std::strstream os;
    switch(m_val->type())  {
      case DevTypeElement::FLOAT:
        os << this->data<float>();
        return os.str();
      case DevTypeElement::CHAR:
        os << this->data<char>();
        return os.str();
      case DevTypeElement::INT:
        os << this->data<short>();
        return os.str();
      case DevTypeElement::UINT:
        os << this->data<int>();
        return os.str();
      case DevTypeElement::TIME:
        os << this->data<time_t>();
        return os.str();
      case DevTypeElement::BIT:
        os << this->data<bool>();
        return os.str();
      case DevTypeElement::TEXT:
        return ConvertRef<std::string>::ref(m_val);
      default: 
        break;
    }
    invalidConversion(typeid(std::string));
  }
  invalidValue(typeid(std::string));
  return "";
}

template <> std::string DataPoint::data<std::string>()  {
  if ( m_val )  {
    std::strstream os;
    switch(m_val->type())  {
      case DevTypeElement::FLOAT:
        os << this->data<float>();
        return os.str();
      case DevTypeElement::CHAR:
        os << this->data<char>();
        return os.str();
      case DevTypeElement::INT:
        os << this->data<short>();
        return os.str();
      case DevTypeElement::UINT:
        os << this->data<int>();
        return os.str();
      case DevTypeElement::TIME:
        os << this->data<time_t>();
        return os.str();
      case DevTypeElement::BIT:
        os << this->data<bool>();
        return os.str();
      case DevTypeElement::TEXT:
        return ConvertRef<std::string>::ref(m_val);
      default: 
        break;
    }
    invalidConversion(typeid(std::string));
  }
  invalidValue(typeid(std::string));
  return "";
}

template <class T> T& DataPoint::reference()  
{  return ConvertRef<T>::ref(m_val);                                }

template <class T> const T& DataPoint::reference()  const
{  return ConvertRef<T>::ref(m_val);                                }

template <typename T> static void do_vector_io(const Variable* variable,T& ref)  
{  DatapointIO::value(variable,insert_vector<T::value_type>,ref);   }
static void do_string_vector_io(const Variable* variable,std::vector<std::string>& ref)  
{  DatapointIO::value(variable,insert_string,ref);                  }

/// Set value data
void DataPoint::setValue(int typ, const Variable* variable)  {
  try {
    if ( !m_val ) m_val = createValue(typ);
    switch(m_val->type())  {
    case DevTypeElement::CHAR:
      DatapointIO::value(variable,reference<char>());
      break;
    case DevTypeElement::INT:
      //DatapointIO::value(variable,reference<short>());
      //break;
    case DevTypeElement::UINT:
      DatapointIO::value(variable,reference<int>());
      break;
    case DevTypeElement::TIME:
      DatapointIO::value(variable,reference<time_t>());
      break;
    case DevTypeElement::BIT:
      DatapointIO::value(variable,reference<bool>());
      break;
    case DevTypeElement::FLOAT:
      DatapointIO::value(variable,reference<float>());
      break;
    case DevTypeElement::DPID:
      DatapointIO::value(variable,reference<DpIdentifier>());
      break;
    case DevTypeElement::TEXT:
      DatapointIO::value(variable,copy_string,reference<std::string>());
      break;
    case DevTypeElement::DYNCHAR:
      do_vector_io(variable,reference<std::vector<char> >());
      break;
    case DevTypeElement::DYNINT:
      //do_vector_io(variable,reference<std::vector<short> >());
      //break;
    case DevTypeElement::DYNUINT:
      do_vector_io(variable,reference<std::vector<int> >());
      break;
    case DevTypeElement::DYNTEXT:
      do_string_vector_io(variable,reference<std::vector<std::string> >());
      break;
    case DevTypeElement::DYNTIME:
      do_vector_io(variable,reference<std::vector<time_t> >());
      break;
    case DevTypeElement::DYNBIT:
      do_vector_io(variable,reference<std::vector<bool> >());
      break;
    case DevTypeElement::DYNFLOAT:
      do_vector_io(variable,reference<std::vector<float> >());
      break;
    case DevTypeElement::DYNDPID:
      do_vector_io(variable,reference<std::vector<DpIdentifier> >());
      break;
    case DevTypeElement::BIT32:
    default:
      invalidValue(typeid(void));
      break;
    }
  }
  catch(const std::exception& e)  {
    throw e;
  }
  catch(const char* msg)  {
    throw std::runtime_error(msg ? msg : "Unknown error");
  }
  catch(...)  {
    throw std::runtime_error("Unknown error in DataPoint::setValue");
  }
}

/// Set value data (for publishing data to PVSS
template <typename T> void DataPoint::set(const T& val)  {
  if ( !m_val ) m_val = createValue(DataValue<T>::type_id());
  if ( m_val->type() != DataValue<T>::type_id() )  {
    invalidValue(typeid(T));
  }
  reference<T>() = val;
}

namespace {
  struct ListHandler {
    DPListActor* m_l;
    ListHandler(DPListActor* l) : m_l(l) {}
    void operator()(const DpIdentifier& dpid)  {
      m_l->operator()(dpid);
    }
  };
}

DPListActor::DPListActor(ControlsManager* m) : m_manager(m)  {
}

DPListActor::~DPListActor() {
}

bool DPListActor::lookup(const std::string& typ_nam, const DevType* typ)  {
  if ( pvss_lookup_dpidset(typ_nam.c_str(),m_dpids,m_count,typ->id()) )  {
    std::for_each(&m_dpids[0],&m_dpids[m_count],ListHandler(this));
    return true;
  }
  throw std::runtime_error("Cannot load datapoint identifiers for type:"+typ_nam);
}

namespace  {
  template <typename T> struct SingleRefs0  {
    static const T& ref0(const DataPoint& dp) { return dp.reference<T>(); }  
  };
  template <typename T> struct SingleRefs {
    static T& ref1(DataPoint& dp)  { return dp.reference<T>();           }  
    static void set(DataPoint& dp) { return dp.set(default_value<T>());  }  
  };
  template <typename T> struct SingleData  {
    T data(DataPoint& dp)                    { return dp.data<T>(); }
    const T data(const DataPoint& dp) const  { return dp.data<T>(); }
    void setData(DataPoint& dp, const T& v)  { dp.set(v);           }
  };

#define MAKE_CONV(x,y) template SingleData<x>; template SingleData<y>;
  MAKE_CONV(char, unsigned char);
  MAKE_CONV(short,unsigned short);
  MAKE_CONV(int,  unsigned int);
  MAKE_CONV(long, unsigned long);
  MAKE_CONV(float,double);
  MAKE_CONV(bool, time_t);

#define MAKE_REFS(x,y) \
  template SingleRefs<x>; template SingleRefs<y >;  \
  template SingleRefs<std::vector<x> >; template SingleRefs<std::vector<y> >;  \
  template SingleRefs0<x >; template SingleRefs0<y >; \
  template SingleRefs0<std::vector<x> >; template SingleRefs0<std::vector<y> >;  
  MAKE_REFS(char,        unsigned char)
  MAKE_REFS(short,       unsigned short);
  MAKE_REFS(int,         unsigned int);
  MAKE_REFS(long,        unsigned long);
  MAKE_REFS(float,       time_t);
  MAKE_REFS(bool,        double);
  MAKE_REFS(std::string, DpIdentifier);
  MAKE_REFS(DPRef,       DPTime);
}
