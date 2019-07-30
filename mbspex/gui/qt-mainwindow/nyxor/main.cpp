#include <QApplication>
#include <QMainWindow>
#include "NyxorGui.h"
#include <iostream>


void usage(const char* progname)
{
  printf (" %s for mbspex library  \n", progname);
  printf (" v0.92 17-January-2018 by JAM (j.adamczewski@gsi.de)\n");
  printf ("********************************************************************\n");
  printf (" usage: %s [-h|-old] \n",progname);
  printf ("\t Options:\n");
  printf ("\t\t  -h           : display this help\n");
  printf ("\t\t  -old         : use old register adresses (before 2016)\n");
  printf ("********************************************************************\n");
  exit (0);
}


int main (int argc, char *argv[])
{
  bool oldsetup = false;
  if ((argc==2) && (!strcmp(argv[1],"?") || !strcmp(argv[1],"-h")  || !strcmp(argv[1],"-help") || !strcmp(argv[1],"--help"))) usage(argv[0]);

  for (int narg = 1; narg < argc; narg++)
  {
    if (strlen (argv[narg]) == 0)
      continue;
    if (argv[narg][0] == '-')
    {
      if (strstr (argv[narg], "-o"))
      {
        printf("Starting %s for old register setup...\n",argv[0]);
        std::cout<<std::endl;
        oldsetup = true;
      }
    }
  }    // for

  argc = 1; // hide all additional parameters from Qt
  QApplication::setStyle("plastique");
  QApplication app (argc, argv);
  NyxorGui main (0, oldsetup);
  main.show ();
  int ret = app.exec ();
  return ret;
}
