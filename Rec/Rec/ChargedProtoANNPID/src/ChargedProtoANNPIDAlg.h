
//-------------------------------------------------------------------------------
/** @file ChargedProtoANNPIDAlg.h
 *
 *  Declaration file for ANN Combined PID algorithm ChargedProtoANNPIDAlg
 *
 *  @author Chris Jones       Christopher.Rob.Jones@cern.ch
 *  @date   10/09/2010
 */
//-------------------------------------------------------------------------------

#ifndef ChargedProtoANNPID_ChargedProtoANNPIDAlg_H
#define ChargedProtoANNPID_ChargedProtoANNPIDAlg_H 1

// STL
#include <sstream>
#include <fstream>

// Gaudi
#include "GaudiKernel/IJobOptionsSvc.h"
#include "GaudiKernel/AlgFactory.h"

// local
#include "ChargedProtoANNPIDBase.h"

// Event Model
#include "Event/ProtoParticle.h"

// NeuroBayes (only on linux with gcc for the moment)
#ifdef __GNUC__
#include "NeuroBayesExpert.hh"
#endif

// interfaces
#include "TrackInterfaces/ITrackSelector.h"

// boost
#include "boost/assign/list_of.hpp"

// FPE exception protection
#include "Kernel/FPEGuard.h"

namespace ANNGlobalPID
{

  //-----------------------------------------------------------------------------
  /** @class ChargedProtoANNPIDAlg ChargedProtoANNPIDAlg.h
   *
   *  Adds ANN PID information to ProtoParticles
   *
   *  @author Chris Jones
   *  @date   2010-03-09
   */
  //-----------------------------------------------------------------------------

  class ChargedProtoANNPIDAlg : public ChargedProtoANNPIDBase
  {

  public:

    /// Standard constructor
    ChargedProtoANNPIDAlg( const std::string& name, ISvcLocator* pSvcLocator );

    virtual ~ChargedProtoANNPIDAlg( ); ///< Destructor

    virtual StatusCode initialize();    ///< Algorithm initialization
    virtual StatusCode execute   ();    ///< Algorithm execution
    virtual StatusCode finalize  ();    ///< Algorithm finalization

  private:

    /** @class ANNHelper ChargedProtoANNPIDAlg.h
     *
     *  Base class for ANN helpers
     *
     *  @author Chris Jones
     *  @date   2010-03-09
     */
    class ANNHelper
    {
    protected:
      /// No default constructor
      ANNHelper() : m_parent(NULL) { }
    public:
      /** Constructor from information
       *  @param inputs The list of inputs needed for this network
       *  @param parent Point to parent algorithm
       */
      ANNHelper( const ChargedProtoANNPIDBase::IntInputs& inputs,
                 const ChargedProtoANNPIDBase *           parent )
        : m_inputs ( inputs ),
          m_parent ( parent ) { }
      /// Destructor
      virtual ~ANNHelper() { }
    public:
      /// Compute the ANN output for the given ProtoParticle
      virtual double getOutput( const LHCb::ProtoParticle * proto ) const = 0;
      /// Number of inputs to the ANN
      inline unsigned int nInputs() const { return m_inputs.size(); }
    protected:
      ChargedProtoANNPIDBase::IntInputs m_inputs; ///< The list of inputs for this network
      const ChargedProtoANNPIDBase    * m_parent; ///< Pointer to parent algorithm
    };

#ifdef __GNUC__
    /** @class ANNHelper ChargedProtoANNPIDAlg.h
     *
     *  Helper class for NeuroBayes networks
     *
     *  @author Chris Jones
     *  @date   2010-03-09
     */
    class NeuroBayesANN : public ANNHelper
    {
    private:
      /// No default constructor
      NeuroBayesANN() : m_expert(NULL) { }
    public:
      /** Constructor from information
       *  @param paramFileName Network tuning parameter file
       *  @param inputs The list of inputs needed for this network
       *  @param parent Point to parent algorithm
       */
      NeuroBayesANN( const std::string &              paramFileName,
                     const ChargedProtoANNPIDBase::IntInputs inputs,
                     const ChargedProtoANNPIDBase *          parent )
        : ANNHelper ( inputs, parent ),
          m_expert  ( new Expert(paramFileName.c_str()) ),
          m_inArray ( new float[inputs.size()]          )
      { }
      /// Destructor
      virtual ~NeuroBayesANN() { delete m_expert; delete[] m_inArray; }
    public:
      /// Compute the ANN output for the given ProtoParticle
      virtual double getOutput( const LHCb::ProtoParticle * proto ) const;
    private:
      Expert * m_expert;  ///< Pointer to the NeuroBayes 'Expert'
      float * m_inArray;  ///< Working array for network inputs
    };
#endif

  private:

    /// Track selector type
    std::string m_trSelType;

    /// Track selector tool
    ITrackSelector * m_trSel;

    // Track Pre-selection
    std::string m_trackPreSel;

    std::string m_configFile;      ///< Configuration file

    ANNHelper * m_netHelper;       ///< Network Helper

    /// The extra info to fill on the ProtoParticle
    LHCb::ProtoParticle::additionalInfo m_protoInfo;

    /// The version of the PID networks trianing to use
    std::string m_netVersion;

  };

}

#endif // ChargedProtoANNPID_ChargedProtoANNPIDAlg_H
