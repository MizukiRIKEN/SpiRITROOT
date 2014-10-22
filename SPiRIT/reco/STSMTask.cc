//-----------------------------------------------------------
// Description:
//   System coordinate manipulating task
//
// Environment:
//   Software developed for the SPiRIT-TPC at RIKEN
//
// Author List:
//   Genie Jhang     Korea University     (original author)
//-----------------------------------------------------------

// SPiRITROOT classes
#include "STSMTask.hh"

// FAIRROOT classes
#include "FairRootManager.h"
#include "FairRun.h"
#include "FairRuntimeDb.h"

#include <iostream>

ClassImp(STSMTask);

STSMTask::STSMTask()
{
  fLogger = FairLogger::GetLogger();
  fPar = NULL;

  fIsPersistence = kFALSE;
  
  fEventHCMArray = new TClonesArray("STEvent");

  fManipulator = new STSystemManipulator();
  fSMMode = kChange;
}

STSMTask::~STSMTask()
{
}

void STSMTask::SetPersistence(Bool_t value)     { fIsPersistence = value; }
void STSMTask::SetMode(ESMMode mode)            { fSMMode = mode; }

InitStatus
STSMTask::Init()
{
  FairRootManager *ioMan = FairRootManager::Instance();
  if (ioMan == 0) {
    fLogger -> Error(MESSAGE_ORIGIN, "Cannot find RootManager!");
    return kERROR;
  }

  fEventHCArray = (TClonesArray *) ioMan -> GetObject("STEventHC");
  if (fEventHCArray == 0) {
    fLogger -> Error(MESSAGE_ORIGIN, "Cannot find STEventHC array!");

    return kERROR;
  }

  ioMan -> Register("STEventHCM", "SPiRIT", fEventHCMArray, fIsPersistence);

  return kSUCCESS;
}

void
STSMTask::SetParContainers()
{
  FairRun *run = FairRun::Instance();
  if (!run)
    fLogger -> Fatal(MESSAGE_ORIGIN, "No analysis run!");

  FairRuntimeDb *db = run -> GetRuntimeDb();
  if (!db)
    fLogger -> Fatal(MESSAGE_ORIGIN, "No runtime database!");

  fPar = (STDigiPar *) db -> getContainer("STDigiPar");
  if (!fPar)
    fLogger -> Fatal(MESSAGE_ORIGIN, "STDigiPar not found!!");
}

void
STSMTask::Exec(Option_t *opt)
{
  fEventHCMArray -> Delete();

  if (fEventHCArray -> GetEntriesFast() == 0)
    return;

  STEvent *eventHC = (STEvent *) fEventHCArray -> At(0);

  STEvent *eventHCM = NULL;
  if (fSMMode == kChange)
    eventHCM = fManipulator -> Change(eventHC);
  else if (fSMMode == kRestore)
    eventHCM = fManipulator -> Restore(eventHC);

  new ((*fEventHCMArray)[0]) STEvent(eventHCM);
  delete eventHCM;
}