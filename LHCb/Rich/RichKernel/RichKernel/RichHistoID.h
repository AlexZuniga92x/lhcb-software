
//-----------------------------------------------------------------------------
/** @file RichHistoID.h
 *
 *  Header file for RICH utility class : RichHistoID
 *
 *  CVS Log :-
 *  $Id: RichHistoID.h,v 1.2 2006-01-17 13:57:02 jonrob Exp $
 *
 *  @author Chris Jones   Christopher.Rob.Jones@cern.ch
 *  @date   27/10/2005
 */
//-----------------------------------------------------------------------------

#ifndef RICHKERNEL_RICHHISTOID_H
#define RICHKERNEL_RICHHISTOID_H 1

// STL
#include <functional>
#include <string>

// Kernel
#include "Kernel/RichRadiatorType.h"
#include "Kernel/RichDetectorType.h"
#include "Kernel/RichParticleIDType.h"

//-----------------------------------------------------------------------------
/** @class RichHistoID RichHistoID.h RichKernel/RichHistoID.h
 *
 *  Utility class providing a convenient ID object for histograms, depending on
 *  various RICH criteria.
 *
 *  @code
 *
 *    // Create an ID object
 *    RichHistoID hid;
 *
 *    // Create the histo ID for a histogram in a given radiator
 *    // id will be "[rad]/shortName" where [rad] is either aerogel, c4f10 or cf4.
 *    std::string id = hid(rad,"shortName");
 *
 *    // Can also be used directly with GaudiHistos methods i.e.
 *    const double ckPull = ....;
 *    const Rich::RadiatorType rad = ...;
 *    plot1D( ckPull, hid(rad,"ckPull"), "(Rec-Exp)/Res Cktheta", -5, 5 );
 *
 *  @endcode  
 *
 *  @author Chris Jones   Christopher.Rob.Jones@cern.ch
 *  @date   05/04/2002
 */
//-----------------------------------------------------------------------------

class RichHistoID
{

public:

  /// Standard constructor
  RichHistoID() { }

  /** Create histo ID for given radiator type and histogram tag
   *
   *  @param rad RICH radiator type
   *  @param tag Histogram tag
   *
   *  @return Full histogram identifier string
   */
  inline const std::string operator() ( const Rich::RadiatorType rad,
                                        const std::string & tag ) const
  {
    return Rich::text(rad)+"/"+tag;
  }

  /** Create histo ID for given radiator type and histogram tag
   *
   *  @param rad RICH detector type
   *  @param tag Histogram tag
   *
   *  @return Full histogram identifier string
   */
  inline const std::string operator() ( const Rich::DetectorType det,
                                        const std::string & tag ) const
  {
    return Rich::text(det)+"/"+tag;
  }

  /** Create histo ID for given radiator type, mass hypothesis and histogram tag
   *
   *  @param rad RICH radiator type
   *  @param pid Particle mass hypothesis
   *  @param tag Histogram tag
   *
   *  @return Full histogram identifier string
   */
  inline const std::string operator() ( const Rich::RadiatorType rad,
                                        const Rich::ParticleIDType pid,
                                        const std::string & tag ) const
  {
    return Rich::text(rad)+"/"+Rich::text(pid)+"/"+tag;
  }

};

#endif // RICHKERNEL_RICHHISTOID_H
