// =================================================
//  STPlot Class
//
//  Description:
//    Plot event data
//
//  Genie Jhang ( geniejhang@majimak.com )
//  2013. 08. 24
// =================================================

#include "STPlot.hh"
#include "STRawEvent.hh"
#include "STPad.hh"
#include "STStatic.hh"
#include "STMap.hh"

#include "TCanvas.h"
#include "TStyle.h"
#include "TH1D.h"
#include "TH2D.h"
#include "TLatex.h"

#include <iostream>

ClassImp(STPlot)

STPlot::STPlot()
{
  Clear();
}

STPlot::STPlot(STCore *core)
{
  Clear();

  SetSTCore(core);
}

void STPlot::SetSTCore(STCore *core)
{
  fCore = core;
  fMap = core -> GetSTMap();
  SetNumTbs(fCore -> GetNumTbs());
}

void STPlot::Clear()
{
  fCore = NULL;
  fMap = NULL;

  fEvent = NULL;
  fNumTbs = 512;

  fPadplaneCvs = NULL;
  fPadplaneHist = NULL;
  fPadplaneTitle = "Event display - Event ID: %d";

  fPadCvs = NULL;
  fPadGraph[0] = NULL;
  fPadGraph[1] = NULL;
}

Bool_t STPlot::CheckEvent()
{
  if (fEvent == NULL) {
    std::cerr << "== [STPlot] Event is not set!" << std::endl;
    return 1;
  }

  return 0;
}

// Setters
void STPlot::SetEvent(STRawEvent *anEvent)
{
  fEvent = anEvent;

  std::cerr << "== [STPlot] Default number of time buckets is 512." << std::endl;
}

void STPlot::SetNumTbs(Int_t numTbs)
{
  fNumTbs = numTbs;
}

void STPlot::SetPadplaneTitle(TString title)
{
  fPadplaneTitle = title;
}

void STPlot::DrawPadplane(Int_t eventID)
{
  if (fCore != NULL)
    fEvent = fCore -> GetRawEvent(eventID);
  
  if (CheckEvent())
    return;

  if (fPadplaneHist)
    fPadplaneHist -> Reset();
  else
    PreparePadplaneHist(kDrawPad);

  Int_t numPads = fEvent -> GetNumPads();
  Double_t max = 0;

  for (Int_t iPad = 0; iPad < numPads; iPad++) {
    STPad *aPad = fEvent -> GetPad(iPad);

    Double_t *adc = aPad -> GetADC();

    Double_t maxADC = 0;
    for (Int_t i = 0; i < fNumTbs; i++) {
      if (maxADC < aPad -> GetADC(i))
        maxADC = aPad -> GetADC(i);
    }

    fPadplaneHist -> SetBinContent(aPad -> GetLayer() + 1, aPad -> GetRow() + 1, maxADC);
    if (maxADC > max) max = maxADC;
  }

  fPadplaneHist -> SetTitle(Form(Form("%s", fPadplaneTitle.Data()), fEvent -> GetEventID()));
  fPadplaneHist -> SetMinimum(1.E-10);
  fPadplaneHist -> SetMaximum(4096.);

  fPadplaneCvs -> Modified();
  fPadplaneCvs -> Update();
}

void STPlot::ClickPad(EClickEvent mode)
{
  TObject *select = gPad -> GetCanvas() -> GetClickSelected();

  if (select == NULL ||
      (!(select -> InheritsFrom(TH2::Class())) && !(select -> InheritsFrom(TGraph::Class()))))
    return;

  Int_t xEvent = gPad -> GetEventX();
  Int_t yEvent = gPad -> GetEventY();

  Double_t xAbs = gPad -> AbsPixeltoX(xEvent);
  Double_t yAbs = gPad -> AbsPixeltoY(yEvent);

  Double_t xOnClick = gPad -> PadtoX(xAbs);
  Double_t yOnClick = gPad -> PadtoY(yAbs);

  Int_t bin = fPadplaneHist -> FindBin(xOnClick, yOnClick);

  gPad -> SetUniqueID(bin);
  gPad -> GetCanvas() -> SetClickSelected(NULL);

  Int_t row = (yOnClick + 432)/8;
  Int_t layer = xOnClick/12;

  if (mode == kDrawPad)
    DrawPad(row, layer);
}

void STPlot::DrawPad(Int_t row, Int_t layer)
{
  if (fPadCvs == NULL)
    PreparePadCanvas();
  else {
    fPadGraph[0] -> Set(0);
    fPadGraph[1] -> Set(0);
  }

  STPad *pad = fEvent -> GetPad(row, layer);
  if (!pad) {
    std::cerr << "== [STPlot] There's no pad (" << row << ", " << layer << ")!" << std::endl;
    return;
  }

  Int_t *tempRawAdc = pad -> GetRawADC();
  Double_t tb[512] = {0};
  Double_t rawAdc[512] = {0};
  for (Int_t iTb = 0; iTb < fNumTbs; iTb++) {
    tb[iTb] = iTb;
    rawAdc[iTb] = tempRawAdc[iTb];
  }

  fPadGraph[0] = new TGraph(fNumTbs, tb, rawAdc);
  fPadGraph[0] -> SetTitle(Form("Raw ADC - (row=%d, layer=%d)", row, layer));
  fPadGraph[0] -> SetLineColor(2);
  fPadGraph[0] -> GetHistogram() -> GetXaxis() -> SetTitle("Time bucket");
  fPadGraph[0] -> GetHistogram() -> GetXaxis() -> CenterTitle();
  fPadGraph[0] -> GetHistogram() -> GetXaxis() -> SetLimits(-10, fNumTbs + 10);
  fPadGraph[0] -> GetHistogram() -> GetXaxis() -> SetRangeUser(-10, fNumTbs + 10);
  fPadGraph[0] -> GetHistogram() -> GetYaxis() -> SetTitle("ADC");
  fPadGraph[0] -> GetHistogram() -> GetYaxis() -> CenterTitle();
  fPadGraph[0] -> GetHistogram() -> GetYaxis() -> SetLimits(-10, 4306);
  fPadGraph[0] -> GetHistogram() -> GetYaxis() -> SetRangeUser(-10, 4306);

  fPadCvs -> cd(1);
  fPadGraph[0] -> Draw("AL");
  if (fMap != NULL) {
    Int_t uaIdx, coboIdx, asadIdx, agetIdx, chIdx;
    fMap -> GetMapData(row, layer, uaIdx, coboIdx, asadIdx, agetIdx, chIdx);

    TLatex *text = new TLatex();
    text -> DrawLatexNDC(0.27, 0.85, Form("UA%03d, C%02d, As%d, Ag%d, Ch%d", uaIdx, coboIdx, asadIdx, agetIdx, chIdx));
  }

  Double_t *adc = pad -> GetADC();
  fPadGraph[1] = new TGraph(fNumTbs, tb, adc);
  fPadGraph[1] -> SetTitle(Form("ADC(FPN subtracted) - (row=%d, layer=%d)", row, layer));
  fPadGraph[1] -> SetLineColor(2);
  fPadGraph[1] -> GetHistogram() -> GetXaxis() -> SetTitle("Time bucket");
  fPadGraph[1] -> GetHistogram() -> GetXaxis() -> CenterTitle();
  fPadGraph[0] -> GetHistogram() -> GetXaxis() -> SetLimits(-10, fNumTbs + 10);
  fPadGraph[0] -> GetHistogram() -> GetXaxis() -> SetRangeUser(-10, fNumTbs + 10);
  fPadGraph[1] -> GetHistogram() -> GetYaxis() -> SetTitle("ADC");
  fPadGraph[1] -> GetHistogram() -> GetYaxis() -> CenterTitle();
  fPadGraph[1] -> GetHistogram() -> GetYaxis() -> SetLimits(-10, 4106);
  fPadGraph[1] -> GetHistogram() -> GetYaxis() -> SetRangeUser(-10, 4106);

  fPadCvs -> cd(2);
  fPadGraph[1] -> Draw("AL");

  fPadCvs -> Modified();
  fPadCvs -> Update();
}

void STPlot::DrawLayer(Int_t layerNo)
{
  std::cerr << "== [STPlot] Not Implemented!" << std::endl;

  if (fLayerHist != NULL)
    delete fLayerHist;

  if (CheckEvent())
    return;
}

TCanvas *STPlot::GetPadplaneCanvas()
{
  PreparePadplaneHist();

  return fPadplaneCvs;
}

// Getters

// -------------------------
void STPlot::PreparePadplaneHist(EClickEvent mode)
{
//    cvs = new TCanvas("Event Display", "", 1600, 1000); // For large monitor
  gStyle -> SetOptStat(0000);
  gStyle -> SetPadRightMargin(0.10);
  gStyle -> SetPadLeftMargin(0.06);
  gStyle -> SetPadTopMargin(0.08);
  gStyle -> SetPadBottomMargin(0.08);
  gStyle -> SetTitleOffset(1.0, "X");
  gStyle -> SetTitleOffset(0.85, "Y");

  fPadplaneCvs = new TCanvas("Event Display", "", 1200, 750);
  fPadplaneCvs -> SetName(Form("EventDisplay_%lx", (Long_t)fPadplaneCvs));
  if (mode == kDrawPad)
    fPadplaneCvs -> AddExec("DrawPad", Form("((STPlot *) STStatic::MakePointer(%ld)) -> ClickPad(STPlot::kDrawPad)", (Long_t)this));
  fPadplaneCvs -> Draw();

  fPadplaneCvs -> cd();
  fPadplaneHist = new TH2D("fPadplaneHist", ";z (mm);x (mm)", 112, 0, 1344, 108, -432, 432);
  fPadplaneHist -> SetName(Form("fPadplaneHist_%lx", (Long_t)fPadplaneHist));
  fPadplaneHist -> SetTitle(Form("%s", fPadplaneTitle.Data()));
  fPadplaneHist -> GetXaxis() -> SetTickLength(0.01);
  fPadplaneHist -> GetXaxis() -> CenterTitle();
  fPadplaneHist -> GetYaxis() -> SetTickLength(0.01);
  fPadplaneHist -> GetYaxis() -> CenterTitle();
  fPadplaneHist -> Draw("colz");

  Double_t padLX = 8; // mm
  Double_t padLZ = 12; // mm

  Double_t x[2], y[2];
  for (Int_t i = 0; i < 108; i++) {
    x[0] = 0;
    x[1] = 1344;
    y[0] = -432 + (i + 1)*padLX;
    y[1] = -432 + (i + 1)*padLX;

    TGraph *graph = new TGraph(2, x, y);
    graph -> SetLineColorAlpha(kBlack, 0.3);
    graph -> SetEditable(kFALSE);

    if ((i + 1)%9 == 0)
      graph -> SetLineStyle(1);
    else
      graph -> SetLineStyle(3);

    graph -> Draw("L SAME");
  }

  for (Int_t i = 0; i < 111; i++) {
    x[0] = (i + 1)*padLZ;
    x[1] = (i + 1)*padLZ;
    y[0] = -432;
    y[1] = 432;

    TGraph *graph = new TGraph(2, x, y);
    graph -> SetLineColorAlpha(kBlack, 0.3);
    graph -> SetEditable(kFALSE);

    if ((i + 1)%7 == 0)
        graph -> SetLineStyle(1);
    else
        graph -> SetLineStyle(3);

    graph -> Draw("L SAME");
  }

  fPadplaneCvs -> Modified();
  fPadplaneCvs -> Update();
}

void STPlot::PreparePadCanvas()
{
  gStyle -> SetOptStat(0000);
  gStyle -> SetPadRightMargin(0.03);
  gStyle -> SetPadLeftMargin(0.16);
  gStyle -> SetPadTopMargin(0.09);
  gStyle -> SetPadBottomMargin(0.11);
  gStyle -> SetTitleOffset(1.05, "X");
  gStyle -> SetTitleOffset(1.75, "Y");
  gStyle -> SetTitleSize(0.05, "X");
  gStyle -> SetTitleSize(0.05, "Y");
  gStyle -> SetLabelSize(0.05, "X");
  gStyle -> SetLabelSize(0.05, "Y");

  fPadCvs = new TCanvas("fPadCvs", "", 1100, 550);
  fPadCvs -> SetName(Form("fPadCvs_%lx", (Long_t)fPadCvs));
  fPadCvs -> Divide(2, 1);
  fPadCvs -> Draw();
}
