#include <QtGui/QApplication>
#include <QtGui/QMainWindow>
#include "NyxorGui.h"

int main(int argc, char *argv[])
{
  QApplication app(argc, argv);
//  QMainWindow main;
//  main.resize(800,600);
//  PolandGui poland(&main);
  NyxorGui main(0);
  main.show();
  int ret = app.exec();
return ret;
}
