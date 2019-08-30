#ifndef GAPGMACROS_H
#define GAPGMACROS_H

/** some useful preprocessor macros to handle the multiple widget organisation:
 * */

#define GALAGUI_CONNECT_TOGGLED_16(X,Y)  \
 QObject::connect (X##00, SIGNAL(toggled(bool)), this, SLOT(Y##00(bool))); \
    QObject::connect (X##01, SIGNAL(toggled(bool)), this, SLOT(Y##01(bool))); \
    QObject::connect (X##02, SIGNAL(toggled(bool)), this, SLOT(Y##02(bool))); \
    QObject::connect (X##03, SIGNAL(toggled(bool)), this, SLOT(Y##03(bool))); \
    QObject::connect (X##04, SIGNAL(toggled(bool)), this, SLOT(Y##04(bool))); \
    QObject::connect (X##05, SIGNAL(toggled(bool)), this, SLOT(Y##05(bool))); \
    QObject::connect (X##06, SIGNAL(toggled(bool)), this, SLOT(Y##06(bool))); \
    QObject::connect (X##07, SIGNAL(toggled(bool)), this, SLOT(Y##07(bool))); \
    QObject::connect (X##08, SIGNAL(toggled(bool)), this, SLOT(Y##08(bool))); \
    QObject::connect (X##09, SIGNAL(toggled(bool)), this, SLOT(Y##09(bool))); \
    QObject::connect (X##10, SIGNAL(toggled(bool)), this, SLOT(Y##10(bool))); \
    QObject::connect (X##11, SIGNAL(toggled(bool)), this, SLOT(Y##11(bool))); \
    QObject::connect (X##12, SIGNAL(toggled(bool)), this, SLOT(Y##12(bool))); \
    QObject::connect (X##13, SIGNAL(toggled(bool)), this, SLOT(Y##13(bool))); \
    QObject::connect (X##14, SIGNAL(toggled(bool)), this, SLOT(Y##14(bool))); \
    QObject::connect (X##15, SIGNAL(toggled(bool)), this, SLOT(Y##15(bool)));


//#define GALAGUI_ASSIGN_WIDGETS_16(X) \
// X##[0]=0;\
//
////= Y##00;\
//
////  X## [1] =##Y##01;\
////  X## [2] = Y##02;\
////  X## [3] = Y##03;\
////  X## [4] = Y##04;\
////  X## [5] = Y##05;\
////  X## [6] = Y##06;\
////  X## [7] = Y##07;\
////  X## [8] = Y##08;\
////  X## [9] = Y##09;\
////  X## [10] = Y##10;\
////  X## [11] = Y##11;\
////  X## [12] = Y##12;\
////  X## [13] = Y##13;\
////  X## [14] = Y##14;\
////  X## [15] = Y##15;



#define GALAGUI_DEFINE_MULTICHANNEL_TOGGLED_16(X) \
virtual void X##_toggled_00 (bool on); \
  virtual void X##_toggled_01 (bool on); \
  virtual void X##_toggled_02 (bool on);\
  virtual void X##_toggled_03 (bool on);\
  virtual void X##_toggled_04 (bool on);\
  virtual void X##_toggled_05 (bool on);\
  virtual void X##_toggled_06 (bool on);\
  virtual void X##_toggled_07 (bool on);\
  virtual void X##_toggled_08 (bool on);\
  virtual void X##_toggled_09 (bool on);\
  virtual void X##_toggled_10 (bool on);\
  virtual void X##_toggled_11 (bool on);\
  virtual void X##_toggled_12 (bool on);\
  virtual void X##_toggled_13 (bool on);\
  virtual void X##_toggled_14 (bool on);\
  virtual void X##_toggled_15 (bool on);



#define GALAGUI_IMPLEMENT_MULTICHANNEL_TOGGLED_16(C,X)\
void C::X##_toggled_00 (bool on)\
{\
  X##_toggled(0,on);\
}\
void C::X##_toggled_01 (bool on)\
{\
   X##_toggled(1,on);\
}\
void C::X##_toggled_02 (bool on)\
{\
  X##_toggled(2,on);\
}\
void C::X##_toggled_03 (bool on)\
{\
   X##_toggled(3,on);\
}\
void C::X##_toggled_04 (bool on)\
{\
   X##_toggled(4,on);\
}\
void C::X##_toggled_05 (bool on)\
{\
   X##_toggled(5,on);\
}\
void C::X##_toggled_06 (bool on)\
{\
   X##_toggled(6,on);\
}\
 void C::X##_toggled_07 (bool on)\
 {\
   X##_toggled(7,on);\
 }\
 void C::X##_toggled_08 (bool on)\
 {\
   X##_toggled(8,on);\
 }\
 void C::X##_toggled_09 (bool on)\
 {\
   X##_toggled(9,on);\
 }\
 void C::X##_toggled_10 (bool on)\
 {\
   X##_toggled(10,on);\
 }\
 void C::X##_toggled_11 (bool on)\
 {\
   X##_toggled(11,on);\
 }\
 void C::X##_toggled_12 (bool on)\
 {\
   X##_toggled(12,on);\
 }\
 void C::X##_toggled_13 (bool on)\
 {\
   X##_toggled(13,on);\
 }\
 void C::X##_toggled_14 (bool on)\
 {\
   X##_toggled(14,on);\
 }\
 void C::X##_toggled_15 (bool on)\
 {\
    X##_toggled(15,on);\
 }


//////////////////////////////////////////////////////////////////


#define GALAGUI_CONNECT_INDEXCHANGED_16(X,Y)  \
    QObject::connect (X##00, SIGNAL(currentIndexChanged(int)), this, SLOT(Y##00(int))); \
    QObject::connect (X##01, SIGNAL(currentIndexChanged(int)), this, SLOT(Y##01(int))); \
    QObject::connect (X##02, SIGNAL(currentIndexChanged(int)), this, SLOT(Y##02(int))); \
    QObject::connect (X##03, SIGNAL(currentIndexChanged(int)), this, SLOT(Y##03(int))); \
    QObject::connect (X##04, SIGNAL(currentIndexChanged(int)), this, SLOT(Y##04(int))); \
    QObject::connect (X##05, SIGNAL(currentIndexChanged(int)), this, SLOT(Y##05(int))); \
    QObject::connect (X##06, SIGNAL(currentIndexChanged(int)), this, SLOT(Y##06(int))); \
    QObject::connect (X##07, SIGNAL(currentIndexChanged(int)), this, SLOT(Y##07(int))); \
    QObject::connect (X##08, SIGNAL(currentIndexChanged(int)), this, SLOT(Y##08(int))); \
    QObject::connect (X##09, SIGNAL(currentIndexChanged(int)), this, SLOT(Y##09(int))); \
    QObject::connect (X##10, SIGNAL(currentIndexChanged(int)), this, SLOT(Y##10(int))); \
    QObject::connect (X##11, SIGNAL(currentIndexChanged(int)), this, SLOT(Y##11(int))); \
    QObject::connect (X##12, SIGNAL(currentIndexChanged(int)), this, SLOT(Y##12(int))); \
    QObject::connect (X##13, SIGNAL(currentIndexChanged(int)), this, SLOT(Y##13(int))); \
    QObject::connect (X##14, SIGNAL(currentIndexChanged(int)), this, SLOT(Y##14(int))); \
    QObject::connect (X##15, SIGNAL(currentIndexChanged(int)), this, SLOT(Y##15(int)));



#define GALAGUI_DEFINE_MULTICHANNEL_CHANGED_16(X) \
virtual void X##_changed_00 (int ix); \
  virtual void X##_changed_01 (int ix); \
  virtual void X##_changed_02 (int ix);\
  virtual void X##_changed_03 (int ix);\
  virtual void X##_changed_04 (int ix);\
  virtual void X##_changed_05 (int ix);\
  virtual void X##_changed_06 (int ix);\
  virtual void X##_changed_07 (int ix);\
  virtual void X##_changed_08 (int ix);\
  virtual void X##_changed_09 (int ix);\
  virtual void X##_changed_10 (int ix);\
  virtual void X##_changed_11 (int ix);\
  virtual void X##_changed_12 (int ix);\
  virtual void X##_changed_13 (int ix);\
  virtual void X##_changed_14 (int ix);\
  virtual void X##_changed_15 (int ix);

#define GALAGUI_IMPLEMENT_MULTICHANNEL_CHANGED_16(C,X)\
void C::X##_changed_00 (int ix)\
{\
  X##_changed(0,ix);\
}\
void C::X##_changed_01 (int ix)\
{\
   X##_changed(1,ix);\
}\
void C::X##_changed_02 (int ix)\
{\
  X##_changed(2,ix);\
}\
void C::X##_changed_03 (int ix)\
{\
   X##_changed(3,ix);\
}\
void C::X##_changed_04 (int ix)\
{\
   X##_changed(4,ix);\
}\
void C::X##_changed_05 (int ix)\
{\
   X##_changed(5,ix);\
}\
void C::X##_changed_06 (int ix)\
{\
   X##_changed(6,ix);\
}\
 void C::X##_changed_07 (int ix)\
 {\
   X##_changed(7,ix);\
 }\
 void C::X##_changed_08 (int ix)\
 {\
   X##_changed(8,ix);\
 }\
 void C::X##_changed_09 (int ix)\
 {\
   X##_changed(9,ix);\
 }\
 void C::X##_changed_10 (int ix)\
 {\
   X##_changed(10,ix);\
 }\
 void C::X##_changed_11 (int ix)\
 {\
   X##_changed(11,ix);\
 }\
 void C::X##_changed_12 (int ix)\
 {\
   X##_changed(12,ix);\
 }\
 void C::X##_changed_13 (int ix)\
 {\
   X##_changed(13,ix);\
 }\
 void C::X##_changed_14 (int ix)\
 {\
   X##_changed(14,ix);\
 }\
 void C::X##_changed_15 (int ix)\
 {\
    X##_changed(15,ix);\
 }

#endif
