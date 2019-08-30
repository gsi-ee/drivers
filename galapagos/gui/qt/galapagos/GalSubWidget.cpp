#include "GalSubWidget.h"
#include <QDir>

GalSubWidget::GalSubWidget (QWidget* parent) :
    QWidget (parent), fSetup(0)
{
  fLastFileDir= QDir::currentPath();


}

GalSubWidget::~GalSubWidget ()
{
}




