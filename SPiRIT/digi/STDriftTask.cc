//---------------------------------------------------------------------
// Description:
//      Drift task class source
//
//      STDriftTask apply diffusions and velocity of electron and
//      produces electrons drifted through the gas
//
//      Input  : STPrimaryCluster
//      Output : STDrifedElectron
//
// Author List:
//      JungWoo Lee     Korea Univ.       (original author)
//
//----------------------------------------------------------------------

// This class header
#include "STDriftTask.hh"

// Fair & Collaborating class headers
#include "FairRootManager.h"
#include "FairRun.h"
#include "FairRuntimeDb.h"


// SPiRIT-TPC headers
#include "STPrimaryCluster.hh"
#include "STDriftedElectron.hh"

// ROOT headers
#include "TRandom.h"

// C/C++ headers
#include "cmath"
#include "iostream"

using std::cout;
using std::endl;


ClassImp(STDriftTask);

STDriftTask::STDriftTask()
: FairTask("SPiRIT Drift"),
  fIsPersistent(kFALSE),
  fTestMode(kFALSE)
{
  fPrimaryClusterBranchName = "STPrimaryCluster";
}

STDriftTask::~STDriftTask()
{
}

InitStatus
STDriftTask::Init()
{
  // Get ROOT Manager
  FairRootManager* ioman = FairRootManager::Instance();

  if(!ioman) {
    Error("STDriftedTask::Init", "RootManager not instantiated!");
    return kERROR;
  }

  // Get input collection
  fPrimaryClusterArray = (TClonesArray *) ioman -> GetObject(fPrimaryClusterBranchName);
  if(!fPrimaryClusterArray) {
    Error("STDriftedTask::Init", "Point-array not found!");
    return kERROR;
  }

  // Create and register output array
  fDriftedElectronArray = new TClonesArray("STDriftedElectron");
  ioman -> Register("STDriftedElectron", "ST", fDriftedElectronArray, fIsPersistent);

  fGas = fPar -> GetGas();

  if(fWriteHistogram)
  {
    //fPadPlaneX = fPar -> GetPadPlaneX();
    //fPadPlaneZ = fPar -> GetPadPlaneZ();
    fPadPlaneX = 96.61;
    fPadPlaneZ = 144.64;
    fElectronDistXZ = new TH2D("ElDistXZ","", 500,0,fPadPlaneZ,
                                              500,-fPadPlaneX/2,fPadPlaneX/2);
    fElectronDistXZRaw = new TH2D("ElDistXZRaw","", 500,0,fPadPlaneZ,
                                                    500,-fPadPlaneX/2,fPadPlaneX/2);
  }

  return kSUCCESS;

}

void
STDriftTask::SetParContainers()
{
  cout << "STDriftTask::SetParContainers" << endl; //cout.flush()

  FairRun* run = FairRun::Instance();
  if(!run) Fatal("SetParContainers","No analysis run");

  FairRuntimeDb* db = run -> GetRuntimeDb();
  if(!db) Fatal("SetParContainers", "no runtime database");

  fPar = (STDigiPar*) db -> getContainer("STDigiPar");
  if(!fPar) Fatal("SetParContainers", "STDigiPar not found");
}

void
STDriftTask::Exec(Option_t *opt)
{
  if(fDriftedElectronArray==0) Fatal("STDriftedElectron::Exec)","No Drifted Electron Array");
  fDriftedElectronArray -> Delete();

  Double_t driftVelocity  = fGas -> GetDriftVelocity();  // make it [cm/ns]
  Double_t coefAttachment = fGas -> GetCoefAttachment();
  Double_t coefDiffusion  = fGas -> GetCoefDiffusionLong();
  Double_t yWirePlane     = fPar -> GetGroundWirePlaneY();  // [mm]
           yWirePlane    /= 10; // [mm] to [cm]

  Int_t    charge;
  Double_t xElectron;
  Double_t zElectron;
  Double_t yCluster;
  Double_t timeCluster;
  Double_t driftLength;
  Double_t driftTime;
  Double_t sigmaDiffusion;

  STPrimaryCluster* cluster;

  Int_t nCluster = fPrimaryClusterArray -> GetEntriesFast();
  if(fTestMode) cout << "no. of clusters   : " << nCluster << endl;
  for(Int_t iCluster=0; iCluster<nCluster; iCluster++)
  {
    cluster        = (STPrimaryCluster*) fPrimaryClusterArray -> At(iCluster);
    charge         = cluster -> GetCharge(); // [eV]
    xElectron      = cluster -> GetPositionX(); // [cm]
    zElectron      = cluster -> GetPositionZ(); // [cm]
    yCluster       = cluster -> GetPositionY(); // [cm]
    driftLength    = yWirePlane - yCluster; // [cm]
    driftTime      = driftLength/driftVelocity; // [ns]
    timeCluster    = driftTime + cluster -> GetTime(); // [ns]
    sigmaDiffusion = coefDiffusion * sqrt(driftLength);

    if(fTestMode){
      cout << "-*-" << endl;
      cout << "y - WirePlane     : " << yWirePlane << endl;
      cout << "y - Cluster       : " << yCluster << endl;
      cout << "x/z position      : " << xElectron << " / " << zElectron << endl;
      cout << "Drift length [cm] : " << driftLength << endl;
      cout << "Drift time   [ns] : " << driftTime << endl;
      cout << "no. of electrons  : " << charge << endl;
      cout << "diffusion coef.   : " << coefDiffusion << endl;
      cout << "attachment coef.  : " << coefAttachment << endl;
      cout << "exp(-drftL*attC)  : " << exp(-driftLength*coefAttachment) << endl;
    }
    if(fTestMode) cout << endl << iCluster << " / " << nCluster << endl;

    if(driftLength<=0) continue;
    if(fWriteHistogram) fElectronDistXZRaw -> Fill(zElectron, xElectron);
    for(Int_t iElectron=0; iElectron<charge; iElectron++)
    {
      if(exp(-driftLength*coefAttachment) < gRandom -> Uniform()) continue;
      xElectron += gRandom -> Gaus(0,sigmaDiffusion);
      zElectron += gRandom -> Gaus(0,sigmaDiffusion);

      //if(fTestMode) cout << ">>> x/z - diffusion position : " << xElectron << " / " << zElectron << endl;

      Int_t size = fDriftedElectronArray -> GetEntriesFast();
      STDriftedElectron* electron 
        = new((*fDriftedElectronArray)[size]) STDriftedElectron(xElectron,
                                                                zElectron,
                                                                timeCluster);
      electron -> SetIndex(size);

      if(fWriteHistogram) fElectronDistXZ -> Fill(zElectron, xElectron);
    }
  }

  if(fWriteHistogram) WriteHistogram();

  cout << "STDriftTask:: " << fDriftedElectronArray -> GetEntriesFast() << " drifted electron created" << endl; 

  return;
}

void
STDriftTask::WriteHistogram()
{
  cout << "[STDriftTask] Writing histogram..." << endl;
  TFile* file = FairRootManager::Instance() -> GetOutFile();

  file -> mkdir("STDriftTask");
  file -> cd("STDriftTask");

  fElectronDistXZRaw -> Write();
  fElectronDistXZ -> Write();
}
