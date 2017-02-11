#ifndef H_ScreenInfo
#define H_ScreenInfo
//----------------------------------------------------------------------------
#include <QRect>
#include <vector>
//----------------------------------------------------------------------------
class QApplication;
//----------------------------------------------------------------------------
/// Information about all attached screens
class ScreenInfo
{
   public:
   /// A screen
   struct Screen {
      /// The total geometry
      QRect geometry;
      /// The (relative) target for the common rectangle
      QRect target;
   };
   private:
   /// All screens
   std::vector<Screen> screens;
   /// The common rectangle
   QRect commonRect;

   public:
   /// Constructor
   ScreenInfo();
   /// Destructor
   ~ScreenInfo();

   /// The number of screens
   unsigned screenCount() const { return screens.size(); }
   /// A specfic screen
   const Screen& screen(unsigned index) const { return screens[index]; }
   /// The common rectangle
   const QRect& common() const { return commonRect; }
};
//----------------------------------------------------------------------------
#endif
