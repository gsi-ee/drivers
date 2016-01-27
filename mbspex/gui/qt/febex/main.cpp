#include <QtGui/QApplication>
#include <QtGui/QMainWindow>
#include "FebexGui.h"

int main(int argc, char *argv[])
{
  QApplication app(argc, argv);
//  QMainWindow main;
//  main.resize(800,600);
//  PolandGui poland(&main);
  FebexGui main(0);
  main.show();
  int ret = app.exec();
return ret;
}
