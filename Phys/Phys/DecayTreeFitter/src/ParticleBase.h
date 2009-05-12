#ifndef __VTX_PARTICLEBASE_HH__
#define __VTX_PARTICLEBASE_HH__

#include <string>
#include <vector> 
#include "Constraint.h"
#include "Projection.h"

namespace LHCb
{
  class Particle ;
  class ParticleProperty ;
  class Trajectory ;
}

namespace decaytreefit
{
  class FitParams ;

  class ParticleBase
  {
  public:
    enum ParticleType {kInteractionPoint,
		       kRecoComposite,kRecoResonance,
		       kInternalParticle,kRecoTrack,
		       kResonance,kRecoPhoton,
		       kMissingParticle} ;

    
    // 'default' constructor
    ParticleBase(const LHCb::Particle& bc, const ParticleBase* mother) ;

    // constructor used for InteractionPoint
    ParticleBase(const std::string& name) ;

    virtual ~ParticleBase() ;

    static ParticleBase* createParticle(const LHCb::Particle& bc, 
					const ParticleBase* mother,
					bool forceFitAll=false) ;

    virtual int dim() const = 0 ;
    virtual void updateIndex(int& offset) ;
    virtual ErrCode initPar1(FitParams*) = 0 ; // init everything that does not need mother vtx
    virtual ErrCode initPar2(FitParams*) = 0 ; // everything else
    virtual ErrCode initCov(FitParams*) const  ;
    virtual std::string parname(int index) const ;
    virtual void print(const FitParams*) const ;

    const ParticleBase* locate(const LHCb::Particle& bc) const ;
    const LHCb::Particle& particle() const { return *m_particle ; }
    
    const int index() const { return m_index ; }
    const ParticleBase* mother() const { return m_mother ; }
    const std::string& name() const { return m_name ; }
    
    virtual ErrCode projectGeoConstraint(const FitParams&, Projection&) const ;
    virtual ErrCode projectMassConstraint(const FitParams&, Projection&) const ;
    virtual ErrCode projectConstraint(Constraint::Type, const FitParams&, Projection&) const ;

    virtual bool setMassConstraint(bool /*add*/) { return false ; }

    // indices to fit parameters
    virtual int type() const = 0 ;
    virtual int posIndex() const { return -1 ; }
    virtual int lenIndex() const { return -1 ; }
    virtual int momIndex() const { return -1 ; }
    
    // does the particle have a 3-momentum or a 4-momentum ?
    virtual bool hasEnergy() const { return false ; }

    // does the particle have is own decay vertex ? (resonances and
    // recoparticles do not)
    virtual bool hasPosition() const { return false ; }

    int eneIndex() const { return hasEnergy() ? momIndex()+3 : -1 ; }
    
    // calculates the global chisquare (pretty useless)
    virtual double chiSquare(const FitParams*) const ;
    
    // access to particle PDT parameters
    double pdtMass() const { return m_pdtMass ; }
    double pdtWidth() const { return m_pdtWidth ; }
    double pdtCLifeTime() const { return m_pdtCLifeTime ; }
    double pdtTau() const { return m_pdtMass >0 ? m_pdtCLifeTime/m_pdtMass : 0 ; } 
    int charge() const { return m_charge ; }

    // return a trajectory
    virtual const LHCb::Trajectory* trajectory() const { return 0 ; }

    // access to daughters
    typedef std::vector<ParticleBase*> daucontainer ;
    typedef daucontainer::const_iterator const_iterator ;
    
    const daucontainer& daughters() const { return m_daughters ; }
    const_iterator begin() const { return m_daughters.begin() ; }
    const_iterator end()   const { return m_daughters.end() ; }
    ParticleBase* addDaughter(const LHCb::Particle&, bool forceFitAll=false) ;
    void removeDaughter(const ParticleBase* pb) ;
    
    typedef std::vector< std::pair<const ParticleBase*,int> > indexmap ;
    virtual void retrieveIndexMap(indexmap& anindexmap) const ;

    void setMother(const ParticleBase* m) { m_mother = m ; } 
    
    typedef std::vector<decaytreefit::Constraint> constraintlist ;
    virtual void addToConstraintList(constraintlist& alist, int depth) const = 0 ;
    virtual int nFinalChargedCandidates() const ;
    void setParticle(const LHCb::Particle* bc) { m_particle = bc ; }

    // collect all particles emitted from vertex with position posindex
    void collectVertexDaughters( daucontainer& particles, int posindex ) ;

    //bool initFromTruth(const BtaMcAssoc& truthmap, FitParams& fitparams) const ;
  protected:
    static double pdtCLifeTime(const LHCb::Particle& bc)  ;
    static bool isAResonance(const LHCb::ParticleProperty& bc) ;
    static double bFieldOverC() { return 0 ; } // Bz/c
    ErrCode initTau(FitParams* par) const ; 
    void makeName(const LHCb::Particle& bc)  ;
    daucontainer& daughters() { return m_daughters ; }
  protected:
    void setIndex(int i) { m_index = i ; }
    void setName(const std::string& n) { m_name = n ; }
  private:
    const LHCb::Particle* m_particle ;
    const ParticleBase* m_mother ;
    std::vector<ParticleBase*> m_daughters ;
    const LHCb::ParticleProperty* m_prop ;
    int m_index ;
    double m_pdtMass ;      // cached mass
    double m_pdtWidth ;     // particle width (for mass constraints)
    double m_pdtCLifeTime ; // cached lifetime
    int m_charge ;      // charge
    std::string m_name ;
 } ;

}


#endif
