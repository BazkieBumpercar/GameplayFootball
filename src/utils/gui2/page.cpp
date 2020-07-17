// written by bastiaan konings schuiling 2008 - 2014
// this work is public domain. the code is undocumented, scruffy, untested, and should generally not be used for anything important.
// i do not offer support, so don't ask. to be used for inspiration :)

#include "page.hpp"

#include "windowmanager.hpp"
#include "base/utils.hpp"

namespace blunted {

  Gui2Page::Gui2Page(Gui2WindowManager *windowManager, const Gui2PageData &pageData) : Gui2Frame(windowManager, "page_" + int_to_str(pageData.pageID), 0, 0, 100, 100), pageData(pageData) {
  }

  Gui2Page::~Gui2Page() {
  }

  void Gui2Page::GoBack() {
    // moved to View::Exit: sig_OnClose();

    this->Exit();

    windowManager->GetPagePath()->Pop();
    if (windowManager->GetPagePath()->GetPath().size() > 0) {
      Gui2PageData prevPage = windowManager->GetPagePath()->GetLast();
      windowManager->GetPagePath()->Pop(); // pop previous page from path too, since it is going to be added with the createpage again
      windowManager->GetPageFactory()->CreatePage(prevPage);
    } // else: no mo menus :[

    delete this;
    return;
  }

  void Gui2Page::ProcessWindowingEvent(WindowingEvent *event) {
    if (event->IsEscape()) {
      GoBack();
      return;
    } else {
      event->Ignore();
    }
  }

  void Gui2Page::CreatePage(int pageID, void *data) {
    Properties properties;
    CreatePage(pageID, properties, data);
  }

  void Gui2Page::CreatePage(int pageID, const Properties &properties, void *data) {
    this->Exit();

    windowManager->GetPageFactory()->CreatePage(pageID, properties, data);

    delete this;
  }




  Gui2PageFactory::Gui2PageFactory() {
    mostRecentlyCreatedPage = 0;
  }

  Gui2PageFactory::~Gui2PageFactory() {
  }

  void Gui2PageFactory::SetWindowManager(Gui2WindowManager *wm) {
    windowManager = wm;
  }

  Gui2Page *Gui2PageFactory::CreatePage(int pageID, const Properties &properties, void *data) {
    Gui2PageData pageData;
    pageData.pageID = pageID;
    pageData.properties = boost::shared_ptr<Properties>(new Properties(properties));
    pageData.data = data;
    Gui2Page *page = CreatePage(pageData);
    mostRecentlyCreatedPage = page;
    return page;
    // not going to work: pointer is not persistent, while pagedata is. pageData.pagePointer = CreatePage(pageData);
    //printf("page created, id %i\n", pageData.pageID);
    //return pageData.pagePointer;
  }

}
