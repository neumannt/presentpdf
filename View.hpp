#ifndef H_View
#define H_View
//----------------------------------------------------------------------------
#include <QWidget>
#include <QTimer>
//----------------------------------------------------------------------------
class Presenter;
//----------------------------------------------------------------------------
/// A viewer window. The renderering is done by the presenter class
class View : public QWidget
{
   Q_OBJECT

   private:
   /// The presenter
   Presenter& presenter;
   /// The target rectangle
   QRect target;
   /// A timer for hiding the cursor
   QTimer cursorTimeout;
   /// Show full screen of next paint?
   bool delayedFullScreen;
   /// Is the cursor hidden
   bool hiddenCursor;
   /// Tablet down?
   bool tabletDown;
   /// Tablet position
   QPoint tabletPos;
   /// Tablet pressure sensitiveness
   bool tabletPressureSensitiveness;
   /// Mouse drawing
   bool mouseDrawing,mouseDown;
   /// Mouse position
   QPoint mousePos;

   friend class Presenter;

   protected:
   /// Paint the view
   void paintEvent(QPaintEvent* event);
   /// Handle input
   void keyPressEvent(QKeyEvent* event);
   /// Handle mouse movement
   void mouseMoveEvent(QMouseEvent* event);
   /// Handle mouse clicks
   void mousePressEvent(QMouseEvent* event);
   /// Handle mouse clicks
   void mouseReleaseEvent(QMouseEvent* event);
   /// Handle tablet events
   void tabletEvent(QTabletEvent *event);

   protected slots:
   /// Show the cursor for a while
   void showCursorTemporarily();
   /// Hide the current cursor
   void hideCursor();

   public:
   /// Constructor
   View(Presenter& presenter,const QRect& target);
   /// Destructor
   ~View();
};
//----------------------------------------------------------------------------
#endif
