#include <TSystem.h>
#include <TGButton.h>
#include <TGTextEntry.h>
#include <TGTextBuffer.h>
#include <TGLabel.h>
#include <TObject.h>
#include <TGMsgBox.h>
#include <TPad.h>
#include <TCanvas.h>

#include <TH1.h>
#include <TH2.h>

#include "OnlineHistDB/OnlineHistDB.h"
#include "DbRootHist.h"

#include "PresenterMainFrame.h"
#include "EditorFrame.h"
#include "PageSaveDialog.h"

ClassImp(PageSaveDialog)

PageSaveDialog::PageSaveDialog(PresenterMainFrame* gui, Int_t Width, Int_t Height, MsgLevel v) : 
    TGTransientFrame(gClient->GetRoot(), gui, Width, Height),
    m_mainFrame(gui),
    m_verbosity(v),
    m_msgBoxReturnCode(0)
{
  SetCleanup(kDeepCleanup);
  SetMWMHints(kMWMDecorAll, kMWMFuncAll, kMWMInputSystemModal);
  build();  
  CenterOnParent();
  MapWindow();
}

PageSaveDialog::~PageSaveDialog()
{
  Cleanup();
}

void PageSaveDialog::build()
{
  SetLayoutBroken(kTRUE);

  TGLabel *fLabel530 = new TGLabel(this,"Enter or select the parent folder:");
  fLabel530->SetTextJustify(36);
  AddFrame(fLabel530, new TGLayoutHints(kLHintsLeft | kLHintsTop,2,2,2,2));
  fLabel530->MoveResize(16,8,192,18);

  folderNameTextEntry = new TGTextEntry(this, new TGTextBuffer(15),-1,kSunkenFrame | kDoubleBorder | kOwnBackground);
  folderNameTextEntry->SetMaxLength(255);
  folderNameTextEntry->SetAlignment(kTextLeft);
  folderNameTextEntry->Resize(456,folderNameTextEntry->GetDefaultHeight());
  AddFrame(folderNameTextEntry, new TGLayoutHints(kLHintsLeft | kLHintsTop,2,2,2,2));
  folderNameTextEntry->MoveResize(16,32,456,22);
  folderNameTextEntry->Connect("ReturnPressed()",
      "PageSaveDialog", this,
      "setOkButton()");

  // canvas widget
  TGCanvas *fCanvas694 = new TGCanvas(this,456,192);

  // canvas viewport
  TGViewPort *fViewPort695 = fCanvas694->GetViewPort();

  // list tree
  m_pageFolderListTree = new TGListTree(fCanvas694,kHorizontalFrame); //dynamic_cast<TObject*>(
  m_pageFolderListTree->AddRoot("Pages");
  m_mainFrame->readFromHistoDatabase(m_pageFolderListTree,
      ByFolderAndPage, WithoutHistograms);
  m_pageFolderListTree->Connect("Clicked(TGListTreeItem*, Int_t, Int_t, Int_t)",
      "PageSaveDialog", this,
      "updateTextFields(TGListTreeItem*, Int_t, Int_t, Int_t)");

  fViewPort695->AddFrame(m_pageFolderListTree);
  m_pageFolderListTree->SetLayoutManager(new TGHorizontalLayout(m_pageFolderListTree));
  m_pageFolderListTree->MapSubwindows();
  fCanvas694->SetContainer(m_pageFolderListTree);
  fCanvas694->MapSubwindows();
  AddFrame(fCanvas694, new TGLayoutHints(kLHintsLeft | kLHintsTop,2,2,2,2));
  fCanvas694->MoveResize(16,56,456,192);

  TGLabel *fLabel717 = new TGLabel(this,"Page name:");
  fLabel717->SetTextJustify(36);
  AddFrame(fLabel717, new TGLayoutHints(kLHintsLeft | kLHintsTop,2,2,2,2));
  fLabel717->MoveResize(16,256,72,18);

  pageNameTextEntry = new TGTextEntry(this, new TGTextBuffer(15),-1,kSunkenFrame | kDoubleBorder | kOwnBackground);
  pageNameTextEntry->SetMaxLength(255);
  pageNameTextEntry->SetAlignment(kTextLeft);
  pageNameTextEntry->Resize(376,pageNameTextEntry->GetDefaultHeight());
  AddFrame(pageNameTextEntry, new TGLayoutHints(kLHintsLeft | kLHintsTop,2,2,2,2));
  pageNameTextEntry->MoveResize(96,256,376,22);
  pageNameTextEntry->Connect("ReturnPressed()",
      "PageSaveDialog", this,
      "setOkButton()");


  m_okButton = new TGTextButton(this,"OK");
  m_okButton->SetTextJustify(36);
  m_okButton->Resize(80,24);
  m_okButton->SetState(kButtonDisabled);
  AddFrame(m_okButton, new TGLayoutHints(kLHintsLeft | kLHintsTop,2,2,2,2));
  m_okButton->MoveResize(280,288,80,24);
  m_okButton->Connect("Clicked()", "PageSaveDialog", this, "ok()");

  TGTextButton *fTextButton755 = new TGTextButton(this,"Cancel");
  fTextButton755->SetTextJustify(36);
  fTextButton755->Resize(80,24);
  AddFrame(fTextButton755, new TGLayoutHints(kLHintsLeft | kLHintsTop,2,2,2,2));
  fTextButton755->MoveResize(376,288,80,24);
  fTextButton755->Connect("Clicked()", "PageSaveDialog", this, "DeleteWindow()");

  MapSubwindows();
  Resize(GetDefaultSize());
  MapWindow();
  Resize(493,339);
}

void PageSaveDialog::ok()
{
  m_okButton->SetEnabled(false);

  if (m_mainFrame->isConnectedToDatabase()) {

    OnlineHistDB* m_histogramDB = m_mainFrame->getDatabase();
    std::string folderName = folderNameTextEntry->GetText();
    std::string pageName = pageNameTextEntry->GetText();

    try {
      OnlineHistPage* page = m_histogramDB->getPage(folderName+"/"+pageName);
      if (page) { page->removeAllHistograms(); //m_histogramDB->removePage(page); GG do not remove page! }


      // Add histos to Page in HistoDB
      TIter nextCanvasElem(m_mainFrame->editorFrame->editorCanvas->GetListOfPrimitives());
      TObject *padCandidate;
      while (padCandidate = nextCanvasElem()) {
        if (padCandidate->InheritsFrom(TPad::Class())) {
          double xlow, ylow, xup, yup;  
          TPad* pad = dynamic_cast<TPad*>(padCandidate);
          pad->GetPadPar(xlow, ylow, xup, yup);            
          TIter nextPadElem(pad->GetListOfPrimitives());
          TObject *histoCandidate;
          while (histoCandidate = nextPadElem()) {
            if (histoCandidate->InheritsFrom(TH1::Class())) {
              m_DbHistosOnPageIt = m_mainFrame->editorFrame->m_DbHistosOnPage.begin();
              while (m_DbHistosOnPageIt != m_mainFrame->editorFrame->m_DbHistosOnPage.end()) {
                if (histoCandidate && *m_DbHistosOnPageIt && (*m_DbHistosOnPageIt)->rootHistogram &&
                    0 == std::string((*m_DbHistosOnPageIt)->rootHistogram->GetName()).compare(histoCandidate->GetName())) {
                  break;
                }
                m_DbHistosOnPageIt++;
              }                

              if ((*m_DbHistosOnPageIt)) {
                OnlineHistogram* newh = page->addHistogram( (*m_DbHistosOnPageIt)->dbHist(), 
                    xlow , ylow, xup, yup);
                if(newh) {(*m_DbHistosOnPageIt)->setdbHist(newh);} 
              }
            }
          } 
        }
      }
      if (page->save()) {
        // now save current ROOT display options of histograms on page
        m_DbHistosOnPageIt = m_mainFrame->editorFrame->m_DbHistosOnPage.begin();
        while (m_DbHistosOnPageIt != m_mainFrame->editorFrame->m_DbHistosOnPage.end()) {
          if( (*m_DbHistosOnPageIt)->dbHist()->page() == page->name())
            (*m_DbHistosOnPageIt)->saveTH1ToDB();
          m_DbHistosOnPageIt++;
        }
      }

      m_histogramDB->commit();
    }
  } catch (SQLException sqlException) {
    // TODO: add error logging backend - MsgStream?
    std::string m_message = sqlException.getMessage();
    m_mainFrame->setStatusBarText(m_message.c_str(), 0);
    if (m_verbosity >= Verbose) { std::cout << m_message << std::endl; }

    new TGMsgBox(fClient->GetRoot(), this, "Database Error",
        Form("Could save the page to OnlineHistDB:\n\n%s\n",
          m_message.c_str()), kMBIconExclamation,
        kMBOk, &m_msgBoxReturnCode);
  }    
  DeleteWindow();
} else {
  m_okButton->SetEnabled(false);
}
}

void PageSaveDialog::updateTextFields(TGListTreeItem* node, Int_t, Int_t, Int_t)
{
  if (node && node->GetText() &&
      m_pageFolderListTree->GetFirstItem() != node) {

    // TODO: regexp replace

    //  TPRegexp dimSvcNameWithUTGIDRegexp("^.{3}/[^/]*_([^/]+)_[^/]*(/.+$)");
    //  TString dimSvcNameTS(dimSvcName);
    //  if (dimSvcNameWithUTGIDRegexp.MatchB(dimSvcName)) {
    //    std::cout << "dimSvcNameWithUTGIDRegexp" << std::endl;
    //    dimSvcNameWithUTGIDRegexp.Substitute(dimSvcNameTS,"$1$2");
    //    std::cout << dimSvcNameTS << std::endl;
    //    return std::string(dimSvcNameTS);
    //  }  

    char path[1024];
    m_pageFolderListTree->GetPathnameFromItem(node, path);

    //    std::cout << "pagesave fields orig path: \t" << path << std::endl;


    // Drop leading slash 
    std::string dbPath = std::string(path).erase(0, std::string(path).find_first_of("/")+1).c_str();
    //    std::cout << "pagesave fields orig dbPath: \t" << dbPath << std::endl;

    // fill in pagename if selected...
    if (node->IsChecked()) {
      pageNameTextEntry->SetText(dbPath.substr(dbPath.find_last_of("/")+1).c_str());
      dbPath = dbPath.erase(dbPath.find_last_of("/")+1);
    }    
    // Drop DB url
    dbPath = dbPath.erase(0, dbPath.find_first_of("/")+1).c_str();    
    dbPath = dbPath.erase(0, dbPath.find_first_of("/")+1).c_str();

    //    std::cout << "pagesave fields end dbPath: \t" << dbPath << std::endl;

    //     folderNameTextEntry->SetText(std::string(path).erase(0, TString(m_pageFolderListTree->GetFirstItem()->GetText()).Length() + 1).c_str());    
    folderNameTextEntry->SetText(dbPath.c_str());
  }
  setOkButton();
}

void PageSaveDialog::setOkButton() {
  if (TString(folderNameTextEntry->GetText()).Sizeof() > 1  &&
      TString(pageNameTextEntry->GetText()).Sizeof() > 1 ) {
    m_okButton->SetEnabled(true);
  } else {
    m_okButton->SetEnabled(false);
  }
}
