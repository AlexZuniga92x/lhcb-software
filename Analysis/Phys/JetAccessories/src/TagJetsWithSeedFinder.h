// $Id: TagJetsWithSeedFinder.cpp,v 1.3 2007/06/18 11:27:44 jpalac Exp $
// TagJetsWithSeedFinder created 25.02.06
// A file to select events with b-jets comming from heavy mother
//
// ============================================================================
// Include files
// ============================================================================
// ============================================================================
// from Gaudi
// ============================================================================
#include "GaudiAlg/GaudiTool.h"
#include "GaudiKernel/IAlgTool.h"
#include "GaudiKernel/AlgFactory.h"
// ============================================================================
// DaVinci Kernel 
// ============================================================================
#include "Kernel/DVAlgorithm.h"
// ============================================================================
// Event 
// ============================================================================
#include "Event/Particle.h"
// ============================================================================
// Event 
// ============================================================================
#include "Event/RecHeader.h"
#include "Kernel/IJetMaker.h"




using namespace Gaudi::Units ;
using namespace LHCb ;
using namespace std ;



// ============================================================================
// Main Algorithm
// ============================================================================
class TagJetsWithSeedFinder :  public DVAlgorithm
{
  friend class AlgFactory<TagJetsWithSeedFinder> ;
 public:
  /** Standard constructor
   *  @param name instance name
   *  @param pSvc pointer to Service Locator
   */
  TagJetsWithSeedFinder(const std::string& name,
			ISvcLocator* pSvcLocator)
    : DVAlgorithm(name,pSvcLocator) 
    , m_jetLoc("")  
    , m_seedLoc("")  
    , m_seedfromjet(true)  
    , m_seedFinderName ( "LoKi__SeedFinder"  )
    , m_seedFinder     ( 0   )

    {

      declareProperty ( "inputJetsLoc"        ,  m_jetLoc    ) ;
      declareProperty ( "inputSeedsLoc"        ,  m_seedLoc, "seedloctation if needed"    ) ;
      declareProperty ( "createSeedsFromJetsDau"        , m_seedfromjet,  "if true the seeds are create with the jets daugthers, if false the jet are tag with the seed located at 'inputSeedsLoc', if not set seed will be created as if the option was true"  ) ;
      declareProperty ( "bestPV"        ,  m_bestPV = false, "chose the best pv (in fct of PT) instead of (0,0,) as the origine of the particle"    ) ;
      declareProperty ( "SeedFinder",m_seedFinderName, "VVSeedFinder, UBSeedFinder, ..." );

    } ;





    /// destructor
    virtual ~TagJetsWithSeedFinder( ){};
 public:
    /** standard initialization of the algorithm
     *  @see LoKi::Algo
     *  @return status code
     */

    virtual StatusCode finalize();
    virtual StatusCode initialize ();

    /** standard execution of the algorithm
     *  @return status code
     */
    virtual StatusCode execute   () ;
 private:
    // the default constructor is disabled
    TagJetsWithSeedFinder () ;
    // the copy constructor is disabled
    TagJetsWithSeedFinder ( const  TagJetsWithSeedFinder& )  ;
    // number of particle that have to verify the cut
    TagJetsWithSeedFinder operator=( const  TagJetsWithSeedFinder& )  ;

    std::string m_jetLoc;
    std::string m_seedLoc;
    bool  m_seedfromjet ;
    bool   m_seedfromjet_save ;

    int cnttagjet;
    int cntuntagjet;
    int cntstagjet;

    int cnttagseed;
    int cntuntagseed;
    int cntstagseed;

    int evtcnt;
    int seedcnt;
    int seedcntnZ;
    int jetcntnZ;

    bool  m_bestPV ;
    std::string  m_seedFinderName ;
    mutable IJetMaker* m_seedFinder ; ///< combiner to be used 

    typedef LHCb::RelationWeighted1D< LHCb::Particle,LHCb::Particle, double >  Table ;



};


DECLARE_ALGORITHM_FACTORY( TagJetsWithSeedFinder );
// ============================================================================


class sortPt {
 public:
  inline bool operator() ( LHCb::Particle* obj1 ,
			   LHCb::Particle* obj2) { 
    return obj1->pt() > obj2->pt();      
  }
};
