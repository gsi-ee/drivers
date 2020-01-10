#include "GalCoreWidget.h"

#include "GalPackageWidget.h"

#include <iostream>

namespace gapg
{

GalCoreWidget::GalCoreWidget (int index, GalPackageWidget* master, QWidget* parent) :
    QWidget(parent), fIndex(index), fPackageWidget(master)
{
  Ui_GalCoreWidget::setupUi (this);

  Core_label->setText(QString ("%1").arg (index, 2));
  QObject::connect (Core_enabled_radio, SIGNAL(toggled(bool)), this, SLOT(CoreEnabled_toggled(bool)));
  QObject::connect (CoreKernel_comboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(CoreKernel_changed(int)));
}

GalCoreWidget::~GalCoreWidget ()
{
}


void GalCoreWidget::RefreshCoreType(gapg::CoreType_t kind)
{
  int index=0;
  switch(kind)
  {
    case  NOP:
    default:
      std::cout << "GalCoreWidget::RefreshCoreType NEVER COME HERE,  unknown core type "<<(int) kind<<std::endl;
    break;

    case CHN:
      index =0;
      break;
    case USP:
      index =1;
      break;
    case TRG:
      index =2;
      break;
    case LJP:
      index =3;
      break;
    case DAC:
      index =4;
      break;

  };
  CoreTypeComboBox->setCurrentIndex(index);
}




void GalCoreWidget::CoreEnabled_toggled (bool on)
{
  std::cout << "CoreEnabled_toggled for ix="<<fIndex<<" sets to "<<on <<std::endl;
  fPackageWidget->CoreEnabled_toggled (fIndex, on);
}

void GalCoreWidget::CoreKernel_changed (int ix)
{
  std::cout << "CoreKernel_changed for ix="<<fIndex<<" sets to "<<ix <<std::endl;
  fPackageWidget->CoreKernel_changed(fIndex,ix);
}

}    // gapg
