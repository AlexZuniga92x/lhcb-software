
//-------------------------------------------------------------------------------
/** @file ChargedProtoANNPIDCommonBase.h
 *
 *  Declaration file for ANNGlobalPID::ChargedProtoANNPIDCommonBase
 *
 *  @author Chris Jones       Christopher.Rob.Jones@cern.ch
 *  @date   03/02/2011
 */
//-------------------------------------------------------------------------------

#ifndef ChargedProtoANNPID_CHARGEDPROTOANNPIDCOMMONBASE_H
#define ChargedProtoANNPID_CHARGEDPROTOANNPIDCOMMONBASE_H 1

// STL
#include <string>
#include <vector>
#include <map>
#include <sstream>

// Gaudi
#include "GaudiKernel/ISvcLocator.h"
#include "GaudiKernel/IIncidentListener.h"
#include "GaudiKernel/IIncidentSvc.h"

// Event Model
#include "Event/ProtoParticle.h"
#include "Event/RecSummary.h"

// Kernel LHCbMath
#include "LHCbMath/GeomFun.h"
#include "LHCbMath/LineTypes.h"

// NeuroBayes
#ifdef _ENABLE_NEUROBAYES
#include "NeuroBayesExpert.hh"
#endif

// TMVA
#include "TMVA/Reader.h"

// TMVA Factory
#include "TMVAImpFactory.h"

//-----------------------------------------------------------------------------
/** @namespace ANNGlobalPID
 *
 *  General namespace for Global PID ANN software
 *
 *  @author Chris Jones  Christopher.Rob.Jones@cern.ch
 *  @date   2010-03-09
 */
//-----------------------------------------------------------------------------

namespace ANNGlobalPID
{

  /** @class ChargedProtoANNPIDCommonBase ChargedProtoANNPIDCommonBase.h
   *
   *  Common base class
   *
   *  @author Chris Jones
   *  @date   03/02/2011
   */

  template <class PBASE>
  class ChargedProtoANNPIDCommonBase : public PBASE,
                                       virtual public IIncidentListener
  {

  public:

    /// Standard algorithm-like constructor
    ChargedProtoANNPIDCommonBase( const std::string& name,
                                  ISvcLocator* pSvcLocator );

    /// Standard tool-like constructor
    ChargedProtoANNPIDCommonBase( const std::string& type,
                                  const std::string& name,
                                  const IInterface* parent );

    virtual ~ChargedProtoANNPIDCommonBase( ); ///< Destructor

  public:

    /** Initialization of the algorithm after creation
     *
     * @return The status of the initialization
     * @retval StatusCode::SUCCESS Initialization was successful
     * @retval StatusCode::FAILURE Initialization failed
     */
    virtual StatusCode initialize();

    /** Finalization of the algorithm before deletion
     *
     * @return The status of the finalization
     * @retval StatusCode::SUCCESS Finalization was successful
     * @retval StatusCode::FAILURE Finalization failed
     */
    virtual StatusCode finalize();

    /// Implement the handle method for the Incident service.
    void handle( const Incident& incident );

  protected:

    /// Type for list of inputs
    typedef std::vector<std::string> StringInputs;


  protected:

    /** @class Input ChargedProtoANNPIDCommonBase.h
     *
     *  Base class for all 'input' classes, that handle getting
     *  a particular value from a given ProtoParticle
     *
     *  @author Chris Jones
     *  @date   15/04/2013
     */
    class Input
    {
    public:
      /// Destructor
      virtual ~Input() { }
    public:
      /// Access the input value for a given ProtoParticle
      virtual double value( const LHCb::ProtoParticle * proto ) const = 0;
    public:
      /// Type for a vector of inputs
      typedef std::vector<const Input *> ConstVector;
    };

  private:

    /// ProtoParticle Extra Info object
    class InProtoExInfo : public Input
    {
    public:
      InProtoExInfo( const LHCb::ProtoParticle::additionalInfo info,
                     const double def = -999 )
        : m_info(info), m_def(def) { }
    private:
      const LHCb::ProtoParticle::additionalInfo m_info;
      const double m_def;
    public:
      virtual double value( const LHCb::ProtoParticle * proto ) const
      {
        return proto->info( m_info, m_def );
      }
    };

    /// Track Extra Info object
    class InTrackExInfo : public Input
    {
    public:
      InTrackExInfo( const int info ) : m_info(info) { }
    private:
      const int m_info;
    public:
      virtual double value( const LHCb::ProtoParticle * proto ) const
      {
        return proto->track()->info( m_info, -999 );
      }
    };

    /// Track Momentum
    class InTrackP : public Input
    {
    public:
      virtual double value( const LHCb::ProtoParticle * proto ) const
      {
        const double MaxP = 5000.0 * Gaudi::Units::GeV;
        const double var = proto->track()->p();
        return ( var < MaxP ? var : -999 );
      }
    };

    /// Track Transverse Momentum
    class InTrackPt : public Input
    {
    public:
      virtual double value( const LHCb::ProtoParticle * proto ) const
      {
        const double MaxPt = 1000.0 * Gaudi::Units::GeV;
        const double var = proto->track()->pt();
        return ( var < MaxPt ? var : -999 );
      }
    };

    /// Track Likelihood
    class InTrackLikelihood : public Input
    {
    public:
      virtual double value( const LHCb::ProtoParticle * proto ) const
      {
        const double var = proto->track()->likelihood();
        return ( var > -120.0 ? var : -999 );
      }
    };

    /// Track Ghost Probability
    class InTrackGhostProb : public Input
    {
    public:
      virtual double value( const LHCb::ProtoParticle * proto ) const
      {
        const double var = proto->track()->ghostProbability();
        return ( var > 0.00001 ? var : -999 );
      }
    };

    /// Track Clone distance
    class InTrackCloneDist : public Input
    {
    public:
      virtual double value( const LHCb::ProtoParticle * proto ) const
      {
        const double var = proto->track()->info(LHCb::Track::CloneDist,-999);
        return ( var >= 0 ? var : -999 );
      }
    };

    /// Track DOCA
    class InTrackDOCA : public Input
    {
    public:
      virtual double value( const LHCb::ProtoParticle * proto ) const
      {
        const LHCb::State & s = proto->track()->firstState() ;
        const Gaudi::Math::XYZLine z_axis ( Gaudi::XYZPoint() ,
                                            Gaudi::XYZVector(0,0,1) ) ;
        const Gaudi::Math::XYZLine track_line ( Gaudi::XYZPoint ( s.x(),  s.y(), s.z() ),
                                                Gaudi::XYZVector( s.tx(), s.ty(), 1.0  ) ) ;
        return Gaudi::Math::distance ( track_line , z_axis );
      }
    };

    /// Used RICH Aerogel
    class InRichUsedAerogel : public Input
    {
    public:
      virtual double value( const LHCb::ProtoParticle * proto ) const
      {
        return ( proto->richPID() ? proto->richPID()->usedAerogel() : 0 );
      }
    };

    /// Used RICH Rich1 Gas
    class InRichUsedR1Gas : public Input
    {
    public:
      virtual double value( const LHCb::ProtoParticle * proto ) const
      {
        return ( proto->richPID() ? proto->richPID()->usedRich1Gas() : 0 );
      }
    };

    /// Used RICH Rich2 Gas
    class InRichUsedR2Gas : public Input
    {
    public:
      virtual double value( const LHCb::ProtoParticle * proto ) const
      {
        return ( proto->richPID() ? proto->richPID()->usedRich2Gas() : 0 );
      }
    };

    /// RICH above electron threshold
    class InRichAboveElThres : public Input
    {
    public:
      virtual double value( const LHCb::ProtoParticle * proto ) const
      {
        return ( proto->richPID() ? proto->richPID()->electronHypoAboveThres() : 0 );
      }
    };

    /// RICH above muon threshold
    class InRichAboveMuThres : public Input
    {
    public:
      virtual double value( const LHCb::ProtoParticle * proto ) const
      {
        return ( proto->richPID() ? proto->richPID()->muonHypoAboveThres() : 0 );
      }
    };

    /// RICH above pion threshold
    class InRichAbovePiThres : public Input
    {
    public:
      virtual double value( const LHCb::ProtoParticle * proto ) const
      {
        return ( proto->richPID() ? proto->richPID()->pionHypoAboveThres() : 0 );
      }
    };

    /// RICH above kaon threshold
    class InRichAboveKaThres : public Input
    {
    public:
      virtual double value( const LHCb::ProtoParticle * proto ) const
      {
        return ( proto->richPID() ? proto->richPID()->kaonHypoAboveThres() : 0 );
      }
    };

    /// RICH above proton threshold
    class InRichAbovePrThres : public Input
    {
    public:
      virtual double value( const LHCb::ProtoParticle * proto ) const
      {
        return ( proto->richPID() ? proto->richPID()->protonHypoAboveThres() : 0 );
      }
    };

    /// RICH DLL accessor
    class InRichDLL : public Input
    {
    public:
      InRichDLL( const Rich::ParticleIDType type )
        : m_type( type ) { }
    private:
      const Rich::ParticleIDType m_type;
    public:
      virtual double value( const LHCb::ProtoParticle * proto ) const
      {
        return ( proto->richPID() ? proto->richPID()->particleDeltaLL(m_type) : -999 );
      }
    };

    /// Muon IsMuon flag
    class InMuonIsMuon : public Input
    {
    public:
      virtual double value( const LHCb::ProtoParticle * proto ) const
      {
        return ( proto->muonPID() ? proto->muonPID()->IsMuon() : 0 );
      }
    };

    /// Muon IsMuonLoose flag
    class InMuonIsMuonLoose : public Input
    {
    public:
      virtual double value( const LHCb::ProtoParticle * proto ) const
      {
        return ( proto->muonPID() ? proto->muonPID()->IsMuonLoose() : 0 );
      }
    };

    /// Muon # shared hits
    class InMuonNShared : public Input
    {
    public:
      virtual double value( const LHCb::ProtoParticle * proto ) const
      {
        return proto->info(LHCb::ProtoParticle::MuonNShared,-1.0) + 1.0;
      }
    };

    /// Muon Muon likelihood
    class InMuonLLMu : public Input
    {
    public:
      virtual double value( const LHCb::ProtoParticle * proto ) const
      {
        return ( proto->muonPID() && proto->muonPID()->IsMuonLoose() ?
                 proto->muonPID()->MuonLLMu() : -999 );
      }
    };

    /// Muon background likelihood
    class InMuonLLBkg : public Input
    {
    public:
      virtual double value( const LHCb::ProtoParticle * proto ) const
      {
        return ( proto->muonPID() && proto->muonPID()->IsMuonLoose() ?
                 proto->muonPID()->MuonLLBg() : -999 );
      }
    };

    /// Number ProtoParticles
    class InNumProtos : public Input
    {
    public:
      virtual double value( const LHCb::ProtoParticle * proto ) const
      {
        const LHCb::ProtoParticles * protos =
          dynamic_cast<const LHCb::ProtoParticles *>(proto->parent());
        return ( protos ? static_cast<double>(protos->size()) : -999 );
      }
    };

    /// Number of CALO Hypos
    class InNumCaloHypos : public Input
    {
    public:
      virtual double value( const LHCb::ProtoParticle * proto ) const
      {
        return static_cast<double>(proto->calo().size());
      }
    };

    /// Access a variable from the RecSummary
    class InRecSummary : public Input
    {
    public:
      InRecSummary( const int info,
                    const ChargedProtoANNPIDCommonBase<PBASE> * parent )
        : m_info( info ), m_parent( parent ) { }
    private:
      const int m_info;   ///< The info variable to access
      const ChargedProtoANNPIDCommonBase<PBASE> * m_parent; ///< Pointer to parent
    public:
      virtual double value( const LHCb::ProtoParticle * ) const
      {
        return m_parent->recSummary()->info( m_info, 0 );
      }
    };

    /// Calo Ecal chi^2
    class InCaloEcalChi2 : public Input
    {
    public:
      virtual double value( const LHCb::ProtoParticle * proto ) const
      {
        double var = proto->info(LHCb::ProtoParticle::CaloEcalChi2,-999);
        if      ( var < -100    ) { var = -999; }
        else if ( var > 9999.99 ) { var = -999; }
        return var;
      }
    };

    /// Calo Brem chi^2
    class InCaloBremChi2 : public Input
    {
    public:
      virtual double value( const LHCb::ProtoParticle * proto ) const
      {
        double var = proto->info(LHCb::ProtoParticle::CaloBremChi2,-999);
        if      ( var < -100    ) { var = -999; }
        else if ( var > 9999.99 ) { var = -999; }
        return var;
      }
    };

    /// Calo Cluster chi^2
    class InCaloClusChi2 : public Input
    {
    public:
      virtual double value( const LHCb::ProtoParticle * proto ) const
      {
        double var = proto->info(LHCb::ProtoParticle::CaloClusChi2,-999);
        if      ( var < -100   ) { var = -999; }
        else if ( var > 999.99 ) { var = -999; }
        return var;
      }
    };

  protected:

    /** @class Cut ChargedProtoANNPIDCommonBase.h
     *
     *  ProtoParticle selection cut
     *
     *  @author Chris Jones
     *  @date   2010-03-09
     */
    class Cut
    {
    public:
      /// Vector of cuts
      typedef std::vector<const Cut*> ConstVector;
    private:
      /// Delimitor enum
      enum Delim { UNDEFINED = -1, GT, LT, GE, LE };
    public:
      /// Constructor
      Cut( const std::string& desc = "NOTDEFINED",
           const ChargedProtoANNPIDCommonBase<PBASE> * parent = NULL );
      /// Destructor
      ~Cut( ) { delete m_variable; }
    private:
      /// No Copy Constructor
      Cut( const Cut& ) { }
    public:
      /// Is this object well defined
      bool isOK() const { return m_OK; }
      /// Does the ProtoParticle pass the cut
      bool isSatisfied( const LHCb::ProtoParticle * proto ) const
      {
        const double var = m_variable->value(proto);
        return ( m_delim == GT ? var >  m_cut :
                 m_delim == LT ? var <  m_cut :
                 m_delim == GE ? var >= m_cut :
                 m_delim == LE ? var <= m_cut :
                 false );
      }
      /// Cut description
      const std::string description() const { return m_desc; }
    public:
      /// Overload output to ostream
      friend inline std::ostream& operator << ( std::ostream& s, const Cut & cut )
      { return s << "'" << cut.description() << "'" ; }
    private:
      /// Set the delimitor enum from a string
      bool setDelim( const std::string & delim )
      {
        bool ok = false;
        if      ( ">"  == delim ) { m_delim = GT; ok = true; }
        else if ( "<"  == delim ) { m_delim = LT; ok = true; }
        else if ( ">=" == delim ) { m_delim = GE; ok = true; }
        else if ( "<=" == delim ) { m_delim = LE; ok = true; }
        return ok;
      }
    private:
      std::string m_desc; ///< The cut description
      bool m_OK;          ///< Is this cut well defined
      const Input * m_variable;  ///< The variable to cut on
      double m_cut;       ///< The cut value
      Delim m_delim;      ///< The delimitor
    };

  protected:

    /** @class ANNHelper ChargedProtoANNPIDCommonBase.h
     *
     *  Base class for ANN helpers
     *
     *  @author Chris Jones
     *  @date   2010-03-09
     */
    class ANNHelper
    {
    protected:
      /// Typedef for list of inputs
      typedef typename Input::ConstVector Inputs;
    protected:
      /// No default constructor
      ANNHelper() : m_ok( false ) { }
    public:
      /** Constructor from information
       *  @param inputs The list of inputs needed for this network
       *  @param parent Point to parent algorithm
       */
      ANNHelper( const ChargedProtoANNPIDCommonBase<PBASE>::StringInputs& inputs,
                 const ChargedProtoANNPIDCommonBase<PBASE> * parent )
        : m_inputs ( parent->getInputs(inputs) ),
          m_ok     ( false  ) { }
      /// Destructor
      virtual ~ANNHelper()
      {
        for ( auto * input : m_inputs ) { delete input; }
      }
    public:
      /// Are we configured properly
      inline bool isOK() const { return m_ok; }
      /// Set the OK flag
      void setOK( const bool ok ) { m_ok = ok; }
      /// Compute the ANN output for the given ProtoParticle
      virtual double getOutput( const LHCb::ProtoParticle * proto ) const = 0;
      /// Number of inputs to the ANN
      inline unsigned int nInputs() const { return m_inputs.size(); }
      /// Access the inputs
      const Inputs& inputs() const { return m_inputs; }
    private:
      /// The list of inputs for this network
      Inputs m_inputs;
      /// Is this reader configured properly
      bool m_ok;
    };

#ifdef _ENABLE_NEUROBAYES
    /** @class NeuroBayesANN ChargedProtoANNPIDCommonBase.h
     *
     *  Helper class for NeuroBayes networks
     *
     *  @author Chris Jones
     *  @date   2013-03-09
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
       *  @param suppressPrintout Supress all output from NeuroBayes
       */
      NeuroBayesANN( const std::string & paramFileName,
                     const StringInputs& inputs,
                     const ChargedProtoANNPIDCommonBase<PBASE> * parent,
                     const bool suppressPrintout = true )
        : ANNHelper ( inputs, parent ),
          m_expert  ( new Expert(paramFileName.c_str(),-2) ),
          m_inArray ( new float[inputs.size()]             ),
          m_suppressPrintout ( suppressPrintout            )
      { 
        this->setOK(true);
      }
      /// Destructor
      virtual ~NeuroBayesANN() { delete m_expert; delete[] m_inArray; }
    public:
      /// Compute the ANN output for the given ProtoParticle
      virtual double getOutput( const LHCb::ProtoParticle * proto ) const;
    private:
      Expert * m_expert;  ///< Pointer to the NeuroBayes 'Expert'
      float * m_inArray;  ///< Working array for network inputs
      bool m_suppressPrintout; ///< Suppress any printout from NeuroBayes
    };
#endif

    /** @class TMVAReaderANN ChargedProtoANNPIDCommonBase.h
     *
     *  Helper class for TMVA ANN networks via the Reader
     *
     *  @author Chris Jones
     *  @date   2013-03-09
     */
    class TMVAReaderANN : public ANNHelper
    {
    private:
      /// No default constructor
      TMVAReaderANN() : m_reader(NULL) { }
    public:
      /** Constructor from information
       *  @param paramFileName Network tuning parameter file
       *  @param inputs The list of inputs needed for this network
       *  @param parent Point to parent algorithm
       */
      TMVAReaderANN( const std::string & paramFileName,
                     const StringInputs& inputs,
                     const ChargedProtoANNPIDCommonBase<PBASE> * parent )
        : ANNHelper ( inputs, parent ),
          m_reader  ( new TMVA::Reader( parent->msgLevel(MSG::DEBUG) ?
                                        "!Color:!Silent" : "!Color:Silent" ) ),
          m_inArray ( new float[inputs.size()] )
      {
        unsigned int i = 0;
        for ( const std::string & input : inputs )
        {
          m_reader->AddVariable( input.c_str(), &(m_inArray[i++]) );
        }
        m_reader->BookMVA( "PID", paramFileName.c_str() );
        this->setOK(true);
      }
      /// Destructor
      virtual ~TMVAReaderANN() { delete m_reader; delete[] m_inArray; }
    public:
      /// Compute the ANN output for the given ProtoParticle
      virtual double getOutput( const LHCb::ProtoParticle * proto ) const;
    private:
      TMVA::Reader * m_reader; ///< The TMVA reader
      float * m_inArray;  ///< Working array for network inputs
    };

    /** @class TMVAImpANN ChargedProtoANNPIDCommonBase.h
     *
     *  Helper class for TMVA ANN networks via the C++ implementation
     *
     *  @author Chris Jones
     *  @date   2013-03-09
     */
    class TMVAImpANN : public ANNHelper
    {
    private:
      /// No default constructor
      TMVAImpANN() : m_reader(NULL) { }
    public:
      /** Constructor from information
       *  @param paramFileName Network tuning parameter file
       *  @param inputs The list of inputs needed for this network
       *  @param parent Point to parent algorithm
       */
      TMVAImpANN( const std::string & config,
                  const std::string & particle,
                  const std::string & track,
                  const StringInputs& inputs,
                  const ChargedProtoANNPIDCommonBase<PBASE> * parent )
        : ANNHelper ( inputs, parent ),
          m_reader  ( tmvaFactory().create( config, particle, track, inputs ) ),
          m_vars    ( inputs.size() )
      {
        this->setOK( m_reader && m_reader->IsStatusClean() );
      }
      /// Destructor
      virtual ~TMVAImpANN() { delete m_reader; }
    public:
      /// Compute the ANN output for the given ProtoParticle
      virtual double getOutput( const LHCb::ProtoParticle * proto ) const;
    private:
      IClassifierReader * m_reader; ///< The TMVA reader
      mutable std::vector<double> m_vars; ///< the input variables
    };

  protected:

    /** @class NetConfig ChargedProtoANNPIDCommonBase.h
     *
     *  Helper class that encapsulates the configration of an ANN nework
     *
     *  @author Chris Jones
     *  @date   2014-06-27
     */
    class NetConfig
    {
    public:

      /// Constructor
      NetConfig( const std::string & trackType,
                 const std::string & pidType,
                 const std::string & netVersion,
                 const bool suppressANNPrintout,
                 const ChargedProtoANNPIDCommonBase<PBASE> * parent );

      /// Destructor
      ~NetConfig();

      /// Status
      inline bool isOK() const { return m_OK; }

      /// Access the Network object
      inline const ANNHelper * netHelper() const { return m_netHelper; }

      /// Access the track type
      inline const std::string& trackType() const { return m_trackType; }

      /// Access the particle type
      inline const std::string& particleType() const { return m_particleType; }

      /// Check a ProtoParticle against the configured cuts
      bool passCuts( const LHCb::ProtoParticle * proto ) const;

    private:

      /// Status
      bool m_OK;

      /// Network Helper
      ANNHelper * m_netHelper;

      /// Vector of cuts to apply
      typename Cut::ConstVector m_cuts;

      /// The particle type
      std::string m_particleType;

      /// The track type
      std::string m_trackType;

    };

  public:

    /** Get the Input object for a given input name
     *  @attention Created on the heap therefore user takes ownership
     */
    const Input * getInput( const std::string& name ) const;

    /** Get a vector of input objects for a given set of names
     *  @attention Created on the heap, and user takes ownership
     */
    typename Input::ConstVector getInputs( const StringInputs& names ) const;

  private:

    /// Common Constructor initisalisations
    void initCommonConstructor();

    /// Access on demand the RecSummary object
    const LHCb::RecSummary * recSummary() const;

  protected:

    std::string m_protoPath; ///< Location in TES of ProtoParticles

    /// Cached pointer to the RecSummary object
    mutable const LHCb::RecSummary * m_summary;

  };

}

#endif // ChargedProtoANNPID_CHARGEDPROTOANNPIDCOMMONBASE_H
