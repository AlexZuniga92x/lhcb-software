// $Id: Link.h,v 1.3 2004-11-17 08:35:00 cattanem Exp $

#ifndef L0MUONKERNEL_LINK_H
#define L0MUONKERNEL_LINK_H     1

/** @class Unit Unit.h L0MuonKernel/Unit.h

   Class representing a unit of data processing logic 
   of the L0Muon Trigger for hardware simulations
   
   @author  Andrei Tsaregorodtsev
   @date  12 January 2002
*/ 

#include "L0MuonKernel/Unit.h"
namespace L0Muon {

class Link : public L0Muon::Unit {

public:
  Link();
  Link(L0Muon::Register* read, L0Muon::Register* write);
  ~Link();
  
  void addInputRegister(L0Muon::Register* in) ;
  void addOutputRegister(L0Muon::Register* out) ;
  void transfer() { execute(); }
  void execute();
  void initialize();
  void finalize();

};

};  // namespace L0Muon

#endif      // L0MUONKERNEL_LINK_H  
