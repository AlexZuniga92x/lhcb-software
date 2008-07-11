// $Id: FarmDisplay.h,v 1.6 2008-07-11 16:37:38 frankb Exp $
//====================================================================
//  ROMon
//--------------------------------------------------------------------
//
//  Package    : ROMon
//
//  Description: Base class for displays for readout monitoring 
//               in the LHCb experiment
//
//  Author     : M.Frank
//  Created    : 29/1/2008
//
//====================================================================
// $Header: /afs/cern.ch/project/cvs/reps/lhcb/Online/ROMon/ROMon/FarmDisplay.h,v 1.6 2008-07-11 16:37:38 frankb Exp $
#ifndef ROMON_FARMDISPLAY_H
#define ROMON_FARMDISPLAY_H 1

// Framework includes
#include "ROMon/ROMonDisplay.h"
#include "CPP/Interactor.h"

// C++ include files
#include <map>
#include <ctime>

class Interactor;
namespace SCR {
  struct Display;
  struct Pasteboard;
}
/*
 *   ROMon namespace declaration
 */
namespace ROMon {

  class ProcessDisplay;
  class RecSubfarmDisplay;
  class SubfarmDisplay;
  class ROMonDisplay;
  class FarmDisplay;
  class ProcFarm;
  class CPUfarm;

  enum { 
    CMD_ADD = 100,
    CMD_CONNECT,
    CMD_CHECK,
    CMD_DELETE,
    CMD_UPDATE,
    CMD_SETCURSOR,
    CMD_POSCURSOR,
    CMD_SHOW,
    CMD_SHOWSUBFARM,
    CMD_LAST
  };


  /**@class InternalDisplay ROMon.h GaudiOnline/FarmDisplay.h
   *
   *   Display base class handling standard actions.
   *
   *   @author M.Frank
   */
  class InternalDisplay   {
  protected:
    SCR::Pasteboard*  m_pasteboard;
    SCR::Display*     m_display;
    FarmDisplay*      m_parent;
    std::string       m_name;
    std::string       m_title;
    int               m_svc;
    time_t            m_lastUpdate;
  public:
    /// Access to pasteboard
    SCR::Pasteboard* pasteboard() const { return m_pasteboard; }
    /// Access to SCR display structure
    SCR::Display* display() const       { return m_display;    }
    /// Access to parent to send interrupts
    FarmDisplay* parent() const         { return m_parent;     }
    /// Access subfarm name
    const std::string& name() const     { return m_name;       }
    /// Access title string
    const std::string& title() const    { return m_title;      }
    /// Access to last update time
    time_t lastUpdate() const           { return m_lastUpdate; }
    /// Initializing constructor
    InternalDisplay(FarmDisplay* parent, const std::string& title);
    /// Standard destructor
    virtual ~InternalDisplay();
    /// Draw bar to show occupancies
    size_t draw_bar(int x, int y, float f1, int scale);
    /// Show the display on the main panel
    void show(int row, int col);
    /// Hide the display from the panel
    void hide();
    /// Close the display
    void close();
    /// Disconnect from services: Only destructor may be called afterwards
    void disconnect();
    /// Update display content
    virtual void update(const void* data) = 0;
    /// Check display for errors
    virtual void check(time_t /* stamp */) {}
    /// Set the focus to this display
    virtual void setFocus() {}
    /// Release the focus of this display
    virtual void releaseFocus() {}
    /// DIM command service callback
    static void dataHandler(void* tag, void* address, int* size);
  };

  /**@class HelpDisplay ROMon.h GaudiOnline/FarmDisplay.h
   *
   *   Help display. Dislay shows up on CTRL-h
   *   Help content is in $ROMONROOT/doc/farmMon.hlp
   *
   *   @author M.Frank
   */
  class HelpDisplay : public InternalDisplay {
  public:
    /// Initializing constructor
    HelpDisplay(FarmDisplay* parent, const std::string& title, const std::string& tag);
    /// Standard destructor
    virtual ~HelpDisplay() {}
    /// Update display content
    virtual void update(const void*) {}
  };

  /**@class FarmSubDisplay ROMon.h GaudiOnline/FarmDisplay.h
   *
   *   Display summarizing one single subfarm. Showed as an array on the
   *   main display.
   *
   *   @author M.Frank
   */
  class FarmSubDisplay : public InternalDisplay, public Interactor  {
    int               m_evtBuilt;
    int               m_evtMoore;
    int               m_evtSent;
    int               m_totBuilt;
    int               m_totMoore;
    int               m_totSent;
    bool              m_hasProblems;
  public:
    /// Access to problem flag
    bool hasProblems() const { return m_hasProblems; }
    /// Set timeout error
    void setTimeoutError();
    /// Initializing constructor
    FarmSubDisplay(FarmDisplay* parent, const std::string& title, bool bad=false);
    /// Standard destructor
    virtual ~FarmSubDisplay();
    /// Initialize default display text
    void init(bool bad);
    /// Update display content
    virtual void update(const void* data);
    /// Check display for errors
    virtual void check(time_t stamp);
    /// Set the focus to this display
    virtual void setFocus();
    /// Release the focus of this display
    virtual void releaseFocus();
    /// Interactor overload: Display callback handler
    virtual void handle(const Event& ev);
    /// Update display content
    virtual void updateContent(const Nodeset& ns);
  };

  /**@class RecFarmSubDisplay ROMon.h GaudiOnline/FarmDisplay.h
   *
   *   Display summarizing one single subfarm. Showed as an array on the
   *   main display.
   *
   *   @author M.Frank
   */
  class RecFarmSubDisplay : public InternalDisplay, public Interactor  {
    int               m_evtRecv;
    int               m_evtReco;
    int               m_evtSent;
    int               m_totRecv;
    int               m_totReco;
    int               m_totSent;
    bool              m_hasProblems;
  public:
    /// Access to problem flag
    bool hasProblems() const { return m_hasProblems; }
    /// Set timeout error
    void setTimeoutError();
    /// Initializing constructor
    RecFarmSubDisplay(FarmDisplay* parent, const std::string& title, bool bad=false);
    /// Standard destructor
    virtual ~RecFarmSubDisplay();
    /// Initialize default display text
    void init(bool bad);
    /// Update display content
    virtual void update(const void* data);
    /// Check display for errors
    virtual void check(time_t stamp);
    /// Set the focus to this display
    virtual void setFocus();
    /// Release the focus of this display
    virtual void releaseFocus();
    /// Interactor overload: Display callback handler
    virtual void handle(const Event& ev);
    /// Update display content
    virtual void updateContent(const Nodeset& ns);
  };

  /**@class ProcessDisplay ROMon.h GaudiOnline/FarmDisplay.h
   *
   *   Display showing all processes on a given node.
   *
   *   @author M.Frank
   */
  class ProcessDisplay : public InternalDisplay {
  public:
    /// Initializing constructor
    ProcessDisplay(FarmDisplay* parent, const std::string& title, int height=55,int width=120);
    /// Standard destructor
    virtual ~ProcessDisplay();
    /// Update display content
    virtual void update(const void* data);
    /// Update display content
    void updateContent(const ProcFarm& pf);
  };

  /**@class CPUDisplay ROMon.h GaudiOnline/FarmDisplay.h
   *
   *   Display showing all processes on a given node.
   *
   *   @author M.Frank
   */
  class CPUDisplay : public InternalDisplay {
  public:
    /// Initializing constructor
    CPUDisplay(FarmDisplay* parent, const std::string& title, int height=55,int width=120);
    /// Standard destructor
    virtual ~CPUDisplay();
    /// Update display content
    virtual void update(const void* data);
    /// Update display content
    void updateContent(const CPUfarm& pf);
  };

  /**@class BufferDisplay ROMon.h GaudiOnline/FarmDisplay.h
   *
   *   Internal MBM monitor display, when spying on individual nodes.
   *
   *   @author M.Frank
   */
  class BufferDisplay : public InternalDisplay {
  protected:
    /// Node number in node set
    int m_node;
  public:
    /// Initializing constructor
    BufferDisplay(FarmDisplay* parent, const std::string& title);
    /// Standard destructor
    virtual ~BufferDisplay() {}
    /// Set the node number for the display
    void setNode(int which) { m_node = which; }
    /// Update display content
    virtual void update(const void* data);
  };

  /**@class PartitionListener ROMon.h GaudiOnline/FarmDisplay.h
   *
   *   Listen to RunInfo datapoints published by DIM
   *
   *   @author M.Frank
   */
  class PartitionListener {
  protected:
    Interactor* m_parent;
    std::string m_name;
    int         m_subFarmDP;
  public:
    /// Standard constructor with object setup through parameters
    PartitionListener(Interactor* parent,const std::string& name);
    /// Standard destructor
    virtual ~PartitionListener();
    /// DIM command service callback
    static void subFarmHandler(void* tag, void* address, int* size);
  };

  /**@class FarmDisplay ROMon.h GaudiOnline/FarmDisplay.h
   *
   *   Monitoring display for the LHCb storage system.
   *
   *   @author M.Frank
   */
  class FarmDisplay : public InternalDisplay, public Interactor  {
  protected:
    enum { HLT_MODE, RECO_MODE };
    typedef std::map<std::string, InternalDisplay*> SubDisplays;
    typedef std::vector<std::string> Farms;
    SubDisplays                      m_farmDisplays;
    std::auto_ptr<PartitionListener> m_listener;
    ROMonDisplay*                    m_subfarmDisplay;
    std::auto_ptr<ProcessDisplay>    m_procDisplay;
    std::auto_ptr<BufferDisplay>     m_mbmDisplay;
    std::auto_ptr<HelpDisplay>       m_helpDisplay;
    std::auto_ptr<CPUDisplay>        m_cpuDisplay;

    std::string        m_partition;
    std::string        m_match;
    /// vector with all farm displays
    Farms              m_farms;
    int                m_height;
    int                m_width;
    int                m_dense;
    /// Cursor position in sub display array
    size_t             m_posCursor;
    size_t             m_subPosCursor;
    int                m_anchorX, m_anchorY;
    int                m_mode;
    /// Keyboard rearm action
    static int key_rearm (unsigned int fac, void* param);
    /// Keyboard action
    static int key_action(unsigned int fac, void* param);

public:
    /// Standard constructor
    FarmDisplay(int argc, char** argv);

    /// Standard destructor
    virtual ~FarmDisplay();

    /// Show subfarm display
    int showSubfarm();

    /// Handle keyboard interrupts
    int handleKeyboard(int key);

    /// Get farm display from cursor position
    InternalDisplay* currentDisplay();

    SubDisplays& subDisplays() {  return m_farmDisplays; }

    /// Set cursor to position
    void set_cursor();

    /// Interactor overload: Display callback handler
    virtual void handle(const Event& ev);

    /// Connect to data sources
    void connect(const std::vector<std::string>& farms);

    /// DIM command service callback
    virtual void update(const void* data);
  };
}      // End namespace ROMon
#endif /* ROMON_FARMDISPLAY_H */

