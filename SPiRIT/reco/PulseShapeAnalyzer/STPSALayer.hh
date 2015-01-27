//-----------------------------------------------------------
// Description:
//   This version uses TSpectrum class in ROOT to find
//   peaks in pads and in one layer averages certain number
//   pads around the pad having the highest peak.
//
// Environment:
//   Software developed for the SPiRIT-TPC at RIKEN
//
// Author List:
//   Genie Jhang     Korea University     (original author)
//-----------------------------------------------------------

#pragma once

// SpiRITROOT classes
#include "STPSA.hh"

// ROOT classes
#include "TSpectrum.h"

// STL
#include <vector>

class STPSALayer : public STPSA
{
  public:
    STPSALayer();
    ~STPSALayer();

    void Analyze(STRawEvent *rawEvent, STEvent *event);

  private:
    void Reset();
    void PreAnalyze(STRawEvent *rawEvent);
    void DeletePeakInfo(STRawEvent *rawEvent, Int_t row, Int_t layer, Int_t peakNum);
    void LSLFit(Int_t numPoints, Double_t *x, Double_t *y, Double_t &constant, Double_t &slope);

    TSpectrum *fPeakFinder;                    ///< TSpectrum object

    std::vector<STPad> *fPadArray;             ///< Pad array pointer in STRawEvent

    Int_t fNumSidePads;                        ///< The number of pads to average side of the pad having the highest peak
    Int_t fNumSideTbs;                         ///< The number of tbs to search peak near the maximum peak
    Int_t **fNumPeaks;                         ///< The number of peaks in the fired pad

    Int_t fPeakStorageSize;                    ///< Maximum number of peaks in a pad
    Int_t ***fPeakTbs;                         ///< Peak positions in amplitude increasing order
    Double_t ***fPeakValues;                   ///< Peak amplitude in increasing order

    Int_t fMinPoints;                          ///< Minimum points of slope for determing the hit time
    Int_t fPercPeakMin;                        ///< Minimum percentage of peak for determinig the hit time
    Int_t fPercPeakMax;                        ///< Maximum percentage of peak for determinig the hit time

  ClassDef(STPSALayer, 1)
};
