#ifndef ELOGDIALOG_H_
#define ELOGDIALOG_H_

#ifdef WIN32
#pragma warning( push )
#pragma warning( disable : 4800 )
#endif
#include <TGFrame.h>
#ifdef WIN32
#pragma warning( pop )
#endif

#include "presenter.h"

class PresenterMainFrame;
class TGComboBox;
class TGTextButton;
class TGTextBuffer;
class TGTextEntry;
class TGTextEdit;
class TGedPatternSelect;
class TGLineWidthComboBox;
class TGColorSelect;
class TGLineStyleComboBox;
class TGListBox ;
class TGComboBox ;

class ElogDialog : public TGTransientFrame
{
public:
  ElogDialog(PresenterMainFrame* gui, int width, int height);
  virtual ~ElogDialog();
  
  void setProblemDatabase( std::string& title );
  
  void setParameters( std::string& logbook, std::string& username, std::string& system, 
                      std::string& subject, std::string& message, std::string& runNumber, 
                      int& isOK );
  
  void ok();
  void build();
  void CloseWindow();

private:
  TGTextButton       *m_okButton;
  TGTextButton       *m_cancelButton;

  std::string* m_logbook;
  std::string* m_username;
  std::string* m_system;
  std::string* m_subject;
  std::string* m_message;
  std::string* m_runNumber;
  int*         m_isOK;
  std::string* m_title;
  int          m_hasProblem;

  TGTextEntry        *m_logbookTextEntry;
  TGTextEntry        *m_usernameTextEntry;
  TGTextEntry        *m_systemTextEntry;
  TGTextEntry        *m_subjectTextEntry;
  TGTextEntry        *m_runNumberTextEntry;
  TGTextEdit         *m_messageTextEntry;
  TGTextEntry        *m_titleTextEntry;

  TGComboBox *m_logbookListBox ;

  ClassDef(ElogDialog, 0);
};
#endif /*ELOGDIALOG_H_*/
