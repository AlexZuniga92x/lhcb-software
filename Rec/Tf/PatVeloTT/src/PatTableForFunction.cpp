// $Id: PatTableForFunction.cpp,v 1.1 2007-08-25 14:27:37 jonrob Exp $
// Include files

// from Gaudi
#include "GaudiKernel/ToolFactory.h"

// local
#include "PatTableForFunction.h"

//-----------------------------------------------------------------------------
// Implementation file for class : PatTableForFunction
//
// 2006-09-25 : Mariusz Witek
//-----------------------------------------------------------------------------

using namespace Tf;

// Declaration of the Tool Factory
DECLARE_TOOL_FACTORY( PatTableForFunction );


//=============================================================================
// Standard constructor, initializes variables
//=============================================================================
PatTableForFunction::PatTableForFunction( const std::string& type,
                                          const std::string& name,
                                          const IInterface* parent )
  : GaudiTool ( type, name , parent )
{
  declareInterface<PatTableForFunction>(this);
}
//=============================================================================
// Destructor
//=============================================================================
PatTableForFunction::~PatTableForFunction() {}


void PatTableForFunction::addVariable(int nBin, double lowVal, double highVal) {
  m_nPointVar.push_back(nBin+1);
  m_minVar.push_back(lowVal);
  m_maxVar.push_back(highVal);

  m_deltaVar.push_back((highVal-lowVal)/nBin);

  m_nVar = m_nPointVar.size();
}

void PatTableForFunction::prepareTable() {

  int i;
  for (i=0; i<m_nVar; i++) {
    m_indexVector.push_back(0);
    m_variableVector.push_back(0.);
  }

  int iSize = 1;
  for (i=0; i<m_nVar; i++) {
    iSize *= (m_nPointVar[i]);
  }

  for (i=0; i<iSize; i++) {
    m_table.push_back(0.);
  }

}

void PatTableForFunction::resetIndexVector() {
  int i;
  for (i=0; i<m_nVar; i++) {
    m_indexVector[i] = 0;
  }
}

void PatTableForFunction::resetVariableVector() {
  int i;
  for (i=0; i<m_nVar; i++) {
    m_variableVector[i] = 0;
  }
}

int PatTableForFunction::incrementIndexVector() {
  int i;
  m_indexVector[0]++;
  for (i=0; i<m_nVar-1; i++) {
    if(m_indexVector[i] > m_nPointVar[i]-1) {
      m_indexVector[i]=0;
      m_indexVector[i+1]++;
    }
    else {
      break;
    }
  }
  if(m_indexVector[m_nVar-1] > m_nPointVar[m_nVar-1]-1) {
    resetIndexVector();
    return 1;
  }

  return 0;
}


void PatTableForFunction::fillTable(double lutValue) {
  m_table[tableLocation()] = lutValue;
}

double PatTableForFunction::getVariable(int iv) {
  calculateVariableVector();
  if(iv<0 || iv> m_nVar-1) return 0.0;
  return m_variableVector[iv];
}

void PatTableForFunction::getVariableVector(std::vector<double>& var) {
  int i;
  calculateVariableVector();
  for (i=0; i<m_nVar; i++) {
    var[i] = m_variableVector[i];
  }
}

double PatTableForFunction::getValueFromTable(std::vector<double>& var) {
  calculateClosestIndexVector(var);
  return (double) m_table[tableLocation()];
}

double PatTableForFunction::getInterpolatedValueFromTable(std::vector<double>& var) {
  int i;
  calculateIndexVector(var);
  calculateVariableVector();
  double tabVal = m_table[tableLocation()];
  double addTabVal = 0.0;
  for(i=0; i<m_nVar; i++) {
    if(var[i]<m_variableVector[i]) continue;
    if(m_indexVector[i] > m_nPointVar[i]-2) continue;
    if(var[i]<m_minVar[i] || var[i]>m_maxVar[i]) continue;
    m_indexVector[i]++;
    double dTab_dVar =  (m_table[tableLocation()] - tabVal) / m_deltaVar[i];
    m_indexVector[i]--;
    double dVar = (var[i]-m_variableVector[i]);
    addTabVal += dTab_dVar*dVar;
  }
  tabVal+=addTabVal;
  return tabVal;
}

int PatTableForFunction::tableLocation() {
  int i;
  if(1==m_nVar) return m_indexVector[0];
  int location;
  location = m_indexVector[m_nVar-1];
  for (i=m_nVar-2; i>=0; i--) {
    location = location*m_nPointVar[i] + m_indexVector[i];
  }
  return location;
}

void PatTableForFunction::calculateVariableVector() {
  int i;
  for (i=0; i<m_nVar; i++) {
    m_variableVector[i]=m_minVar[i]+m_indexVector[i]*m_deltaVar[i];
  }
}

void PatTableForFunction::calculateIndexVector(std::vector<double>& var) {
  int i;
  for (i=0; i<m_nVar; i++) {
    int idx = (int) ((var[i]-m_minVar[i])/m_deltaVar[i] + 0.000000001);
    if(idx<0) idx=0;
    if(idx>(m_nPointVar[i]-1)) idx=m_nPointVar[i]-1;
    m_indexVector[i]=idx;
  }
}

void PatTableForFunction::calculateClosestIndexVector(std::vector<double>& var) {
  int i;
  for (i=0; i<m_nVar; i++) {
    int idx = (int) ((var[i]-m_minVar[i])/m_deltaVar[i] + 0.5);
    if(idx<0) idx=0;
    if(idx>(m_nPointVar[i]-1)) idx=m_nPointVar[i]-1;
    m_indexVector[i]=idx;
  }
}




//=============================================================================
