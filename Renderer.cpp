#include "Renderer.hpp"
#include <QImage>
#include <poppler/qt5/poppler-qt5.h>
#include <iostream>
#include <fcntl.h>
#include <sys/mman.h>
#include <cstdio>
//----------------------------------------------------------------------------
using namespace std;
//----------------------------------------------------------------------------
Renderer::Renderer()
   : doc(0),file(0),cacheStart(0),cacheEnd(0),stopped(false),mustStop(false)
   // Constructor
{
}
//----------------------------------------------------------------------------
Renderer::~Renderer()
   // Destructor
{
   cleanup();
}
//----------------------------------------------------------------------------
static unsigned long maxSizeBytes(const QSize& size)
   // Estimate the maximum space consumpton in bytes
{
   unsigned long pixels=static_cast<unsigned long>(size.width()+1)*static_cast<unsigned long>(size.height()+1);
   unsigned long bytes=4*pixels;

   // Add some safety
   return bytes+100+(bytes/8);
}
//----------------------------------------------------------------------------
bool Renderer::prepare(Poppler::Document& doc,const QSize& imageSize,const QSize& thumbSize)
   // Prepare the rendering. Must be called before run or starting a thread
{
   cleanup();

   this->doc=&doc;
   this->imageSize=imageSize;
   this->thumbSize=thumbSize;
   images.resize(doc.numPages());
   thumbnails.resize(doc.numPages());
   darkThumbnails.resize(doc.numPages());
   //doc.setRenderBackend(Poppler::Document::ArthurBackend);
   doc.setRenderHint(Poppler::Document::Antialiasing);
   doc.setRenderHint(Poppler::Document::TextAntialiasing);

   // Allocate a cache file
   unsigned long reservedSpace=(maxSizeBytes(imageSize)+2*maxSizeBytes(thumbSize))*(images.size()+1);
   file=tmpfile();
   int fd=fileno(static_cast<FILE*>(file));
   if (posix_fallocate(fd,0,reservedSpace)) {
      cerr << "unable to allocate " << (reservedSpace/1024/1024) << " MB for caching." << endl;
      fclose(static_cast<FILE*>(file));
      file=0;
      return false;
   }
   void* mapping=mmap(0,reservedSpace,PROT_READ|PROT_WRITE,MAP_SHARED,fd,0);
   if (mapping==MAP_FAILED) {
      cerr << "unable to map cache into memory" << endl;
      fclose(static_cast<FILE*>(file));
      file=0;
      return false;
   }
   cacheStart=static_cast<unsigned char*>(mapping);
   cacheEnd=cacheStart+reservedSpace;

   return true;
}
//----------------------------------------------------------------------------
void Renderer::cleanup()
   // Cleanup
{
   for (unsigned index=0;index<images.size();index++) {
      delete images[index]; images[index]=0;
      delete thumbnails[index]; thumbnails[index]=0;
      delete darkThumbnails[index]; darkThumbnails[index]=0;
   }
   images.clear();
   thumbnails.clear();
   if (cacheStart) {
      munmap(cacheStart,cacheEnd-cacheStart);
      cacheStart=cacheEnd=0;
   }
   if (file) {
      fclose(static_cast<FILE*>(file));
      file=0;
   }
}
//----------------------------------------------------------------------------
void Renderer::run()
   // Render the images
{
   // Render all pages
   unsigned char* writer=cacheStart;
   for (unsigned index=0;(index<images.size())&&(!mustStop);index++) {
      delete images[index]; images[index]=0;
      delete thumbnails[index]; thumbnails[index]=0;
      delete darkThumbnails[index]; darkThumbnails[index]=0;

      // Compute the desired DPI
      Poppler::Page* page=doc->page(index);
      double DPIx=static_cast<double>(imageSize.width())/(page->pageSizeF().width()/72.0);
      double DPIy=static_cast<double>(imageSize.height())/(page->pageSizeF().height()/72.0);
      double DPI=(DPIx<DPIy)?DPIx:DPIy;

      // Render
      QImage rawImg=page->renderToImage(DPI,DPI);
      if (rawImg.isNull()) {
         cerr << "unable to render page " << (index+1) << endl;
         continue;
      }
      QImage img=rawImg.convertToFormat(QImage::Format_RGB32);

      // Store in cache file
      unsigned len=img.byteCount();
      if (writer+len>cacheEnd) {
         cerr << "out of cache space, stopping rendering!" << endl;
         break;
      }
      memcpy(writer,img.bits(),len);
      images[index]=new QImage(writer,img.width(),img.height(),img.bytesPerLine(),img.format());
      writer+=len;

      // Create a thumbnail
      QImage thumb=img.scaled(thumbSize,Qt::KeepAspectRatio,Qt::SmoothTransformation);
      len=thumb.byteCount();
      if (writer+len>cacheEnd) {
         cerr << "out of cache space, stopping rendering!" << endl;
         break;
      }
      memcpy(writer,thumb.bits(),len);
      thumbnails[index]=new QImage(writer,thumb.width(),thumb.height(),thumb.bytesPerLine(),thumb.format());
      writer+=len;

      // Create a grayed thumnail
      if (writer+len>cacheEnd) {
         cerr << "out of cache space, stopping rendering!" << endl;
         break;
      }
      unsigned char* reader=thumb.bits();
      for (unsigned index2=0;index2<len;index2++)
         writer[index2]=reader[index2]/2;
      darkThumbnails[index]=new QImage(writer,thumb.width(),thumb.height(),thumb.bytesPerLine(),thumb.format());
      writer+=len;

      /// Notify
      QMetaObject::invokeMethod(this,"pageRendered",Qt::QueuedConnection,Q_ARG(unsigned,index));
   }
   stopped=true;
}
//----------------------------------------------------------------------------
void Renderer::stop()
   // Stop the rendered
{
   if (!stopped) {
      mustStop=true;
      while (!stopped)
         QThread::msleep(10);
   }
}
//----------------------------------------------------------------------------
