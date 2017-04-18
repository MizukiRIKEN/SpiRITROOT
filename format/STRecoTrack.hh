#ifndef STRECOTRACK_HH
#define STRECOTRACK_HH

#include "TObject.h"
#include "TVector3.h"
#include "STRecoTrackCand.hh"
#include "STPID.hh"
#include "STdEdxPoint.hh"

#include <vector>
using namespace std;

/**
 * From STRecoTrackCand
 *   STPID::PID fPID;
 *   Double_t fPIDProbability;
 *   TVector3 fMomentum;
 *   TVector3 fPosTargetPlane;
 *   vector<STdEdxPoint> fdEdxPointArray;
*/
class STRecoTrack : public STRecoTrackCand
{
  private:
    Int_t fCharge; ///< Charge of the particle
    Int_t fVertexID; ///< ID from vertex branch

    TVector3 fPosKyotoL; ///< position at extrapolation to left kyoto plane
    TVector3 fPosKyotoR; ///< position at extrapolation to right kyoto plane
    TVector3 fPosKatana; ///< position at extrapolation to katana plane

  public:
    STRecoTrack();
    virtual ~STRecoTrack() {}

    virtual void Clear(Option_t *option = "");

    void SetCharge(Int_t val) { fCharge = val; }
    Int_t GetCharge() { return fCharge; }

    void SetVertexID(Int_t val) { fVertexID = val; }
    Int_t GetVertexID() { return fVertexID; }

    void SetPosKyotoL(TVector3 val) { fPosKyotoL = val; }
    TVector3 GetPosKyotoL() { return fPosKyotoL; }

    void SetPosKyotoR(TVector3 val) { fPosKyotoR = val; }
    TVector3 GetPosKyotoR() { return fPosKyotoR; }

    void SetPosKatana(TVector3 val) { fPosKatana = val; }
    TVector3 GetPosKatana() { return fPosKatana; }


  ClassDef(STRecoTrack, 1);
};

#endif