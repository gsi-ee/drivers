#ifndef GAPG_BASICSETUP_H
#define GAPG_BASICSETUP_H


extern "C"
{
#ifdef USE_GALAPAGOS_LIB
#include "galapagos/libgalapagos.h"
#else
void printm (char *, ...);
#endif
}


namespace gapg {



/** the (A)BC for all frontend setup structures */
class BasicSetup
{

  public:

  BasicSetup(){;}
  virtual ~BasicSetup(){;}
  virtual void Dump(){printm("Empty status structure - Please implement a new frontend first");}

};




} // namespace

#endif
