//-----------------------------------------------------------
// Description:
//   Embed pulses onto the data
//
// Environment:
//   Software developed for the SPiRIT-TPC at RIKEN
//
// Author List:
//   Genie Jhang     Korea University     (original author)
//   Tommy Tsang     MSU                  (decouple this class from STDecoder class)
//-----------------------------------------------------------

#ifndef _STPIDCUTTASK_H_
#define _STPIDCUTTASK_H_

// FAIRROOT classes
#include "FairTask.h"
#include "FairLogger.h"

// SPiRITROOT classes
#include "STData.hh"
#include "STDigiPar.hh"

// ROOT classes
#include "TClonesArray.h"
#include "TString.h"
#include "TH2.h"
#include "TCutG.h"

// STL
#include <vector>

using std::vector;

class STPIDCutTask : public FairTask {
  public:
    /// Constructor
    STPIDCutTask();
    /// Destructor
    ~STPIDCutTask();

  
    /// Initializing the task. This will be called when Init() method invoked from FairRun.
    virtual InitStatus Init();
    /// Setting parameter containers. This will be called inbetween Init() and Run().
    virtual void SetParContainers();
    /// Running the task. This will be called when Run() method invoked from FairRun.
    virtual void Exec(Option_t *opt);
    virtual void FinishTask();
    void SetPersistence(Bool_t value);

    void SetNPitch(int pitches);
    void SetNYaw(int yaws);

    void SetCutConditions(int minNClus, double maxPOCA);
    void SetCutFile(const std::string& cutfile);

    

  private:
    int _ToPitchId(const TVector3& vec);
    int _ToYawId(const TVector3& vec);
    FairLogger *fLogger;                ///< FairLogger singleton

    Bool_t fIsPersistence;              ///< Persistence check variable
  
    STDigiPar *fPar;                    ///< Parameter read-out class pointer
    TClonesArray *fData;

    Int_t fNPitches = 5;
    Int_t fNYaw = 4;

    Int_t fMomBins = 1000;
    Int_t fMinMom = -700;
    Int_t fMaxMom = 3000;

    Int_t fdEdXBins = 1000;
    Int_t fMindEdX = 0;
    Int_t fMaxdEdX = 1000;

    Int_t fMinNClus = 15;
    Double_t fMaxDPOCA = 20;

    TFile *fCutFile = nullptr;
    const std::vector<std::string> fParticleNames = {"Pim", "Pip", "Proton", "Deuteron", "Triton", "He3", "He4"};
    const std::vector<int> fPDG = {-211, 211, 2212, 1000010020, 1000010030, 1000020030, 1000020040};
    const std::vector<double> fMass = {0.144, 0.144, 1, 2, 3, 3, 4};
    std::vector<TCutG*> fCuts;

    std::vector<std::vector<TH2F*>> fPIDHists;
    TClonesArray *fPDGLists = nullptr;
    TClonesArray *fMassLists = nullptr;
  
  ClassDef(STPIDCutTask, 1);
};

#endif
