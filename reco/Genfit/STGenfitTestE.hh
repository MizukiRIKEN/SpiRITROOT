#ifndef STGENFITTESTE
#define STGENFITTESTE

// SPiRITROOT classes
#include "STHitCluster.hh"
#include "STSpacepointMeasurement.hh"
#include "STEvent.hh"
#include "STRiemannTrack.hh"
#include "STRiemannHit.hh"
#include "STTrack.hh"

// GENFIT2 classes
#include "AbsKalmanFitter.h"
#include "KalmanFitterRefTrack.h"
#include "DAF.h"
#include "GFRaveVertexFactory.h"
#include "ConstField.h"
#include "FieldManager.h"
#include "MaterialEffects.h"
#include "TGeoMaterialInterface.h"
#include "MeasurementFactory.h"
#include "MeasurementProducer.h"
#include "EventDisplay.h"

#include "TClonesArray.h" 

class STGenfitTestE
{
  public:
    STGenfitTestE();
    ~STGenfitTestE();

    void SetTargetPlane(TVector3 position, TVector3 normal);
    void SetMinIterations(Int_t value);
    void SetMaxIterations(Int_t value);

    void Init();
    genfit::Track* FitTrack(STTrack *recoTrack, TClonesArray *hitArray, STRiemannTrack *riemannTrack);
    void SetTrack(STTrack *recotrack, genfit::Track *gfTrack);

  private:
    Bool_t ProcessTrack(genfit::Track *gfTrack);
    void ProcessExtrapolation(STTrack *recoTrack, genfit::Track *gfTrack);
    Bool_t CalculatedEdx(STTrack *recoTrack, genfit::Track *gfTrack);

  private:
    Int_t fTPCDetID;

    TClonesArray *fHitClusterArray;
    TClonesArray *fGenfitTrackArray;

    genfit::AbsKalmanFitter *fKalmanFitter;
    genfit::MeasurementProducer<STHitCluster, genfit::STSpacepointMeasurement> *fMeasurementProducer;
    genfit::MeasurementFactory<genfit::AbsMeasurement> *fMeasurementFactory;

    genfit::SharedPlanePtr fTargetPlane;
    genfit::SharedPlanePtr fKyotoLPlane;
    genfit::SharedPlanePtr fKyotoRPlane;
    genfit::SharedPlanePtr fKatanaPlane;

  ClassDef(STGenfitTestE, 1)
};

#endif