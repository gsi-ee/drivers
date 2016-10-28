#include <QtGui/QApplication>
#include <QtGui/QMainWindow>
#include "ApfelGui.h"

int main(int argc, char *argv[])
{
  QApplication app(argc, argv);
//  QMainWindow main;
//  main.resize(800,600);
//  PolandGui poland(&main);
  ApfelGui main(0);
  main.show();
  int ret = app.exec();
return ret;
}
