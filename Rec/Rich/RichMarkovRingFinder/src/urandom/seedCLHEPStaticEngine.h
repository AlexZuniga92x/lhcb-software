#ifndef LESTER_SEED_CLHEP_STATIC_ENGINE_H
#define LESTER_SEED_CLHEP_STATIC_ENGINE_H

namespace Lester {
  void seedCLHEPStaticEngine(); // FwdDec
}

#include "urandom/jamesRandomSeed.h"
#include "CLHEP/Random/Random.h"
#include "CLHEP/Random/JamesRandom.h"
#include <typeinfo>
#include <string>

namespace Lester {

  void seedCLHEPStaticEngine() {
    try {
      HepJamesRandom & engine
        = dynamic_cast<HepJamesRandom &>(*HepRandom::getTheEngine());
      // If we got this far, the static engine WAS a jamesRandom, so:
      engine.setSeed(jamesRandomSeed());
    }
    catch (std::bad_cast) {
      // Oh dear! The CLHEP standard changed, and the static engine is no
      // longer a JamesRandom :(
      throw FinderExternalException(std::string("Oh dear! The CLHEP standard changed, and the static engine is no longer a JamesRandom :("));
    };
  };

}

#endif
