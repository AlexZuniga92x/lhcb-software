// $Id$
// ============================================================================
#ifndef LOKI_ALGOMC_H 
#define LOKI_ALGOMC_H 1
// ============================================================================
// Include files
// ============================================================================
// MCInterfaces
// ============================================================================
#include "MCInterfaces/IMCDecayFinder.h"
// ============================================================================
// LoKiAlgo 
// ============================================================================
#include "LoKi/Algo.h"
// ============================================================================
// LoKiMC 
// ============================================================================
#include "LoKi/MCTypes.h"
#include "LoKi/MCFinder.h"
#include "LoKi/MCFinderObj.h"
#include "LoKi/MCMatch.h"
#include "LoKi/MCMatchObj.h"
// ============================================================================
// LoKiGen
// ============================================================================
#include "LoKi/MCTypes.h"
#include "LoKi/IMCDecay.h"
#include "LoKi/GenTypes.h"
#include "LoKi/IGenDecay.h"
// ============================================================================
// LoKiGenMC
// ============================================================================
#include "Kernel/IMC2Collision.h"
#include "Kernel/IHepMC2MC.h"
// ============================================================================
// LoKiPhysMC
// ============================================================================
#include "Kernel/IPV2MC.h"
// ============================================================================
/** @file
 *
 *  This file is a part of LoKi project - 
 *    "C++ ToolKit  for Smart and Friendly Physics Analysis"
 *
 *  The package has been designed with the kind help from
 *  Galina PAKHLOVA and Sergey BARSUK.  Many bright ideas, 
 *  contributions and advices from G.Raven, J.van Tilburg, 
 *  A.Golutvin, P.Koppenburg have been used in the design.
 *
 *  @author Vanya BELYAEV ibelyaev@physics.syr.edu
 *  @date 2006-03-31 
 */
// ============================================================================
namespace LoKi 
{
  // ==========================================================================
  /** @class AlgoMC LoKi/AlgoMC.h
   *
   *  @author Vanya BELYAEV ibelyaev@physics.syr.edu 
   *  @date   2006-03-31
   */
  class GAUDI_API AlgoMC : public LoKi::Algo
  {
  public:
    // ========================================================================
    /** 'Select' the MC particles to be used in local storage
     *  
     *  - The MC Particles are selected from the TES location
     *
     *  @code
     *
     *  MCRange kaons = mcselect( "Kaons" , 
     *                  "K+" == MCABSID  && MCP > 5 * GeV );
     *
     *  @endcode
     *
     *  - The example illustrate the 'selection'/'filtering of MC particles, 
     *    which are @c K+ or @c K- and have a
     *  momentum in excess of 5GeV/c
     *  - The selected MC particles
     *  are stored inside local LoKi storage under the tag @c "Kaons"
     *  and returned as a sequence of MC particles @c kaons .
     *
     *  @see LHCb::MCParticle 
     *  @see LoKi::Types::MCRange
     *  @see LoKi::Cuts::MCABSID 
     *  @see LoKi::Cuts::MCP
     *  @param tag      name/tag assigned to the selected particles
     *  @param cut      cut to be applied
     *  @param location TES location of MC-particles
     *  @return selected range of particles
     */
    LoKi::Types::MCRange 
    mcselect 
    ( const std::string&         tag                                          , 
      const LoKi::Types::MCCuts& cut                                          ,
      const std::string&         location = LHCb::MCParticleLocation::Default ) ;
    // ========================================================================
    /** 'Select' the MC particles to be used in local storage
     *  
     *  @code
     *
     *  MCRange kaons = mcselect( "Kaons" , 
     *                  "K+" == MCABSID  && MCP > 5 * GeV );
     *  MCRange kPlus = mcselect( "K+" , kaons , MC3Q > 0 ) ;
     *
     *  @endcode
     *
     *  - The example illustrate the 'selection'/'filtering of MC particles, 
     *    which are @c K+ or @c K- and have a
     *  momentum in excess of 5GeV/c
     *  - The selected MC particles
     *  are stored inside local LoKi storage under the tag @c "Kaons"
     *  and returned as a sequence of MC particles @c kaons .
     *
     *  @see LHCb::MCParticle 
     *  @see LoKi::Types::MCRange
     *  @see LoKi::Cuts::MCABSID 
     *  @see LoKi::Cuts::MCP
     *  @see LoKi::Cuts::MC3Q
     *  @param tag   name/tag assigned to the selected particles
     *  @param range input MC-particles 
     *  @param cut   cut to be applied
     *  @return selected range of particles
     */    
    LoKi::Types::MCRange 
    mcselect 
    ( const std::string&          tag   , 
      const LoKi::Types::MCRange& range ,
      const LoKi::Types::MCCuts&  cut   ) 
    {
      return mcselect ( tag , range.begin() , range.end() , cut ) ;
    } 
    // ========================================================================    
    /** 'Select' the MC particles to be used in local storage
     *  
     *  @code
     *
     *  LHCb::MCParticle::ConstVector kaons = ... ;
     *  MCRange kPlus = mcselect( "K+" , kaons , MC3Q > 0 ) ;
     *
     *  @endcode
     *
     *  @see LHCb::MCParticle 
     *  @see LoKi::Types::MCRange
     *  @see LoKi::Cuts::MC3Q
     *  @param tag   name/tag assigned to the selected particles
     *  @param range input MC-particles 
     *  @param cut   cut to be applied
     *  @return selected range of particles
     */
    LoKi::Types::MCRange 
    mcselect 
    ( const std::string&                   tag   , 
      const LHCb::MCParticle::ConstVector& range ,
      const LoKi::Types::MCCuts&           cut   ) 
    {
       return mcselect ( tag , range.begin() , range.end() , cut ) ;
    } 
    // ========================================================================    
    /** 'Select' the MC particles to be used in local storage
     *  
     *  @code
     *
     *  const LHCb::MCParticles* kaons = ... ;
     *  MCRange kPlus = mcselect( "K+" , kaons , MC3Q > 0 ) ;
     *
     *  @endcode
     *
     *  @see LHCb::MCParticle 
     *  @see LoKi::Types::MCRange
     *  @see LoKi::Cuts::MC3Q
     *  @param tag   name/tag assigned to the selected particles
     *  @param range input MC-particles 
     *  @param cut   cut to be applied
     *  @return selected range of particles
     */
    LoKi::Types::MCRange 
    mcselect 
    ( const std::string&                   tag   , 
      const LHCb::MCParticle::Container*   range ,
      const LoKi::Types::MCCuts&           cut   ) 
    {
      if ( 0 == range ) 
      {
        Error ( "LHCb::MCParticles* points to null, return empty range!" ) ;
        return LoKi::Types::MCRange() ;
      }
      return mcselect ( tag , range->begin() , range->end() , cut ) ;
    } 
    // ========================================================================    
    /** 'Select' the MC particles to be used in local storage
     *  
     *  @code
     *
     *  SEQUENCE kaons = ... ;
     *  MCRange kPlus = mcselect( "K+" , 
     *                 kaons.begin () , 
     *                 kaons.end   () , MC3Q > 0 ) ;
     *
     *  @endcode
     *
     *  @see LHCb::MCParticle 
     *  @see LoKi::Types::MCRange
     *  @see LoKi::Cuts::MC3Q
     *  @param name name/tag assigned to the selected particles
     *  @param first 'begin'-itertor for input  MC-particles 
     *  @param last  'end'-itertor for input  MC-particles 
     *  @param cut  cut to be applied
     *  @return selected range of particles
     */
    template <class MCPARTICLE, class CUT>
    LoKi::Types::MCRange          
    mcselect    
    ( const std::string&   name   , 
      MCPARTICLE           first  , 
      MCPARTICLE           last   , 
      const CUT&           cut    ) 
    {
      return m_mcselected.add ( name , first , last , cut ) ;
    } 
    // ========================================================================    
  private:
    // ========================================================================
    /// select (mc)particles according to decay-finder and cuts
    template <class MCPARTICLE, class MCCUT>
    LoKi::Types::MCRange 
    mcselect 
    ( const std::string&              tag    ,
      MCPARTICLE                      first  , 
      MCPARTICLE                      last   ,
      const Decays::IMCDecay::Finder& finder ,
      const MCCUT&                    cut    ) ;
    // ========================================================================    
  public: // use decay finders
    // ========================================================================
    /** 'Select' the MC particles to be used in local storage 
     *  
     *  - The MC Particles are selected from the TES location
     *
     *  @code
     *
     *  const Decays:::IMCDecay::Finder& finder = ... ; 
     *  MCRange good = mcselect( "Good" , finder ) ;
     *
     *  @endcode
     *
     *  @see LHCb::MCParticle 
     *  @see Decays::IMCDecay
     *  @see Decays::IMCDecay::Finder 
     *
     *  @param tag      name/tag assigned to the selected particles
     *  @param finder   the decay finder to be used 
     *  @param location TES location of MC-particles
     *  @return selected range of particles
     */
    LoKi::Types::MCRange 
    mcselect 
    ( const std::string&              tag                                          , 
      const Decays::IMCDecay::Finder& finder                                       ,
      const std::string&              location = LHCb::MCParticleLocation::Default ) ;
    // ========================================================================    
    /** 'Select' the MC particles to be used in local storage 
     *  
     *  - The MC Particles are selected from the TES location
     *
     *  @code
     *
     *  const Decays:::IMCDecay::iTree& tree = ... ; 
     *  MCRange good = mcselect( "Good" , tree ) ;
     *
     *  @endcode
     *
     *  @see LHCb::MCParticle 
     *  @see Decays::IMCDecay
     *  @see Decays::IMCDecay::iTree
     *
     *  @param tag      name/tag assigned to the selected particles
     *  @param finder   the decay finder to be used 
     *  @param location TES location of MC-particles
     *  @return selected range of particles
     */
    LoKi::Types::MCRange 
    mcselect 
    ( const std::string&              tag                                          , 
      const Decays::IMCDecay::iTree&  finder                                       ,
      const std::string&              location = LHCb::MCParticleLocation::Default ) ;
    // ========================================================================    
    /** 'Select' the MC particles to be used in local storage 
     *  
     *  - The MC Particles are selected from the TES location
     *
     *  @code
     *
     *  const std::string& descriptor  = ... ; 
     *  MCRange good = mcselect( "Good" , descriptor ) ;
     *
     *  @endcode
     *
     *  @see LHCb::MCParticle 
     *
     *  @param tag         name/tag assigned to the selected particles
     *  @param descriptor  the decay descriptor 
     *  @param location TES location of MC-particles
     *  @return selected range of particles
     */
    LoKi::Types::MCRange 
    mcselect 
    ( const std::string& tag                                          , 
      const std::string& descriptor                                   ,
      const std::string& location = LHCb::MCParticleLocation::Default ) ;
    // ========================================================================    
  public: // use decay finders
    // ========================================================================
    /** 'Select' the MC particles to be used in local storage 
     *  
     *  - The MC Particles are selected from the TES location
     *
     *  @code
     *
     *  const Decays:::IMCDecay::Finder& finder = ... ; 
     *  const MCRange input = ...; 
     *
     *  MCRange good = mcselect( "Good" , input , finder ) ;
     *
     *  @endcode
     *
     *  @see LHCb::MCParticle 
     *  @see Decays::IMCDecay
     *  @see Decays::IMCDecay::Finder 
     *
     *  @param tag      name/tag assigned to the selected particles
     *  @param input    the inptu range 
     *  @param finder   the decay finder to be used 
     *  @param location TES location of MC-particles
     *  @return selected range of particles
     */
    LoKi::Types::MCRange 
    mcselect 
    ( const std::string&              tag    , 
      const LoKi::Types::MCRange&     input  , 
      const Decays::IMCDecay::Finder& finder ) ;
    // ========================================================================    
    /** 'Select' the MC particles to be used in local storage 
     *  
     *  - The MC Particles are selected from the TES location
     *
     *  @code
     *
     *  const Decays:::IMCDecay::Finder& finder = ... ; 
     *  const LHCb::MCParticle::ConstVector input = ...; 
     *
     *  MCRange good = mcselect( "Good" , input , finder ) ;
     *
     *  @endcode
     *
     *  @see LHCb::MCParticle 
     *  @see Decays::IMCDecay
     *  @see Decays::IMCDecay::Finder 
     *
     *  @param tag      name/tag assigned to the selected particles
     *  @param input    the inptu range 
     *  @param finder   the decay finder to be used 
     *  @param location TES location of MC-particles
     *  @return selected range of particles
     */
    LoKi::Types::MCRange 
    mcselect 
    ( const std::string&                   tag    , 
      const LHCb::MCParticle::ConstVector& input  , 
      const Decays::IMCDecay::Finder&      finder ) ;
    // ========================================================================    
    /** 'Select' the MC particles to be used in local storage 
     *  
     *  - The MC Particles are selected from the TES location
     *
     *  @code
     *
     *  const Decays:::IMCDecay::iTree& tree = ... ; 
     *  const MCRange input = ...; 
     *
     *  MCRange good = mcselect( "Good" , input , tree ) ;
     *
     *  @endcode
     *
     *  @see LHCb::MCParticle 
     *  @see Decays::IMCDecay
     *  @see Decays::IMCDecay::Finder 
     *
     *  @param tag      name/tag assigned to the selected particles
     *  @param input    the input range 
     *  @param yree    the decay tree 
     *  @param location TES location of MC-particles
     *  @return selected range of particles
     */
    LoKi::Types::MCRange 
    mcselect 
    ( const std::string&              tag    , 
      const LoKi::Types::MCRange&     input  , 
      const Decays::IMCDecay::iTree&  tree   ) ;
    // ========================================================================    
    /** 'Select' the MC particles to be used in local storage 
     *  
     *  - The MC Particles are selected from the TES location
     *
     *  @code
     *
     *  const Decays:::IMCDecay::iTree& tree = ... ; 
     *  const LHCb::MCParticle::ConstVector input = ...; 
     *
     *  MCRange good = mcselect( "Good" , input , tree ) ;
     *
     *  @endcode
     *
     *  @see LHCb::MCParticle 
     *  @see Decays::IMCDecay
     *  @see Decays::IMCDecay::Finder 
     *
     *  @param tag      name/tag assigned to the selected particles
     *  @param input    the input range 
     *  @param tree    the decay tree 
     *  @param location TES location of MC-particles
     *  @return selected range of particles
     */
    LoKi::Types::MCRange 
    mcselect 
    ( const std::string&                   tag    , 
      const LHCb::MCParticle::ConstVector& input  , 
      const Decays::IMCDecay::iTree&       tree   ) ;
    // ========================================================================    
    /** 'Select' the MC particles to be used in local storage 
     *  
     *  - The MC Particles are selected from the TES location
     *
     *  @code
     *
     *  const std::string& descriptor = ... ; 
     *  const MCRange input = ...; 
     *
     *  MCRange good = mcselect( "Good" , input , descriptor ) ;
     *
     *  @endcode
     *
     *  @see LHCb::MCParticle 
     *  @see Decays::IMCDecay
     *  @see Decays::IMCDecay::Finder 
     *
     *  @param tag      name/tag assigned to the selected particles
     *  @param input    the input range 
     *  @param descriptor the decay descriptor
     *  @param location TES location of MC-particles
     *  @return selected range of particles
     */
    LoKi::Types::MCRange 
    mcselect 
    ( const std::string&          tag        , 
      const LoKi::Types::MCRange& input      , 
      const std::string&          descriptor ) ;
    // ========================================================================    
    /** 'Select' the MC particles to be used in local storage 
     *  
     *  - The MC Particles are selected from the TES location
     *
     *  @code
     *
     *  const std::string& descriptor = ... ; 
     *  const LHCb::MCParticle::ConstVector input = ...; 
     *
     *  MCRange good = mcselect( "Good" , input , descriptor ) ;
     *
     *  @endcode
     *
     *  @see LHCb::MCParticle 
     *  @see Decays::IMCDecay
     *  @see Decays::IMCDecay::Finder 
     *
     *  @param tag      name/tag assigned to the selected particles
     *  @param input    the input range 
     *  @param descriptor the decay descriptor
     *  @param location TES location of MC-particles
     *  @return selected range of particles
     */
    LoKi::Types::MCRange 
    mcselect 
    ( const std::string&                   tag        , 
      const LHCb::MCParticle::ConstVector& input      , 
      const std::string&                   descriptor ) ;
    // ========================================================================    
  public:
    // ========================================================================    
    /** 'Select' the MC vertices to be used in local storage
     *  
     *  - The MC vertices are selected from the TES location
     *
     *  @code
     *
     *  MCVRange primary = mcvselect( "MCPVs" , MCISPRIMARY );
     *
     *  @endcode
     *
     *  @see LHCb::MCVertex
     *  @see LoKi::Types::MCVRange
     *  @see LoKi::Cuts::MCISPRIMARY 
     *  @param tag      name/tag assigned to the selected particles
     *  @param cut      cut to be applied
     *  @param location TES location of MC-vertices
     *  @return selected range of particles
     */
    LoKi::Types::MCVRange 
    mcvselect 
    ( const std::string&          tag                                          , 
      const LoKi::Types::MCVCuts& cut                                          ,
      const std::string&          location = LHCb::MCVertexLocation::Default ) ;
    // ========================================================================    
    /** 'Select' the MC vertices to be used in local storage
     *  
     *  @code
     *
     *  MCVRange all     =  ... ;
     *  MCVRange primary = mcvselect( "MCPVs" , all , MCISPRIMARY );
     *
     *  @endcode
     *
     *  @see LHCb::MCVertex
     *  @see LoKi::Types::MCVRange
     *  @see LoKi::Cuts::MCISPRIMARY 
     *  @param tag   name/tag assigned to the selected particles
     *  @param range input MC-vertices
     *  @param cut   cut to be applied
     *  @return selected range of particles
     */
    LoKi::Types::MCVRange 
    mcvselect 
    ( const std::string&           tag   ,
      const LoKi::Types::MCVRange& range ,
      const LoKi::Types::MCVCuts&  cut   )
    {
      return mcvselect ( tag , range.begin() , range.end()  , cut ) ;
    } 
    // ========================================================================    
    /** 'Select' the MC vertices to be used in local storage
     *  
     *  @code
     *
     *  LHCb::MCVertex::ConstVector all = ... ;  
     *  MCVRange primary = mcvselect( "MCPVs" , all , MCISPRIMARY );
     *
     *  @endcode
     *
     *  @see LHCb::MCVertex
     *  @see LoKi::Types::MCVRange
     *  @see LoKi::Cuts::MCISPRIMARY 
     *  @param tag   name/tag assigned to the selected particles
     *  @param range input MC-vertices
     *  @param cut   cut to be applied
     *  @return selected range of particles
     */
    LoKi::Types::MCVRange 
    mcvselect 
    ( const std::string&                   tag   ,
      const LHCb::MCVertex::ConstVector&   range ,
      const LoKi::Types::MCVCuts&          cut   )
    {
      return mcvselect ( tag , range.begin() , range.end()  , cut ) ;
    } 
    // ========================================================================
    /** 'Select' the MC vertices to be used in local storage
     *  
     *  @code
     *
     *  const LHCb::MCVertices* all = ... ;  
     *  MCVRange primary = mcvselect( "MCPVs" , all , MCISPRIMARY );
     *
     *  @endcode
     *
     *  @see LHCb::MCVertex
     *  @see LoKi::Types::MCVRange
     *  @see LoKi::Cuts::MCISPRIMARY 
     *  @param tag   name/tag assigned to the selected particles
     *  @param range input MC-vertices
     *  @param cut   cut to be applied
     *  @return selected range of particles
     */
    LoKi::Types::MCVRange 
    mcvselect 
    ( const std::string&                   tag   ,
      const LHCb::MCVertex::Container*     range ,
      const LoKi::Types::MCVCuts&          cut   )
    {
      if ( 0 == range ) 
      {
        Error ( "LHCb::MCVertices* points to null, return empty range!" ) ;
        return LoKi::Types::MCVRange() ;
      }
      return mcvselect ( tag , range->begin() , range->end()  , cut ) ;
    } 
    // ========================================================================
    /** 'Select' the MC particles to be used in local storage
     *  
     *  @code
     *
     *  SEQUENCE all = ... ;
     *  MCVRange primary = mcvselect( "MCPVs" , 
     *                     all.begin () , 
     *                     all.end   () , MCPRIMARY ) ;
     *
     *  @endcode
     *
     *  @see LHCb::MCVertex
     *  @see LoKi::Types::MCVRange
     *  @see LoKi::Cuts::MCPRIMARY
     *  @param name name/tag assigned to the selected particles
     *  @param first 'begin'-itertor for input  MC-particles 
     *  @param last  'end'-itertor for input  MC-particles 
     *  @param cut  cut to be applied
     *  @return selected range of particles
     */
    template <class MCVERTEX, class CUT>
    LoKi::Types::MCVRange          
    mcvselect    
    ( const std::string&   name   , 
      MCVERTEX             first  , 
      MCVERTEX             last   , 
      const CUT&           cut    ) 
    {
      return m_mcvselected.add ( name , first , last , cut ) ;
    } 
    // ========================================================================
  public:    
    // ========================================================================
    /** Extract the selected MC-particles from local LoKi storage  
     *   by their name/tag
     *  
     *  @code
     *
     *  MCRange kaons = mcselected( "Kaons");
     *
     *  @endcode
     *
     *  @param  tag name/tag assigned to the selected vertices
     *  @return selected range of particles
     */
    LoKi::Types::MCRange 
    mcselected 
    ( const std::string& tag ) const { return m_mcselected( tag ) ; }
    // ========================================================================
    /** Extract the selected MC-vertices from local LoKi storage  
     *  by their name/tag
     *  
     *  @code
     *
     *  MCVRange primaries = mcvselected( "MCPVs");
     *
     *  @endcode
     *
     *  @param  tag name/tag assigned to the selected vertices
     *  @return selected range of particles
     */
    LoKi::Types::MCVRange 
    mcvselected
    ( const std::string& tag ) const { return m_mcvselected( tag ) ; } 
    // ========================================================================    
  public:
    // ========================================================================    
    /** 'Select' generator(HepMC) particles to be used in local storage.
     *  
     *   @code
     * 
     *     GRange pions = gselect("gpi" , "pi+" == GABSID && GPT > 1 * GeV ) ;
     *
     *   @endcode 
     * 
     *   @see HepMC::GenParticle
     *   @see LoKi::Types::GRange
     *   @see LoKi::Types::GCuts
     *   @see LHCb::HepMCEvent 
     *   @see LoKi::Cuts::GABSID 
     *   @see LoKi::Cuts::GPT
     *
     *   @param tag the unique tag to be associated with selecte dparticles 
     *   @param cuts the selection cuts to be applied 
     *   @param location TES location of LHCb::HepMCEvent::Container
     *   @return selected particles, saved into the local storage 
     */
    LoKi::Types::GRange 
    gselect 
    ( const std::string&        tag                                          , 
      const LoKi::Types::GCuts& cuts                                         ,
      const std::string&        location = LHCb::HepMCEventLocation::Default ) ;
    // ========================================================================    
    /** 'Select' generator(HepMC) from previously selected 'container' 
     *  
     *   @code
     *   
     *   // select all good pions 
     *   GRange pions = gselect("gpi" , "pi+" == GABSID && GPT > 1 * GeV ) ;
     *
     *   // select only positive pions from all good pions 
     *   GRange positive = gselect ( "gpi+" , pions , 0 < G3Q ) ;
     *
     *   @endcode 
     * 
     *   @see HepMC::GenParticle
     *   @see LoKi::Types::GRange
     *   @see LoKi::Types::GCuts
     *   @see LoKi::Cuts::GABSID 
     *   @see LoKi::Cuts::GPT
     *   @see LoKi::Cuts::G3Q
     *
     *   @param tag the unique tag to be associated with selecte dparticles 
     *   @param range the source 
     *   @param cut   the selection cuts to be applied 
     *   @return selected particles, saved into the local storage 
     */
    LoKi::Types::GRange 
    gselect 
    ( const std::string&         tag   ,
      const LoKi::Types::GRange& range , 
      const LoKi::Types::GCuts&  cut   ) 
    {
      return gselect ( tag , range.begin() , range.end() , cut ) ;
    } 
    // ========================================================================    
    /** 'Select' generator(HepMC) from previously selected 'container' 
     *  
     *   @code
     *   
     *   LoKi::GenTypes::GenContainer pions = ... ;
     *   // select only positive particles form the container 
     *   GRange positive = gselect ( "gpi+" , pions , 0 < G3Q ) ;
     *
     *   @endcode 
     * 
     *   @see HepMC::GenParticle
     *   @see LoKi::Types::GRange
     *   @see LoKi::GenTypes::GenContainer
     *   @see LoKi::Types::GCuts
     *   @see LoKi::Cuts::G3Q
     *
     *   @param tag   the unique tag to be associated with selecte dparticles 
     *   @param range the source 
     *   @param cut   the selection cuts to be applied 
     *   @return selected particles, saved into the local storage 
     */
    LoKi::Types::GRange 
    gselect 
    ( const std::string&                  tag   ,
      const LoKi::GenTypes::GenContainer& range , 
      const LoKi::Types::GCuts&           cut   ) 
    {
      return gselect ( tag , range.begin() , range.end() , cut ) ;
    } 
    // ========================================================================    
    /** 'Select' generator(HepMC) from HepMC tree 
     *  
     *   @code
     *   
     *   const LHCb::HepMCEvent::Container* all = ... ;
     *   // select only positive particles form the container 
     *   GRange positive = gselect ( "positive" , all , 0 < G3Q ) ;
     *
     *   @endcode 
     * 
     *   @see HepMC::GenParticle
     *   @see LoKi::Types::GRange
     *   @see LHCb::HepMCEvent
     *   @see LHCb::HepMCEvent::Container 
     *   @see LoKi::Types::GCuts
     *   @see LoKi::Cuts::G3Q
     *
     *   @param tag    the unique tag to be associated with selecte dparticles 
     *   @param events the source 
     *   @param cut    the selection cuts to be applied 
     *   @return selected particles, saved into the local storage 
     */
    LoKi::Types::GRange 
    gselect 
    ( const std::string&                  tag    ,
      const LHCb::HepMCEvent::Container*  events , 
      const LoKi::Types::GCuts&           cut    ) ;
    // ========================================================================    
    /** 'Select' generator(HepMC) from HepMC tree 
     *  
     *   @code
     *   
     *   const LHCb::HepMCEvent* event = ... ;
     *   // select only positive particles form the event
     *   GRange positive = gselect ( "positive" ,event , 0 < G3Q ) ;
     *
     *   @endcode 
     * 
     *   @see HepMC::GenParticle
     *   @see LoKi::Types::GRange
     *   @see LHCb::HepMCEvent
     *   @see LoKi::Types::GCuts
     *   @see LoKi::Cuts::G3Q
     *
     *   @param tag   the unique tag to be associated with selecte dparticles 
     *   @param event the source 
     *   @param cut   the selection cuts to be applied 
     *   @return selected particles, saved into the local storage 
     */
    LoKi::Types::GRange 
    gselect 
    ( const std::string&                  tag    ,
      const LHCb::HepMCEvent*             event  , 
      const LoKi::Types::GCuts&           cut    ) ;
    // ========================================================================    
    /** 'Select' generator(HepMC) from HepMC tree 
     *  
     *   @code
     *   
     *   const HepMC::GenEvent* event = ... ;
     *   // select only positive particles form the event
     *   GRange positive = gselect ( "positive" ,event , 0 < G3Q ) ;
     *
     *   @endcode 
     * 
     *   @see HepMC::GenParticle
     *   @see LoKi::Types::GRange
     *   @see HepMC::GenEvent
     *   @see LoKi::Types::GCuts
     *   @see LoKi::Cuts::G3Q
     *
     *   @param tag   the unique tag to be associated with selecte dparticles 
     *   @param event the source 
     *   @param cut   the selection cuts to be applied 
     *   @return selected particles, saved into the local storage 
     */
    LoKi::Types::GRange 
    gselect 
    ( const std::string&                  tag    ,
      const HepMC::GenEvent*              event  , 
      const LoKi::Types::GCuts&           cut    ) ;
    // ========================================================================    
    /** 'Select' generator(HepMC) from HepMC tree 
     *  
     *   @code
     *   
     *   HepMC::GenEvent* vertex = ... ;
     *   // select only positive particles form the event
     *   GRange positive = gselect ( "positive" , vertex , children , 0 < G3Q ) ;
     *
     *   @endcode 
     * 
     *   @see HepMC::GenParticle
     *   @see LoKi::Types::GRange
     *   @see HepMC::GenVertex
     *   @see HepMC::IteratorRange
     *   @see LoKi::Types::GCuts
     *   @see LoKi::Cuts::G3Q
     *
     *   @param tag   the unique tag to be associated with selected particles 
     *   @param vertex the source 
     *   @param cut   the selection cuts to be applied 
     *   @return selected particles, saved into the local storage 
     */
    LoKi::Types::GRange 
    gselect 
    ( const std::string&                  tag    ,
      const HepMC::GenVertex*             vertex ,
      HepMC::IteratorRange                range  ,
      const LoKi::Types::GCuts&           cut    ) ;
    // ========================================================================
    /** 'Select' generator(HepMC) from arbitrary container
     *   @param tag   the unique tag to be associated with selected particles 
     *   @param first begin-iterator for container 
     *   @param last  end-iterator for the ocnatiner 
     *   @param cut   the selection cuts to be applied 
     *   @return selected particles, saved into the local storage 
     */
    template <class GENPARTICLE, class GENCUT>
    inline LoKi::Types::GRange 
    gselect 
    ( const std::string& tag   ,
      GENPARTICLE        first , 
      GENPARTICLE        last  , 
      const GENCUT&      cut   ) 
    {
      return m_genselected.add ( tag , first , last , cut ) ;
    } 
    // ========================================================================
  private:
    // ========================================================================
    /// select (gen)particles according to decay-finder and cuts
    template <class GENPARTICLE, class GENCUT>
    LoKi::Types::GRange 
    gselect 
    ( const std::string&               tag    ,
      GENPARTICLE                      first  , 
      GENPARTICLE                      last   ,
      const Decays::IGenDecay::Finder& finder ,
      const GENCUT&                    cut    ) ;
    // ========================================================================
  public:
    // ========================================================================    
    /** 'Select' generator(HepMC) particles to be used in local storage.
     *  
     *   @code
     * 
     *     const Decays::IGenDecay::Finder& finder = ... ;
     *     GRange pions = gselect("gpi" , finder ) ;
     *
     *   @endcode 
     * 
     *   @param tag the unique tag to be associated with selecte dparticles 
     *   @param finder the decay finder 
     *   @param location TES location of LHCb::HepMCEvent::Container
     *   @return selected particles, saved into the local storage 
     */
    LoKi::Types::GRange 
    gselect 
    ( const std::string&               tag    , 
      const Decays::IGenDecay::Finder& finder ,
      const std::string&               location = LHCb::HepMCEventLocation::Default ) ;
    // ========================================================================
    /** 'Select' generator(HepMC) particles to be used in local storage.
     *  
     *   @code
     * 
     *     const Decays::IGenDecay::iTree& tree = ... ;
     *     GRange pions = gselect("gpi" , tree ) ;
     *
     *   @endcode 
     * 
     *   @param tag the unique tag to be associated with selecte dparticles 
     *   @param tree  the decay tree
     *   @param location TES location of LHCb::HepMCEvent::Container
     *   @return selected particles, saved into the local storage 
     */
    LoKi::Types::GRange 
    gselect 
    ( const std::string&               tag    , 
      const Decays::IGenDecay::iTree&  tree   ,
      const std::string&               location = LHCb::HepMCEventLocation::Default ) ;
    // ========================================================================
    /** 'Select' generator(HepMC) particles to be used in local storage.
     *  
     *   @code
     * 
     *     GRange psis = gselect("gpsi" , "Xb --> ^( J/psi(1S) => l+ l- ) ... ") ;
     *
     *   @endcode 
     * 
     *   @param tag the unique tag to be associated with selecte dparticles 
     *   @param descriptor the decay descriptor 
     *   @param location TES location of LHCb::HepMCEvent::Container
     *   @return selected particles, saved into the local storage 
     */
    LoKi::Types::GRange 
    gselect 
    ( const std::string& tag        , 
      const std::string& descriptot ,
      const std::string& location = LHCb::HepMCEventLocation::Default ) ;
    // ========================================================================
  public:
    // ========================================================================    
    /** extract the selected HepMC-paricles from Local LoKi storages 
     *  by their name/tag 
     *
     *  @code
     *
     *  GRange pions = gselected( "gpions");
     *
     *  @endcode
     *
     *  @param  tag name/tag assigned to the selected vertices
     *  @return selected range of HepMC-particles
     */
    LoKi::Types::GRange 
    gselected ( const std::string& tag ) const { return m_genselected ( tag ) ; }
    // ========================================================================    
  public:
    // ========================================================================    
    /// get LoKi::MCFinder object
    LoKi::MCFinder 
    mcFinder ( const std::string& name = "" ) const ;
    // ========================================================================
    // get LoKi::MCMatch obejct
    LoKi::MCMatch 
    mcTruth  ( const std::string& name = "" ) const ;
    // ========================================================================
  public:
    // ========================================================================    
    /// get the pointer to IMC2Collision tool 
    const IMC2Collision*            mc2collision () const ;
    // ========================================================================    
    /// get the pointer to IHepMC2MC tool 
    const IHepMC2MC*                hepMC2MC     () const ;
    // ========================================================================    
    /// get the pointer to IPV2MC tool  
    const IPV2MC*                   pv2MC        () const ;
    // ========================================================================    
  public:
    // ========================================================================    
    /// clear the internal LoKi storages 
    virtual StatusCode clear() ;
    // ========================================================================    
  public:
    // ========================================================================    
    /// initialize the algorithm 
    virtual StatusCode initialize () ;
    // ========================================================================    
    /// make the execution of the algorithm 
    virtual StatusCode execute    () ;
    // ========================================================================    
    /// perform the real analysis 
    virtual StatusCode analyse    () ;
    // ========================================================================    
    /// finalize the algorithm 
    virtual StatusCode finalize   () ;
    // ========================================================================    
  protected:
    // ========================================================================    
    /** standard constructor 
     *  @param name algorithm instance name 
     *  @param pSvc pointer to Service Locator 
     */
    AlgoMC
    ( const std::string& name , 
      ISvcLocator*       pSvc ) ;
    // ========================================================================    
    /// virtual and protected destructor 
    virtual ~AlgoMC() ;
    // ========================================================================    
  protected:
    // ========================================================================    
    /// the actual type for collection of TES address
    typedef std::vector<std::string> Addresses ;
    // ========================================================================    
  private:
    // ========================================================================    
    /// helper fuction to feed LoKi::MCMatchObj object with the data 
    template <class TABLE>
      StatusCode _feedIt 
      ( LoKi::MCMatchObj* object          , 
        const Addresses&  addresses       , 
        const bool        report   = true ) const ;
    // ========================================================================    
  private:
    // ========================================================================    
    // the actual storage of selected MC-particles
    LoKi::MCTypes::MCSelected  m_mcselected  ; ///< the actual storage of MC-particles
    // ========================================================================    
    // the actual storage of selected MC-vertices
    LoKi::MCTypes::MCVSelected m_mcvselected ; ///< the actual storage of MC-vertices
    // ========================================================================    
    // the actual storage of selected HepMC-particles
    LoKi::GenTypes::GenSelected  m_genselected  ; ///< the actual storage of HepMC-particles
    // the actual storage of selected HepMC-vertices
    // ========================================================================    
    LoKi::GenTypes::GenVSelected m_genvselected ; ///< the actual storage of HepMC-vertices
    // ========================================================================    
    // collection  of MCFinder objects
    typedef GaudiUtils::HashMap<std::string,LoKi::MCFinderObj*> MCfinders ;
    mutable MCfinders  m_mcfinders   ;
    // ========================================================================    
    // collection  of IMCDecayFinder objects
    typedef GaudiUtils::HashMap<std::string,IMCDecayFinder*>    IMCfinders ;
    mutable IMCfinders m_imcfinders  ;
    // ========================================================================    
    // collection of LoKi::MCMatchObj objects 
    typedef GaudiUtils::HashMap<std::string,LoKi::MCMatchObj*>  MCmatchers ;
    mutable MCmatchers m_mcmatchers  ;
    // ========================================================================    
    Addresses    m_P2MC   ;
    Addresses    m_P2MCW  ;
    Addresses    m_PP2MC  ;
    Addresses    m_T2MC   ;
    Addresses    m_T2MCW  ;
    // ========================================================================    
    std::string                  m_mc2collisionName  ;
    mutable const IMC2Collision* m_mc2collision      ;
    // ========================================================================    
    std::string                  m_hepmc2mcName      ;
    mutable const IHepMC2MC*     m_hepmc2mc          ;
    // ========================================================================    
    std::string                  m_pv2mcName         ;
    mutable const IPV2MC*        m_pv2mc             ;
    // ========================================================================    
    //  explicitly disable MC-truth matching ? 
    bool                         m_disableMCMatch    ;
    //  auto-collect P->MC plinks for uDST ? 
    bool                         m_collectP2MCLinks  ;
    // ========================================================================
    /// the factory to create the (MC)decay trees 
    LoKi::Interface<Decays::IMCDecay>  m_mcdecay  ;          // the decay trees 
    /// the factory to create the (Gen)decay trees 
    LoKi::Interface<Decays::IGenDecay> m_gendecay ;          // the decay trees 
    // ========================================================================
  } ;
  // ==========================================================================
} //                                                      end of namespace LoKi
// ============================================================================
// helper fuction to feed LoKi::MCMatchObj object with the data 
// ============================================================================
template <class TABLE>
inline StatusCode 
LoKi::AlgoMC::_feedIt 
( LoKi::MCMatchObj*              object    ,
  const LoKi::AlgoMC::Addresses& addresses , 
  const bool                     report    ) const
{
  //
  if ( 0 == object ) { return StatusCode::FAILURE ; }
  //
  for ( Addresses::const_iterator item = 
          addresses.begin() ; addresses.end() != item ; ++item ) 
  {
    const std::string& address = *item ;
    // check the data 
    if ( !exist<TABLE>( address ) )
    { 
      if ( !report ) { counter ( "NO-links:" + address ) += 1 ; }
      else  { Warning ( " There is no valid data at '" + address + "'" ) ; }
      continue ; 
    }
    // get the table from the Transient Store 
    const TABLE* table = get<TABLE> ( address ) ;
    // make some statistics of MC-links 
    counter("MC-links:" + address ) += table->relations().size();
    // feed it! 
    object -> addMatchInfo ( table ) ;
  } 
  //
  return StatusCode::SUCCESS ;
}
// ============================================================================
// select (mc)particles according to decay-finder and cuts
// ============================================================================
template <class MCPARTICLE, class MCCUT>
LoKi::Types::MCRange 
LoKi::AlgoMC::mcselect 
( const std::string&              tag    ,
  MCPARTICLE                      first  , 
  MCPARTICLE                      last   ,
  const Decays::IMCDecay::Finder& finder ,
  const MCCUT&                    cut    ) 
{
  // check the validity of finder 
  if ( !finder ) 
  {
    StatusCode sc = finder.validate ( ppSvc() ) ;
    if ( sc.isFailure() ) 
    {
      Error ( "Invalid (MC)decay finder: '" + finder.tree().toString() + "'") ;
      return LoKi::Types::MCRange () ;
    }
  }
  //
  LHCb::MCParticle::ConstVector results ;
  results.reserve ( 100 ) ;
  finder.findDecay ( first , last , results ) ;
  //
  return this->mcselect ( tag              , 
                          results.begin () , 
                          results.end   () , cut ) ;
}
// ============================================================================
// select (Gen)particles according to decay-finder and cuts
// ============================================================================
template <class GENPARTICLE, class GENCUT>
LoKi::Types::GRange 
LoKi::AlgoMC::gselect 
( const std::string&               tag    ,
  GENPARTICLE                      first  , 
  GENPARTICLE                      last   ,
  const Decays::IGenDecay::Finder& finder ,
  const GENCUT&                    cut    ) 
{
  // check the validity of finder 
  if ( !finder ) 
  {
    StatusCode sc = finder.validate ( ppSvc() ) ;
    if ( sc.isFailure() ) 
    {
      Error ( "Invalid (Gen)decay finder: '" + finder.tree().toString() + "'") ;
      return LoKi::Types::GRange () ;
    }
  }
  //
  LoKi::GenTypes::GenContainer results ;
  results.reserve ( 100 ) ;
  finder.findDecay ( first , last , results ) ;
  //
  return this->gselect ( tag              , 
                         results.begin () , 
                         results.end   () , cut ) ;
}
// ============================================================================
/** @def LOKI_MCALGORITHM_BODY 
 *
 *  Simple macros to avoid the typing of "standard" header file for 
 *  arbitrary LoKi-based alrorithm: 
 * 
 *  @code 
 *  
 *  LOKI_MCALGORITHM_BODY( BdJPsiPhiAlg );
 *
 *  @endcode 
 *  
 *  "Standard" LoKi-based algorithm contains only one essential 
 *   <tt> virtual StatusCode analyse() </tt> method
 * 
 *  This macros could be used directly in implementation (*.cpp) file 
 *  of the algorithm, there is no nesessity to put only 1 line with macro 
 *  into the separate header file 
 *
 *  One need to implement factory, constructor & destructor of algorithm
 *  and "analyse" method.
 *
 *  @author Vanya Belyaev Ivan.Belyaev@itep.ru
 *  @date   2003-01-18
 */
// ============================================================================
#define LOKI_MCALGORITHM_BODY( ALGNAME )                  \
namespace LoKi                                            \
{                                                         \
   class ALGNAME : public LoKi::AlgoMC                    \
   {                                                      \
     /** friend factory for instantiation      */         \
     friend class AlgFactory<ALGNAME>        ;            \
   public:                                                \
     /** standard method for event analysis    */         \
     virtual StatusCode analyse  ()          ;            \
   protected:                                             \
     /** standard constructor                  */         \
     ALGNAME( const std::string& name ,                   \
              ISvcLocator*       svc  )    ;              \
     /** virtual destructor                    */         \
     virtual ~ALGNAME ()                   ;              \
    private:                                              \
     /** default constructor  is private       */         \
     ALGNAME             ()                  ;            \
     /** copy constructor     is private       */         \
     ALGNAME             ( const ALGNAME & ) ;            \
     /** assignement operator is private       */         \
     ALGNAME & operator= ( const ALGNAME & ) ;            \
  };                                                      \
} // end of namespace LoKi
// ============================================================================
/** @def LOKI_MCALGORITHM_IMPLEMENT 
 *
 *  Simple macros to avoid the typing of "standard" implementation 
 *  of mandatory algorithm methods (constructors, destructors and factories) 
 *  for arbitrary LoKi-based alrorithm: 
 * 
 *  @code 
 *  #include "LoKi/Macros.h"
 *  
 *  LOKI_ALGORITHM_BODY_IMPLEMENT( BdJPsiPhiAlg );
 *
 *  /// standard LoKi method for event analysis
 *  StatusCode LoKi::BdJPsiPhiAlg::analyse() 
 *  {
 *    return Print("analyse() method is invoked ", StatusCode::SUCCESS ); 
 *  }
 *
 *  @endcode 
 *
 *  One need to implement only "analyse" method.
 * 
 *  This macro could be easily combined with LOKI_ALGORITHM_BODY macro
 *
 *  @code 
 *  #include "LoKi/Macros.h"
 *  
 *  LOKI_ALGORITHM_BODY           ( BdJPsiPhiAlg ) ;
 *  LOKI_ALGORITHM_IMPLEMENTATION ( BdJPsiPhiAlg ) ;
 *
 *  /// standard LoKi method for event analysis
 *  StatusCode LoKi::BdJPsiPhiAlg::analyse() 
 *  {
 *    return Print("analyse() method is invoked ", StatusCode::SUCCESS ); 
 *  }
 *
 *  @endcode 
 *  
 *  @author Vanya Belyaev Ivan.Belyaev@itep.ru
 *  @date   2003-01-18
 */
#define LOKI_MCALGORITHM_IMPLEMENT( ALGNAME )                      \
/* ============================================================ */ \
/** Declaration of the Algorithm Factory                        */ \
/* ============================================================ */ \
DECLARE_NAMESPACE_ALGORITHM_FACTORY( LoKi, ALGNAME )               \
/* ============================================================ */ \
/** Standard constructor                                        */ \
/* ============================================================ */ \
LoKi::ALGNAME::ALGNAME ( const std::string& name  ,                \
                         ISvcLocator*       svc   )                \
: LoKi::AlgoMC( name , svc ) {}   /* constructor for base class */ \
/* ============================================================ */ \
/** destructor (empty)                                          */ \
/* =============================================================*/ \
LoKi::ALGNAME ::~ALGNAME () {} 
// ============================================================================
/** @def LOKI_MCALGORITHM_FULLIMPLEMENT 
 *
 *  Simple macros to avoid the typing of "standard" implementation 
 *  of mandatory algorithm methods (constructors, destructors and factories) 
 *  for arbitrary LoKi-based alrorithm: 
 * 
 *  @code 
 *
 *  #include "LoKi/Macros.h"
 *  
 *  LOKI_MCALGORITHM_FULLIMPLEMENT( BdJPsiPhiAlg );
 *
 *  /// standard LoKi method for event analysis
 *  StatusCode BdJPsiPhiAlg::analyse() 
 *  {
 *    return Print("analyse() method is invoked ", StatusCode::SUCCESS ); 
 *  }
 *
 *  @endcode 
 *
 *  @author Vanya Belyaev Ivan.Belyaev@itep.ru
 *  @date   2003-01-18
 */
// ============================================================================
#define LOKI_MCALGORITHM_FULLIMPLEMENT(   ALGNAME   )     \
        LOKI_MCALGORITHM_BODY         (   ALGNAME   )     \
        LOKI_MCALGORITHM_IMPLEMENT    (   ALGNAME   ) 
// ============================================================================
/** @def LOKI_MCALGORITHM
 * 
 *  The most advanced macro to avoid the typeing of "standard" implementation
 *  of all mandatory but tediouse and totoriouse lines and methods 
 *  (algorithsm bidy, constructors, destructors factories etc)
 *  
 *  @code 
 *  
 *  #include "Loki/Macros.h"
 *  
 *  LOKI_MCALGORITHM( MyAlg )
 *  {
 *    using namespace LoKi                ;  
 *    using namespace LoKi::Cuts          ;  
 *    using namespace LoKi::Fits          ;
 *  
 *    /// some implementation of algorithm
 * 
 *   return StatusCode::SUCCESS ;
 *  }
 *
 *  @endcode 
 *  
 *  @author Vanya BELYAEV Ivan.Belyaev@itep.ru
 *  @date   2003-03-11
 *  
 */
// ============================================================================
#define LOKI_MCALGORITHM(   ALGNAME   )             \
        LOKI_MCALGORITHM_FULLIMPLEMENT ( ALGNAME )  \
        StatusCode LoKi::ALGNAME::analyse()                      
// ============================================================================


// ============================================================================
//                                                                      The END 
// ============================================================================
#endif // LOKI_ALGOMC_H
// ============================================================================
