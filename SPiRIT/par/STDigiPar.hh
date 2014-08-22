//---------------------------------------------------------------------
// Description:
//      STDigiPar reads in parameters and stores them for later use.
//
// Author List:
//      Genie Jhang     Korea Univ.            (original author)
//      JungWoo Lee     Korea Univ.
//----------------------------------------------------------------------

#ifndef _STDIGIPAR_H_
#define _STDIGIPAR_H_

// FAIRROOT classes
#include "FairParGenericSet.h"
#include "FairParamList.h"
#include "FairLogger.h"

// SPiRITROOT classes
#include "STGas.hh"

// ROOT classes
#include "TString.h"
#include "TSystem.h"

class STDigiPar : public FairParGenericSet
{
  public:
    // Constructors and Destructors
    STDigiPar(const Char_t *name, const Char_t *title, const Char_t *context);
    ~STDigiPar();

    // Operators
    
    // Getters
    Int_t GetPadPlaneX();
    Int_t GetPadPlaneZ();
    Int_t GetPadSizeX();
    Int_t GetPadSizeZ();
    Double_t GetAnodeWirePlaneY();
    Double_t GetGroundWirePlaneY();
    Double_t GetGatingWirePlaneY();
    STGas *GetGas();
    Int_t GetNumTbs();
    //!< returns the time duration of a time bucket in given sampling time in ns.
    Int_t GetTBTime();
    //!< returns the drift velocity in cm/us.
    Double_t GetDriftVelocity();
    //!< returns the drift length in mm
    Double_t GetDriftLength();
    //!< returns the slice divider
    Int_t GetYDivider();
    virtual Bool_t getParams(FairParamList *paramList);

    TString GetFile(Int_t fileNum);

    // Setters
    virtual void putParams(FairParamList *paramList);

    // Main methods

  private:
    FairLogger *fLogger;

    STGas *fGas;
    TString fGasFileName;

    Bool_t fInitialized;

    Int_t fPadPlaneX;
    Int_t fPadPlaneZ;
    Int_t fPadSizeX;
    Int_t fPadSizeZ;
    Double_t fAnodeWirePlaneY;
    Double_t fGroundWirePlaneY;
    Double_t fGatingWirePlaneY;
    Double_t fEField;
    Int_t fNumTbs;
    Int_t fSamplingRate;
    Double_t fDriftVelocity;
    Double_t fDriftLength;
    Int_t fYDivider;
    Int_t fGasFile;
    Int_t fPadPlaneFile;
    Int_t fPadShapeFile;

  ClassDef(STDigiPar, 1);
};

#endif
