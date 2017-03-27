#include <QtGui/QApplication>
#include <QtGui/QMainWindow>
#include "GosipGui.h"

int main(int argc, char *argv[])
{
  QApplication app(argc, argv);
//  QMainWindow main;
//  main.resize(800,600);
//  GosipGui poland(&main);
  GosipGui main(0);
  main.show();
  int ret = app.exec();
return ret;
}
