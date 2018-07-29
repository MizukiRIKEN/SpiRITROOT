#ifndef STDEDXPOINT_HH
#define STDEDXPOINT_HH

#include "TObject.h"
#include "TVector3.h"

class STdEdxPoint : public TObject {
  public:
    STdEdxPoint();
    STdEdxPoint(Int_t id, Double_t de, Double_t dx, Double_t length, Bool_t isContinuousHits, Int_t clusterSize);
    virtual ~STdEdxPoint() {};

    virtual void Print(Option_t *option = "") const;

    Int_t fClusterID;   ///< Hit-Cluster ID
    Double_t fdE;       ///< (ADC)
    Double_t fdx;       ///< (mm)
    Double_t fLength;   ///< Length from vertex to this point (mm)
    TVector3 fPosition; ///<

    Bool_t fIsContinuousHits; ///< Hit distribution of the cluster is continuous?
    Int_t fClusterSize;       ///< # of hits used to the cluster

  ClassDef(STdEdxPoint, 3)
};

class STdEdxPointSortByLength {
  public:
    bool operator() (STdEdxPoint p1, STdEdxPoint p2) { return p1.fLength < p2.fLength; }
};

#endif
