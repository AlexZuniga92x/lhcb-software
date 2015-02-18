
#include "TrackKernel/CubicStateInterpolationTraj.h"

#include "LHCbMath/Similarity.h"

namespace LHCb
{

  LHCb::State CubicStateInterpolationTraj::state( double z ) const
  {
    const Gaudi::TrackVector vec(x(z),y(z),tx(z),ty(z),qop(z)) ;
    return LHCb::State(vec, covariance(z), z,  LHCb::State::LocationUnknown) ;
  }
  
  Gaudi::TrackSymMatrix CubicStateInterpolationTraj::covariance( double z ) const
  {
    static Gaudi::TrackMatrix Fb = Gaudi::TrackMatrix( ROOT::Math::SMatrixIdentity() );
    Fb(1,3) = Fb(0,2) = (z - zbegin()) ;
    
    static Gaudi::TrackMatrix Fe = Gaudi::TrackMatrix( ROOT::Math::SMatrixIdentity() );
    Fe(1,3) = Fe(0,2) = (z - zend()) ;
    
    static Gaudi::TrackSymMatrix cov ;

    if( z <= zbegin() ) {
      cov = LHCb::Math::Similarity(Fb, *m_covbegin) ;
    } else if (z>= zend()) {
      cov = LHCb::Math::Similarity(Fe, *m_covend) ;
    } else {
      // what is the right weight? FIXME!
      const double zfrac = (z - zbegin())/(zend()-zbegin()) ;
      const double weight = (1-zfrac) ; //linear
      //double weight = 0.5 - 4*std::pow(zfrac-0.5,3) ; // cubic
      cov = ( ( weight     * LHCb::Math::Similarity(Fb, *m_covbegin) ) +
              ( (1-weight) * LHCb::Math::Similarity(Fe, *m_covend)   ) );
    }
    
    return cov ;
  }

}
