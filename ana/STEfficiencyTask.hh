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

#ifndef _STEFFICIENCYTASK_H_
#define _STEFFICIENCYTASK_H_

// FAIRROOT classes
#include "FairTask.h"
#include "FairLogger.h"

// SPiRITROOT classes
#include "STData.hh"
#include "STDigiPar.hh"
#include "STVector.hh"
#include "EfficiencyFactory.hh"

// ROOT classes
#include "TClonesArray.h"
#include "TLorentzVector.h"
#include "TString.h"
#include "TH2.h"

// STL
#include <vector>
#include <memory>

using std::vector;

class STEfficiencyTask : public FairTask {
  public:
    struct EfficiencySettings
    {int NClusters = 15; double DPoca = 20; 
     int NThetaBins = 20; double ThetaMin = 0; double ThetaMax = 90;
     int NMomBins = 20; double MomMin = 0; double MomMax = 3000;
     std::vector<std::pair<double, double>> PhiCuts = {{0,360}};};

    /// Constructor
    STEfficiencyTask(EfficiencyFactory* t_factory);
    /// Destructor
    ~STEfficiencyTask();

    void SetParticleList(const std::vector<int>& t_plist);
    std::vector<int> GetParticleList() { return fSupportedPDG; };
    EfficiencySettings& AccessSettings(int t_pdg) { return fEfficiencySettings[t_pdg]; }
    /// Initializing the task. This will be called when Init() method invoked from FairRun.
    virtual InitStatus Init();
    /// Setting parameter containers. This will be called inbetween Init() and Run().
    virtual void SetParContainers();
    /// Running the task. This will be called when Run() method invoked from FairRun.
    virtual void Exec(Option_t *opt);
    virtual void FinishTask();
    void SetPersistence(Bool_t value);

  private:
    FairLogger *fLogger;                ///< FairLogger singleton
    Bool_t fIsPersistence;              ///< Persistence check variable
  
    STDigiPar *fPar;                    ///< Parameter read-out class pointer
    STVectorI *fPDG;                 ///<
    TClonesArray *fData;                ///< STData from the conc files
    TClonesArray *fEff;                 ///< Efficiency of each type of particle

    std::vector<int> fSupportedPDG;
    EfficiencyFactory *fFactory = nullptr;
    std::map<int, TEfficiency> fEfficiency; ///<
    std::map<int, EfficiencySettings> fEfficiencySettings; ///<
  
  ClassDef(STEfficiencyTask, 1);
};

#endif