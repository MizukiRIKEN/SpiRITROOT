//-------------------------------------------------------------
// Description:
//      Pattern recognition = track finding in the SPiRIT-TPC
//      Using the Riemann track fit
//
// Environment:
//      Software developed for the SPiRIT-TPC at RIBF-RIKEN
//
// Author List:
//      Sebastian Neubert    TUM            (original author)
//
// Redesigned by:
//      Genie Jhang          Korea University
//-----------------------------------------------------------

#ifndef STRIEMANNTRACKINGTASK_H
#define STRIEMANNTRACKINGTASK_H

// FairROOT classes
#include "FairTask.h"
#include "FairLogger.h"

// SpiRITROOT classes 
#include "STRiemannTrack.hh"
#include "STHitCluster.hh"

class TClonesArray;
class STRiemannTrackFinder;
class STDigiPar;

class STRiemannTrackingTask : public FairTask
{
  public:
    STRiemannTrackingTask();
    ~STRiemannTrackingTask();

    void SetVerbose(Bool_t value = kTRUE);     /// If set, much logging information will be written in log file.
    void SetPersistence(Bool_t value = kTRUE); /// store processed data into output ROOT file

    void SetSortingParameters(Bool_t sortingMode = kTRUE,  /// false: sort only according to _sorting; true: use internal sorting when adding hits to trackcands
                               Int_t sorting = 1,          /// -1: no sorting, 0: sort Clusters by X, 1: Y, 2: Z, 3: R, 4: distance to interaction point, 5: Phi, -5: -Phi
                            Double_t interactionZ = 0);    /// set if you use sorting = 4

    void SetMultistepParameters(Bool_t doMultistep,      /// do a multistep approach:
                                                         ///  1. find steep tracks (presort clusters along z)
                                                         ///  2. find circle tracks  (presort clusters by angle)
                                                         ///  3. find all other tracks (presort clusters by decreasing radius)
                                UInt_t minHitsZ = 10,    /// minimum number of hits for a track to be found in step 1
                                UInt_t minHitsR = 10,    /// minimum number of hits for a track to be found in step 1
                                UInt_t minHitsPhi = 10); /// minimum number of hits for a track to be found in step 2

    void SetTrkFinderParameters(Double_t proxcut,          /// proximity cut in 3D
                                Double_t helixcut,         /// distance to helix cut
                                  UInt_t minpointsforfit,  /// minimum number of hits in track before a helix is fitted
                                Double_t zStretch = 1.);   /// stretch proximity cut in z direction

    void SetMaxRMS(Double_t value);   /// max RMS of distances to helix for a track to be written out

    void SetMergeTracks(Bool_t mergeTracks = kTRUE);   /// merge tracklets

    void SetMergeCurlers(Bool_t mergeCurlers = kTRUE, Double_t blowUp = 5.);  //!< merge curlers

    void SetTrkMergerParameters(Double_t TTproxcut,    /// proximity cut in 3D
                                Double_t TTdipcut,     /// cut on difference of dip angles of tracklets
                                Double_t TThelixcut,   /// distance of the two helices
                                Double_t TTplanecut);  /// cut on rms of distances of the riemann hits to intersection of the plane with the sphere of a combined fit

    void SetRiemannScale(Double_t riemannscale = 8.7);  /// blow up factor of the riemann sphere
    
    void SkipCrossingAreas(Bool_t value = kTRUE);    /// skip and remove hits which would match to more than one track at highest correlator level

    virtual InitStatus Init();
    virtual void SetParContainers();
    virtual void Exec(Option_t *opt);


  private:
    FairLogger *fLogger;

    Bool_t fVerbose;

    void BuildTracks(STRiemannTrackFinder *trackfinder,
                     std::vector<STHitCluster *> *clusterBuffer,
                     std::vector<STRiemannTrack*> *TrackletList,
                     Int_t sorting,
                     UInt_t minHits,
                     Double_t maxRMS,
                     Bool_t skipCrossingAreas = kTRUE,
                     Bool_t skipAndDelete = kTRUE);

    TClonesArray *fEventHCArray;
    TClonesArray *fEventHCMArray;
    TClonesArray *fMvdArray;
    TClonesArray *fRiemannTrackArray;
    TClonesArray *fRiemannHitArray;

    STDigiPar *fPar;

    std::vector<STHitCluster *> *fClusterBuffer;
    std::vector<STRiemannTrack *> fRiemannList;

    STRiemannTrackFinder *fTrackFinder;
    STRiemannTrackFinder *fTrackFinderCurl;

    Bool_t fIsPersistence;

    Double_t fMaxRadius; /// outer radius of padplane

    Int_t fCounter;

    // tuning parameters for Conformal Map TrackFinder
    Bool_t fSortingMode;
    Int_t fSorting;
    Double_t fInteractionZ;
    
    Double_t fRiemannScale;

    UInt_t fMinPoints;
    Double_t fProxCut;
    Double_t fProxZStretch;
    Double_t fHelixCut;

    Bool_t fMergeTracks;
    Double_t fTTProxCut;
    Double_t fTTDipCut;
    Double_t fTTHelixCut;
    Double_t fTTPlaneCut;

    Bool_t fMergeCurlers;
    Double_t fBlowUp;

    Bool_t fSkipCrossingAreas;

    // parameters for multistep approach
    Bool_t fDoMultiStep;
    UInt_t fMinHitsZ;
    UInt_t fMinHitsR;
    UInt_t fMinHitsPhi;

    Double_t fMaxRMS;

    TString fRiemannTrackBranchName;
    TString fRiemannHitBranchName;

  ClassDef(STRiemannTrackingTask, 1);
};

#endif
