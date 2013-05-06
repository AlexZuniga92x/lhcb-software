// $Id$ 
// ============================================================================
#ifndef ANALYSIS_MUTE_H 
#define ANALYSIS_MUTE_H 1
// ============================================================================
// Include files
// ============================================================================
// STD& STL 
// ============================================================================
#include <fstream>
#include <streambuf>
// ============================================================================
// GaudiKernel
// ============================================================================
#include "GaudiKernel/Kernel.h"
// ============================================================================
namespace Gaudi
{
  // ==========================================================================
  namespace Utils 
  {
    // ========================================================================
    /** @class Mute Mute.h Analysis/Mute.h
     * 
     *
     *  @author Vanya Belyaev
     *  @date   2013-02-19
     */
    class GAUDI_API Mute 
    {
    public:
      // ======================================================================
      /// constructor with file name 
      Mute  ( const std::string& fname        , 
              const bool         out   = true ) ; 
      /// default consructor (redirect output to "dev/null") 
      Mute  ( const bool         out   = true ) ; // default consructor
      ///  desctructor 
      ~Mute ( ) ;            //  destructor 
      // ======================================================================
    private: 
      // ======================================================================
      /// keep the buffer 
      bool            m_cout   ;
      std::ofstream   m_output ;
      std::streambuf* m_buffer ;
      // ======================================================================
    };
    // ========================================================================
  } //                                            end of namespace Gaudi::Utils 
  // ==========================================================================
} //                                                     end of namespace Gaudi
// ============================================================================
//                                                                      The END 
// ============================================================================
#endif // ANALYSIS_MUTE_H
// ============================================================================
