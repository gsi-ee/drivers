#include "nxyterwidget.h"
#include "NyxorGui.h"

#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QSignalMapper>
#include <QPushButton>

const int OtherRegsId[] = { 38, 39, 43, 44, 45 };

NxyterWidget::NxyterWidget(QWidget* parent, NyxorGui* owner, uint8_t id):
   QWidget(parent),fUpdateFlags(0)
{
    fI2C= new nxyter::NxI2c(owner, id);


    setupUi(this);

   fIgnore = false;

   //     =============== Bias registers =====================

   QGridLayout* grid = new QGridLayout(BiasGroup);
   grid->setMargin(3);
   grid->setSpacing(3);

   QSignalMapper* map = new QSignalMapper(this);

   for (int n=0;n<NumBias;n++) {
      // first column - label
      grid->addWidget(new QLabel(QString("#%1: %2").arg(BiasShift+n).arg(nxyter::NxI2c::registerName(BiasShift+n)), this), n, 0);

      fBiasSlider[n] = new QSlider(Qt::Horizontal, this);
      fBiasSlider[n]->setMinimum(0);
      fBiasSlider[n]->setMaximum(255);
      fBiasSlider[n]->setPageStep(16);
      //QObject::connect(fBiasSlider[n], SIGNAL(sliderReleased()), map, SLOT(map()));
      QObject::connect(fBiasSlider[n], SIGNAL(valueChanged(int)), map, SLOT(map()));
      map->setMapping(fBiasSlider[n], n);

      grid->addWidget(fBiasSlider[n], n, 1);

      fBiasSpins[n] = new QSpinBox(this);
      fBiasSpins[n]->setMinimum(0);
      fBiasSpins[n]->setMaximum(255);
      fBiasSpins[n]->setMinimumWidth(70);
      QObject::connect(fBiasSpins[n], SIGNAL(valueChanged(int)), map, SLOT(map()));
      map->setMapping(fBiasSpins[n], 100+n);

      grid->addWidget(fBiasSpins[n], n, 2);
   }

   QObject::connect(map, SIGNAL(mapped(int)), this, SLOT(biasRegChanged(int)));

   //  ================ channel masks ======================

   map = new QSignalMapper(this);

   grid = new QGridLayout(MaskGroup);
   grid->setMargin(3);
   grid->setSpacing(3);

   QPushButton* btn = new QPushButton("all", this);
   btn->setMaximumHeight(20);
   grid->addWidget(btn, 0, 0);
   QObject::connect(btn, SIGNAL(clicked()), map, SLOT(map()));
   map->setMapping(btn, 999);

   for (int y=0;y<8;y++) {
      btn = new QPushButton("o", this);
      btn->setMaximumHeight(20);
      btn->setMaximumWidth(20);
      grid->addWidget(btn, 0, y+2);

      QObject::connect(btn, SIGNAL(clicked()), map, SLOT(map()));
      map->setMapping(btn, 700+y);
   }

   for (int x=0;x<16;x++) {
      btn = new QPushButton(QString("Chls %1-%2").arg(x*8, 2, 16, QLatin1Char('0')).arg(x*8+7, 2, 16, QLatin1Char('0')), this);
      btn->setMaximumHeight(20);
      grid->addWidget(btn, x+1, 0);

      QObject::connect(btn, SIGNAL(clicked()), map, SLOT(map()));
      map->setMapping(btn, 500+x);

      for (int y=0;y<8;y++) {
         int nch = x*8 + y;
         fMaskChks[nch] = new QCheckBox(this);
         grid->addWidget(fMaskChks[nch], x+1, y+2);

         QObject::connect(fMaskChks[nch], SIGNAL(clicked(bool)), map, SLOT(map()));
         map->setMapping(fMaskChks[nch], nch);
      }
   }

   QObject::connect(map, SIGNAL(mapped(int)), this, SLOT(maskRowColumn(int)));

   MaskGroup->adjustSize();

   // ================ channel thresholds =================

   grid = new QGridLayout(ThresholdsGroup);
   grid->setMargin(3);
   grid->setSpacing(3);
   map = new QSignalMapper(this);

   btn = new QPushButton("all", this);
   btn->setMaximumHeight(20);
   grid->addWidget(btn, 0, 0);
   QObject::connect(btn, SIGNAL(clicked()), map, SLOT(map()));
   map->setMapping(btn, 999);

   btn = new QPushButton("+", this);
   btn->setMaximumHeight(16);
   btn->setMaximumWidth(16);
   grid->addWidget(btn, 0, 1);
   QObject::connect(btn, SIGNAL(clicked()), map, SLOT(map()));
   map->setMapping(btn, 888);


   for (int y=0;y<8;y++) {
      QHBoxLayout* hbox = new QHBoxLayout();
      hbox->setMargin(0);
      hbox->setSpacing(1);

      btn = new QPushButton("v", this);
      btn->setMaximumHeight(16);
      btn->setMaximumWidth(16);
      hbox->addWidget(btn);

      QObject::connect(btn, SIGNAL(clicked()), map, SLOT(map()));
      map->setMapping(btn, 700+y);

      btn = new QPushButton("+", this);
      btn->setMaximumHeight(16);
      btn->setMaximumWidth(16);
      hbox->addWidget(btn);

      QObject::connect(btn, SIGNAL(clicked()), map, SLOT(map()));
      map->setMapping(btn, 800+y);

      grid->addLayout(hbox, 0, y+2);
   }

   for (int x=0;x<17;x++) {
      QString lbl = QString("Chls %1-%2").arg(x*8, 2, 16, QLatin1Char('0')).arg(x*8+7, 2, 16, QLatin1Char('0'));
      if (x==16) lbl = "Test chl";

      btn = new QPushButton(lbl, this);
      btn->setMaximumHeight(20);
      grid->addWidget(btn, x+1, 0);

      QObject::connect(btn, SIGNAL(clicked()), map, SLOT(map()));
      map->setMapping(btn, 500+x);

      btn = new QPushButton("+", this);
      btn->setMaximumHeight(16);
      btn->setMaximumWidth(16);
      grid->addWidget(btn, x+1, 1);

      QObject::connect(btn, SIGNAL(clicked()), map, SLOT(map()));
      map->setMapping(btn, 600+x);

      for (int y=0;y<8;y++) {
         int nch = x*8 + y;

         fThrdSpins[nch] = new QSpinBox(this);
         fThrdSpins[nch]->setMinimum(-1);
         fThrdSpins[nch]->setMaximum(32);

         grid->addWidget(fThrdSpins[nch], x+1, y+2);

         QObject::connect(fThrdSpins[nch], SIGNAL(valueChanged(int)), map, SLOT(map()));
         map->setMapping(fThrdSpins[nch], nch);

         if (x==16) break;
      }
   }

   ThresholdsGroup->adjustSize();

   QObject::connect(map, SIGNAL(mapped(int)), this, SLOT(thresholdRowColumn(int)));

   // I2C configuration bits 32-33

   grid = new QGridLayout(I2CRegsGroup);
   grid->setMargin(3);
   grid->setSpacing(3);
   map = new QSignalMapper(this);

   for (int n=0;n<NumConfigBits;n++) {
      fConfigChks[n] = new QCheckBox(this);
      fConfigChks[n]->setText(nxyter::NxI2c::configurationBitName(n));
      grid->addWidget(fConfigChks[n], n, 0);

      QObject::connect(fConfigChks[n], SIGNAL(clicked(bool)), map, SLOT(map()));
      map->setMapping(fConfigChks[n], n);
   }

   QObject::connect(map, SIGNAL(mapped(int)), this, SLOT(configBitChanged(int)));

   // other registers

   grid = new QGridLayout(OtherRegsGroup);
   grid->setMargin(3);
   grid->setSpacing(3);
   map = new QSignalMapper(this);

   for (int n=0;n<NumOtherRegs;n++) {
      grid->addWidget(new QLabel(QString("#%1: %2").arg(OtherRegsId[n]).arg(nxyter::NxI2c::registerName(OtherRegsId[n])), this), n, 0);

      fOtherSpins[n] = new QSpinBox(this);
      fOtherSpins[n]->setMinimum(0);
      fOtherSpins[n]->setMaximum(255);
      fOtherSpins[n]->setMinimumWidth(70);
      grid->addWidget(fOtherSpins[n], n, 1);

      QObject::connect(fOtherSpins[n], SIGNAL(valueChanged(int)), map, SLOT(map()));
      map->setMapping(fOtherSpins[n], n);
   }
   QObject::connect(map, SIGNAL(mapped(int)), this, SLOT(otherRegsChanged(int)));

   //getSubConfig(); // do not rretrieve registers immediately. We might have to init chains first!
   showContext();
}


bool NxyterWidget::getSubConfig()
{
   int res = fI2C->getContext(fContext);

   showContext();

   return res==0;
}

bool NxyterWidget::setSubConfig()
{
  bool rev=true;
  rev=(fI2C->setContext(fContext, fUpdateFlags) == 0);
  fUpdateFlags=0;
   return rev;
}

bool NxyterWidget::fillCmdFile(FILE* f)
{
   fprintf(f, "\n// NXYTER relevant settings\n");

   return fContext.fillCmdFile(f, 0 /*fChip->getNxNumber()*/);
}

bool NxyterWidget::setSubToDefault()
{
   fContext.setToDefault();

   showContext();

//   setSubChangedOn();

   return true;
}


void NxyterWidget::showContext()
{
   fIgnore = true;

   for (int n=0;n<NumBias;n++) {
     uint8_t value=fContext.getRegister(BiasShift+n);
      fBiasSpins[n]->setValue(value);
      fBiasSlider[n]->setValue(value);
      QString tooltip = QString("0x%1").arg(value,0,16);
      fBiasSpins[n]->setToolTip(tooltip);
   }

   for (int n=0;n<NumConfigBits;n++) {
      bool on = fContext.getConfigurationBit(n);
      fConfigChks[n]->setCheckState(on ? Qt::Checked : Qt::Unchecked);
   }

   for (int n=0;n<NumOtherRegs;n++){
      uint8_t value=fContext.getRegister(OtherRegsId[n]);
      fOtherSpins[n]->setValue(value);
      QString tooltip = QString("0x%1 - delay=%2").arg(value,0,16).arg( nxyter::NxI2c::settingToDelay(value));
      fOtherSpins[n]->setToolTip(tooltip);
   }
   fIgnore = false;

   showMask();

   showThreshold();
   fUpdateFlags=0;
}


void NxyterWidget::showMask()
{
   fIgnore = true;
   for (int n=0;n<MaskSize;n++) {
      bool off = fContext.getChannelMaskBit(n);
      fMaskChks[n]->setCheckState(off ? Qt::Unchecked : Qt::Checked);
   }
   fIgnore = false;
}

void NxyterWidget::showThreshold(int select)
{
   fIgnore = true;

   for (int n=0;n<MaskSize+1;n++) {
      if ((select>=0) && (n!=select)) continue;

      uint8_t value = fContext.getThresholdTrim(n);

      fThrdSpins[n]->setValue(value);
      QString tooltip = QString("0x%1").arg(value,0,16);
      fThrdSpins[n]->setToolTip(tooltip);

      bool off = fContext.getPowerOffMaskBit(n);

      fThrdSpins[n]->setStyleSheet(off ? "QWidget { background-color:red; }" : "QWidget { background-color:0; }");
   }

   fIgnore = false;

}

void NxyterWidget::biasRegChanged(int nreg)
{
   if (fIgnore) return;
   //printf("biasRegChanged - %d \n",nreg);
   //fUpdateFlags |= nxyter::kDoCore; // NOTE: we do not need this here, since we change values directly
   fIgnore = true;

   int value = 0;

   if (nreg>=100) {
      nreg -= 100;
      value = fBiasSpins[nreg]->value();
      fBiasSlider[nreg]->setValue(value);
   } else {
      value = fBiasSlider[nreg]->value();
      fBiasSpins[nreg]->setValue(value);
   }

   fContext.setRegister(BiasShift+nreg, value);

   // JAM2016: need to activate i2c on nyxor first:
   fI2C->enableI2C();
   fI2C->setRegister(BiasShift+nreg, value);
   fI2C->disableI2C();

   fIgnore = false;
}

void NxyterWidget::maskRowColumn(int pos)
{
   if (fIgnore) return;
   //printf("maskRowColumn - %d\n",pos);

   fUpdateFlags |= nxyter::kDoMask;
   //setSubChangedOn();

   if (pos==999) {
      for (int n=0;n<MaskSize;n++) {
         bool off = fContext.getChannelMaskBit(n);
         fContext.setChannelMaskBit(n, !off);
      }
   } else
   if (pos >= 700) {
      pos-=700;
      for (int n=0;n<16;n++) {
         bool off = fContext.getChannelMaskBit(n*8+pos);
         fContext.setChannelMaskBit(n*8+pos, !off);
      }
   } else
   if (pos >= 500) {
      pos-=500;
      for (int n=0;n<8;n++) {
         bool off = fContext.getChannelMaskBit(pos*8+n);
         fContext.setChannelMaskBit(pos*8+n, !off);
      }
   } else
   if (pos<MaskSize) {
      bool off = fMaskChks[pos]->checkState() == Qt::Unchecked;
      fContext.setChannelMaskBit(pos, off);
      return;
   }

   showMask();
}

void NxyterWidget::thresholdRowColumn(int pos)
{
   if (fIgnore) return;
   //printf("thresholdRowColumn - %d\n",pos);
   fUpdateFlags |= nxyter::kDoTrim; // JAM2016
   //setSubChangedOn();

   if (pos==999) {
      for (int n=0;n<MaskSize;n++) {
         bool off = fContext.getPowerOffMaskBit(n);
         fContext.setPowerOffMaskBit(n, !off);
      }
   } else
   if (pos==888) {
      for (int n=0;n<MaskSize;n++) {
         uint8_t trim = fContext.getThresholdTrim(n);
         fContext.setThresholdTrim(n, (trim+1) % 0x20);
      }
   } else
   if (pos >= 800) {  // vertical +
      pos-=800;
      for (int n=0;n<16;n++) {
         uint8_t trim = fContext.getThresholdTrim(n*8+pos);
         fContext.setThresholdTrim(n*8+pos, (trim+1) % 0x20);
      }
   } else
   if (pos >= 700) { // vertical v
      pos-=700;
      for (int n=0;n<16;n++) {
         bool off = fContext.getPowerOffMaskBit(n*8+pos);
         fContext.setPowerOffMaskBit(n*8+pos, !off);
      }
   } else
   if (pos == 616) {
      uint8_t trim = fContext.getThresholdTrim(MaskSize);
      fContext.setThresholdTrim(MaskSize, (trim+1) % 0x20);
   } else
   if (pos >= 600) { // vertical v
      pos-=600;
      for (int n=0;n<8;n++) {
         uint8_t trim = fContext.getThresholdTrim(pos*8+n);
         fContext.setThresholdTrim(pos*8+n, (trim+1) % 0x20);
      }
   } else
   if (pos==516) {
      bool off = fContext.getPowerOffMaskBit(MaskSize);
      fContext.setPowerOffMaskBit(MaskSize, !off);
   } else

   if (pos >= 500) {
      pos-=500;
      for (int n=0;n<8;n++) {
         bool off = fContext.getPowerOffMaskBit(pos*8+n);
         fContext.setPowerOffMaskBit(pos*8+n, !off);
      }
   } else
   if (pos<=MaskSize) {
      int value = fThrdSpins[pos]->value();
      if (value==-1) {
         fContext.setThresholdTrim(pos, 31);
         fContext.setPowerOffMaskBit(pos, !fContext.getPowerOffMaskBit(pos));
         showThreshold(pos);
      } else
      if (value==32) {
         fContext.setThresholdTrim(pos, 0);
         fContext.setPowerOffMaskBit(pos, !fContext.getPowerOffMaskBit(pos));
         showThreshold(pos);
      } else {
         fContext.setThresholdTrim(pos, value);
      }

      return;
   }

   showThreshold();
}

void NxyterWidget::configBitChanged(int nbit)
{
   if (fIgnore) return;
   //printf("configBitChanged - %d\n",nbit);
   fUpdateFlags |= nxyter::kDoCore; // JAM2016
   bool on = fConfigChks[nbit]->checkState() == Qt::Checked;

   fContext.setConfigurationBit(nbit, on);

   //setSubChangedOn();
}

void NxyterWidget::otherRegsChanged(int n)
{
   if (fIgnore) return;
   //printf("otherRegsChanged - %d\n",n);
   fUpdateFlags |= nxyter::kDoCore;// JAM2016
   int nreg = OtherRegsId[n];

   int value = fOtherSpins[n]->value();

   fContext.setRegister(nreg, value);

   //setSubChangedOn();
}


void NxyterWidget::dumpConfig(std::ostream& os)
{
    os << "NxyterWidget dumping configuration";
    if(fI2C){
      os <<" of id:"<< (int) fI2C->getId() << std::endl;
      os << *fI2C << std::endl;

    }
    os<< std::endl;

}

