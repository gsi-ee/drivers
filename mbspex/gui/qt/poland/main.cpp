#include <QApplication>
#include <QMainWindow>


#include "PolandGui.h"

int main(int argc, char *argv[])
{

  QApplication::setStyle("plastique");
  QApplication app(argc, argv);
  PolandGui main(0);
  main.show();
  int ret = app.exec();
return ret;
}
