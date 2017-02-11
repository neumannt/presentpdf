#ifndef H_Scribble
#define H_Scribble
//----------------------------------------------------------------------------
#include <QColor>
#include <vector>
//----------------------------------------------------------------------------
class QPainter;
class QRect;
//----------------------------------------------------------------------------
/// A simple line drawing
class Scribble
{
   private:
   /// A line
   struct Line {
      /// The coordinates
      int x1,y1,x2,y2;
      /// Line color
      QColor color;
      /// Line width
      unsigned width;

      /// Get the bounding box
      QRect getBB() const;
      /// Compute distance
      double distanceToSquared(int x,int y) const;
   };
   /// The lines
   std::vector<Line> lines;

   public:
   /// Constructor
   Scribble();
   /// Destructor
   ~Scribble();

   /// Draw the scribble
   void paint(QPainter& painter,const QRect& target);

   /// Delete all lines
   void clear();
   /// Add a line
   QRect drawLine(int x1,int y1,int x2,int y2,unsigned width,QColor color);
   /// Erase a previously drawn line
   QRect eraseLine(int x1,int y1,int x2,int y2,unsigned width);
};
//----------------------------------------------------------------------------
#endif
