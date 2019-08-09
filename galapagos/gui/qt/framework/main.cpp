#include <QApplication>
#include <QMainWindow>
#include "BasicGui.h"

int main(int argc, char *argv[])
{
  QApplication app(argc, argv);
//  QMainWindow main;
//  main.resize(800,600);
//  BasicGui poland(&main);
  BasicGui main(0);
  main.show();
  int ret = app.exec();
return ret;
}
