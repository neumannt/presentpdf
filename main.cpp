#include <QApplication>
#include <poppler/qt5/poppler-qt5.h>
#include <iostream>
#include <fstream>
#include "Presenter.hpp"
#include "Renderer.hpp"
//----------------------------------------------------------------------------
using namespace std;
//----------------------------------------------------------------------------
static bool readProfile(const char* file,vector<unsigned>& profile)
   // Read a stored profile from disk
{
   ifstream in(file);
   if (!in.is_open()) {
      cerr << "unable to open " << file << endl;
      return false;
   }
   // Check the header
   {
      string c1,c2,c3,c4;
      in >> c1 >> c2 >> c3 >> c4;
      if ((c1!="page")||(c2!="duration")||(c3!="enter")||(c4!="leave")) {
         cerr << "profile format not recognized" << endl;
         return false;
      }
   }
   // Read the transitions
   profile.clear();
   while (in) {
      int page,lm,ls;
      char c;
      string d,e;
      in >> page >> d >> e >> lm >> c >> ls;
      if (!in) break;
      if (page<1) {
         cerr << "invalid page number" << endl;
         return false;
      }
      if (c!=':') {
         cerr << "invalid timing information" << endl;
         return false;
      }
      while (static_cast<unsigned>(page)>profile.size())
         profile.push_back(0);
      if (profile[page-1]) {
         cerr << "warning: revisited page " << page << ", ignored" << endl;
      } else {
         profile[page-1]=(60*lm)+ls;
      }
   }

   // Enforce monotonicity
   unsigned time=0;
   for (unsigned index=0;index<profile.size();index++) {
      if (time<=profile[index]) {
         time=profile[index];
      } else {
         if (profile[index])
            cerr << "warning: page " << index << " accessed too early, profile must be linear!" << endl;
         profile[index]=time;
      }
   }

   return true;
}
//----------------------------------------------------------------------------
int main(int argc, char *argv[])
{
   // Check command line arguments
   QApplication app(argc, argv);
   if ((argc!=2)&&(argc!=3))  {
      cerr << "usage: " << argv[0] << " [pdf] <profile>" << endl;
      return 1;
   }

   // Read the profile
   vector<unsigned> timings;
   if ((argc==3)&&(!readProfile(argv[2],timings)))
      return 1;

   // Open the PDF
   Poppler::Document* doc=Poppler::Document::load(argv[1]);
   if (!doc) {
      cerr << "unable to open " << argv[1] << endl;
      return 1;
   }

   // Prepare rendererer and presenter
   Renderer renderer;
   Presenter presenter(renderer,doc->numPages());
   if (!renderer.prepare(*doc,presenter.presentationSize(),presenter.thumbnailSize()))
      return 1;
   renderer.start();

   // Show the presentation
   if (argc==3)
      presenter.setProfile(timings);
   presenter.createViews();
   int result=app.exec();

   // Cleanup
   renderer.stop();
   delete doc;
   return result;
}
//----------------------------------------------------------------------------

