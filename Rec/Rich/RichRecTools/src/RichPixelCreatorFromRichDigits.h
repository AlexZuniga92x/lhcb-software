// $Id: RichPixelCreatorFromRichDigits.h,v 1.4 2004-02-02 14:27:01 jonesc Exp $
#ifndef RICHRECTOOLS_RICHPIXELCREATORFROMRICHDIGITS_H
#define RICHRECTOOLS_RICHPIXELCREATORFROMRICHDIGITS_H 1

// from Gaudi
#include "GaudiKernel/IIncidentListener.h"
#include "GaudiKernel/IIncidentSvc.h"
#include "GaudiKernel/SmartDataPtr.h"
#include "GaudiKernel/ToolFactory.h"

// base class
#include "RichRecBase/RichRecToolBase.h"

// interfaces
#include "RichRecBase/IRichPixelCreator.h"
#include "RichDetTools/IRichSmartIDTool.h"

// Event
#include "Event/RichDigit.h"

/** @class RichPixelCreatorFromRichDigits RichPixelCreatorFromRichDigits.h
 *
 *  Tool for the creation and book-keeping of RichRecPixel objects.
 *  Uses RichDigits from the OO digitisation as the parent objects.
 *
 *  @author Chris Jones   Christopher.Rob.Jones@cern.ch
 *  @date   15/03/2002
 */

class RichPixelCreatorFromRichDigits : public RichRecToolBase,
                                       virtual public IRichPixelCreator,
                                       virtual public IIncidentListener {

public:

  /// Standard constructor
  RichPixelCreatorFromRichDigits( const std::string& type,
                                  const std::string& name,
                                  const IInterface* parent );

  /// Destructor
  virtual ~RichPixelCreatorFromRichDigits(){}

  /// Initialize method
  StatusCode initialize();

  /// Finalize method
  StatusCode finalize();

  /// Implement the handle method for the Incident service.
  /// This is used to inform the tool of software incidents.
  void handle( const Incident& incident );
  
  /// Returns a RichRecPixel object pointer for given ContainedObject.
  /// If if it not possible NULL is return.
  RichRecPixel * newPixel( const ContainedObject * obj ) const;
  
  /// Form all possible RichRecPixels from input RichDigits.
  /// The most efficient way to make all RichRecPixel objects in the event.
  StatusCode newPixels() const;
  
  /// Returns a pointer to the RichRecPixels
  RichRecPixels * richPixels() const;

private:

  /// Pointer to RichRecPixels
  RichRecPixels * m_pixels;

  /// Pointer to RichSmartID tool
  IRichSmartIDTool * m_smartIDTool;

  /// String containing input RichDigits location in TES
  std::string m_recoDigitsLocation;

  /// Location of RichRecPixels in TES
  std::string m_richRecPixelLocation;

  /// Flag to signify all pixels have been formed
  mutable bool m_allDone;

  /// Pointer to pixel map
  mutable std::map< RichSmartID::KeyType, RichRecPixel* > m_pixelExists;
  mutable std::map< RichSmartID::KeyType, bool > m_pixelDone;

};

#endif // RICHRECTOOLS_RICHPIXELCREATORFROMRICHDIGITS_H
