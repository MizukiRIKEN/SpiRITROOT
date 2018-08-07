#ifndef STGENFITPIDTASK_HH
#define STGENFITPIDTASK_HH

#include "TMatrixTSym.h"
#include "STRecoTask.hh"
#include "STHelixTrack.hh"
#include "STGenfitTest2.hh"
#include "STPIDTest.hh"
#include "STRecoTrack.hh"
#include "STRecoTrackCand.hh"
#include "STRecoTrackCandList.hh"
#include "STVertex.hh"

#include "TFile.h"
#include "TTree.h"

class STGenfitPIDTask : public STRecoTask
{
  public:
    STGenfitPIDTask();
    STGenfitPIDTask(Bool_t persistence);
    ~STGenfitPIDTask();

    void SetListPersistence(bool val = true) { fIsListPersistence = val; }
    void SetPersistence(bool val = true) { fIsPersistence = val; }

    void SetClusteringType(Int_t type);
    void SetConstantField();

    virtual InitStatus Init();
    virtual void Exec(Option_t *opt);

    void SetBDCFile(TString fileName);

    void SetTargetPlane(Double_t x, Double_t y, Double_t z);

    void MakeYOffsetCalibrationFile(TString fileName);
    void WriteYOffsetCalibrationFile();

    void MakeClusterCovFile(TString fileName);
    void WriteClusterCovFile();

  private:
    TClonesArray *fHelixTrackArray = nullptr;
    TClonesArray *fCandListArray = nullptr;
    TClonesArray *fRecoTrackArray = nullptr;
    TClonesArray *fVertexArray = nullptr;
    TClonesArray *fPadHitArray = nullptr;

    bool fIsListPersistence = false;
    bool fIsSamurai = true;

    TString fGFRaveVertexMethod;
    genfit::GFRaveVertexFactory *fVertexFactory;

    STGenfitTest2 *fGenfitTest;
    STPIDTest *fPIDTest;

    Int_t fClusteringType = 2;

    TString fBDCName = "";
    TFile *fFileBDC;
    TTree *fTreeBDC;
    Double_t fXBDC, fYBDC, fZBDC;
    Double_t fdXBDC, fdYBDC, fdZBDC;
    TMatrixDSym *fCovMatBDC;

    // Target plane position in mm.
    // Default position is set from the dimension measurement.
    Double_t fTargetX = 0;
    Double_t fTargetY = -213.3;
    Double_t fTargetZ = -8.9;

    TFile *fYOffsetCalibrationFile = nullptr;
    TTree *fHitTree = nullptr;
    Float_t fHitDY;
    Float_t fHitDW;
    Short_t fHitRow;
    Short_t fHitLayer;

    TFile *fClusterCovFile = nullptr;
    TTree *fClusterTree = nullptr;
    Float_t fClusterDX;
    Float_t fClusterDY;
    Float_t fClusterDZ;
    Short_t fClusterSize;
    Bool_t fClusterIsLayer;
    Float_t fClusterTLTL;
    Float_t fClusterTATA;
    Float_t fClusterL;

  ClassDef(STGenfitPIDTask, 1)
};

#endif
