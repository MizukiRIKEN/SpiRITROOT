#include "STHelixTrackingTask.hh"

#include "FairRun.h"
#include "FairRootManager.h"
#include "FairRuntimeDb.h"

#include <iostream>
using namespace std;

ClassImp(STHelixTrackingTask)

STHelixTrackingTask::STHelixTrackingTask()
: STRecoTask("Helix Tracking Task", 1, false)
{
}

STHelixTrackingTask::STHelixTrackingTask(Bool_t persistence, Bool_t clusterPersistence)
: STRecoTask("Helix Tracking Task", 1, persistence)
{
  fIsClusterPersistence = clusterPersistence;
}

STHelixTrackingTask::~STHelixTrackingTask()
{
}

void STHelixTrackingTask::SetClusterPersistence(Bool_t value) { fIsClusterPersistence = value; }

void STHelixTrackingTask::SetNumTracksLowLimit(Int_t limit) { fNumTracksLowLimit = limit; }
void STHelixTrackingTask::SetClusteringOption(Int_t opt)
{
  cout << "STHelixTrackingTask::SetClusteringOption() is not used any more." << endl;
}
void STHelixTrackingTask::SetSaturationOption(Int_t opt) { fSaturationOption = opt; }
void STHelixTrackingTask::SetClusterCutLRTB(Double_t left, Double_t right, Double_t top, Double_t bottom)
{
  fCCLeft = left;
  fCCRight = right;
  fCCTop = top;
  fCCBottom = bottom;
}
STHelixTrackFinder *STHelixTrackingTask::GetTrackFinder() { return fTrackFinder; }

InitStatus STHelixTrackingTask::Init()
{
  if (STRecoTask::Init() == kERROR)
    return kERROR;

  fHitArray = (TClonesArray *) fRootManager -> GetObject("STHit");
  if (fHitArray == nullptr) {
    LOG(ERROR) << "Cannot find STHit array!" << FairLogger::endl;
    return kERROR;
  }
  
  fTrackArray = new TClonesArray("STHelixTrack", 100);
  fRootManager -> Register("STHelixTrack", "SpiRIT", fTrackArray, fIsPersistence);

  fHitClusterArray = new TClonesArray("STHitCluster", 100);
  fRootManager -> Register("STHitCluster", "SpiRIT", fHitClusterArray, fIsClusterPersistence);

  fTrackFinder = new STHelixTrackFinder();
  fTrackFinder -> SetSaturationOption(fSaturationOption);
  fTrackFinder -> SetClusterCutLRTB(fCCLeft, fCCRight, fCCTop, fCCBottom);

  if (fRecoHeader != nullptr) {
    fRecoHeader -> SetPar("helix_numTracksLowLimit", fNumTracksLowLimit);
    fRecoHeader -> Write("RecoHeader", TObject::kWriteDelete);
  }

  return kSUCCESS;
}

void STHelixTrackingTask::Exec(Option_t *opt)
{
  fTrackArray -> Delete();
  fHitClusterArray -> Delete();

  if (fEventHeader -> IsBadEvent())
    return;

  fTrackFinder -> BuildTracks(fHitArray, fTrackArray, fHitClusterArray);

  if (fTrackArray -> GetEntriesFast() < fNumTracksLowLimit) {
    fEventHeader -> SetIsBadEvent();
    LOG(INFO) << Space() << "Found less than " << fNumTracksLowLimit << " helix tracks. Bad event!" << FairLogger::endl;
    fTrackArray -> Delete();
    fHitClusterArray -> Delete();
    return;
  }

  LOG(INFO) << Space() << "STHelixTrack " << fTrackArray -> GetEntriesFast() << FairLogger::endl;
}
