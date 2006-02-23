// $Id: SiHeaderWord.h,v 1.5 2006-02-23 10:26:09 mneedham Exp $
#ifndef _SiHeaderWord_H
#define _SiHeaderWord_H 1

#include <iostream>

/** @class SiHeaderWord SiHeaderWord.h "STDAQ/SiHeaderWord.h"
 *
 *  Class for encapsulating header word in RAW data format
 *  for the Si detectors. 
 *
 *  @author M.Needham
 *  @date   18/10/2005
 */


class SiHeaderWord {

public:

  /** constructer 
      @param number of clusters
      @param pcn
  */
  SiHeaderWord(unsigned int nClusters,
               unsigned int pcn, 
               unsigned int errorFlag = 0){

    m_value = (nClusters<<clusterBits) +
      (pcn <<pcnBits) +
      (errorFlag << errorBits);
  }

  /** constructer with int 
      @param value
  */
  explicit SiHeaderWord(unsigned int value = 0):
    m_value(value){}

  /** Destructer **/
  ~SiHeaderWord(){}

  /** cast to int */ 
  operator int() const; 

  /** The actual value 
      @return value
  */
  unsigned int value() const;

  /** number of clusters
      @return number of clusters
  */
  unsigned int nClusters() const;

  /** pipeline column number
      @return pcn value 
  */
  unsigned int pcn() const;


  /** error bits set 
      @return true if in error 
  */
  bool hasError() const;

  /** Operator overloading for stringoutput */
  friend std::ostream& operator<< (std::ostream& s, const SiHeaderWord& obj)
  {
    return obj.fillStream(s);
  }

  /** Fill the ASCII output stream */
  std::ostream& fillStream(std::ostream& s) const;

private:

  enum bits {clusterBits = 0, pcnBits = 16, errorBits = 25};
  enum masks {clusterMask = 0x00ffff , pcnMask =  0xff0000, errorMask = 0x1000000}; 

  unsigned int m_value;

};

inline  SiHeaderWord::operator int() const
{
  return m_value;
}

inline unsigned int SiHeaderWord::value() const
{
  return m_value;
}

inline bool SiHeaderWord::hasError() const{
  return static_cast<bool>((m_value & errorMask) >> errorBits);
}

inline unsigned int SiHeaderWord::nClusters() const{
  return ((m_value & clusterMask) >> clusterBits);
}

inline unsigned int SiHeaderWord::pcn() const{
  return ((m_value & pcnMask) >> pcnBits);
}

inline std::ostream& SiHeaderWord::fillStream(std::ostream& s) const
{
  s << "{ "
    << " value:\t" << std::hex << value() <<  std::endl
    << " n Clusters:\t" << std::dec  << nClusters() << std::endl
    << " pcn:\t" << std::hex << pcn() << std::dec  << std::endl;
  return s;
}

#endif
