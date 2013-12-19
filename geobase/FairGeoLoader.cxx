/***************************************************************************
                          FairGeoLoader.cxx  -  description
                             -------------------
    begin                : Thu Jun 17 2004
    copyright            : (C) 2004 by ilse koenig , M. Al-turany
    email                : m.al-turany@gsi.de
 ***************************************************************************/
#include "FairGeoLoader.h"

#include "FairGeoInterface.h"           // for FairGeoInterface
#include "FairGeoRootBuilder.h"         // for FairGeoRootBuilder

#include "Riosfwd.h"                    // for ostream
#include "TGeoManager.h"                // for TGeoManager

#include <string.h>                     // for strncmp
#include <cstdlib>                      // for NULL, exit
#include <iostream>                     // for cout, endl, operator<<, etc

using std::cout;
using std::endl;

FairGeoLoader* FairGeoLoader::fgInstance=0;
//_____________________________________________________________________________
FairGeoLoader* FairGeoLoader::Instance()
{
  return fgInstance;
}
//_____________________________________________________________________________
FairGeoLoader::FairGeoLoader()
  :TNamed(),
   fInterface(NULL),
   fGeoBuilder(NULL)
{
  fgInstance=this;
}
//_____________________________________________________________________________
FairGeoLoader::FairGeoLoader(const char* Name, const char* title)
  :TNamed(Name,title),
   fInterface(NULL),
   fGeoBuilder(NULL)
{
  if (fgInstance) {
    Fatal("FairGeoLoader", "Singleton instance already exists.");
    return;
  }
  fgInstance = this;
  fInterface= new FairGeoInterface;
  if ( strncmp(Name,"TGeo",4) == 0 ) {
    TGeoManager* geom = new TGeoManager("FAIRGeom", "FAIR geometry");
    fGeoBuilder=new FairGeoRootBuilder("TGeo builder","geometry builder");
    ((FairGeoRootBuilder*)fGeoBuilder)->setGeoManager(geom);
  } else if ( strncmp(Name,"G3Native",8) == 0) {
    cout << "-I- FairGeoLoader() : Native G3 Geometry is used: This option is not supported any more!" << endl;
    exit(0);
    //        gGeoManager = NULL;
    //        fGeoBuilder=new FairGeoG3Builder("G3 builder","geometry builder");
  }

  fInterface->setGeomBuilder(fGeoBuilder);

}
//_____________________________________________________________________________
FairGeoLoader::~FairGeoLoader()
{
  delete fInterface;

}
//_____________________________________________________________________________

ClassImp(FairGeoLoader)
