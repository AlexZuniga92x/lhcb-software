// $Id: FwdParameters.cpp,v 1.1.1.1 2007-10-09 18:50:43 smenzeme Exp $
// Include files 
#include <stdio.h> 

// from Gaudi

// local
#include "FwdParameters.h"

//-----------------------------------------------------------------------------
// Implementation file for class : FwdParameters
//
// 27/11/2001 : Olivier Callot
//-----------------------------------------------------------------------------

//=============================================================================
// Standard constructor, initializes variables
//=============================================================================
FwdParameters::FwdParameters(  ) {
  m_nbPar = 0;
  for ( int i=0 ; 10>i ; i++ ) {
    m_par[i] = 0.;
    m_dat[i] = 0.;
    m_fun[i] = 0.;
    for ( int j=0 ; 10>j ; j++ ) {
      m_mat[i][j] = 0.;
    }
  }
}

//=============================================================================
// Destructor
//=============================================================================
FwdParameters::~FwdParameters() {}; 


//=========================================================================
//  Initialisation, give the size and the parameters
//=========================================================================
void FwdParameters::init ( std::string title, 
                           std::vector<double> param ) {
  m_title = title;
  m_nbPar = param.size();
  int    i;
  for ( i=0 ; m_nbPar>i ; i++ ) {
    m_par[i] = param[i];
  }
}

//=========================================================================
//  Add an event in the computation. 
//=========================================================================
void FwdParameters::addEvent ( double delta ) {
  for ( int i=0 ; m_nbPar>i ; i++ ) {
    m_dat[i] += delta * m_fun[i];
    for ( int j=0 ; m_nbPar>j ; j++ ) {
      m_mat[i][j] += m_fun[i] * m_fun[j];
    }
  }
}

//=========================================================================
//  Get the resultant modified parameters, log the results
//=========================================================================
bool FwdParameters::updateParameters ( MsgStream& log ) {

  int im, jm, i, j;
  double den;
  double c1, c2;
  char line[120];

  if ( 0 == m_nbPar ) { return true; }

  log << MSG::INFO << endreq 
      << "** " << m_title << "Params **" << endreq 
      << endreq;

  // Resolve the relation m_dat = m_mat * change

  for ( im = 0 ; m_nbPar > im  ; im++  ) {
    for ( jm = m_nbPar-1 ; 0 <= jm ; jm-- ) {

      // Normalize each row
      for ( i=0 ; m_nbPar > i ; i++ ) {
        if ( 0 != m_mat[i][i] ) {
          den = m_mat[i][i];
          m_dat[i] /= den;
          for ( j=0 ; m_nbPar > j ; j++ ) {
            m_mat[i][j] /= den;
          }
        }
      }

      if ( im == jm ) continue;
      if ( (0 == m_mat[jm][im]) && 
           (0 == m_mat[im][jm])    ) continue;

      log << MSG::VERBOSE << "..Handling line " << im
          << " column " << jm << endreq;
      for ( i=0 ; m_nbPar > i ; i++ ) {
        sprintf( line, "%10.3e ", m_dat[i] );
        log << MSG::VERBOSE << "[" << i << "] " << line << " : ";
        for ( j=0 ; m_nbPar > j ; j++ ) {
          sprintf( line, "%10.3e ", m_mat[i][j] );
          log << line;
        }
        log << endreq;
      }
      
      den = m_mat[im][im]*m_mat[jm][jm] - m_mat[im][jm]*m_mat[jm][im];

      if ( 0 != den ) {
        c1 =   m_mat[jm][jm]/den;
        c2 = - m_mat[im][jm]/den;
        m_dat[m_nbPar] = m_dat[im]*c1 + m_dat[jm]*c2;
        for ( i=0 ; m_nbPar > i ; i++ ) {
          m_mat[m_nbPar][i] = m_mat[im][i]*c1 + m_mat[jm][i]*c2;
        }
        c1 = - m_mat[jm][im]/den;
        c2 =   m_mat[im][im]/den;
        m_dat[jm] = m_dat[im]*c1 + m_dat[jm]*c2;
        for ( i=0 ; m_nbPar > i ; i++ ) {
          m_mat[jm][i] = m_mat[im][i]*c1 + m_mat[jm][i]*c2;
        }
        m_mat[jm][im] = 0.;
        m_mat[m_nbPar][jm] = 0.;

        m_dat[im] = m_dat[m_nbPar];
        for ( i=0 ; m_nbPar > i ; i++ ) {
          m_mat[im][i] = m_mat[m_nbPar][i];
        }
      } else {
        log << MSG::WARNING << "*** Singular local matrix ! ***" << endreq;
        return false;
      }
    }
  }

  for ( i=0 ; m_nbPar>i ; i++ ) {
    if( 0 !=  m_mat[i][i] ) {
      double denom = m_par[i];
      if ( 0 == denom ) denom = 1;
        
      sprintf( line, "    [%d] %12.4e + %12.4e ( %8.3f ) => %12.4e", 
               i, 
               m_par[i], 
               m_dat[i], 
               m_dat[i]/denom, 
               m_par[i]+m_dat[i] );
      log << MSG::INFO << line << endreq;
      m_par[i] += m_dat[i];
    }
  }
  return true;
}

//=========================================================================
//  Print on cout the new input line
//=========================================================================
void FwdParameters::printParams ( std::string prefix ) {
  if ( 0 == m_nbPar ) { return; }
  
  std::cout << prefix << "."
            << m_title << "Params = { "<< m_par[0];
  int i = 1;
  while ( 0 != m_mat[i][i] ) {
    std::cout << ", " << m_par[i++];
  }
  std::cout << " };" << std::endl;
}

//=============================================================================
