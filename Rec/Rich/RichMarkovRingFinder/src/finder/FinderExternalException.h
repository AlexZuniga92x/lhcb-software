#ifndef LESTER_FINDER_EXTERNAL_EXCEPTION_H
#define LESTER_FINDER_EXTERNAL_EXCEPTION_H

#include <string>
#include "FinderException.h"

namespace Lester {

  struct FinderExternalException : public FinderException {
    const std::string m_message;
    FinderExternalException()                      : m_message()  {};
    FinderExternalException(const std::string & s) : m_message(s) {};
    const std::string & message() const { return m_message; };
  };

};

#endif
