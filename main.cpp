#include "ImageViewerWidget.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    ImageViewerWidget* viewer = new ImageViewerWidget(nullptr);
    viewer->SetImage(QImage(":/quitlogo.png"));
    viewer->AddSubpixelDotPixelCornerConv(30,30,0,255,0);
    viewer->AddSubpixelLinePixelCornerConv(30,30,40,40,255,0,0);
    viewer->AddSubpixelTextPixelCornerConv(30,30,0,255,0,"hello");
    viewer->show();
    return a.exec();
}
