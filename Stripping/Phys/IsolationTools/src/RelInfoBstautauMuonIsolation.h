#ifndef RELINFOMUONISOVARIABLES_H 
#define RELINFOMUONISOVARIABLES_H 1

// Include files
// from DaVinci, this is a specialized GaudiAlgorithm
#include "Kernel/DaVinciAlgorithm.h"
#include "Kernel/GetIDVAlgorithm.h"
#include "Kernel/IRelatedInfoTool.h"
#include "Event/RelatedInfoMap.h"
#include "GaudiAlg/GaudiTool.h"
#include "Kernel/IDistanceCalculator.h"
//needed for TMVA
#include "MVADictTools/TMVATransform.h"
#include "Kernel/IParticleDictTool.h"
#include "Event/Particle.h"
#include "IsolationInfo.h"


/** @class RelInfoBstautauMuonIsolation RelInfoBstautauMuonIsolation.h
 *
 * \brief Calculate muon isolation.
 *    Compare daughter
 *
 *    m_bdt1 : bdt value
 *
 * Options:
 *
 *   None
 *
 *  Converted from ConeVariables by A. Shires 18/06/2014
 *
 *  @author Giampiero Mancinelli
 *  @date   2014-07-25
 *
 */

class IDVAlgorithm;
class IDistanceCalculator;
class IParticleDictTool ;

class RelInfoBstautauMuonIsolation : public GaudiTool, virtual public IRelatedInfoTool {
    public:
        /// Standard constructor
        RelInfoBstautauMuonIsolation( const std::string& type,
                const std::string& name,
                const IInterface* parent );

        virtual StatusCode initialize();

        virtual StatusCode calculateRelatedInfo( const LHCb::Particle*,
                                         const LHCb::Particle*);

        virtual LHCb::RelatedInfoMap* getInfo(void);
  
        virtual std::string infoPath(void);

        virtual ~RelInfoBstautauMuonIsolation( ); ///< Destructor

    protected:

    private:

        std::vector<std::string> m_variables;
        LHCb::RelatedInfoMap m_map;
        std::vector<short int> m_keys; 

        double m_bdt1;
        double m_bdt2;

        //distance calculator
        //
        IDVAlgorithm* m_dva;
        const IDistanceCalculator*  m_dist;

        std::string m_weightsName;
        /// TMVA transform
        //Reader
        //TMVA::Reader *m_Reader;
        std::string m_transformName ;
        //variables
  double m_pvdis_h;
  double m_pvdis;
  double m_svdis_h;
  double m_svdis;
  int m_tracktype;
  double m_ips;
  double m_fc;
  double m_angle;
  double m_doca_iso;
  std::string m_TracksPath;
  std::string m_ParticlePath;
  std::string m_PVInputLocation;		\

        IParticleDictTool::DICT m_varmap ; 
        IParticleDictTool::DICT m_out ; 


        //save the vertice
        LHCb::RecVertex::Container * m_vertices ;

        std::vector<const LHCb::Particle*> m_decayParticles;

        /// Save all particles in your decay descriptor in a vector
        void saveDecayParticles( const LHCb::Particle *top);

        /// Check if your track belongs to your decay or not
        bool isTrackInDecay(const LHCb::Track* track);


        ///============================================================================
        /// Muon isolation method
        ///============================================================================

        bool calcValue( const LHCb::Particle * part
                , const LHCb::Particles * tracks
                , const LHCb::VertexBase * PV
                , const LHCb::VertexBase * SV
                ) ;
        ///============================================================================
        /// Helper functions 
        ///============================================================================
        double calcIPToAnyPV( const LHCb::Track * track ) ;
        double  enclosedAngle( Gaudi::XYZVector, Gaudi::XYZVector );
        void    getPerpFeet(  Gaudi::XYZPoint track_pos,
                Gaudi::XYZVector track_p,
                Gaudi::XYZPoint mu_pos,
                Gaudi::XYZVector mu_p, 
                Gaudi::XYZPoint& perpFootTrack, 
                Gaudi::XYZPoint& perpFootMu, 
                Gaudi::XYZPoint& vertex, bool& fail); 
        double calcFC(  Gaudi::XYZVector track_mom, 
                Gaudi::XYZVector mu_mom, 
                Gaudi::XYZPoint mu_track_vertex,
                const LHCb::VertexBase* PV);
        double arcosine (Gaudi::XYZVector, Gaudi::XYZVector);
        double calcVertexDist(Gaudi::XYZPoint muTrack, 
                const LHCb::VertexBase* v);



};

#endif // CONEVARIABLES_H
