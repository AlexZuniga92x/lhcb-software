// $Id: IFilterCriteriaTESTool.h,v 1.1 2005-06-28 15:32:27 lazzecri Exp $
#ifndef DAVINCITOOLS_IFILTERCRITERIATESTOOL_H
#define DAVINCITOOLS_IFILTERCRITERIATESTOOL_H 1

// Include files
// from STL
#include <string>

// from Gaudi
#include "GaudiKernel/IAlgTool.h"

// from DaVinciTools
class IFilterCriterion;

// Event
class Particle;
#include "Event/FilterCriterionResult.h"

static const InterfaceID IID_IFilterCriteriaTESTool ( "IFilterCriteriaTESTool", 1, 0 );

/** @class IFilterCriteriaTESTool IFilterCriteriaTESTool.h DaVinciTools/IFilterCriteriaTESTool.h
 *
 *  Interface to tool to save filter results to TES and provides access methods
 *  to retrieve the results for analyses.
 *
 *  @author Cristina Lazzeroni
 *  @date   2005-06-23
 */

class IFilterCriteriaTESTool : virtual public IAlgTool {

public: // definitions

  typedef std::vector<const FilterCriterionResult *> ResultsVector;

public:

  // Return the interface ID
  static const InterfaceID& interfaceID() { return IID_IFilterCriteriaTESTool; }

  /// Save the given filter result to the TES container
  virtual bool saveFilterResult( IFilterCriterion * critTool,
                                 Particle * part,
                                 const bool result ) const = 0;

  /// Access to the vector of all filter results
  virtual const FilterCriterionResults * criterionResults() const = 0;

  /// Returns a vector of all filter results that either failed or succeeded
  virtual bool criterionResults( ResultsVector & results,
                                 const bool result = true ) const = 0;

  /// Returns all filter results that are associated to the given name
  virtual bool criterionResults( ResultsVector & results,
                                 std::string & name ) const = 0;

  /// Returns all filter results that were applied to a given particle
  virtual bool criterionResults( ResultsVector & results,
                                 const Particle * part ) const = 0;

};

#endif // DAVINCITOOLS_IFILTERCRITERIATESTOOL_H
