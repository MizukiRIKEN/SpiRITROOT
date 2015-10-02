#ifndef STCLUSTERIZERSCAN
#define STCLUSTERIZERSCAN

#include "STClusterizer.hh"

#include "TVector3.h"
#include "TVector2.h"

//#define DEBUG

#include "TCanvas.h"
#include "TH2D.h"
#include "TGraph.h"
#include "TGraphErrors.h"

class STClusterizerScan : public STClusterizer
{
  public:
    STClusterizerScan();
    ~STClusterizerScan();

    void Analyze(STEvent* eventIn, STEvent* eventOut = NULL);
    void SetParameters(Double_t *par);

    void SetVerticalCut(Double_t vCut);
    void SetVerticalCutTbUnit(Double_t vCut);
    void SetHorizontalCut(Double_t hCut);
    void SetHorizontalCutPadUnit(Double_t hCut);

  private:
    std::vector<STHit *> *fHitArray;
    std::vector<STHitCluster *> *fHitClusterArray;

    /**
     * Correlator between hit and cluster.
     * Returns true if hit should be added to the cluster.
     */
    Bool_t CorrelateHC(STHit* hit, STHitCluster* cluster);

    Double_t fVerticalCut;
    Double_t fHorizontalCut;

    void AddClusterToEvent(STEvent* eventOut, STHitCluster* cluster);

    STHitCluster* fClusterTemp;

    TCanvas* fCvs;
    TH2D*    fFrame;
    TGraph*  fGraphHit;
    TGraph*  fGraphAddedHit;
    TGraphErrors* fGraphCurrentCluster;
    TGraphErrors* fGraphCluster;
    TGraphErrors* fGraphFinishedCluster;

  ClassDef(STClusterizerScan, 1)
};


class STHitSortR
{
  public:
    Bool_t operator() (STHit* hit1, STHit* hit2)
    { return (hit1 -> GetPosition()).Mag() < (hit2 -> GetPosition()).Mag(); }
};

#endif
