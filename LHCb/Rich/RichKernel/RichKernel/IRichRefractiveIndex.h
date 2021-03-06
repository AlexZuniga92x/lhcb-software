
//-----------------------------------------------------------------------------
/** @file IRichRefractiveIndex.h
 *
 *  Header file for tool interface : Rich::IRefractiveIndex
 *
 *  @author Chris Jones   Christopher.Rob.Jones@cern.ch
 *  @date   15/03/2002
 */
//-----------------------------------------------------------------------------

#ifndef RICHKERNEL_IRICHREFRACTIVEINDEX_H
#define RICHKERNEL_IRICHREFRACTIVEINDEX_H 1

// Gaudi
#include "GaudiKernel/IAlgTool.h"

// LHCbKernel
#include "Kernel/RichRadiatorType.h"
#include "Kernel/RichRadIntersection.h"

/// Static Interface Identification
static const InterfaceID IID_IRichRefractiveIndex( "Rich::IRefractiveIndex", 1, 0 );

namespace Rich
{

  /** @class IRefractiveIndex IRichRefractiveIndex.h
   *
   *  Interface for tool to calculate effective refractive indices for
   *  each radiator
   *
   *  @author Chris Jones   Christopher.Rob.Jones@cern.ch
   *  @date   15/03/2002
   */

  class IRefractiveIndex : public virtual IAlgTool
  {

  public:

    /** static interface identification
     *  @return unique interface identifier
     */
    static const InterfaceID& interfaceID() { return IID_IRichRefractiveIndex; }

    /** Calculates the refractive index for a given radiator type at a
     *  given photon energy.
     *
     *  @param rad    The radiator type
     *  @param energy The photon energy at which the refractive index is to be calculated
     *
     *  @return The refractive index at the requested energy
     */
    virtual double refractiveIndex ( const Rich::RadiatorType rad,
                                     const double energy ) const = 0;

    /** Calculates the average refractive index for a given radiator type
     *  for a given range of photon energies.
     *
     *  @param rad       The radiator type
     *  @param energyBot The lower bound of photon energies
     *  @param energyTop The upper bound of photon energies
     *
     *  @return The average refractive index between the two energy bounds
     */
    virtual double refractiveIndex ( const Rich::RadiatorType rad,
                                     const double energyBot,
                                     const double energyTop ) const = 0;

    /** Calculates the average refractive index for a given radiator type
     *  for all visable photon energies.
     *
     *  @param rad       The radiator type
     *
     *  @return The overall average refractive index
     */
    virtual double refractiveIndex ( const Rich::RadiatorType rad ) const = 0;

    /** Calculates the average refractive index for a given set of radiator intersections
     *  for all visable photon energies.
     *
     *  @param intersections The radiator intersections
     *
     *  @return The overall average refractive index
     */
    virtual double refractiveIndex ( const Rich::RadIntersection::Vector & intersections ) const = 0;

    /** Calculates the refractive index R.M.S. for a given set of radiator intersections
     *  for all visable photon energies.
     *
     *  @param intersections The radiator intersections
     *
     *  @return The refractive index R.M.S.
     */
    virtual double refractiveIndexRMS ( const Rich::RadIntersection::Vector & intersections ) const = 0;

    /** Calculates the refractive index standard deviation  for a given set of radiator intersections
     *  for all visable photon energies.
     *
     *  @param intersections The radiator intersections
     *
     *  @return The refractive index S.D.
     */
    virtual double refractiveIndexSD ( const Rich::RadIntersection::Vector & intersections ) const = 0;

    /** Calculates the average refractive index for a given set of radiator intersections
     *  for all visable photon energies.
     *
     *  @param intersections The radiator intersections
     *  @param energy The photon energy at which the refractive index is to be calculated
     *
     *  @return The overall average refractive index
     */
    virtual double refractiveIndex ( const Rich::RadIntersection::Vector & intersections,
                                     const double energy ) const = 0;

  };

}

#endif // RICHKERNEL_IRICHREFRACTIVEINDEX_H
