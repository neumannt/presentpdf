#ifndef H_Presenter
#define H_Presenter
//----------------------------------------------------------------------------
#include "ScreenInfo.hpp"
#include "Scribble.hpp"
#include <QObject>
#include <QTimer>
#include <unordered_map>
//----------------------------------------------------------------------------
class QPainter;
//----------------------------------------------------------------------------
class Renderer;
class View;
//----------------------------------------------------------------------------
/// Controll logic for presenting PDFs
class Presenter : public QObject
{
   Q_OBJECT

   private:
   /// Information about all screens
   ScreenInfo screens;
   /// The renderer
   Renderer& renderer;
   /// Thumbnails layout
   unsigned thumbX,thumbY;
   /// Thumbnail size
   QSize thumbSize,thumbSpacing;
   /// The views
   std::vector<View*> views;
   /// Timer
   QTimer timer;
   /// Scribbles associated with pages
   std::unordered_map<unsigned,Scribble> scribbles;
   /// The current scratch-scribble
   Scribble scratchScribble;
   /// The line width
   unsigned lineWidth;
   /// The line color
   QColor lineColor;

   /// Possible display modi
   enum Mode { Normal, Overview, Black, White };
   /// The current mode
   Mode mode;
   /// The current page
   unsigned page;
   /// Show the timer?
   bool showTimer;
   /// Timing log (if any)
   std::vector<std::pair<unsigned,unsigned> > slidesLog;
   /// Transition profile (if any)
   std::vector<unsigned> profile;

   /// Invalidate all views
   void invalidateViews();
   /// Go to a specific page
   void goTo(unsigned page);
   /// Increment the current page
   void incPage(unsigned step);
   /// Decrement the current page
   void decPage(unsigned steps);

   /// Get the current scribble (if any)
   Scribble* getCurrentScribble(bool createIfNeeded=false);
   /// Draw the current page
   void paintPage(QPainter& painter,View* view);
   /// Draw the overview page
   void paintOverview(QPainter& painter,View* view);

   public:
   /// Constructor
   Presenter(Renderer& renderer,unsigned pageCount);
   /// Destructor
   ~Presenter();

   /// Set a presentation profile
   void setProfile(const std::vector<unsigned>& profile);
   /// Create a full screen view on each screen
   void createViews();
   /// Draw the current state
   void paint(QPainter& painter,View* view);

   /// The size of the presentation area
   QSize presentationSize() const;
   /// The size of thumbnails
   QSize thumbnailSize() const;

   /// Go to the first page
   void firstPage();
   /// Go to the previous page
   void pageUp();
   /// Go to the previous page
   void previousPage();
   /// Go to the next page
   void nextPage();
   /// Go to the next page
   void pageDown();
   /// Go to the last page
   void lastPage();
   /// Confirm the current page
   void confirmPage();
   /// Toggle to white
   void toggleWhite();
   /// Toggle to black
   void toggleBlack();
   /// Toggle between thumnail view
   void toggleThumbnails();
   /// Toggle the timer display
   void toggleTimer();
   /// Reset the timer
   void resetTimer();
   /// Handle a mouse click
   void clicked(unsigned x,unsigned y);

   /// Clear the scribble
   void clearScribble();
   /// Add a line
   void drawLine(int x1,int y1,int x2,int y2,double intensity=1.0);
   /// Erase a previously drawn line
   void eraseLine(int x1,int y1,int x2,int y2,double intensity=1.0);
   /// Set the line width
   void setLineWidth(unsigned width);
   /// Set the line color
   void setLineColor(QColor color);

   public slots:
   /// A page changed
   void pageChanged(unsigned index);
   /// Another second passed
   void tick();
};
//----------------------------------------------------------------------------
#endif
