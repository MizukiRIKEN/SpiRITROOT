#include "STMCPoint.hh"

#include <iostream>
using std::cout;
using std::endl;


// -----   Default constructor   -------------------------------------------
STMCPoint::STMCPoint()
  : FairMCPoint(), fPdg(0)
{
}
// -------------------------------------------------------------------------

// -----   Standard constructor   ------------------------------------------
STMCPoint::STMCPoint(Int_t trackID, Int_t detID,
                                   TVector3 pos, TVector3 mom,
                                   Double_t tof, Double_t length,
                                   Double_t eLoss, Int_t pdg)
  : FairMCPoint(trackID, detID, pos, mom, tof, length, eLoss), fPdg(pdg)
{
}
// -------------------------------------------------------------------------

// -----   Destructor   ----------------------------------------------------
STMCPoint::~STMCPoint() { }
// -------------------------------------------------------------------------

// -----   Public method Print   -------------------------------------------
void STMCPoint::Print(const Option_t* opt) const
{
  cout << "-I- STMCPoint: SPiRIT point for track " << fTrackID
       << " in detector " << fDetectorID << endl;
  cout << "    Position (" << fX << ", " << fY << ", " << fZ
       << ") cm" << endl;
  cout << "    Momentum (" << fPx << ", " << fPy << ", " << fPz
       << ") GeV" << endl;
  cout << "    Time " << fTime << " ns,  Length " << fLength
       << " cm,  Energy loss " << fELoss*1.0e06 << " keV" << endl;
}
// -------------------------------------------------------------------------

ClassImp(STMCPoint)

