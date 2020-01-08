#ifndef GAPG_BASICOBJECTMANAGER_H
#define GAOG_BASICOBJECTMANAGER_H

#include <stdint.h>
#include <stddef.h>
#include <vector>

namespace gapg
{

/** Template for managing list of objects with uniue id and name.
 * User may select one entry in this list as "currently active" to be used by default*/
template<class T>
class BasicObjectManager
{

protected:

  /* list of known obejcts  as visible in the object editor*/
  std::vector<T> fKnownObjects;

  /* index of the currently active object in the list of known*/
  size_t fCurrentObjectIndex;

public:

  BasicObjectManager ();

  virtual ~BasicObjectManager ()
  {
  }

  size_t GetCurrentObjectIndex ()
  {
    return fCurrentObjectIndex;
  }

  void SetCurrentObjectIndex (size_t ix)
  {
    if (fCurrentObjectIndex >= fKnownObjects.size ())
      fCurrentObjectIndex = fKnownObjects.size () - 1;
    else
      fCurrentObjectIndex = ix;
  }

  /** Access the currently active Object in list*/
  T* GetCurrentObject ();

  T& AddObject (T& pak);

  /** Access a known  Object by unique id number. May be redundant if we rely on name*/
  T* GetObject (uint32_t id);

  /** Access a known Object by unique name*/
  T* GetObject (const char* name);

  /** find out the index of object in the list of known by id. returns -1 if not found*/
  int GetObjectIndex (uint32_t id);

  /** find out the index of object in the list of known by unique name. returns -1 if not found*/
  int GetObjectIndex (const char* name);

  size_t NumKnownObjects ();

  /** access to list of known Object by index */
  T* GetKnownObject (size_t ix);

  /** remove Object from list by index*/
  void RemoveKnownObject (size_t ix);

  void ClearObjects ();
};

/////////////////// Template Definition:

template<class T>
BasicObjectManager<T>::BasicObjectManager () :
    fCurrentObjectIndex (0)
{
  ClearObjects ();
}

template<class T>
T& BasicObjectManager<T>::AddObject (T& pak)
{
  fKnownObjects.push_back (pak);
  return pak;
}

template<class T>
T* BasicObjectManager<T>::GetObject (uint32_t id)
{
  for (int t = 0; t < fKnownObjects.size (); ++t)
  {
    if (fKnownObjects[t].Id () == id)
      return &(fKnownObjects[t]);
  }
  return 0;
}


template<class T>
int BasicObjectManager<T>::GetObjectIndex (uint32_t id)
{
  for (int t = 0; t < fKnownObjects.size (); ++t)
  {
    if (fKnownObjects[t].Id () == id)
      return t;
  }
  return -1;
}


template<class T>
T* BasicObjectManager<T>::GetObject (const char* name)
{
  for (int t = 0; t < fKnownObjects.size (); ++t)
  {
    if (fKnownObjects[t].EqualsName (name))
      return &(fKnownObjects[t]);
  }
  return 0;
}

template<class T>
int BasicObjectManager<T>::GetObjectIndex (const char* name)
{
  for (int t = 0; t < fKnownObjects.size (); ++t)
  {
    if (fKnownObjects[t].EqualsName (name))
      return t;
  }
  return -1;
}



template<class T>
T* BasicObjectManager<T>::GetCurrentObject ()
{
  return GetKnownObject (fCurrentObjectIndex);
}

template<class T>
size_t BasicObjectManager<T>::NumKnownObjects ()
{
  return fKnownObjects.size ();
}

template<class T>
T* BasicObjectManager<T>::GetKnownObject (size_t ix)
{
  if (ix > fKnownObjects.size ())
    return 0;
  return &(fKnownObjects[ix]);
}

template<class T>
void BasicObjectManager<T>::RemoveKnownObject (size_t ix)
{
  //uint32_t pid = fKnownObjects[ix].Id ();
  fKnownObjects.erase (fKnownObjects.begin () + ix);
}





template<class T>
void BasicObjectManager<T>::ClearObjects ()
{
  fKnownObjects.clear ();
}


}// namespace

#endif
