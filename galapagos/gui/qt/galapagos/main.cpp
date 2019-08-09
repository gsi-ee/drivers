#include <QApplication>
#include <QMainWindow>

#include "GalapagosGui.h"

int main(int argc, char *argv[])
{

 // Q_INIT_RESOURCE(gosipicons);

  QApplication::setStyle("plastique");
  QApplication app(argc, argv);


  GalapagosGui main(0);
  main.show();
  int ret = app.exec();
return ret;
}
