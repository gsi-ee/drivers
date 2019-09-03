#include <QApplication>
#include <QMainWindow>

#include "GalapagosGui.h"

int main(int argc, char *argv[])
{

  Q_INIT_RESOURCE(galapicons);

  QApplication::setStyle("plastique");
  QApplication app(argc, argv);


  gapg::GalapagosGui main(0);
  main.show();
  int ret = app.exec();
return ret;
}
