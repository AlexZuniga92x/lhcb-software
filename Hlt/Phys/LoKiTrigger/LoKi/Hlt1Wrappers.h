// $Id: Hlt1Wrappers.h,v 1.1 2009-04-01 12:36:09 ibelyaev Exp $
// ============================================================================
#ifndef LOKI_HLT1WRAPPERS_H 
#define LOKI_HLT1WRAPPERS_H 1
// ============================================================================
// Include files 
// ============================================================================
// LoKi
// ============================================================================
#include "LoKi/TriggerTypes.h"
#include "LoKi/Interface.h"
// ============================================================================
// TrackInterfaces 
// ============================================================================
#include "TrackInterfaces/IFunctionTool.h"
// ============================================================================
// HltBase 
// ============================================================================
#include "HltBase/IBiFunctionTool.h"
#include "HltBase/IMatchTVeloTracks.h"
// ============================================================================
// LoKi
// ============================================================================
#include "LoKi/HltTool.h"
// ============================================================================
/** @file LoKi/Hlt1Wrappers.h
 *  Collection of specific wrappers for Hl1 implementation
 * 
 *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
 *  @date   2009-03-31
 */
// ============================================================================
namespace LoKi
{
  // ==========================================================================
  namespace Tracks 
  {
    // ========================================================================
    /** @class TrFunction
     *  Simple wrapper for ITrackFunctionTool
     *  @see ITrackFunctionTool 
     *  @see LoKi::Cuts::TrFUN 
     *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
     *  @date   2009-03-31
     */
    class TrFunction : public LoKi::TrackTypes::TrFunc 
    {
    public:
      // ======================================================================
      /// construct from the configuration tool 
      TrFunction ( const LoKi::Hlt1::TrackFunction& tool ) ;
      /// MANDATORY: virtual destructor 
      virtual ~TrFunction() {}
      /// MANDATORY: clone method ("virtual constructor")
      virtual  TrFunction* clone() const ;
      /// MANDATORY: the only one essential method 
      virtual  result_type operator() ( argument a ) const ;
      /// OPTIONAL: nice printout 
      virtual  std::ostream& fillStream ( std::ostream& s ) const ;
      // ======================================================================
    private:
      // ======================================================================
      /// the default constructor is disabled 
      TrFunction() ;                     // the default constructor is disabled      
      // ======================================================================
    private:
      // ======================================================================
      void init() ;
      // ======================================================================
    private:
      // ======================================================================
      /// tool configuration 
      LoKi::Hlt1::TrackFunction            m_config ;     // tool configuration 
      /// the actual tool 
      LoKi::Interface<ITrackFunctionTool>  m_tool   ;        // the actual tool 
      // ======================================================================
    };
    // ========================================================================
    /** @class TTrFunction
     *  Simple wrapper for ITrackBiFunctionTool
     *  @see ITrackBiFunctionTool 
     *  @see LoKi::Cuts::TTrFUN 
     *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
     *  @date   2009-03-31
     */
    class TTrFunction : public LoKi::TrackTypes::TTrFunc 
    {
    public:
      // ======================================================================
      /// construct from the configuration tool 
      TTrFunction ( const LoKi::Hlt1::TrackBiFunction& tool ) ;
      /// MANDATORY: virtual destructor 
      virtual ~TTrFunction() {}
      /// MANDATORY: clone method ("virtual constructor")
      virtual  TTrFunction* clone() const ;
      /// MANDATORY: the only one essential method 
      virtual  result_type operator() ( argument a ) const ;
      /// OPTIONAL: nice printout 
      virtual  std::ostream& fillStream ( std::ostream& s ) const ;
      // ======================================================================
    private:
      // ======================================================================
      /// the default constructor is disabled 
      TTrFunction() ;                     // the default constructor is disabled      
      // ======================================================================
    private:
      // ======================================================================
      void init() ;
      // ======================================================================
    protected:
      // ======================================================================
      /// tool configuration 
      LoKi::Hlt1::TrackBiFunction            m_config ;   // tool configuration 
      /// the actual tool 
      LoKi::Interface<ITrackBiFunctionTool>  m_tool   ;      // the actual tool 
      // ======================================================================
    };
    // ========================================================================
    /** @class TTrFunction2
     *  Simple wrapper for ITrackBiFunctionTool
     *  @see ITrackBiFunctionTool 
     *  @see LoKi::Cuts::TTrFUN2 
     *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
     *  @date   2009-03-31
     */
    class TTrFunction2 : public LoKi::Tracks::TTrFunction 
    {
    public:
      // ======================================================================
      /// construct from the configuration tool 
      TTrFunction2 ( const LoKi::Hlt1::TrackBiFunction& tool ) ;
      /// MANDATORY: virtual destructor 
      virtual ~TTrFunction2() {}
      /// MANDATORY: clone method ("virtual constructor")
      virtual  TTrFunction2* clone() const ;
      /// MANDATORY: the only one essential method 
      virtual  result_type operator() ( argument a ) const ;
      /// OPTIONAL: nice printout 
      virtual  std::ostream& fillStream ( std::ostream& s ) const ;
      // ======================================================================
    private:
      // ======================================================================
      /// the default constructor is disabled 
      TTrFunction2() ;                     // the default constructor is disabled      
      // ======================================================================
    };
    // ========================================================================
    /** @class TTrMatchTVelo
     *  Simple wrapper for IMatchTVeloTrack
     *  @see IMatchTVeloTrack
     *  @see LoKi::Cuts::TTrTVELOMATCH
     *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
     *  @date   2009-03-31
     */
    class TTrMatchTVelo : public LoKi::TrackTypes::TTrFunc 
    {
    public:
      // ======================================================================
      /// construct from the configuration tool 
      TTrMatchTVelo ( const LoKi::Hlt1::MatchTVelo& tool ) ;
      /// MANDATORY: virtual destructor 
      virtual ~TTrMatchTVelo() {}
      /// MANDATORY: clone method ("virtual constructor")
      virtual  TTrMatchTVelo* clone() const ;
      /// MANDATORY: the only one essential method 
      virtual  result_type operator() ( argument a ) const ;
      /// OPTIONAL: nice printout 
      virtual  std::ostream& fillStream ( std::ostream& s ) const ;
      // ======================================================================
    private:
      // ======================================================================
      /// the default constructor is disabled 
      TTrMatchTVelo() ;                 // the default constructor is disabled      
      // ======================================================================
    private:
      // ======================================================================
      void init() ;
      // ======================================================================
    protected:
      // ======================================================================
      /// tool configuration 
      LoKi::Hlt1::MatchTVelo                 m_config ;   // tool configuration 
      /// the actual tool 
      LoKi::Interface<IMatchTVeloTracks>     m_tool   ;      // the actual tool 
      // ======================================================================
    } ;
    // ========================================================================
    /** @class TTrMatchTVelo2
     *  Simple wrapper for IMatchTVeloTrack
     *  @see IMatchTVeloTrack
     *  @see LoKi::Cuts::TTrTVELOMATCH
     *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
     *  @date   2009-03-31
     */
    class TTrMatchTVelo2 : public LoKi::Tracks::TTrMatchTVelo 
    {
    public:
      // ======================================================================
      /// construct from the configuration tool 
      TTrMatchTVelo2 ( const LoKi::Hlt1::MatchTVelo& tool ) ;
      /// MANDATORY: virtual destructor 
      virtual ~TTrMatchTVelo2() {}
      /// MANDATORY: clone method ("virtual constructor")
      virtual  TTrMatchTVelo2* clone() const ;
      /// MANDATORY: the only one essential method 
      virtual  result_type operator() ( argument a ) const ;
      /// OPTIONAL: nice printout 
      virtual  std::ostream& fillStream ( std::ostream& s ) const ;
      // ======================================================================
    private:
      // ======================================================================
      /// the default constructor is disabled 
      TTrMatchTVelo2() ;                 // the default constructor is disabled      
      // ======================================================================
    } ;
    // ========================================================================
    /** @class TrVFunction
     *  Simple wrapper for ITrackVertexBiFunctionTool
     *  @see ITrackVertexBiFunctionTool 
     *  @see LoKi::Cuts::TrVFUN 
     *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
     *  @date   2009-03-31
     */
    class TrVFunction : public LoKi::TrackTypes::TrVFunc 
    {
    public:
      // ======================================================================
      /// construct from the configuration tool 
      TrVFunction ( const LoKi::Hlt1::TrackVertexFunction& tool ) ;
      /// MANDATORY: virtual destructor 
      virtual ~TrVFunction() {}
      /// MANDATORY: clone method ("virtual constructor")
      virtual  TrVFunction* clone() const ;
      /// MANDATORY: the only one essential method 
      virtual  result_type operator() ( argument a ) const ;
      /// OPTIONAL: nice printout 
      virtual  std::ostream& fillStream ( std::ostream& s ) const ;
      // ======================================================================
    private:
      // ======================================================================
      /// the default constructor is disabled 
      TrVFunction() ;                     // the default constructor is disabled      
      // ======================================================================
    private:
      // ======================================================================
      void init() ;
      // ======================================================================
    protected:
      // ======================================================================
      /// tool configuration 
      LoKi::Hlt1::TrackVertexFunction          m_config ; // tool configuration 
      /// the actual tool 
      LoKi::Interface<ITrackVertexBiFunctionTool> m_tool ;   // the actual tool 
      // ======================================================================
    };
    // ========================================================================
  } // end of namespace LoKi::Tracks 
  // ==========================================================================
} // end of namespace LoKi 
// ============================================================================
namespace LoKi
{
  // ==========================================================================
  namespace Cuts
  {
    // ========================================================================
    /** @typedef TrFUN
     *  Simple wrapper over ITrackFunctionTool
     *  @see ITrackFunctionTool 
     *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
     *  @date   2009-03-31
     */
    typedef LoKi::Tracks::TrFunction                                    TrFUN ;
    // ========================================================================
    /** @typedef TTrFUN
     *  Simple wrapper over ITrackBiFunctionTool
     *  @see ITrackBiFunctionTool 
     *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
     *  @date   2009-03-31
     */
    typedef LoKi::Tracks::TTrFunction                                  TTrFUN ;
    // ========================================================================
    /** @typedef TTrFUN2
     *  Simple wrapper over ITrackBiFunctionTool
     *  @see ITrackBiFunctionTool 
     *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
     *  @date   2009-03-31
     */
    typedef LoKi::Tracks::TTrFunction2                                TTrFUN2 ;
    // ========================================================================
    /** @typedef TTrTVELOMATCH
     *  Simple wrapper over IMatchTVeloTracks
     *  @see IMatchTVeloTracks
     *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
     *  @date   2009-03-31
     */
    typedef LoKi::Tracks::TTrMatchTVelo                         TTrTVELOMATCH ;
    // ========================================================================
    /** @typedef TTrTVELOMATCH2
     *  Simple wrapper over IMatchTVeloTracks
     *  @see IMatchTVeloTracks
     *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
     *  @date   2009-03-31
     */
    typedef LoKi::Tracks::TTrMatchTVelo2                       TTrTVELOMATCH2 ;
    // ========================================================================
    /** @typedef TrTFUN
     *  Simple wrapper over ITrackVertexBiFunctionTool
     *  @see ITrackVertexBiFunctionTool
     *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
     *  @date   2009-03-31
     */
    typedef LoKi::Tracks::TrVFunction                                  TrVFUN ;
    // ========================================================================
  } //                                              end of namespace LoKi::Cuts 
  // ==========================================================================
} //                                                      end of namespace LoKi 
// ============================================================================
// The END 
// ============================================================================
#endif // LOKI_HLT1WRAPPERS_H
// ============================================================================
