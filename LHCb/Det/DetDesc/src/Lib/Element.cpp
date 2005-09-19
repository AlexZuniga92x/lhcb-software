// $Id: Element.cpp,v 1.8 2005-09-19 14:25:29 mneedham Exp $
/// STL and STD 
#include <math.h>
/// GaudiKernel
#include "GaudiKernel/StatusCode.h"
/// DetDesc 
#include "DetDesc/MaterialException.h"
#include "DetDesc/Element.h"
#include "DetDesc/Isotope.h"
///


///////////////////////////////////////////////////////////////////////////////////////
Element::Element( const std::string& name    , 
                  const std::string& symb    ,
                  const double       a       , 
                  const double       z       , 
                  const double       density ,                    
                  const double       rl      , 
                  const double       al      ,
                  const double       temp    ,
                  const double       press   ,
                  const eState       s       )
  : Material( name , density , rl , al , temp, press , s )
  , m_Aeff ( a )
  , m_Zeff ( z )
  , m_isotopes()
  , m_coulomb ()
  , m_tsai    ()
  , m_symb ( symb )
{
  if( 0 < Z() ) { ComputeCoulombFactor  (); } 
  if( 0 < Z() ) { ComputeLradTsaiFactor (); }
};
/////////////////////////////////////////////////////////////////////////////////
Element::~Element() { m_isotopes.clear();  }
/////////////////////////////////////////////////////////////////////////////////
void  Element::addIsotope ( const SmartRef<Isotope>& iPtr , const double Fract, const bool comp )
{ addIsotope( Entry( Fract, iPtr ) , comp ); }
/////////////////////////////////////////////////////////////////////////////////
void  Element::addIsotope ( const Entry&             iPtr                     , const bool comp )
{ 
  m_isotopes.push_back( iPtr ) ; 
  if( comp ) { compute(); } 
}; 
/////////////////////////////////////////////////////////////////////////////////
void  Element::removeIsotope ( const SmartRef<Isotope>& iPtr , const bool comp )
{
  for( Isotopes::iterator it = m_isotopes.begin() ; m_isotopes.end() != it ; ++it )
    { if( it->second == iPtr ) { m_isotopes.erase(it); break; }  } 
  if( comp ) { compute(); } 
};
/////////////////////////////////////////////////////////////////////////////////
const SmartRef<Isotope>&  Element::isotope( const unsigned int i ) const 
{
  if( i >= isotopes().size() )
    { throw MaterialException("Element::isotope(indx), wrong index!", this );}
  return isotopes()[i].second;  
};
/////////////////////////////////////////////////////////////////////////////////
      SmartRef<Isotope>&  Element::isotope( const unsigned int i )       
{
  if( i >= isotopes().size() )
    { throw MaterialException("Element::isotope(indx), wrong index! ", this );}
  return isotopes()[i].second;  
};
/////////////////////////////////////////////////////////////////////////////////
const double               Element::isotopeFraction( const unsigned int i ) const 
{
  if( i >= isotopes().size() )
    { throw MaterialException("Element::isotope(indx), wrong index! " , this );}
  return isotopes()[i].first;  
};
/////////////////////////////////////////////////////////////////////////////////
void Element::compute()
{
  // Effective Z is taken from the first isotope, anyway they should be the
  // same for all the isotopes in the element
  
  setZ( isotope(0)->Z() );

  double sum = 0.0;
  double aEff = 0.0;
  
  for( Isotopes::const_iterator it = m_isotopes.begin() ; m_isotopes.end() != it ; ++it )
    {
      aEff += (it->first) * ( it->second->A() ) ;
      sum  += (it->first) ;
    } 
  
  aEff /= sum;
  
  setA( aEff );
  
  ComputeCoulombFactor();
  ComputeLradTsaiFactor();

  // radiation length
  ComputeRadiationLength();
 
  // compute and set the interaction length
  ComputeInteractionLength(); 

}
/////////////////////////////////////////////////////////////////////////////////
void Element::ComputeCoulombFactor()
{
  // Compute Coulomb correction factor (Phys Rev. D50 3-1 (1994) page 1254)
  const double k1 = 0.0083 , k2 = 0.20206 ,k3 = 0.0020 , k4 = 0.0369 ;
  
  double az2 = (fine_structure_const * Z() ) * ( fine_structure_const * Z() );
  double az4 = az2 * az2;
  
  m_coulomb = (k1*az4 + k2 + 1./(1.+az2))*az2 - (k3*az4 + k4)*az4;
}
/////////////////////////////////////////////////////////////////////////////////
void Element::ComputeLradTsaiFactor()
{
  // Compute Tsai's Expression for the Radiation Length
  // (Phys Rev. D50 3-1 (1994) page 1254)

  const double Lrad_light[]  = {5.31  , 4.79  , 4.74 ,  4.71} ;
  const double Lprad_light[] = {6.144 , 5.621 , 5.805 , 5.924} ;
  
  const double logZ3 = log( Z() )/3.;

  double Lrad, Lprad;
  int iz = (int)(m_Zeff+0.5) - 1 ;
  if (iz <= 3)
    {
      Lrad = Lrad_light[iz] ;  Lprad = Lprad_light[iz];
    }
  else
    {
      Lrad = log(184.15) - logZ3 ; Lprad = log(1194.) - 2*logZ3 ;
    }
  
  m_tsai = 4*alpha_rcl2*m_Zeff*(m_Zeff*(Lrad-m_coulomb) + Lprad); 
}
/////////////////////////////////////////////////////////////////
StreamBuffer&     Element::serialize ( StreamBuffer& s ) const 
{
  Material::serialize( s );
  s << symbol() 
    << A() 
    << Z() 
    << coulombFactor () 
    << tsaiFactor    () 
    << isotopes().size();
  for( Isotopes::const_iterator it = isotopes().begin() ; isotopes().end() != it ; ++it )
    { s << it->first << it->second(this) ; }
  return s ;
};
/////////////////////////////////////////////////////////////////////////////////
StreamBuffer&     Element::serialize ( StreamBuffer& s )       
{
  Material::serialize( s );
  s >> m_symb 
    >> m_Aeff 
    >> m_Zeff
    >> m_coulomb 
    >> m_tsai  ;
  Isotopes::size_type size;
  s >> size ;
  m_isotopes.clear();
  m_isotopes = Isotopes(size);
  for( Isotopes::iterator it = m_isotopes.begin() ; m_isotopes.end() != it ; ++it )
    { s >> it->first >> it->second(this) ; }
  compute();
  return s ;
};
/////////////////////////////////////////////////////////////////////////////////
MsgStream&        Element::fillStream ( MsgStream&   s ) const 
{
  Material::fillStream( s ) ;
  s << " \t Symbol='"     << symbol() << "'\t" 
    << " CoulombFactor="  << std::setw(8) << coulombFactor() 
    << " TsaiFactor="     << std::setw(8) << tsaiFactor   ()
    << " #isotopes="      << std::setw(2) << isotopes().size() 
    << endreq;
  for( Isotopes::const_iterator it = isotopes().begin() ; isotopes().end() != it ; ++it )
    { 
      s << " \t\tisotope#"      << std::setw(2) << it - isotopes().begin() 
        << " fraction="  << std::setw(8) << it->first 
        << endreq 
        << "\t"           << it->second; 
    }
  return s;
};    
/////////////////////////////////////////////////////////////////////////////////
std::ostream&     Element::fillStream ( std::ostream& s ) const 
{
  Material::fillStream( s ) ;
  s << " \t Symbol='"     << symbol()     << "'\t" 
    << " CoulombFactor="  << std::setw(8) << coulombFactor() 
    << " TsaiFactor="     << std::setw(8) << tsaiFactor   ()
    << " #isotopes="      << std::setw(2) << isotopes().size() 
    << std::endl;
  for( Isotopes::const_iterator it = isotopes().begin() ; isotopes().end() != it ; ++it )
    { 
      s << " \t\tisotope#"      << std::setw(2) << it - isotopes().begin() 
        << " fraction="  << std::setw(8) << it->first 
        << std::endl 
        << "\t"           << it->second; 
    }
  return s;
};    

void Element::ComputeInteractionLength(){
  double intLen = 35.0 * pow(N(),1./3.) * (1./density()) * g/cm2;
  setAbsorptionLength(intLen);
}


void  Element::ComputeRadiationLength(){

 // compute radiation length
 double radleninv = Avogadro * tsaiFactor() / A();
 setRadiationLength(1.0 / radleninv / density());

};
