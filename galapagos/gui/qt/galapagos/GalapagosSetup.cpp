#include "GalapagosSetup.h"
#include <QByteArray>
#include <stdint.h>

namespace gapg {


GalapagosSetup::GalapagosSetup () :
    BasicSetup (), fGeneratorActive (false), fCoreStatus_0(0), fCoreStatus_1(1), fCurrentPackageIndex(0)
{

  fKnownPatterns.clear ();
  fKnownKernels.clear ();
  fKnownPackages.clear();

}



uint64_t GalapagosSetup::GetCoreStatus ()
{
  uint64_t reg = 0;
  reg = fCoreStatus_0 | (fCoreStatus_1 << 32);
  return reg;
}


void GalapagosSetup::SetCoreStatus (uint64_t val)
{
  fCoreStatus_0 = (val & 0xFFFFFFFF);
  fCoreStatus_1 = (val >> 32) & 0xFFFFFFFF;

}


void GalapagosSetup::SetCoreRunning (uint8_t ch, bool on)
{
  //std::cout<< "GalapagosPackage::SetCoreEnabled ch="<< (int) ch<<", on="<<on << std::endl;
           if (ch > GAPG_CORES)
             return;
           uint64_t reg = GetCoreStatus ();
           uint64_t flags = (1 << ch);
           if (on)
             reg |= flags;
           else
             reg &= ~flags;
           SetCoreStatus (reg);
}

bool GalapagosSetup::IsCoreRunning (uint8_t ch)
  {
  if (ch > GAPG_CORES)
            return true;
          uint64_t reg = GetCoreStatus ();
          uint64_t flags = (1 << ch);
          bool rev = ((reg & flags) == flags);
          return rev;
  }


void GalapagosSetup::SetCoreEnabled (uint8_t core, bool on)
{
  GalapagosPackage* curpak=GetPackage(fCurrentPackageIndex);
    if(!curpak) return ;
    return (curpak->SetCoreEnabled (core, on));
}

bool GalapagosSetup::IsCoreEnabled (uint8_t core)
  {
    GalapagosPackage* curpak=GetPackage(fCurrentPackageIndex);
     if(!curpak) return false;
     return (curpak->IsCoreEnabled (core));
  }



GalapagosPackage& GalapagosSetup::AddPackage (GalapagosPackage& pak)
{
     fKnownPackages.push_back (pak);
     return pak;
}

GalapagosPackage* GalapagosSetup::GetPackage (uint32_t id)
{
  for (int t = 0; t < fKnownPackages.size (); ++t)
    {
      if (fKnownPackages[t].Id () == id)
        return &(fKnownPackages[t]);
    }
    return 0;
}

GalapagosPackage* GalapagosSetup::GetPackage (const char* name)
{
  for (int t = 0; t < fKnownPackages.size (); ++t)
   {
     if (fKnownPackages[t].EqualsName (name))
       return &(fKnownPackages[t]);
   }
   return 0;
}

size_t GalapagosSetup::NumKnownPackages ()
{
  return fKnownPackages.size ();
}

GalapagosPackage* GalapagosSetup::GetKnownPackage (size_t ix)
{
  if (ix > fKnownPackages.size ())
     return 0;
   return &(fKnownPackages[ix]);
}


/** remove pattern from list by index*/
void GalapagosSetup::RemoveKnownPackage (size_t ix)
{
  uint32_t pid = fKnownPackages[ix].Id ();
  fKnownPackages.erase (fKnownPackages.begin () + ix);

  // no further cleanup requireed here, package is top level object
}

bool GalapagosSetup::CompilePackage(size_t ix)
{
  GalapagosPackage* pak=GetPackage(ix);
  if(pak==0) return false;
  // now compile all participating kernel objects:
  for(int core=0; core<GAPG_CORES; ++core)
  {
    if(!pak->IsCoreEnabled(core)) continue;
    uint32_t cid=pak->GetKernelID(core);
    GalapagosKernel* ker=GetKernel(cid);
    if(ker==0){
      printm("GalapagosSetup::CompilePackage error (package:%s) - core %d has configured unknown kernel of id %d! skip it.",
          pak->Name(), core, cid);
      continue;
    }
    ker->Compile(); //this we do anyway, even for kernels without pattern data
      uint32_t pid=ker->GetPatternID();
    GalapagosPattern* pat=GetPattern(pid);
    if(pat!=0) {
       ker->UpdatePattern(*pat);
    }
    else
    {
      printm("GalapagosSetup::CompilePackage error (package:%s)  - core %d with kernel %s has unknown pattern of id %d!",
             pak->Name(), core,ker->Name()); // this case may appear regularly for special cores! later suppress message?
      }
   pak->SetKernel(core,*ker);
   printm("GalapagosSetup::CompilePackage :%s - has assigned kernel %s to core %dit.",
            pak->Name(), ker->Name(), core);
  } // for core


}




void GalapagosSetup::ClearKernels ()
{
  fKnownKernels.clear ();

}

GalapagosKernel& GalapagosSetup::AddKernel (GalapagosKernel& seq)
{
  // TODO: may need ot check if kernel of this id/name already exists?
  fKnownKernels.push_back (seq);
  return seq;
}

/* Access a known sequence by unique id number. May be redundant if we rely on name*/
GalapagosKernel* GalapagosSetup::GetKernel (uint32_t id)
{
  for (int t = 0; t < fKnownKernels.size (); ++t)
  {
    if (fKnownKernels[t].Id () == id)
      return &(fKnownKernels[t]);
  }
  return 0;
}

/* Access a known sequence by unique name*/
GalapagosKernel* GalapagosSetup::GetKernel (const char* name)
{
  for (int t = 0; t < fKnownKernels.size (); ++t)
  {
    if (fKnownKernels[t].EqualsName (name))
      return &(fKnownKernels[t]);
  }
  return 0;
}

size_t GalapagosSetup::NumKnownKernels ()
{
  return fKnownKernels.size ();
}

/** access to list of known sequences by index */
GalapagosKernel* GalapagosSetup::GetKnownKernel (size_t ix)
{
  if (ix > fKnownKernels.size ())
    return 0;
  return &(fKnownKernels[ix]);
}

/** remove sequence from list by index. also clean up all references in channels*/
void GalapagosSetup::RemoveKnownKernel (size_t ix)
{
  uint32_t sid = fKnownKernels[ix].Id ();
  fKnownKernels.erase (fKnownKernels.begin () + ix);

  uint32_t newsid = 0;
  if (fKnownKernels.size () > 0)
    newsid = fKnownKernels[0].Id ();    // channels with erased seqs will be assigned to first sequence

// proper cleanup of all references in the packages:
  for(int p=0; p<NumKnownPackages();++p)
  {
    GalapagosPackage* pak=GetKnownPackage(p);
    if(!pak) continue;
    pak->CleanupRemovedKernel(sid,newsid);
  }

}




bool GalapagosSetup::SetCoreKernel (int core, uint32_t id)
{
  GalapagosKernel* ker = GetKernel (id);
  if (ker == 0)
    return false;
  GalapagosPackage* curpak=GetPackage(fCurrentPackageIndex);
  if(!curpak) return 0;
  return (curpak->SetKernelID (core, ker->Id()));
}

bool GalapagosSetup::SetCoreKernel (int core, const char* name)
{
  GalapagosKernel* ker = GetKernel (name);
  if (ker == 0)
    return false;
  GalapagosPackage* curpak=GetPackage(fCurrentPackageIndex);
   if(!curpak) return 0;
   return (curpak->SetKernelID (core, ker->Id()));
}

GalapagosKernel* GalapagosSetup::GetCoreKernel (int core)
{
  GalapagosPackage* curpak=GetPackage(fCurrentPackageIndex);
  if(!curpak) return 0;
  uint32_t id = curpak->GetKernelID (core);
  if (id == 0)
    return 0;
  return GetKernel (id);
}

uint32_t GalapagosSetup::GetCoreKernelID (int chan)
{
  if (chan > GAPG_CORES)
    return 0;
  GalapagosPackage* curpak=GetPackage(fCurrentPackageIndex);
  if(!curpak) return 0;
  return curpak->GetKernelID (chan);
}
/////////////////////////////77

GalapagosPattern& GalapagosSetup::AddPattern (GalapagosPattern& pat)
{
  // TODO: may need ot check if sequence of this id/name already exists?
  fKnownPatterns.push_back (pat);
  return pat;
}

/* Access a known sequence by unique id number. May be redundant if we rely on name*/
GalapagosPattern* GalapagosSetup::GetPattern (uint32_t id)
{
  for (int t = 0; t < fKnownPatterns.size (); ++t)
  {
    if (fKnownPatterns[t].Id () == id)
      return &(fKnownPatterns[t]);
  }
  return 0;
}

/* Access a known sequence by unique name*/
GalapagosPattern*GalapagosSetup::GetPattern (const char* name)
{
  for (int t = 0; t < fKnownPatterns.size (); ++t)
  {
    if (fKnownPatterns[t].EqualsName (name))
      return &(fKnownPatterns[t]);
  }
  return 0;
}

size_t GalapagosSetup::NumKnownPatterns ()
{
  return fKnownPatterns.size ();
}

/** access to list of known sequences by index */
GalapagosPattern* GalapagosSetup::GetKnownPattern (size_t ix)
{
  if (ix > fKnownPatterns.size ())
    return 0;
  return &(fKnownPatterns[ix]);
}

/** remove pattern from list by index*/
void GalapagosSetup::RemoveKnownPattern (size_t ix)
{
  uint32_t pid = fKnownPatterns[ix].Id ();
  fKnownPatterns.erase (fKnownPatterns.begin () + ix);
  uint32_t newpid = 0;
  if (fKnownPatterns.size () > 0)
    newpid = fKnownPatterns[0].Id ();    // channels with erased patterns will be assigned to first pattern
// TODO: clean up references to patterns in kernels/Packages? maybe redundant, we will use names instead of ids there

  for(int p=0; p<NumKnownKernels();++p)
   {
     GalapagosKernel* ker=GetKnownKernel(p);
     if(!ker) continue;
     ker->CleanupRemovedPattern(pid,newpid);
   }
  for(int p=0; p<NumKnownPackages();++p)
   {
     GalapagosPackage* pak=GetKnownPackage(p);
     if(!pak) continue;
     pak->CleanupRemovedPattern(pid,newpid);
   }

}

} // gapg
