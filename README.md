PDF viewer with tablet support.

Requires QT5 and Poppler (e.g., the libpoppler-qt5-dev packe) to build.

Usage: pdfviewer <file>

Keyboard commands:

left/right	change page
tab		switch between overview and slides
w		switch to a white page and back
d		enable drawing with the mouse (tablet is always enabled)
c		clear current drawing
1-9		change pen width and colour
t		enable timining

A previous timing run can be given as additional parameter, the viewer will
then report how the current timing is relative to the recorded run.

To build run

qmake presentpdf.pro 
(e.g., using /usr/lib/x86_64-linux-gnu/qt5/bin/qmake)

make

