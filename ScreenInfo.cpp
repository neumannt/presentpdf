#include "ScreenInfo.hpp"
#include <QApplication>
#include <QDesktopWidget>
//----------------------------------------------------------------------------
ScreenInfo::ScreenInfo()
   // Constructor
{
   QDesktopWidget* desktop=QApplication::desktop();

   // Collect all screens
   unsigned count=desktop->numScreens(),primary=desktop->primaryScreen();
   screens.resize(count);
   for (unsigned index=0;index<count;index++)
      screens[index].geometry=desktop->screenGeometry(index);
   if (primary)
      std::swap(screens[primary],screens[0]);

   // Compute the common part
   // XXX currently just the large display, should pick reasonable rectangle based upon common resolutions, aspect ratio etc.
   commonRect=QRect(0,0,screens[0].geometry.width(),screens[0].geometry.height());
   for (unsigned index=1;index<count;index++) {
      if (screens[index].geometry.width()<commonRect.width())
         commonRect=QRect(0,0,screens[index].geometry.width(),commonRect.height());
      if (screens[index].geometry.height()<commonRect.height())
         commonRect=QRect(0,0,commonRect.width(),screens[index].geometry.height());
   }

   // Build the target rectangles
   for (unsigned index=0;index<count;index++) {
      unsigned w,h;
      if ((commonRect.width()>screens[index].geometry.width())||(commonRect.height()>screens[index].geometry.height())) {
         double cx=static_cast<double>(screens[index].geometry.width())/static_cast<double>(commonRect.width());
         double cy=static_cast<double>(screens[index].geometry.height())/static_cast<double>(commonRect.height());
         double c=(cx<cy)?cx:cy;
         w=commonRect.width()*c;
         h=commonRect.height()*c;
      } else {
         w=commonRect.width();
         h=commonRect.height();
      }
      unsigned x=(screens[index].geometry.width()-w)/2;
      unsigned y=(screens[index].geometry.height()-h)/2;
      screens[index].target=QRect(x,y,w,h);
   }
}
//----------------------------------------------------------------------------
ScreenInfo::~ScreenInfo()
   // Destructor
{
}
//----------------------------------------------------------------------------
