// $Id: DeVeloPixSquareType.cpp,v 1.5 2009-10-21 15:04:56 cocov Exp $
//==============================================================================
#define VELOPIXDET_DEVELOPIXSQUARETYPE_CPP 1
//==============================================================================
// Include files 

// From Gaudi
#include "GaudiKernel/Bootstrap.h"
#include "GaudiKernel/PropertyMgr.h"
#include "GaudiKernel/IJobOptionsSvc.h"
#include "GaudiKernel/ISvcLocator.h"
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/PhysicalConstants.h"
#include "GaudiKernel/IUpdateManagerSvc.h"

// From LHCb
#include "LHCbMath/LHCbMath.h"
#include "Kernel/CircleTraj.h"
#include "gsl/gsl_math.h"

// From VeloPix
#include "VeloPixDet/DeVeloPixSquareType.h"
#include "Kernel/VeloPixChannelID.h"


// used to control initialization
bool DeVeloPixSquareType::m_staticDataInvalid = true;

/** @file DeVeloPixSquareType.cpp
 *
 *  Implementation of class : DeVeloPixSquareType
 *
 *  @author Victor Coco victor.coco@cern.ch
 */

//==============================================================================
/// Standard constructor
//==============================================================================
DeVeloPixSquareType::DeVeloPixSquareType(const std::string& name) : 
  DeVeloPixSensor(name),
  m_ladders(),
  m_otherSideSensor(0)
{
}
//==============================================================================
/// Destructor
//==============================================================================
DeVeloPixSquareType::~DeVeloPixSquareType()
{
}
//==============================================================================
/// Object identification
//==============================================================================
const CLID& DeVeloPixSquareType::clID() 
  const { return DeVeloPixSquareType::classID(); }
//==============================================================================
/// Initialisation method
//==============================================================================
StatusCode DeVeloPixSquareType::initialize() 
{
   // Trick from old DeVelo to set the output level
  PropertyMgr* pmgr = new PropertyMgr();
  int outputLevel=0;
  pmgr->declareProperty("OutputLevel", outputLevel);
  IJobOptionsSvc* jobSvc;
  ISvcLocator* svcLoc = Gaudi::svcLocator();
  StatusCode sc = svcLoc->service("JobOptionsSvc", jobSvc);
  if( sc.isSuccess() ) sc = jobSvc->setMyProperties("DeVeloPixSquareType", pmgr);
  if( !sc ) return sc;
  if ( 0 < outputLevel ) {
    msgSvc()->setOutputLevel("DeVeloPixSquareType", outputLevel);
  }
  delete pmgr;
  MsgStream msg(msgSvc(), "DeVeloPixSquareType");
  sc = DeVeloPixSensor::initialize();
  if(!sc.isSuccess()) {
    msg << MSG::ERROR << "Failed to initialise DeVeloPixSensor" << endreq;
    return sc;
  }
  m_debug   = (msgSvc()->outputLevel("DeVeloPixSquareType") == MSG::DEBUG  ) ;
  m_verbose = (msgSvc()->outputLevel("DeVeloPixSquareType") == MSG::VERBOSE) ;

  // Fill the ladder vector with parameters from the DDDB
  for (int nl = 0 ; nl < ladderNumber() ; nl ++)
  {
    std::stringstream s;
    s << nl+1;
    //bottom left corner of the active part of ladder nl
    Gaudi::XYZPoint point(param<double>("XRef"+s.str()),param<double>("YRef"+s.str()),param<double>("ZRef"+s.str()));
    PixelLadder tmp_ladder(point,param<int>("NChip"+s.str()));
    std::vector<int> tmp_interPixPos (tmp_ladder.nChip());
    for ( int nchip = 0 ; nchip < tmp_ladder.nChip() ; nchip++)
    {
      std::stringstream sch;
      sch << nchip+1;
      tmp_interPixPos[nchip]=param<int>("PosInterChip"+sch.str()+"_Ladd"+s.str());    
    }
    tmp_ladder.setEdgesOrientation(tmp_interPixPos);
    m_ladders.push_back(tmp_ladder);
  }
  return StatusCode::SUCCESS; 
}



//==============================================================================
/// Calculate the nearest channel to a 3-d point 
/// (assuming the DeVeloPixSquareType instance is already the correct one: z is corresponding to the sensor number)
//==============================================================================
StatusCode DeVeloPixSquareType::pointToChannel(const Gaudi::XYZPoint& point,
                                       LHCb::VeloPixChannelID& channel,
                                       std::pair <double, double>& fraction) const
{
  MsgStream msg(msgSvc(), "DeVeloPixSquareType");
  Gaudi::XYZPoint localPoint = globalToLocal(point);

  // Check that the point is in the active area of the sensor
  StatusCode sc = isInActiveArea(localPoint);
  if(!sc.isSuccess()) return sc;  
  unsigned int sensor=sensorNumber();
  // Create the associated VeloPixChannelID
  channel.setSensor(sensor);
  // Get the ladder number in which the point is
  int ladderIndex = WhichLadder(point);
  // Get the chip number in which the point is
  int chipIndex = WhichChip(point,ladderIndex);
  // Compute the proper chip number for VeloPixChanelID
  int prop_chipIndex = chipIndex;
  for(int ilad = 0 ; ilad < ladderIndex ; ilad ++){
    prop_chipIndex += m_ladders[ilad].nChip();
  }
  // Set the chip number in the VeloPixChannelID
  channel.setChip(prop_chipIndex);  
  // Get the pixel position in which the point is
  std::pair <int,int> pixelPos = WhichPixel(point,ladderIndex,chipIndex,fraction);
  if( pixelPos.first < 0. || pixelPos.second < 0. ){
    return StatusCode::FAILURE;
  }
  // Set the pixel position in the VeloPixChannelID
  channel.setPixel_lp(pixelPos.first);
  channel.setPixel_hp(pixelPos.second);
  return StatusCode::SUCCESS;
}

//==============================================================================
/// Calculate the center of the pixel from a given channel
//==============================================================================
StatusCode DeVeloPixSquareType::channelToPoint( const LHCb::VeloPixChannelID& channel,
                                                Gaudi::XYZPoint& point) const
{
  MsgStream msg(msgSvc(), "DeVeloPixSquareType");
  Gaudi::XYZPoint LocalPoint(0.,0.,0.);
  int chipNum = channel.chip();  
  int ladderIndex = 0;
  int ntotChip = 0;
  int chipInLadd = 0;
  for(int ilad = 0 ; ilad < ladderIndex ; ilad ++){
    ntotChip += m_ladders[ilad].nChip();
    if ( chipNum < ntotChip ) {
      // Get the ladder
      ladderIndex = ilad;
      // Set the position in the pixel
      std::pair <double, double> size = PixelSize(channel);
      if ( channel.pixel_lp()> 0 ) LocalPoint.SetX((channel.pixel_lp()-1)*lpSize()+size.first/2);
      else LocalPoint.SetX(size.first/2);
      if ( channel.pixel_hp()> 0 ) LocalPoint.SetY((channel.pixel_hp()-1)*hpSize()+size.second/2);
      else LocalPoint.SetX(size.second/2);
      // chipInLadd start at one...
      chipInLadd = chipNum-(ntotChip-m_ladders[ilad].nChip());
      continue;
    }
  }
  // Add the bottom left position of the chip in the ladder
  LocalPoint.SetX((chipInLadd-1)*(chipLength()+interChipDist())+LocalPoint.x());
  // Add the postition of the reference LocalPoint of the ladder
  LocalPoint.SetX(m_ladders[ladderIndex].ReferencePoint().x()+LocalPoint.x());
  LocalPoint.SetY(m_ladders[ladderIndex].ReferencePoint().y()+LocalPoint.y());
  // BECAREFULL, Z is set to ReferencePoint().z()  but it is not so sure.. to be checked
  LocalPoint.SetZ(m_ladders[ladderIndex].ReferencePoint().z());
  point = localToGlobal(LocalPoint);
  return StatusCode::SUCCESS;
}

//==============================================================================
/// Calculate the nearest 3x3 list of channel to a 3-d point 
/// (assuming the DeVeloPixSquareType instance is already the correct one: z is corresponding to the sensor number)
//==============================================================================
StatusCode  DeVeloPixSquareType::pointTo3x3Channels(const Gaudi::XYZPoint& point,
                                       std::vector <LHCb::VeloPixChannelID>& channels) const
{
  MsgStream msg(msgSvc(), "DeVeloPixSquareType");
  // Get the channel 
  LHCb::VeloPixChannelID  channelCentral;
  std::pair <double, double> fraction;
  StatusCode sc = pointToChannel( point, channelCentral, fraction);
  for (int x = -1 ; x < 2 ; x ++){
    for (int y = -1 ; y < 2 ; y ++){
      std::pair <double, double> size (0.,0.);
      if( x == 0 && y == 0 && sc.isSuccess() ) {
        channels.push_back(channelCentral);
        size = PixelSize(channelCentral);
      }
      else continue;
      double relx = 0;
      double rely = 0;
      if (x < 0) relx = - fraction.first*size.first - lpSize()/2;
      if (x > 0) relx = (1- fraction.first)*size.first + lpSize()/2;
      if (y < 0) rely = - fraction.second*size.second - hpSize()/2;
      if (y < 0) rely = (1- fraction.second)*size.second + hpSize()/2;
      if (!sc.isSuccess()){
        relx = x*lpSize();
        rely = y*hpSize();
      }
      Gaudi::XYZPoint neigh_point (point.x()+relx,point.y()+rely,point.z());       
      LHCb::VeloPixChannelID neig_channel;
      std::pair <double,double> frac (0.,0.);
      if ( pointToChannel( neigh_point, neig_channel, frac).isSuccess()){
        channels.push_back(neig_channel);
      }
    }
  }
  return StatusCode::SUCCESS;
}


//==============================================================================
/// Checks if local point is inside sensor                      
//==============================================================================
StatusCode DeVeloPixSquareType::isInActiveArea(const Gaudi::XYZPoint& point) const
{
  // Check the the point is in the active part of a ladder
  if(WhichLadder(point)>-0.5)return StatusCode::SUCCESS;
  return StatusCode::FAILURE;
}


//==============================================================================
/// Get the index of the ladder containing the point
//==============================================================================
int DeVeloPixSquareType::WhichLadder(const Gaudi::XYZPoint& point) const
{
  for ( int index = 0 ; index <(int) m_ladders.size() ; index++){
    const Gaudi::XYZPoint pointRef = m_ladders[index].ReferencePoint();
    if (
        (point.x()-pointRef.x() > 0. && point.x()-pointRef.x()<m_ladders[index].nChip()*(chipLength()+interChipDist())) 
        && (point.y()-pointRef.y() > 0. && point.y()-pointRef.y()<chipWidth())
        && (fabs(point.z()-z()-pointRef.z())<6*siliconThickness()/10)
        ) return index;
  }
  return -1;
}

//==============================================================================
/// Get the index relative to ladder of the chip containing the point
//==============================================================================
int DeVeloPixSquareType::WhichChip(const Gaudi::XYZPoint& point, int ladderIndex) const
{  
  Gaudi::XYZPoint refPoint = m_ladders[ladderIndex].ReferencePoint();
  Gaudi::XYZPoint LocalPoint(point.x()-refPoint.x()-interChipDist()/2,point.y()-refPoint.y(),point.z()-refPoint.z());
  int ChipNum = (int)(LocalPoint.x()/(chipLength()+interChipDist()));
  // treat the edge configuration (long pixel in the interchip area)
  
  return ChipNum;

}

//==============================================================================
/// Get the index relative to the chip of the pixel containing the point 
//==============================================================================
std::pair<int,int> DeVeloPixSquareType::WhichPixel(const Gaudi::XYZPoint& point, int ladderIndex, int chipIndex, std::pair<double,double>& fraction) const
{
  double alongAxisOffset = 0.;
  if( chipIndex>0)  alongAxisOffset = chipIndex*(chipLength()+interChipDist()/2)+(chipIndex-1)*interChipDist()/2;

  Gaudi::XYZPoint refPoint = m_ladders[ladderIndex].ReferencePoint();
  Gaudi::XYZPoint LocalPoint(point.x()-refPoint.x()-alongAxisOffset,point.y()-refPoint.y(),point.z()-refPoint.z());

  std::vector<int> positionEdgePix =  (m_ladders[ladderIndex]).edgesOrientation();
  std::pair< int , int > thePixel;

  // Set the size of the edges (left/right) pixels
  double interchipPixSizeLEFT = lpSize();
  double interchipPixSizeRIGHT = lpSize();
  if ( positionEdgePix[chipIndex]== 1 || positionEdgePix[chipIndex]== 0 )interchipPixSizeRIGHT = interchipPixSize();
  if ( positionEdgePix[chipIndex]== -1 || positionEdgePix[chipIndex]== 0 )interchipPixSizeLEFT = interchipPixSize();
  
  // correct for the extra length of the left pixel
  double newx = LocalPoint.x() - (interchipPixSizeLEFT-lpSize());
  // set the results for most of the case
  thePixel.first = (int)(newx/lpSize());
  fraction.first = newx/lpSize() - (int)(newx/lpSize());
  //modify in the case where it is on the left edge
  if (newx < lpSize()  && newx > - (interchipPixSizeLEFT-lpSize())){
    thePixel.first=0; // in case it would have been negative
      fraction.first = LocalPoint.x()/interchipPixSizeLEFT;
  }
  else if (newx <((nPixCol()-1)*lpSize())+interchipPixSizeRIGHT && newx >((nPixCol()-2)*lpSize())){
    fraction.first = (newx-(nPixCol()-1)*lpSize())/interchipPixSizeRIGHT;
  }
  else{
    thePixel.first = -1;
    fraction.first = -1; 
  }
  thePixel.second=(int)(LocalPoint.y()/hpSize());  
  fraction.second=LocalPoint.y()/hpSize()-thePixel.second;  
  return thePixel;
}

std::pair<double,double> DeVeloPixSquareType::PixelSize(LHCb::VeloPixChannelID channel) const
{
  int ladderIndex = -1;
  int chipNum = channel.chip();
  int ntotChip = 0;
  for(int ilad = 0 ; ilad < ladderIndex ; ilad ++){
    ntotChip += m_ladders[ilad].nChip();
    if ( chipNum < ntotChip ) {
      ladderIndex = ilad;
      continue;
    }
  }
  std::pair<double,double> size (-1.,-1.);
  size.first = lpSize();
  size.second = hpSize();
  std::vector<int> positionEdgePix =  (m_ladders[ladderIndex]).edgesOrientation();
  // case where the edge pixel is on the right and the channel correspond to a edge pixel
  if ( ( positionEdgePix[chipNum]== 1 || positionEdgePix[chipNum]== 0 ) && channel.pixel_lp() == (unsigned int)(nPixCol()-1)){
    size.first = interchipPixSize();
  }
  if ( ( positionEdgePix[chipNum]== -1 || positionEdgePix[chipNum]== 0) && channel.pixel_lp() == 0 ){
    size.first = interchipPixSize();
  }
  return size;
  
}
