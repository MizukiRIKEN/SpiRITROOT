#ifndef STGENFITVATASK_HH
#define STGENFITVATASK_HH

#include "TMatrixTSym.h"
#include "FairRunAna.h"
#include "STRecoTask.hh"
#include "STGenfitPIDTask.hh"
#include "STHelixTrack.hh"
#include "STGenfitTest2.hh"
#include "STPIDTest.hh"
#include "STRecoTrack.hh"
#include "STRecoTrackCand.hh"
#include "STRecoTrackCandList.hh"
#include "STVertex.hh"
#include "STBeamEnergy.hh"
#include "STBDCProjection.hh"

class STGenfitVATask : public STRecoTask
{
  public:
    STGenfitVATask();
    STGenfitVATask(Bool_t persistence);
    ~STGenfitVATask();

    void SetListPersistence(bool val = true) { fIsListPersistence = val; }
    void SetExPersistence(bool val = true) { fIsExPersistence = val; }
    void SetPersistence(bool val = true) { fIsPersistence = val; }

    void SetClusteringType(Int_t type);
    void SetConstantField();
    void SetFieldOffset(Double_t yOffset, Double_t zOffset);

    virtual InitStatus Init();
    virtual void Exec(Option_t *opt);

    void SetBeamFile(TString fileName);
    void SetInformationForBDC(Int_t runNo, Double_t offsetX, Double_t offsetY, Double_t offsetZ = 0);
    void SetZtoProject(Double_t peakZ, Double_t sigma, Double_t sigmaMultiple);
    void SetFixedVertex(Double_t x, Double_t y, Double_t z);

    void SetUseRave(Bool_t val = kTRUE);

  private:
    TClonesArray *fHelixTrackArray = nullptr;
    TClonesArray *fRecoTrackArray = nullptr;
    TClonesArray *fVertexArray = nullptr;
    TClonesArray *fCandListArray = nullptr;
    TClonesArray *fVATrackArray = nullptr;
    TClonesArray *fBDCVertexArray = nullptr;
    TClonesArray *fVAVertexArray = nullptr;

    TClonesArray *fExTrackArray = nullptr;

    bool fIsListPersistence = false;
    bool fIsExPersistence = false;
    bool fIsSamurai = true;

    Bool_t fUseRave = kFALSE;
    genfit::GFRaveVertexFactory *fVertexFactory = nullptr;

    STGenfitTest2 *fGenfitTest;
    STPIDTest *fPIDTest;

    Int_t fClusteringType = 2;

    TString fBeamFilename = "";
    TFile *fBeamFile;
    TTree *fBeamTree;
    Double_t fZ, fAoQ, fBeta37;
    TTree *fBDCTree;
    Double_t fBDC1x, fBDC1y, fBDC2x, fBDC2y, fBDCax, fBDCby;

    Int_t fRunNo;
    Double_t fOffsetX, fOffsetY, fOffsetZ;
    STBeamEnergy *fBeamEnergy = nullptr;
    STBDCProjection *fBDCProjection = nullptr;

    Double_t fPeakZ = -9999;
    Double_t fSigma = 0;
    Double_t fSigmaMultiple = 0;

    Double_t fFixedVertexX = -9999;
    Double_t fFixedVertexY = -9999;
    Double_t fFixedVertexZ = -9999;

    Double_t fFieldYOffset = -20.43;
    Double_t fFieldZOffset = 58;

  ClassDef(STGenfitVATask, 1)
};

#endif
