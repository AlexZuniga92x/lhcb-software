#ifndef SRC_CONDDBDQSCANNER_H
#define SRC_CONDDBDQSCANNER_H 1
// Include files
// from Gaudi
#include "GaudiAlg/GaudiTool.h"
#include "GaudiKernel/SmartIF.h"

// Implemented interfaces
#include "Kernel/IDQScanner.h" // IDQScanner

class ICondDBAccessSvc;

/** Basic implementation of an IDQScanner based on the Conditions Database.
 *
 * @author Marco Clemencic
 * @date 04/11/2011
 */
class CondDBDQScanner: public extends1<GaudiTool, IDQScanner> {
public:
  /// Standard constructor
  CondDBDQScanner(const std::string& type, const std::string& name, const IInterface* parent);
  virtual ~CondDBDQScanner(); ///< Destructor

  /// Scan all the Data Quality flags in the give time range in the CondDB.
  /// @return merged list of DQ flags
  virtual IDQFilter::FlagsType scan(const Gaudi::Time& since, const Gaudi::Time& until) const;

  virtual StatusCode initialize(); ///< Initialize the instance.
  virtual StatusCode finalize(); ///< Finalize the instance.

protected:
private:

  /// Path to the condition object containing the Data Quality flags.
  /// (property ConditionPath)
  std::string m_condPath;

  /// ICondDBReader implementation to use to access the Conditions Database.
  /// (property CondDBReader)
  std::string m_condDBAccessSvcName;

  /// ICondDBReader instance.
  SmartIF<ICondDBAccessSvc> m_condDB;

};

#endif // SRC_CONDDBDQSCANNER_H
