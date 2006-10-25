// DECLARATIONS FOR OMT MODULE dialogdisplay
#ifndef _DIALOGDISPLAY_H
#define _DIALOGDISPLAY_H 1

#include "GaudiUPI/AbstractDisplay.h"
#include "CPP/Interactor.h"

class DialogDisplayFactory;
class DialogMainDisplay;
class Event;

/**@class DialogMainDisplay DialogDisplay.h Gaudi/DialogDisplay.h
  *
  *  Class representing application sub-window.
  *
  * @author   M.Frank
  * @version  1.0
  * @date     20/04/2006
  */
class DialogSubDisplay : public AbstractSubDisplay   {
  friend class DialogDisplayFactory;
  friend class DialogMainDisplay;
public:
  /// Default destructor
  virtual ~DialogSubDisplay();
  int map();
  int close();
  int repaint();
  int endUpdate();
  int beginUpdate();
  int putString (const std::string& text,int video);
  int init (int num_col,int num_row, AbstractMainDisplay& parent);
protected:
  DialogSubDisplay (AbstractMainDisplay& source);
  DialogSubMenu  *window() {     return m_window;   }
  DisplayBoard& buffer(size_t i) { 
    return i < height() ? *m_board[i] : *m_board[0]; 
  }
  DisplayLabel& Overflow() {    return m_overflow;  }
protected:
  DisplayLabel   m_overflow;
  DisplayBoard** m_board;
  DialogSubMenu* m_window;
};

/**@class DialogMainDisplay DialogDisplay.h Gaudi/DialogDisplay.h
  *
  *  Class representing main application window.
  *
  * @author   M.Frank
  * @version  1.0
  * @date     20/04/2006
  */
class DialogMainDisplay : public AbstractMainDisplay, public Interactor   {
  friend class DialogDisplayFactory;
  friend class DialogSubDisplay;
public:
  /// Default destructor
  virtual ~DialogMainDisplay();
  int print (const std::string& printer);
  int endUpdate();
  DialogMainDisplay (DialogMenu *parent);
protected:
  DialogMainDisplay (const AbstractMainDisplay& source);
  void handle (Event& ev);
  DialogMenu* window() {     return m_window;   }
protected:
  DialogItem *m_par;
  DialogMenu *m_window;
};
#endif // _DIALOGDISPLAY_H
