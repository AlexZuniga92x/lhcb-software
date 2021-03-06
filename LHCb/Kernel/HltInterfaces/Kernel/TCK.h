#ifndef TCK_H
#define TCK_H
#include "boost/operators.hpp"
#include "boost/format.hpp"

class TCK final : public boost::equality_comparable<TCK>,
                         boost::equality_comparable2<TCK,unsigned int> {
public:
    TCK() = default;
    explicit TCK(unsigned int i) : m_unsigned(i) { set(i); }
    explicit TCK(std::string s) { set(s); }
    bool operator<(const TCK& rhs) const { return m_unsigned  < rhs.m_unsigned; }
    bool operator==(const TCK& rhs) const { return m_unsigned == rhs.m_unsigned; }
    bool operator==(unsigned int rhs) const { return m_unsigned == rhs; }
    TCK& operator++() { return set( ++m_unsigned ); }
    const std::string&  str() const { return m_stringRep; }
    unsigned int uint() const { return m_unsigned;  }
    TCK& normalize() {
        // if upper bit set, ignore L0... in that case, the L0 TCK is generated by PVSS
        // and not by us, and hence we only know it with the L0 part blanked out...
        if ((uint() & 0x80000000)!=0 ) maskL0();
        return *this;
    }
    TCK& maskL0() { set( uint() & 0xFFFF0000 ); return *this; }
    TCK& set(const std::string& s);
    TCK& set(unsigned i) {
        m_unsigned = i;
        m_stringRep = boost::str( boost::format("0x%08x")%i ) ;
        return *this;
    }
    bool valid() const { return m_unsigned != 0 ; }
private:
    std::string m_stringRep;
    unsigned int m_unsigned = 0;
};
inline std::ostream& operator<<(std::ostream& os, const TCK& tck)
{ return os << tck.str(); }
#endif
