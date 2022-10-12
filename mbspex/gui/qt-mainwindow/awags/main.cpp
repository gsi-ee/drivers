#include <QApplication>
#include <QMainWindow>


#include "AwagsGui.h"

int main(int argc, char *argv[])
{
  QApplication::setStyle("plastique");
  QApplication app(argc, argv);
//  QMainWindow main;
//  main.resize(800,600);
//  PolandGui poland(&main);
  AwagsGui main(0);
  main.show();
  int ret = app.exec();
return ret;
}
