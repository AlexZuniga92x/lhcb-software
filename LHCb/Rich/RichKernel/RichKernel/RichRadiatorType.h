// $Id: RichRadiatorType.h,v 1.5 2003-10-13 16:04:52 jonrob Exp $
#ifndef RICHKERNEL_RICHRADIATORTYPE_H
#define RICHKERNEL_RICHRADIATORTYPE_H 1

// Include files
#include <string>
#include <vector>
#include <iostream>

// from Gaudi
#include "GaudiKernel/MsgStream.h"

/// namespace for RICH detector specifics
namespace Rich {

  /// Number of RICH radiators
  static const int NRadiatorTypes = 3;

  /** Enumeration for RICH radiator types
   *
   *  @author Chris Jones   Christopher.Rob.Jones@cern.ch
   *  @date   2002-06-19
   */
  enum RadiatorType {
    Aerogel  = 0,
    Rich1Gas = 1,
    Rich2Gas = 2,
    C4F10    = 1,  // backwards compatibility
    CF4      = 2,  // backwards compatibility
  };

  /// Text conversion for RadiatorType enumeration
  std::string text( const Rich::RadiatorType& radiator ) ;

  /// Std Vector typedef
  typedef std::vector<RadiatorType> RadiatorTypeVector;

}

/// Implement textual ostream << method for Rich::RadiatorType enumeration
inline std::ostream& operator << ( std::ostream& s,
                                   const Rich::RadiatorType& radiator ) {
  s << Rich::text( radiator );
  return s;
}

/// Implement textual MsgStream << method for Rich::RadiatorType enumeration
inline MsgStream& operator << ( MsgStream& s,
                                const Rich::RadiatorType& radiator ) {
  s << Rich::text( radiator );
  return s;
}

#endif // RICHKERNEL_RICHRADIATORTYPE_H
