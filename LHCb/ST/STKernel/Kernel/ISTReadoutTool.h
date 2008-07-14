// $Id: ISTReadoutTool.h,v 1.5 2008-07-14 07:32:30 mneedham Exp $
#ifndef _ISTReadoutTool_H
#define _ISTReadoutTool_H

#include "GaudiKernel/IAlgTool.h"

#include <string>
#include <vector>
#include "Kernel/STDAQDefinitions.h"
#include "Kernel/STTell1ID.h"
#include "Event/STCluster.h"

class STTell1Board;
class DeSTSector;

namespace LHCb{
 class STChannelID;
};

/** @class ISTReadoutTool ISTReadoutTool.h  STDAQ/ISTReadoutTool
*
*  Interface Class for things related to the Readout - Tell1 Board
*
*  @author M.Needham
*  @date   13/3/2002
*/

// Declaration of the interface ID ( interface id, major version, minor version)

static const InterfaceID IID_ISTReadoutTool("ISTReadoutTool", 0 , 0);


class ISTReadoutTool: virtual public IAlgTool{

public:

  /// Static access to interface id
  static const InterfaceID& interfaceID() { return IID_ISTReadoutTool; }

  /// number of boards
  virtual unsigned int nBoard() const = 0;


  ///return vector of Tell1IDs
  virtual std::vector<STTell1ID> boardIDs() const = 0;

  /// convert STChannelID to DAQ ChannelID
  virtual STDAQ::chanPair offlineChanToDAQ(const LHCb::STChannelID aOfflineChan,
                                           double isf) const = 0 ;

  /// convert offline interStripFraction to DAQ interStripFraction
  virtual double interStripToDAQ(const LHCb::STChannelID aOfflineChan, 
                                 const STTell1ID aBoardID,
                                 const double isf) const = 0 ;

  virtual bool ADCOfflineToDAQ(const LHCb::STChannelID aOfflineChan,
			       const STTell1ID aBoardID,
			       LHCb::STCluster::ADCVector& adcs) const = 0;
  
  /// find the Tell1 board given a board ID
  virtual STTell1Board* findByBoardID(const STTell1ID aBoardID) const = 0;

  /// find a Tell board by storage order
  virtual STTell1Board* findByOrder(const unsigned int aValue) const = 0;

  /// get region
  virtual unsigned int region(const LHCb::STChannelID aChan) const = 0;

  /// printout
  virtual void printMapping() const = 0;

  /// list of the readout sector ids on the board in a board
  virtual std::vector<LHCb::STChannelID> sectorIDs(const STTell1ID board) const = 0;
  
  /// list of the readout sectors in a board
  virtual std::vector<DeSTSector*> sectors(const STTell1ID board) const = 0;
  
};


#endif // _ISTReadoutTool_H
