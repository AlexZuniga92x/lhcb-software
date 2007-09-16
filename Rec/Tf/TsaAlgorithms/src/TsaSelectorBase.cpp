// $Id: TsaSelectorBase.cpp,v 1.3 2007-09-16 10:01:51 mneedham Exp $

// GaudiKernel
#include "GaudiKernel/ToolFactory.h"

#include "TsaSelectorBase.h"
#include "TsaKernel_Tf/SeedFunctor.h"
#include "TsaKernel_Tf/SeedTrack.h"

#include <algorithm>
#include <utility>

#include <boost/lambda/bind.hpp>
#include <boost/lambda/lambda.hpp>

#include "Event/State.h"

using namespace boost::lambda;
using namespace Tf::Tsa;

SelectorBase::SelectorBase(const std::string& type,
                           const std::string& name,
                           const IInterface* parent):
  GaudiTool(type, name, parent)
{
  declareProperty("fracUsed", m_fracUsed = 0.35);

  declareInterface<ITsaSeedStep>(this);
};

SelectorBase::~SelectorBase(){
  // destructer
}

StatusCode SelectorBase::select(std::vector<SeedTrack*>& seeds)
{

  //-------------------------------------------------------------------------
  //  Select tracks in 3-D - tracks are sorted by some criteria in the derived
  //  class
  //-------------------------------------------------------------------------

  std::vector<SeedTrack*>::iterator iter = seeds.begin();
  verbose() << "Found " << seeds.size() << " SeedTracks" << endreq;
  for ( ; iter != seeds.end(); ++iter)
  {
    verbose() << " SeedTrack " << **iter << endreq;
    if ( (*iter)->live() == false || (*iter)->select() == true )
    { verbose() << " -> Fails live and select test" << endreq; continue; }

    std::vector<SeedPnt> clusVector = (*iter)->usedPnts();
    const double fracUsed = (double)std::count_if(clusVector.begin(),clusVector.end(),
                                                  bind(&SeedPnt::onTrack,_1))/(double)clusVector.size();
    verbose() << "  -> fracUsed = " << fracUsed << endreq;

    if (fracUsed < m_fracUsed) {
      (*iter)->setSelect(true);
      std::for_each(clusVector.begin(),clusVector.end(), bind(&SeedPnt::setOnTrack,_1,true));
    }
  } //iter

  // loop again and reset used.
  for (iter = seeds.begin() ; iter != seeds.end(); ++iter){
    std::vector<SeedPnt> clusVector = (*iter)->usedPnts();
    if ((*iter)->select()  ==false){
      std::for_each(clusVector.begin(),clusVector.end(), bind(&SeedPnt::setOnTrack,_1, false));
    }
    else {
      std::for_each(clusVector.begin(),clusVector.end(), bind(&SeedPnt::setOnTrack,_1, true));
    }
  }

  return StatusCode::SUCCESS;
}
