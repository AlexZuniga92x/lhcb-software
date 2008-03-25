
//--------------------------------------------------------------------------
/** @file RichGlobalPIDLikelihoodTool.h
 *
 *  Header file for RICH Global PID tool : Rich::Rec::GlobalPID::LikelihoodTool
 *
 *  CVS Log :-
 *  $Id: RichGlobalPIDLikelihoodTool.h,v 1.1 2008-03-25 16:26:44 jonrob Exp $
 *
 *  @author Chris Jones   Christopher.Rob.Jones@cern.ch
 *  @date   2008-03-01
 */
//--------------------------------------------------------------------------

#ifndef RICHGLOBALPID_RichGlobalPIDLikelihoodTool_H
#define RICHGLOBALPID_RichGlobalPIDLikelihoodTool_H 1

// STD
#include <sstream>

// Base class
#include "RichGlobalPIDToolBase.h"
#include "RichRecBase/IRichPID.h"
#include "IRichGlobalPID.h"

// interfaces
#include "RichRecBase/IRichExpectedTrackSignal.h"
#include "RichRecBase/IRichPhotonSignal.h"
#include "IRichGlobalPIDTrackCreator.h"

// from Gaudi
#include "GaudiKernel/ToolFactory.h"

// RichKernel
#include "RichKernel/RichMap.h"

// boost
#include "boost/format.hpp"

namespace Rich
{
  namespace Rec
  {
    namespace GlobalPID
    {

      /** @class LikelihoodTool RichGlobalPIDLikelihoodTool.h
       *
       *  Tool to create RichGlobalPIDTrack track objects for the RICH
       *  Global PID algorithm
       *
       *  @author Chris Jones  Christopher.Rob.Jones@cern.ch
       *  @date   2008-03-01
       */
      class LikelihoodTool : public Rich::Rec::GlobalPID::ToolBase,
                             virtual public Rich::Rec::IRichPID,
                             virtual public Rich::Rec::GlobalPID::IRichGlobalPID
      {

      public:

        /// Standard constructor
        LikelihoodTool( const std::string& type,
                        const std::string& name,
                        const IInterface* parent );

        /// Destructor
        virtual ~LikelihoodTool( );

        // Initialize method
        virtual StatusCode initialize();

      public: // IRichPID

        // Access the RichPID associated to the given Track
        virtual const LHCb::RichPID * pid( const LHCb::Track * track ) const;

        // Access the RichPID data objects for the given Tracks
        virtual void pids( const LHCb::Track::ConstVector & tracks,
                           LHCb::RichPID::ConstVector & richpids ) const;

      public: // IRichGlobalPID

        // Access the RichGlobalPID associated to the given Track
        virtual LHCb::RichGlobalPID * pid( LHCb::RichGlobalPIDTrack * track ) const;

        // Create the RichGlobalPID data objects for the given Tracks
        virtual void pids( const LHCb::RichGlobalPIDTrack::Vector & tracks ) const;

      private: // helper classes

        // private typedefs

        /// Container for changes to be made following an event iterations
        /// Contains a pointer to a track and the its new hypothesis
        typedef Rich::Map<LHCb::RichGlobalPIDTrack*,Rich::ParticleIDType> MinTrList;

        /// Track list entry. Its current best DLL change and a pointer to the trackk
        typedef std::pair<double,LHCb::RichGlobalPIDTrack*> TrackPair;

        /// List of all track list entries
        typedef std::vector<TrackPair> TrackList;

        class InitTrackInfo
        {
        public:
          /// Container
          typedef std::vector<InitTrackInfo> Vector;
          /// Constructor
          InitTrackInfo( LHCb::RichGlobalPIDTrack * track = NULL,
                         const Rich::ParticleIDType h = Rich::Pion,
                         const double mindll = 0 )
            : pidTrack(track), hypo(h), minDLL(mindll) { }
        public:
          LHCb::RichGlobalPIDTrack * pidTrack; ///< Pointer to the track
          Rich::ParticleIDType hypo;     ///< Track hypothesis
          double minDLL;                 ///< The DLL value
        };

      private: // Private methods

        /** Starting with all tracks pion, calculate logLikelihood. Then for
         * each track in turn, holding all others to pion, calculate new
         * logLikelihood for each particle code. If less than with all pion,
         * set new minimum track hypothesis.
         * @return Number of tracks that changed mass hypothesis
         */
        unsigned int initBestLogLikelihood() const;

        /** Starting with all tracks with best hypotheses as set by
         * initBestLogLikelihood(), for each track in turn get
         * logLikelihood for each particle code, and return the track and
         * particle code which gave the optimal log likelihood.
         * @return Number of tracks that changed mass hypothesis
         */
        void findBestLogLikelihood( MinTrList & minTracks ) const;

        /// Do the event iterations
        unsigned int doIterations() const;

        /// Calculates logLikelihood for event with a given set of track hypotheses.
        /// Performs full loop over all tracks and hypotheses
        double logLikelihood() const;

        /// Computes the change in the logLikelihood produced by changing given
        /// track to the given hypothesis
        double deltaLogLikelihood( LHCb::RichRecTrack * track,
                                   const Rich::ParticleIDType newHypo ) const;

        /// Returns log( exp(signal) - 1 ) or an approximation for small signals
        double sigFunc( const double s ) const;

        /// Returns the freeze out Dll value
        double freezeOutDll() const;

        /// Returns the force change Dll value
        double forceChangeDll() const;

        /// Sets flags to say if the given set of tracks are in Rich1 and/or Rich2
        void updateRichFlags( const MinTrList & minTracks ) const;

      private:  // Private data members

        // tool pointers
        const ITrackCreator * m_gtkCreator;      ///< RichGlobalPIDTrack creator
        const IExpectedTrackSignal * m_tkSignal; ///< Track signal tool
        const IPhotonSignal * m_photonSig;       ///< Photon signal tool

        /// Pointer to the track container being considered
        mutable const LHCb::RichGlobalPIDTrack::Vector * m_gpidTracksToPID;

        /// Track DLL value to freeze track out from future iterations
        double m_freezeOutDll;

        /// Track DLL value for a forced change
        double m_forceChangeDll;

        /// Threshold for likelihood maximisation
        double m_epsilon;

        /// Maximum number of track iterations
        unsigned int m_maxEventIterations;

        /// Minimum signal value for full calculation of log(exp(signal)-1)
        double m_minSig;

        /// Cached value of log(exp(m_minSig) - 1) for efficiency
        double m_logMinSig;

        /// List of tracks ordered by change in likelihood
        mutable TrackList m_trackList;

        /// Current best event log likelihood value
        mutable double m_currentBestLL;

        /// Flag to turn on final DLL and hypothesis check
        bool m_doFinalDllCheck;

        /// Flag to turn on RICH check in LL minimisation
        bool m_richCheck;

        /// Maximum number of tracks to change in a single event iteration
        unsigned int m_maxTkChanges;

        /// Maximum number of iteration retries
        unsigned int m_maxItRetries;

        // working flags and variables
        mutable bool m_inR1, m_inR2;

      };

      inline double LikelihoodTool::forceChangeDll() const
      {
        return m_forceChangeDll;
      }

      inline double LikelihoodTool::sigFunc( const double s ) const
      {
        return ( s>m_minSig ? std::log( std::exp(s) - 1.0 ) : m_logMinSig );
      }

      inline double LikelihoodTool::freezeOutDll() const
      {
        return m_freezeOutDll;
      }

    }
  }
}

#endif // RICHGLOBALPID_RichGlobalPIDLikelihoodTool_H
