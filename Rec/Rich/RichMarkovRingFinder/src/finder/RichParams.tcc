#ifndef LESTER_RICH_PARAMS_TCC
#define LESTER_RICH_PARAMS_TCC

#include "RichParams.h"

#include <iostream>

template<class Mode, const Mode & mode>
std::ostream & Lester::RichParams<Mode,mode>::printMeTo(std::ostream & os) const {
  int i=0;
  os << "RichParams[" << std::endl;
  for(typename Circs::const_iterator it=circs.begin();
      it!=circs.end();
      ++it) {
    os << "circ " << i << "=" << *it << std::endl;
    ++i;
  };
  os << ";SB="<<meanBackground<<"]";
  
  return os;
};

template<class Mode, const Mode & mode>
std::ostream & operator<<(std::ostream & os, const Lester::RichParams<Mode,mode> & rp) {
  return rp.printMeTo(os);
};

#endif

