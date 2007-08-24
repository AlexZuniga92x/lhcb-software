// $Id: TableForFunction.h,v 1.2 2007-08-24 12:44:08 jonrob Exp $
#ifndef TABLEFORFUNCTION_H
#define TABLEFORFUNCTION_H 1

// Include files
// from Gaudi
#include "GaudiAlg/GaudiTool.h"

static const InterfaceID IID_TableForFunction ( "TableForFunction", 1, 0 );

namespace Tf {

  /** @class TableForFunction TableForFunction.h newtool/TableForFunction.h
   *
   *  Some internal class for pat
   *
   * @code
   * Example:
   *
   * Fill table
   * ----------
   * addVariable(10,-5.,5.);
   * addVariable(20,-50.,50.);
   *
   * resetIndexVector();
   * std::vector<double> var;
   * int iover = 0;
   * while(!iover) {
   *  getVariableVector(var);
   *  fillTable( fun (var[0],var[1]) );
   *  iover= incrementIndexVector();
   * }
   *
   * Retrieve information
   * --------------------
   * var[0]=1.5;
   * var[1]=4.1;
   * getValueFromTable(var);
   * or
   * getInterpolatedValueFromTable(var);
   * @endcode
   *
   *  @author Mariusz Witek
   *  @date   2006-09-25
   *  @update for A-Team framework 2007-08-20 SHM
   *
   */

  class TableForFunction : public GaudiTool {
  public:

    // Return the interface ID
    static const InterfaceID& interfaceID() { return IID_TableForFunction; }

    /// Standard constructor
    TableForFunction( const std::string& type,
                      const std::string& name,
                      const IInterface* parent);

    virtual ~TableForFunction( ); ///< Destructor

    void   addVariable(int nBin, double lowVal, double highVal);
    void   prepareTable();
    void   resetIndexVector();
    int    incrementIndexVector();
    void   fillTable(double lutValue);
    double getVariable(int ivar);
    void   getVariableVector(std::vector<double>& var);
    double getValueFromTable(std::vector<double>& var);
    double getInterpolatedValueFromTable(std::vector<double>& var);

  protected:

  private:

    void   createTable();
    void   deleteTable();
    void   resetVariableVector();
    int    tableLocation();
    void   calculateVariableVector();
    void   calculateIndexVector(std::vector<double>& var);
    void   calculateClosestIndexVector(std::vector<double>& var);

    int     m_nVar;
    std::vector<int>    m_nPointVar;
    std::vector<double> m_minVar;
    std::vector<double> m_maxVar;
    std::vector<double> m_deltaVar;
    std::vector<double> m_table;

    std::vector<int>    m_indexVector;
    std::vector<double> m_variableVector;

  };
}
#endif // TABLEFORFUNCTION_H
