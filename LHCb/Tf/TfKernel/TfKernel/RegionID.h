
//-----------------------------------------------------------------------------
/** @file RegionID.h
 *
 *  Header file for track find class Tf::RegionID
 *
 *  CVS Log :-
 *  $Id: RegionID.h,v 1.4 2007-08-20 11:07:07 jonrob Exp $
 *
 *  @author S. Hansmann-Menzemer, W. Hulsbergen, C. Jones, K. Rinnert
 *  @date   2007-05-30
 */
//-----------------------------------------------------------------------------

#ifndef TFKERNEL_REGIONID_H
#define TFKERNEL_REGIONID_H 1

#include "VeloDet/DeVeloSensor.h"
#include "Kernel/VeloChannelID.h"

namespace Tf
{

  /** @class RegionID TfKernel/RegionID.h
   *
   *  Simple class to store detector region information and definitions 
   *  (station, layer, region etc.)
   *
   *  @author S. Hansmann-Menzemer, W. Hulsbergen, C. Jones, K. Rinnert
   *  @date   2007-05-30
   */
  class RegionID
  {

  public:

    /** @enum DetType TkKernel/RegionID.h
     *
     *  Enumerated type for detector type
     *
     *  @author S. Hansmann-Menzemer, W. Hulsbergen, C. Jones, K. Rinnert
     *  @date   2007-05-30
     */
    enum DetType
      {
        VeloR=0,    ///< Velo R sensor
        VeloPhi=1,  ///< Velo Phi sensor
        TT=2,       ///< TT sensor
        IT=3,       ///< IT sensor
        OT=4,       ///< OT detector
        UNKNOWN=5   ///< Unknown type
      };

  public:

    /** Default constructor */
    RegionID() : m_dettype(UNKNOWN),m_station(0),m_layer(0),m_region(0) {}

    /** Constructor from an OTChannelID
     *  @param[in] id The OT channel identifier for the region
     */
    RegionID( const LHCb::OTChannelID& id) ;

    /** Constructor from an STChannelID
     *  @param[in] id The ST channel identifier for the region
     */
    RegionID( const LHCb::STChannelID& id) ;

    /** Constructor from an VeloChannelID ID
     *  @param[in] id The Velo channel identifier for the region
     *  @param[in] sensor Pointer to the associated DeVeloSensor
     */
    RegionID( const LHCb::VeloChannelID& id, const DeVeloSensor* sensor) ;

    /** Returns the detector type for this region ID 
     *  XXX???XXX Would it make more sense for this method to return a DetType enum ? */
    inline unsigned char type()     const { return m_dettype ; }

    /** Returns the station number */
    inline unsigned char station()  const { return m_station ; }

    /** Returns the layer number */
    inline unsigned char layer()    const { return m_layer ; }

    /** Returns the region number */
    inline unsigned char region()   const { return m_region ; }

    /** Returns the 'velohalf' number 
     *  ( XXX???XXX Same as layer(), so why do we need both ? ) */
    inline unsigned char veloHalf() const { return m_layer ; }

    /** Return the 'zone' number
     *  ( XXX???XXX Same as region(), so again, why do we need both ? ) */
    inline unsigned char zone()     const { return m_region ; }

    /** @struct OTIndex TkKernel/RegionID.h
     *  Collection of detector information for OT
     *  @author S. Hansmann-Menzemer, W. Hulsbergen, C. Jones, K. Rinnert
     *  @date   2007-05-30
     */
    struct OTIndex 
    {
      /// Anonymous enum for OT information
      enum 
        { 
          kMinStation=0, ///< Minimum valid station number for OT
          kMaxStation=2, ///< Maximum valid station number for OT
          kNStations=3,  ///< Number of OT stations
          kMinLayer=0,   ///< Minimum valid layer number for an OT station
          kMaxLayer=3,   ///< Maxmimum valid layer number for an OT station
          kNLayers=4,    ///< Number of OT layers per station
          kMinRegion=0,  ///< Minimum valid region number for an OT layer
          kMaxRegion=1,  ///< Maximum valid region number for an OT layer
          kNRegions=2    ///< Number of OT regions per layer
        } ;
    } ;

   /** @struct ITIndex TkKernel/RegionID.h
     * Collection of detector information for IT
     * @author S. Hansmann-Menzemer, W. Hulsbergen, C. Jones, K. Rinnert
     * @date   2007-05-30
     */
    struct ITIndex 
    {
      /// Anonymous enum for IT information
      enum 
        { 
          kMinStation=0, ///< Minimum valid station number for IT
          kMaxStation=2, ///< Maximum valid station number for IT
          kNStations=3,  ///< Number of IT stations
          kMinLayer=0,   ///< Minimum valid layer number for an IT station
          kMaxLayer=3,   ///< Maximum valid layer number for an IT station
          kNLayers=4,    ///< Number of IT layers per station
          kMinRegion=0,  ///< Minimum valid region number for an IT layer
          kMaxRegion=3,  ///< Maximum valid region number for an IT layer
          kNRegions=4    ///< Number of IT regions per layer
        } ;
    } ;

   /** @struct TTIndex TkKernel/RegionID.h
     * Collection of detector information for TT
     * @author S. Hansmann-Menzemer, W. Hulsbergen, C. Jones, K. Rinnert
     * @date   2007-05-30
     */
    struct TTIndex 
    {
      enum 
        { 
          kMinStation=0, ///< Minimum valid station number for TT
          kMaxStation=1, ///< Maximum valid station number for TT
          kNStations=2,  ///< Number of TT stations
          MinLayer=0,    ///< Minimum valid layer number for a TT station
          kMaxLayer=1,   ///< Maximum valid layer number for a TT station
          kNLayers=2,    ///< Number of TT layers within a station
          kMinRegion=0,  ///< Minimum valid region number for a TT layer
          kMaxRegion=11, ///< Maximum valid region number for a TT layer
          kNRegions=12   ///< Number of TT regions within a layer
        } ;
    } ;

    /** @struct VeloRIndex TkKernel/RegionID.h
     * Collection of detector information for Velo R sensors
     * @author S. Hansmann-Menzemer, W. Hulsbergen, C. Jones, K. Rinnert
     * @date   2007-05-30
     */
    struct VeloRIndex 
    {
      enum 
        { 
          kMinStation=0,  ///< Minimum valid station number for Velo-R
          kMaxStation=20, ///< Maximum valid station number for Velo-R
          kNStations=21,  ///< Number of Velo-R stations
          kMinHalf=0,     ///< Minimum valid Velo-R half number
          kMaxHalf=1,     ///< Maximum valid Velo-R half number
          kNHalfs=2,      ///< Number of Velo-R halfs
          kMinZone=0,     ///< Minimum valid Velo-R zone number (XXX???XXX Why do we call these zones and not regions like other sub-dets ?)
          kMaxZone=3,     ///< Maximum valid Velo-R zone number (XXX???XXX Why do we call these zones and not regions like other sub-dets ?)
          kNZones=4       ///< Number of Velo-R zones
        } ;
    } ;

    /** @struct VeloPhiIndex TkKernel/RegionID.h
     *  Collection of detector information for Velo Phi sensors
     *  @author S. Hansmann-Menzemer, W. Hulsbergen, C. Jones, K. Rinnert
     *  @date   2007-05-30
     */
    struct VeloPhiIndex 
    {
      enum 
        { 
          kMinStation=0,  ///< Minimum valid station number for Velo-Phi
          kMaxStation=20, ///< Maximum valid station number for Velo-Phi
          kNStations=21,  ///< Number of Velo-Phi stations
          kMinHalf=0,     ///< Minimum valid Velo-Phi half number
          kMaxHalf=1,     ///< Maximum valid Velo-Phi half number
          kNHalfs=2,      ///< Number of Velo-Phi halfs
          kMinZone=0,     ///< Minimum valid Velo-Phi zone number (XXX???XXX Why do we call these zones and not regions like other sub-dets ?)
          kMaxZone=1,     ///< Maximum valid Velo-Phi zone number (XXX???XXX Why do we call these zones and not regions like other sub-dets ?)
          kNZones=2       ///< Number of Velo-Phi zones
        } ;
    } ;

    /** Comparision operator for RegionID objects
     *  @param[in] rhs RHS of comparision == operator
     *  @return The result of the comparison
     *  @retval TRUE  The regions are the same
     *  @retval FALSE The regions are different
     */
    bool operator==(const RegionID& rhs) const 
    {
      return ( this->type()    == rhs.type()    && 
               this->station() == rhs.station() && 
               this->layer()   == rhs.layer()   && 
               this->region()  == rhs.region()   ); 
    }

  private:

    unsigned char m_dettype ; ///< The detector type
    unsigned char m_station ; ///< The station number (all indices start numbering from 0)
    unsigned char m_layer   ; ///< The layer number within a station (also encodes VELO half)
    unsigned char m_region  ; ///< The region number within a layer (also encodes zone on a VELO sensor in the global frame)

  } ;

  inline RegionID::RegionID( const LHCb::OTChannelID& id )
    : m_dettype ( OT             ), 
      m_station ( id.station()-1 ), 
      m_layer   ( id.layer()     ), 
      m_region  ( id.quarter()/2 ) 
  { }

  inline RegionID::RegionID( const LHCb::STChannelID& id )
    : m_dettype ( id.isTT() ? TT : IT ), 
      m_station ( id.station()-1      ), 
      m_layer   ( id.layer()-1        ),
      // XXX???XXX Would be nice to document the reasoning behind this 'magic conversion' somewhere ?
      m_region  ( id.isTT() ? (id.detRegion()-1)*4 + (id.sector()-1)%4 : id.detRegion()-1 ) 
  { }

  inline RegionID::RegionID( const LHCb::VeloChannelID& id, 
                             const DeVeloSensor* sensor )
    : m_dettype ( VeloR+static_cast<int>(sensor->isPhi()) ),
      m_station ( 0 ),
      m_layer   ( static_cast<unsigned char>(sensor->isRight()) ),
      // XXX???XXX Would be nice to document the reasoning behind this 'magic conversion' somewhere ?
      m_region  ( sensor->isR() && sensor->isDownstream() ? 3-sensor->zoneOfStrip(id.strip()) : sensor->zoneOfStrip(id.strip()) )
  { }

} // Tf namespace ends

#endif // TFKERNEL_REGIONID_H

