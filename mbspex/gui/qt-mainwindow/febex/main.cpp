#include <QApplication>
#include <QMainWindow>

#include "FebexGui.h"

int main(int argc, char *argv[])
{

 // Q_INIT_RESOURCE(gosipicons);

  QApplication::setStyle("plastique");
  QApplication app(argc, argv);


  FebexGui main(0);
  main.show();
  int ret = app.exec();
return ret;
}
