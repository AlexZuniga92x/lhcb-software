
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

// Event Model
#include "Event/ProtoParticle.h"

//-----------------------------------------------------------------------------
/** @namespaceANNGlobalPID
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
  class ChargedProtoANNPIDCommonBase : public PBASE
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

  protected:

    /// Type for list of inputs
    typedef std::vector<std::string> StringInputs;

    /// Type for list of inputs
    typedef std::vector<int> IntInputs;

    /// Type for Mapping between string and int IDs
    typedef std::map<std::string,int> StringToInt;

  protected:
    
    /// Mapping between string and int IDs
    const StringToInt & stringToIntMap() const;

    /// Convert a variable name string to ID number
    int variableID( const std::string & name ) const;

    /// Convert a variable name string to ID number
    std::string stringID( const int id ) const;

    /// Convert variable names string to ID numbers
    IntInputs variableIDs( const StringInputs & names ) const;

  public:

    /// Get the value for the given input int ID
    double getInput( const LHCb::ProtoParticle * proto,
                     const int input ) const;

    /// Get the value for the given input string ID
    inline double getInput( const LHCb::ProtoParticle * proto,
                            const std::string& input ) const
    {
      return getInput( proto, this->variableID(input) );
    }

  private:

    /// Common Constructor initisalisations
    void initCommonConstructor();

  protected:
    
    std::string m_protoPath; ///< Location in TES of ProtoParticles

  };

}

#endif // ChargedProtoANNPID_CHARGEDPROTOANNPIDCOMMONBASE_H
