//-----------------------------------------------------------------
// Description:
//      Conformal Mapping of a hit
//      (x,y) -> (r,phi)-> riemann sphere
//
// Environment:
//      Software developed for the SpiRIT-TPC at RIBF-RIKEN
//
// Author List:
//      Genie Jhang          Korea University
//-----------------------------------------------------------------

#ifndef STRIEMANNHIT_HH
#define STRIEMANNHIT_HH

// SpiRITROOT classes
#include "STHitCluster.hh"

// ROOT classes
#include "TObject.h"
#include "TVector3.h"

// FairRoot classes
#include "FairLogger.h"

class STRiemannHit : public TObject
{
  public:
    STRiemannHit();
    STRiemannHit(Double_t riemannScale);
    STRiemannHit(Double_t r, Double_t phi, Double_t riemannScale = 86.1);
    STRiemannHit(STHitCluster *cluster, Double_t riemannScale = 86.1);
    ~STRiemannHit();

    const TVector3 &GetX() const;
    STHitCluster *GetCluster() const;
    Double_t GetS() const;
    Double_t GetAngleOnHelix() const;
    Double_t GetRiemannScale() const;
    Double_t GetZ() const;

    Double_t SetS(Double_t value);
    Double_t SetAngleOnHelix(Double_t value);

  private:
    void InitVariables(Double_t r, Double_t phi, Double_t riemannScale = 86.1);

    TVector3 fX;            /// Position on Riemann sphere in cartesian coordinates
    STHitCluster *fCluster; /// STHitCluster pointer, no ownership over this pointer!
    Double_t fS;            /// Pathlength along track
    Double_t fAngleOnHelix; /// Angle on helix
    Double_t fRiemannScale; /// Scaling in xy plane: 8.7 for prototype, 24.6 for panda. What about our case?

    FairLogger *fLogger;    //!

  ClassDef(STRiemannHit, 1);
};

#endif
