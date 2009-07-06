#ifndef ALIGNSOLVTOOLS_ALEQUATIONS_H
#define ALIGNSOLVTOOLS_ALEQUATIONS_H

#include <vector>
#include <map>
#include <ostream>
#include <istream>
#include "GaudiKernel/GenericMatrixTypes.h"
#include "GaudiKernel/SymmetricMatrixTypes.h"
#include "GaudiKernel/GenericVectorTypes.h"
#include "GaudiKernel/DataObject.h"
  
namespace Al
{
  class ElementData
  {
  public:
    typedef std::map<size_t, Gaudi::Matrix6x6> OffdiagonalContainer ;
    typedef ROOT::Math::SMatrix<double,6,5> TrackDerivatives ;
    typedef ROOT::Math::SMatrix<double,6,3> VertexDerivatives ;
    
    ElementData() ;
    void add(const ElementData& rhs) ;
    void transform( const Gaudi::Matrix6x6& jacobian ) ;
    const Gaudi::Vector6& dChi2DAlpha() const { return m_dChi2DAlpha ; }
    const Gaudi::SymMatrix6x6& d2Chi2DAlpha2() const { return m_d2Chi2DAlpha2 ; }
    const OffdiagonalContainer& d2Chi2DAlphaDBeta() const { return m_d2Chi2DAlphaDBeta ; }
    const TrackDerivatives& dStateDAlpha() const { return m_dStateDAlpha ; }
    const VertexDerivatives& dVertexDAlpha() const { return m_dVertexDAlpha ; }

    double fracNonOutlier() const { return 1 - (m_numHits >0 ? m_numOutliers/double(m_numHits): 0 ) ; }
    double weightV() const { return m_weightV ; }
    double weightR() const { return m_weightR ; }
    size_t numHits() const { return m_numHits ; }
    size_t numOutliers() const { return m_numOutliers ; }
    void addHitSummary(double V, double R) {
      m_numHits += 1 ;
      m_weightV += 1/V ;
      m_weightR += R/(V*V) ;
    }

    Gaudi::Vector6       m_dChi2DAlpha ;      // (half) 1st derivative
    Gaudi::SymMatrix6x6  m_d2Chi2DAlpha2;     // (half) 2nd derivative diagonal ('this-module')
    OffdiagonalContainer m_d2Chi2DAlphaDBeta; // (half) 2nd derivative off-diagonal ('module-to-module')
    TrackDerivatives     m_dStateDAlpha ;     // derivative of track parameter to alpha
    VertexDerivatives    m_dVertexDAlpha ;    // derivative of vertex position to alpha
    size_t               m_numHits ;
    size_t               m_numOutliers ;
    double               m_weightV ; // sum V^{-1}          --> weight of 1st derivative
    double               m_weightR ; // sum V^{-1} R V^{-1} --> weight of 2nd derivative
  } ;
  
  class Equations {
  public:
    typedef std::vector<ElementData> ElementContainer  ;
    Equations(size_t nElem=0) ;
    void clear() ;
    size_t nElem() const { return m_elements.size() ; }
    ElementData& element(size_t i) { return m_elements[i] ; }
    const ElementData& element(size_t i) const { return m_elements[i] ; }

     void addChi2Summary( double chisq, size_t ndof, size_t nexternal) {
      m_totalChiSquare +=chisq;
      m_totalNumDofs   +=ndof ;
      m_numExternalHits+=nexternal ;
    }
    
    void addVertexChi2Summary( double chisq, size_t ndof ) {
      m_totalVertexChiSquare +=chisq;
      m_totalVertexNumDofs   +=ndof ;
    }
    
    void addEventSummary( size_t numtracks, 
			  size_t numvertices,
			  size_t numdimuons ) {
      ++m_numEvents ;
      m_numTracks += numtracks ;
      m_numVertices += numvertices ;
      m_numDiMuons += numdimuons ;
    }

    size_t numTracks() const { return m_numTracks ; }
    size_t numVertices() const { return m_numVertices ; }
    size_t numDiMuons() const { return m_numDiMuons ; }
    size_t numEvents() const { return m_numEvents ; }
    size_t numHits() const ;
    double totalChiSquare() const { return m_totalChiSquare ; }
    size_t totalNumDofs() const { return m_totalNumDofs ; }
    double totalVertexChiSquare() const { return m_totalVertexChiSquare ; }
    size_t totalVertexNumDofs() const { return m_totalVertexNumDofs ; }
    double totalTrackChiSquare() const { return totalChiSquare() - m_totalVertexChiSquare ; }
    size_t totalTrackNumDofs() const { return totalNumDofs() - m_totalVertexNumDofs ; }
    size_t numExternalHits() const { return m_numExternalHits ; }
    
    void writeToFile(const char* filename) const ;
    void readFromFile(const char* filename) ;
    void writeToBuffer(std::ostream& buffer) const ;
    void readFromBuffer(std::istream& buffer) ;
    void add(const Al::Equations&) ;

    std::ostream& fillStream(std::ostream& os) const ;

    const ElementContainer& elements() const { return m_elements ; }
  private:
    ElementContainer m_elements ;
    size_t           m_numEvents ;
    size_t           m_numTracks ;
    size_t           m_numVertices ;
    size_t           m_numDiMuons ;
    double           m_totalChiSquare ;
    size_t           m_totalNumDofs ;
    size_t           m_numExternalHits ;
    double           m_totalVertexChiSquare ;
    size_t           m_totalVertexNumDofs ;
  };
  
} ;
#endif // ALIGNSOLVTOOLS_ALEQUATIONS_H
