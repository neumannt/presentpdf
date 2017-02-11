#ifndef H_BackgroundRenderer
#define H_BackgroundRenderer
//----------------------------------------------------------------------------
#include <QThread>
#include <QSize>
#include <vector>
//----------------------------------------------------------------------------
namespace Poppler { class Document; }
//----------------------------------------------------------------------------
class QImage;
//----------------------------------------------------------------------------
/// Renders PDF files to images in a background thread
class Renderer  : public QThread
{
   Q_OBJECT

   private:
   /// The document
   Poppler::Document* doc;
   /// The desired image size
   QSize imageSize;
   /// The desired thumbnail size
   QSize thumbSize;

   /// The images
   std::vector<QImage*> images;
   /// The thumbnails
   std::vector<QImage*> thumbnails,darkThumbnails;

   /// The cache file
   void* file;
   /// The cache
   unsigned char* cacheStart,*cacheEnd;
   /// Done?
   bool stopped,mustStop;

   /// Cleanup
   void cleanup();

   Renderer(const Renderer&);
   void operator=(const Renderer&);

   public:
   /// Constructor
   Renderer();
   /// Destructor
   ~Renderer();

   /// Prepare the rendering. Must be called before run or starting a thread
   bool prepare(Poppler::Document& doc,const QSize& imageSize,const QSize& thumbSize);

   /// Run the renderer. Usually called by starting the thread, but can be called directly, too.
   void run();
   /// Stop the rendered
   void stop();

   /// The number of pages
   unsigned getPageCount() const { return images.size(); }
   /// Get a specific page
   QImage* getPage(unsigned index) const { return (index<images.size())?images[index]:0; }
   /// Get a specific thumbnail page
   QImage* getThumbnailPage(unsigned index) const { return (index<images.size())?thumbnails[index]:0; }
   /// Get a specific thumbnail page
   QImage* getDarkThumbnailPage(unsigned index) const { return (index<images.size())?darkThumbnails[index]:0; }

   signals:
   /// A page was rendered
   void pageRendered(unsigned index);
};
//----------------------------------------------------------------------------
#endif
