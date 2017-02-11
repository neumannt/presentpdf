#include "Presenter.hpp"
#include "Renderer.hpp"
#include "View.hpp"
#include <QPainter>
#include <iostream>
#include <iomanip>
#include <cstdio>
//----------------------------------------------------------------------------
using namespace std;
//----------------------------------------------------------------------------
Presenter::Presenter(Renderer& renderer,unsigned pageCount)
   : renderer(renderer),lineWidth(3),lineColor(Qt::black),mode(Normal),page(0),showTimer(false)
   // Constructor
{
   // Compute the appropriate thumbnail sizes
   thumbX=1;
   while ((thumbX*thumbX)<pageCount)
      thumbX++;
   thumbY=thumbX;
   thumbSize=QSize(screens.common().width()/thumbX-10,screens.common().height()/thumbY-10);
   thumbSpacing=QSize(screens.common().width()/thumbX,screens.common().height()/thumbY);

   connect(&renderer, SIGNAL(pageRendered(unsigned)), this, SLOT(pageChanged(unsigned)));
   connect(&timer, SIGNAL(timeout()), this, SLOT(tick()));
}
//----------------------------------------------------------------------------
static void printMinutes(unsigned seconds)
   // Print seconds formatted as minutes:seconds
{
   cout << setw(5) << (seconds/60) << ":" << setfill('0') << setw(2) << (seconds%60) << setfill(' ');
}
//----------------------------------------------------------------------------
Presenter::~Presenter()
   // Destructor
{
   // Show timing at the end
   if (!slidesLog.empty()) {
      slidesLog.push_back(pair<unsigned,unsigned>(~0u,time(0)));
      cout << setw(8) << "page" << " " << setw(8) << "duration" << " " << setw(8) << "enter" << " " << setw(8) << "leave" << endl;
      for (unsigned index=0;index<slidesLog.size()-1;index++) {
         unsigned duration=slidesLog[index+1].second-slidesLog[index].second;
         unsigned enter=slidesLog[index].second-slidesLog[0].second;
         unsigned leave=slidesLog[index+1].second-slidesLog[0].second;
         cout << setw(8) << (slidesLog[index].first+1) << " ";
         printMinutes(duration);
         cout << " ";
         printMinutes(enter);
         cout << " ";
         printMinutes(leave);
         cout << endl;
      }
   }
}
//----------------------------------------------------------------------------
void Presenter::setProfile(const vector<unsigned>& profile)
   // Set a presentation profile
{
   this->profile=profile;
}
//----------------------------------------------------------------------------
void Presenter::createViews()
   // Create a full screen view on each screen
{
   for (unsigned index=0;index<screens.screenCount();index++) {
      View* v=new View(*this,screens.screen(index).target);
      v->move(screens.screen(index).geometry.topLeft());
      v->resize(screens.screen(index).geometry.width(),screens.screen(index).geometry.height());
      if (!index)
         v->activateWindow();
      v->show();
      views.push_back(v);
   }
}
//----------------------------------------------------------------------------
Scribble* Presenter::getCurrentScribble(bool createIfNeeded)
   // Get the current scribble (if any)
{
   switch (mode) {
      case Overview: return nullptr;
      case Normal:
         if (scribbles.count(page)||createIfNeeded) {
            return &(scribbles[page]);
         } else return nullptr;
      case Black: return nullptr;
      case White: return &scratchScribble;
   }
   throw; // unreachable
}
//----------------------------------------------------------------------------
void Presenter::paintPage(QPainter& painter,View* view)
   // Draw the current page
{
   // Rendet the PDF page
   painter.fillRect(painter.viewport(),QBrush(Qt::black));
   QImage* img=renderer.getPage(page);
   if (img)
      painter.drawImage(view->target.topLeft(),*img);

   // Timer functionality
   if (showTimer&&(view==views.front())) {
      unsigned duration=time(0)-slidesLog.front().second;
      // Show the time
      char buffer[50];
      snprintf(buffer,sizeof(buffer),"%u:%02u",duration/60,duration%60);
      QRectF bounds=painter.boundingRect(QRect(0,0,1,1),buffer);
      painter.setBackground(Qt::white);
      painter.setBackgroundMode(Qt::OpaqueMode);
      painter.drawText(view->width()-bounds.width(),bounds.height(),buffer);
      // Show the progress if a profile is available
      if (!profile.empty()) {
         // Compute bar bounds
         unsigned maxDuration=profile.back();
         if (!maxDuration) maxDuration=1;
         unsigned showDuration=duration;
         if (showDuration>maxDuration) showDuration=maxDuration;
         unsigned expectedDuration=(page<profile.size())?profile[page]:maxDuration;
         if (expectedDuration>maxDuration) expectedDuration=maxDuration;
         unsigned right=view->width()-bounds.width();

         // And paint
         painter.setBrush(Qt::white);
         painter.setPen(Qt::black);
         painter.drawRect(0,0,(right*expectedDuration)/maxDuration,5);
         painter.drawRect(0,8,(right*showDuration)/maxDuration,5);
      }
   }
}
//----------------------------------------------------------------------------
void Presenter::paintOverview(QPainter& painter,View* view)
   // Draw the overview page
{
   painter.fillRect(painter.viewport(),QBrush(Qt::black));
   for (unsigned index=0;index<renderer.getPageCount();index++) {
      QImage* img=(index==page)?renderer.getThumbnailPage(index):renderer.getDarkThumbnailPage(index);
      if (!img) continue;
      unsigned x=index%thumbX,y=index/thumbX;
      unsigned px=view->target.left()+(x*thumbSpacing.width()),py=view->target.top()+(y*thumbSpacing.height());
      painter.drawImage(px,py,*img);
   }
}
//----------------------------------------------------------------------------
void Presenter::paint(QPainter& painter,View* view)
   // Draw the current state
{
   switch (mode) {
      case Overview:
         if (view==views.front()) {
            paintOverview(painter,view);
            break;
         }
         // fallthrough
      case Normal:
         paintPage(painter,view);
         if (auto scribble=getCurrentScribble()) {
            painter.setRenderHint(QPainter::Antialiasing);
            scribble->paint(painter,view->target);
         }
         break;
      case Black:
         painter.fillRect(painter.viewport(),QBrush(Qt::black));
         break;
      case White:
         painter.fillRect(painter.viewport(),QBrush(Qt::white));
         if (auto scribble=getCurrentScribble()) {
            painter.setRenderHint(QPainter::Antialiasing);
            scribble->paint(painter,view->target);
         }
         break;
   }
}
//----------------------------------------------------------------------------
QSize Presenter::presentationSize() const
   // The size of the presentation area
{
   return QSize(screens.common().width(),screens.common().height());
}
//----------------------------------------------------------------------------
QSize Presenter::thumbnailSize() const
   // The size of thumbnails
{
   return thumbSize;
}
//----------------------------------------------------------------------------
void Presenter::invalidateViews()
   // Invalidate all views
{
   for (unsigned index=0;index<views.size();index++)
      views[index]->update();
}
//----------------------------------------------------------------------------
void Presenter::goTo(unsigned page)
   // Go to a specific page
{
   if (page!=this->page) {
      this->page=page;
      if (!slidesLog.empty())
         slidesLog.push_back(pair<unsigned,unsigned>(page,time(0)));
      invalidateViews();
   }
}
//----------------------------------------------------------------------------
void Presenter::incPage(unsigned step)
   // Increment the current page
{
   if (page+step>=renderer.getPageCount())
      goTo(renderer.getPageCount()-1); else
      goTo(page+step);
}
//----------------------------------------------------------------------------
void Presenter::decPage(unsigned steps)
   // Decrement the current page
{
   if (page<=steps)
      goTo(0); else
      goTo(page-steps);
}
//----------------------------------------------------------------------------
void Presenter::firstPage()
   // Go to the first page
{
   if (((mode==Normal)||(mode==Overview))&&(page)) {
      goTo(0);
   }
}
//----------------------------------------------------------------------------
void Presenter::pageUp()
   // Go to the previous page
{
   if (mode==Normal) {
      decPage(1);
   } else if (mode==Overview) {
      decPage(thumbX);
   }
}
//----------------------------------------------------------------------------
void Presenter::previousPage()
   // Go to the previous page
{
   if (((mode==Normal)||(mode==Overview))&&(page)) {
      decPage(1);
   }
}
//----------------------------------------------------------------------------
void Presenter::nextPage()
   // Go to the next page
{
   if (((mode==Normal)||(mode==Overview))&&((page+1)<renderer.getPageCount())) {
      incPage(1);
   }
}
//----------------------------------------------------------------------------
void Presenter::pageDown()
   // Go to the next page
{
   if (mode==Normal) {
      incPage(1);
   } else if (mode==Overview) {
      incPage(thumbX);
   }
}
//----------------------------------------------------------------------------
void Presenter::lastPage()
   // Go to the last page
{
   if (((mode==Normal)||(mode==Overview))&&((page+1)<renderer.getPageCount())) {
      goTo(renderer.getPageCount()-1);
   }
}
//----------------------------------------------------------------------------
void Presenter::confirmPage()
   // Confirm the current page
{
   if (mode!=Normal) {
      mode=Normal;
      invalidateViews();
   } else {
      incPage(1);
   }
}
//----------------------------------------------------------------------------
void Presenter::toggleWhite()
   // Toggle to white
{
   if (mode==White)
      mode=Normal; else
      mode=White;
   invalidateViews();
}
//----------------------------------------------------------------------------
void Presenter::toggleBlack()
   // Toggle to black
{
   if (mode==Black)
      mode=Normal; else
      mode=Black;
   invalidateViews();
}
//----------------------------------------------------------------------------
void Presenter::toggleThumbnails()
   // Toggle between thumbnail view
{
   if (mode==Overview)
      mode=Normal; else
      mode=Overview;
   invalidateViews();
}
//----------------------------------------------------------------------------
void Presenter::clicked(unsigned x,unsigned y)
   // Handle a mouse click
{
   if (mode==Overview) {
      unsigned tx=x/thumbSpacing.width();
      unsigned ty=y/thumbSpacing.height();
      unsigned p=tx+(thumbX*ty);
      if ((tx<thumbX)&&(ty<thumbY)&&(p<renderer.getPageCount())) {
         mode=Normal;
         if (page!=p)
            goTo(p); else
            invalidateViews();
      }
   }
}
//----------------------------------------------------------------------------
void Presenter::toggleTimer()
   // Toggle the timer display
{
   if (showTimer) {
      showTimer=false;
      timer.stop();
   } else {
      showTimer=true;
      timer.start(1000);
   }
   if (showTimer&&slidesLog.empty())
      resetTimer();
   invalidateViews();
}
//----------------------------------------------------------------------------
void Presenter::resetTimer()
   // Reset the timer
{
   slidesLog.clear();
   slidesLog.push_back(pair<unsigned,unsigned>(page,time(0)));
}
//----------------------------------------------------------------------------
void Presenter::pageChanged(unsigned index)
   // A page changed
{
   if (mode==Normal) {
      if (page==index)
         invalidateViews();
   } else if (mode==Overview) {
      invalidateViews();
   }
}
//----------------------------------------------------------------------------
void Presenter::tick()
   // A second passed
{
   if (showTimer) {
      View* v=views.front();
      QRect rect=QRect(0,0,v->width(),v->fontMetrics().height());
      v->update(rect);
   }
}
//----------------------------------------------------------------------------
void Presenter::clearScribble()
   // Clear the scribble
{
   if (auto scribble=getCurrentScribble()) {
      scribble->clear();
      invalidateViews();
   }
}
//----------------------------------------------------------------------------
void Presenter::drawLine(int x1,int y1,int x2,int y2,double intensity)
   // Add a line
{
   if (auto scribble=getCurrentScribble(true)) {
      QRect bb=scribble->drawLine(x1,y1,x2,y2,lineWidth*intensity,lineColor);
      if (!bb.isEmpty())
         for (auto view:views)
            view->update(bb.translated(view->target.topLeft()));
   }
}
//----------------------------------------------------------------------------
void Presenter::eraseLine(int x1,int y1,int x2,int y2,double intensity)
   // Erase a previously drawn line
{
   if (auto scribble=getCurrentScribble()) {
      QRect bb=scribble->eraseLine(x1,y1,x2,y2,2*lineWidth*intensity);
      if (!bb.isEmpty())
         for (auto view:views)
            view->update(bb.translated(view->target.topLeft()));
   }
}
//----------------------------------------------------------------------------
void Presenter::setLineWidth(unsigned width)
   // Set the line width
{
   lineWidth=width;
}
//----------------------------------------------------------------------------
void Presenter::setLineColor(QColor color)
   // Set the line color
{
   lineColor=color;
}
//----------------------------------------------------------------------------
