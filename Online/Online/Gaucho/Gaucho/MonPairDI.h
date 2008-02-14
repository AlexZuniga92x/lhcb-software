#ifndef GAUCHO_MONPAIRDI_H
#define GAUCHO_MONPAIRDI_H 1

#include "Gaucho/MonObject.h"

class MonPairDI: public MonObject {

protected:
  std::pair<double,int> m_pair;


public:
  BOOST_SERIALIZATION_SPLIT_MEMBER()

  MonPairDI(IMessageSvc* msgSvc, const std::string& source, int version=0);
  virtual ~MonPairDI();

  virtual void save(boost::archive::binary_oarchive & ar, const unsigned int version);
  virtual void load(boost::archive::binary_iarchive  & ar, const unsigned int version);

  void setValue(std::pair<double,int> p){m_pair = p;}
  std::pair<double,int> value(){return m_pair;}

  virtual void combine(MonObject * monPair);  
  virtual void copyFrom(MonObject* monPair);  
  virtual void reset(){m_pair.first=0.00;m_pair.second=0;}
  virtual void add(MonPairDI * monPair);
  virtual void print();
  virtual void write(){};
};

#endif //GAUCHO_MONPAIRDI_H

