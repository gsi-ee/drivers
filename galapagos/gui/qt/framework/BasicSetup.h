#ifndef GAPG_BASICSETUP_H
#define GAPG_BASICSETUP_H

#ifdef USE_GALAPAGOS_LIB
extern "C"
{
#include "galapagos/libgalapagos.h"
}
#else
extern void printm (char *, ...);
#endif

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
