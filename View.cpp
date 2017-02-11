#include "View.hpp"
#include "Presenter.hpp"
#include <QPainter>
#include <QKeyEvent>
#include <QApplication>
//----------------------------------------------------------------------------
/// Time before the cursor is hidden
static const unsigned cursorHideDelay = 3000;
//----------------------------------------------------------------------------
View::View(Presenter& presenter,const QRect& target)
   : presenter(presenter),target(target),cursorTimeout(this),delayedFullScreen(true),hiddenCursor(true),tabletDown(false),tabletPressureSensitiveness(true),mouseDrawing(false),mouseDown(false)
   // Constructor
{
   setFocusPolicy(Qt::StrongFocus);

   cursorTimeout.setSingleShot(true);
   setCursor(Qt::BlankCursor);
   connect(&cursorTimeout, SIGNAL(timeout()), this, SLOT(hideCursor()));
   setMouseTracking(true);
}
//----------------------------------------------------------------------------
View::~View()
   // Destructor
{
}
//----------------------------------------------------------------------------
void View::paintEvent(QPaintEvent* event)
   // Paint the view
{
   QPainter painter(this);
   painter.setClipRegion(event->region());
   presenter.paint(painter,this);

   if (delayedFullScreen) {
      showFullScreen();
      delayedFullScreen=false;
   }
}
//----------------------------------------------------------------------------
void View::keyPressEvent(QKeyEvent* event)
   // Handle input
{
   switch (event->key()) {
      case Qt::Key_Left:
         presenter.previousPage();
         break;
      case Qt::Key_Up: case Qt::Key_PageUp:
         presenter.pageUp();
         break;
      case Qt::Key_Right:
         presenter.nextPage();
         break;
      case Qt::Key_Down: case Qt::Key_PageDown:
         presenter.pageDown();
         break;
      case Qt::Key_Return: case Qt::Key_Enter: case Qt::Key_Space:
         presenter.confirmPage();
         break;
      case Qt::Key_Home:
         presenter.firstPage();
         break;
      case Qt::Key_End:
         presenter.lastPage();
         break;
      case Qt::Key_Escape:
         static_cast<QApplication*>(QApplication::instance())->closeAllWindows();
         break;
      case Qt::Key_D:
         mouseDrawing=!mouseDrawing;
         mouseDown=false;
         break;
      case Qt::Key_P:
         tabletPressureSensitiveness=!tabletPressureSensitiveness;
         break;
      case Qt::Key_C:
         presenter.clearScribble();
         break;
      case Qt::Key_W:
         presenter.toggleWhite();
         break;
      case Qt::Key_B:
         presenter.toggleBlack();
         break;
      case Qt::Key_T:
         presenter.toggleTimer();
         break;
      case Qt::Key_R:
         presenter.resetTimer();
         break;
      case Qt::Key_Tab:
         presenter.toggleThumbnails();
         break;
      case Qt::Key_1: presenter.setLineWidth(1); break;
      case Qt::Key_2: presenter.setLineWidth(3); break;
      case Qt::Key_3: presenter.setLineWidth(5); break;
      case Qt::Key_4: presenter.setLineWidth(7); break;
      case Qt::Key_5: presenter.setLineWidth(10); break;
      case Qt::Key_6: presenter.setLineColor(Qt::black); break;
      case Qt::Key_7: presenter.setLineColor(Qt::red); break;
      case Qt::Key_8: presenter.setLineColor(Qt::blue); break;
      case Qt::Key_9: presenter.setLineColor(Qt::green); break;
      case Qt::Key_0: presenter.setLineColor(Qt::white); break;
   }

   QWidget::keyPressEvent(event);
}
//----------------------------------------------------------------------------
void View::mouseMoveEvent(QMouseEvent* event)
   // Handle mouse movement
{
   QWidget::mouseMoveEvent(event);

   if (mouseDrawing) {
      if (mouseDown&&target.contains(event->pos())) {
         int x1=mousePos.x()-target.left(),y1=mousePos.y()-target.top();
         int x2=event->x()-target.left(),y2=event->y()-target.top();

         if (event->buttons()&Qt::LeftButton) {
            presenter.drawLine(x1,y1,x2,y2);
         } else if (event->buttons()&Qt::RightButton) {
            presenter.eraseLine(x1,y1,x2,y2);
         }
      }
      mousePos=event->pos();
   }
   showCursorTemporarily();
}
//----------------------------------------------------------------------------
void View::mousePressEvent(QMouseEvent* event)
   // Handle mouse clicks
{
   if (mouseDrawing) {
      mouseDown=true;
      mousePos=event->pos();
   } else if (target.contains(event->pos())) {
      unsigned x=event->x()-target.left();
      unsigned y=event->y()-target.top();
      presenter.clicked(x,y);
   }
   QWidget::mousePressEvent(event);
}
//----------------------------------------------------------------------------
void View::mouseReleaseEvent(QMouseEvent* /*event*/)
   // Handle mouse clicks
{
   if (mouseDrawing) {
      mouseDown=false;
   }
}
//----------------------------------------------------------------------------
void View::showCursorTemporarily()
   // Show the cursor for a while
{
   if (hiddenCursor) {
      hiddenCursor=false;

      setCursor(Qt::ArrowCursor);
      cursorTimeout.stop();
      cursorTimeout.setSingleShot(true);
      cursorTimeout.start(cursorHideDelay);
   }
}
//----------------------------------------------------------------------------
void View::hideCursor()
   // Hide the current cursor
{
   if (!hiddenCursor) {
      hiddenCursor=true;
      setCursor(Qt::BlankCursor);
   }
}
//----------------------------------------------------------------------------
void View::tabletEvent(QTabletEvent *event)
   // Handle tablet events
{
   switch (event->type()) {
      case QEvent::TabletPress:
         tabletDown=true;
         tabletPos=event->pos();
         break;
      case QEvent::TabletRelease:
         tabletDown=false;
         break;
      case QEvent::TabletMove:
         if (tabletDown&&target.contains(event->pos())) {
            int x1=tabletPos.x()-target.left(),y1=tabletPos.y()-target.top();
            int x2=event->x()-target.left(),y2=event->y()-target.top();
            double intensity=tabletPressureSensitiveness?event->pressure():1.0;
            if (event->pointerType()==QTabletEvent::Pen) {
               presenter.drawLine(x1,y1,x2,y2,intensity);
            } else if (event->pointerType()==QTabletEvent::Eraser) {
               presenter.eraseLine(x1,y1,x2,y2,intensity);
            }
         }
         tabletPos=event->pos();
         showCursorTemporarily();
         break;
      default: return;
   }
   event->accept();
}
//----------------------------------------------------------------------------
