#ifndef GAPG_BASICOBJECT_H
#define GAOG_BASICOBJECT_H

#include <QByteArray>
#include <stdint.h>

namespace gapg {

/** Base class for identifieable objects*/
class BasicObject
{
  public:

  /** unique id number */
  uint32_t fId;
  /** user readable name, same as in sequencer control window */
  std::string fName;

  BasicObject(uint32_t id, const char* name): fId(id), fName(name){}

  BasicObject(const BasicObject& ob)
  {
    //std::cout<< "BasicObject copy ctor for "<<ob.fName.c_str() << std::endl;
    fId=ob.fId;
    fName=ob.fName;

  }

  BasicObject& operator=(const BasicObject& rhs)
  {
    if (this != &rhs) {
    //std::cout<< "BasicObject operator= for "<<rhs.fName.c_str() << std::endl;
    fId=rhs.fId;
    fName=rhs.fName;
    }
    return *this;
  }

  virtual ~BasicObject(){}

  uint32_t Id() {return fId;}

  void SetId(uint32_t val){fId=val;}

  const char* Name() {return fName.c_str();}

  void SetName(const char* nm){fName=nm;}

  bool EqualsName(const char* nm)
  {
    return (fName.compare(std::string(nm)) ==0) ? true : false;
  }

};



} // namespace

#endif
