
//-----------------------------------------------------------------------------
/** @file RichPIDPlots.h
 *
 *  Header file for RICH tool : Rich::PIDPlots
 *
 *  CVS Log :-
 *  $Id: RichPIDPlots.h,v 1.5 2009-05-04 13:54:44 jonrob Exp $
 *
 *  @author Chris Jones     Christopher.Rob.Jones@cern.ch
 *  @date   2008-04-14
 */
//-----------------------------------------------------------------------------

#ifndef RICHRECTOOLS_RICHPIDPLOTS_H
#define RICHRECTOOLS_RICHPIDPLOTS_H 1

// base class
#include "RichRecBase/RichRecHistoToolBase.h"

// interfaces
#include "RichRecBase/IRichPIDPlots.h"
#include "RichRecBase/IRichTrackSelector.h"

// Event
#include "Event/Track.h"
#include "Event/RichPID.h"
#include "Event/ProtoParticle.h"

namespace Rich
{
  namespace Rec
  {

    /** @class PIDPlots RichPIDPlots.h
     *
     *  Tool which creates standardised PID plots for
     *  monitoring and calibration purposes.
     *
     *  @author Chris Jones   Christopher.Rob.Jones@cern.ch
     *  @date   2008-04-14
     */

    class PIDPlots : public HistoToolBase,
                     virtual public IPIDPlots
    {

    public:

      /// Standard constructor
      PIDPlots( const std::string& type,
                const std::string& name,
                const IInterface* parent );

      virtual ~PIDPlots( ); ///< Destructor

      virtual StatusCode initialize();  // Tool initialization

    public:

      // Fill the plots for the given RichPID and PID hypothesis
      virtual void plots( const LHCb::RichPID * pid,
                          const Rich::ParticleIDType hypo,
                          const Configuration & config = Configuration() ) const;

      // Fill the plots for the given track and PID hypothesis
      virtual void plots( const LHCb::Track * track,
                          const Rich::ParticleIDType hypo,
                          const Configuration & config = Configuration() ) const;

      // Fill the plots for the given ProtoParticle and PID hypothesis
      virtual void plots( const LHCb::ProtoParticle * proto,
                          const Rich::ParticleIDType hypo,
                          const Configuration & config = Configuration() ) const;

    private:

      /// Get the track momentum (first State) in GeV
      inline double trackP( const LHCb::Track * track ) const
      {
        return ( track ? track->p() : 0 );
      }

      /// Get the track momentum (first State) in GeV
      inline double trackP( const LHCb::RichPID * pid ) const
      {
        return ( pid ? trackP(pid->track()) : 0 );
      }

      /// Get the track momentum (first State) in GeV
      inline double trackPt( const LHCb::Track * track ) const
      {
        return ( track ? track->pt() : 0 );
      }

      /// Get the track momentum (first State) in GeV
      inline double trackPt( const LHCb::RichPID * pid ) const
      {
        return ( pid ? trackPt(pid->track()) : 0 );
      }

      /// Return the correct histogram path
      inline const std::string hPath( const Rich::ParticleIDType hypo ) const
      {
        return Rich::text(hypo)+"/";
      }

      /// Track selection
      inline bool selected( const LHCb::Track * track,
                            const Configuration & config ) const
      {
        return ( track && 
                 trackP(track)  > config.minP &&
                 trackP(track)  < config.maxP &&
                 trackPt(track) > config.minPt &&
                 trackPt(track) < config.maxPt );
      }

    private: // data

      mutable const ITrackSelector * m_trSelector;  ///< Track selector

      // Job Options
      int m_bins;                 ///< Number of bins
      bool m_extraHistos;         ///< Fill full set of histograms
      double m_dllCut;            ///< Dll cut value  
      double m_dllRange;          ///< Dll range for plots

    };

  }
}

#endif // RICHRECTOOLS_RICHPIDPLOTS_H
