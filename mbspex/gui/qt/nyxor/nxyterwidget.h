#ifndef NxyterWidget_H
#define NxyterWidget_H

#include <QWidget>

#include <QSpinBox>
#include <QSlider>
#include <QCheckBox>
#include <QLineEdit>

#include "ui_nxyterwidget.h"

#include "NxContext.h"
#include "NxI2c.h"

class NyxorGui;

class NxyterWidget : public QWidget , public Ui::NxyterWidget {
   Q_OBJECT

   enum { NumBias = 14, BiasShift = 16, MaskSize = 128, NumConfigBits=12, NumOtherRegs = 5 };

   protected:

      NyxorGui* fxOwner;
      nxyter::NxI2c*    fI2C;
      nxyter::NxContext  fContext;

      QSpinBox*  fBiasSpins[NumBias];
      QSlider*   fBiasSlider[NumBias];
      QCheckBox* fMaskChks[MaskSize];
      QSpinBox*  fThrdSpins[MaskSize+1];
      QCheckBox* fConfigChks[NumConfigBits];
      QSpinBox*  fOtherSpins[NumOtherRegs];

      bool  fIgnore;

      int fUpdateFlags; //< mask here which parts of nxyter have to be updated.
                       // can be nxytger::kDoAll, kDoMask, kDoCore, kDoTrim;

   public:
      NxyterWidget(QWidget* parent, NyxorGui* owner, uint8_t id);

      void showMask();
      void showThreshold(int select = -1);

      /** JAM new: printout of current configuration*/
      void dumpConfig(std::ostream& os);

      bool needSetSubConfig(){return (fUpdateFlags!=0);}

      virtual bool getSubConfig();
      virtual bool setSubConfig();
      virtual bool setSubToDefault();
      virtual bool fillCmdFile(FILE* f);

      const nxyter::NxContext* getContext(){return &fContext;}

   public slots:

      void showContext();
      void biasRegChanged(int);
      void maskRowColumn(int);
      void thresholdRowColumn(int);
      void configBitChanged(int);
      void otherRegsChanged(int);
};

#endif
