#include "Scribble.hpp"
#include <QPainter>
//----------------------------------------------------------------------------
using namespace std;
//----------------------------------------------------------------------------
QRect Scribble::Line::getBB() const
   // Get the bounding box
{
   int minx=min(x1,x2),maxx=max(x1,x2);
   int miny=min(y1,y2),maxy=max(y1,y2);
   return QRect(QPoint(minx-2*width,miny-2*width),QPoint(maxx+2*width,maxy+2*width));
}
//----------------------------------------------------------------------------
double Scribble::Line::distanceToSquared(int x,int y) const
{
   // Length squared
   double len=(x2-x1)*(x2-x1)+(y2-y1)*(y2-y1);

   // Just a point?
   if (!len)
      return (x1-x)*(x1-x)+(y1-y)*(y1-1);

   // Find the projection on the line
   double t=((x-x1)*(x2-x1)+(y-x1)*(y2-y1))/len;
   if (t<=0.0)
      return (x1-x)*(x1-x)+(y1-y)*(y1-1);
   if (t>=1.0)
      return (x2-x)*(x2-x)+(y2-y)*(y2-1);

   // Compute the projection
   double px=x1+(t*(x2-x1)),py=y1+(t*(y2-y1));
   return (px-x)*(px-x)+(py-y)*(py-y);
}
//----------------------------------------------------------------------------
Scribble::Scribble()
  // Constructor
{
}
//----------------------------------------------------------------------------
Scribble::~Scribble()
   // Destructor
{
}
//----------------------------------------------------------------------------
void Scribble::paint(QPainter& painter,const QRect& target)
   // Draw the scribble
{
   if (lines.empty())
      return;

   // Initialize
   int dx=target.left(),dy=target.top();
   unsigned lastStyle=0;
   QPen pen;
   pen.setColor(lines[lastStyle].color);
   pen.setWidth(lines[lastStyle].width);
   painter.setPen(pen);

   // Find segments
   for (unsigned index=0,limit=lines.size();index!=limit;) {
      // Update the pen if needed
      if ((lines[index].width!=lines[lastStyle].width)||(lines[index].color!=lines[lastStyle].color)) {
         lastStyle=index;
         pen.setColor(lines[lastStyle].color);
         pen.setWidth(lines[lastStyle].width);
         painter.setPen(pen);
      }


      // Can we continue the line?
      unsigned step=index+1;
      while ((step!=limit)&&(lines[step].x1==lines[step-1].x2)&&(lines[step].y1==lines[step-1].y2)&&(lines[step].color==lines[step-1].color)&&(lines[step].width==lines[step-1].width))
         ++step;

      // Construct the line segment
      vector<QPoint> segment;
      for (;index!=step;++index)
         segment.push_back(QPoint(lines[index].x1+dx,lines[index].y1+dy));
      segment.push_back(QPoint(lines[index-1].x2+dx,lines[index-1].y2+dy));
      painter.drawPolyline(segment.data(),segment.size());
   }
}
//----------------------------------------------------------------------------
void Scribble::clear()
   // Delete all lines
{
   lines.clear();
}
//----------------------------------------------------------------------------
QRect Scribble::drawLine(int x1,int y1,int x2,int y2,unsigned width,QColor color)
   // Add a line
{
   lines.push_back(Line{x1,y1,x2,y2,color,width});

   return lines.back().getBB();
}
//----------------------------------------------------------------------------
QRect Scribble::eraseLine(int x1,int y1,int x2,int y2,unsigned width)
   // Erase a previously drawn line
{
   unsigned widthSq=width*width;
   Line l{x1,y1,x2,y2,Qt::black,width};
   QRect lineBB=l.getBB();

   QRect bb;
   bool first=true;
   for (unsigned index=0,limit=lines.size();index!=limit;++index) {
      // Plausibility check
      if (!lineBB.intersects(lines[index].getBB()))
         continue;

      // Compute the distance
      double d=min(lines[index].distanceToSquared(x1,y1),lines[index].distanceToSquared(x2,y2));
      if (d<=widthSq) {
         // Update bounding box
         if (first) {
            bb=lines[index].getBB();
            first=false;
         } else bb|=lines[index].getBB();

         // Remove
         swap(lines[index],lines[limit-1]);
         lines.pop_back();
         --limit; --index;
      }
   }

   return bb;
}
//----------------------------------------------------------------------------
