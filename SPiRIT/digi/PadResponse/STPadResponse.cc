#include "STPadResponse.hh"

#include "FairRunAna.h"
#include "FairRuntimeDb.h"
#include "FairRootManager.h"

#include <iostream>
#include <fstream>

#include <omp.h>

using namespace std;

ClassImp(STPadResponse);

STPadResponse::STPadResponse()
{
  FairRunAna* ana = FairRunAna::Instance();
  FairRuntimeDb* rtdb = ana->GetRuntimeDb();
  fPar = (STDigiPar*) rtdb->getContainer("STDigiPar");

  fNTBs = fPar -> GetNumTbs();

  fTimeMax = 10000.;    //[ns]

  fXPadPlane = (fPar -> GetPadPlaneX()); // [mm]
  fZPadPlane = (fPar -> GetPadPlaneZ()); // [mm]

  fBinSizeRow = 8;   //[mm]
  fBinSizeLayer = 12;  //[mm]

  fNRows = fXPadPlane/fBinSizeRow; // 108
  fNLayers = fZPadPlane/fBinSizeLayer; // 112

  // type 0 (tot=.982)
  fFillRatio[0][0] = 0.491;
  fFillRatio[0][1] = 0.491;
  fFillRatio[0][2] = 0;

  // type 1 (tot=.999)
  fFillRatio[1][0] = 0.215;
  fFillRatio[1][1] = 0.733;
  fFillRatio[1][2] = 0.051;

  // type 2 (tot=.999)
  fFillRatio[2][0] = 0.051;
  fFillRatio[2][1] = 0.733;
  fFillRatio[2][2] = 0.215;

  /*
  TString  workDir = gSystem -> Getenv("SPIRITDIR");
  TString  integralDataName = workDir + "/parameters/PadResponseIntegral.dat";
  ifstream integralData(integralDataName.Data());

  Double_t x, val;
  Int_t nPoints = 0;

  fPadResponseIntegralData = new TGraph();
  while(integralData >> x >> val) fPadResponseIntegralData -> SetPoint(nPoints++, x, val);

  integralData.close();
  */

  InitializeRawEventDummy();
}

void 
STPadResponse::FindPad(Double_t xElectron, 
                          Int_t zWire, 
                          Int_t &row, 
                          Int_t &layer, 
                          Int_t &type)
{
  row   = floor(xElectron/fBinSizeRow) + fNRows/2;
  layer = floor(zWire/fBinSizeLayer);

  if(row<0 || row>fNRows || layer<0 || layer>fNLayers) {
    type=-1;
    return;
  }

  Int_t d = zWire%fBinSizeLayer;

       if(d==0) type=0;
  else if(d==4) type=1;
  else if(d==8) type=2;
}

void STPadResponse::FillPad(Int_t gain, Double_t x, Double_t t, Int_t zWire)
{
  Int_t row, layer, type;
  FindPad(x, zWire, row, layer, type);

  if(type==-1) return;

  Int_t iTB = floor( t * fNTBs / fTimeMax );

  for(Int_t iLayer=0; iLayer<3; iLayer++){ Int_t jLayer = layer+iLayer-1;
  for(Int_t iRow=0;   iRow<5;   iRow++)  { Int_t jRow   = row+iRow-2;

    //totally depends on how you set pads in STRawEvent
    STPad* pad = fRawEventDummy -> GetPad(jRow*fNLayers+jLayer);
    if(!pad) continue;

    Double_t x1 = jRow*fBinSizeRow - fXPadPlane/2;
    Double_t x2 = (jRow+1)*fBinSizeRow - fXPadPlane/2;
    Double_t content = gain*fFillRatio[type][iLayer]
                      *(TMath::Erf((x2-x)/5.85941)/2
                       -TMath::Erf((x1-x)/5.85941)/2);
                      //*(fPadResponseIntegralData -> Eval(x2-x)
                      // -fPadResponseIntegralData -> Eval(x1-x));
    pad -> SetADC(iTB, content + (pad -> GetADC(iTB)) );
    fIsActivePad[jRow*fNLayers+jLayer] = kTRUE;
  }
  }
}

void
STPadResponse::InitializeRawEventDummy()
{
  fRawEventDummy = new STRawEvent();
  fRawEventDummy -> SetName("RawEventDummy");

  for(Int_t iRow=0; iRow<fNRows; iRow++){ 
    for(Int_t iLayer=0; iLayer<fNLayers; iLayer++){ 

      STPad* pad = new STPad(iRow,iLayer);
      pad -> SetPedestalSubtracted(kTRUE);

      fRawEventDummy -> SetPad(pad);
      delete pad;
    }
  }
}

void
STPadResponse::Init()
{
  for(Int_t iRow=0; iRow<fNRows; iRow++) {
    for(Int_t iLayer=0; iLayer<fNLayers; iLayer++){
      STPad* pad = fRawEventDummy -> GetPad(iRow*fNLayers+iLayer);
      for(int iTB=0; iTB<fNTBs; iTB++) pad -> SetADC(iTB, 0);
      fIsActivePad[iRow*fNLayers+iLayer] = kFALSE;
    }
  }
}

void 
STPadResponse::CloneRawEvent(STRawEvent *rawEvent)
{
  for(Int_t iRow=0; iRow<fNRows; iRow++){
    for(Int_t iLayer=0; iLayer<fNLayers; iLayer++){
      if(!fIsActivePad[iRow*fNLayers+iLayer]) continue;
      STPad* pad = fRawEventDummy -> GetPad(iRow*fNLayers+iLayer);
      rawEvent -> SetPad(pad);
    }
  }
}
