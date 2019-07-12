/**
 * @brief Process drifting of electron from created position to anode wire
 * plane. 
 *
 * @author JungWoo Lee (Korea Univ.)
 *
 * @detail See header file for detail.
 */

// This class & SPiRIT class headers
#include "STSpaceChargeTask.hh"
#include "STProcessManager.hh"

// Fair class header
#include "FairRootManager.h"
#include "FairRunAna.h"
#include "FairRuntimeDb.h"

// STL class headers
#include <cmath>
#include <iostream>
#include <iomanip>
#include <cstdlib>

// Root class headers
#include "TLorentzVector.h"
#include "TString.h"
#include "TRandom.h"
#include "TError.h"

EquationOfMotion::EquationOfMotion(const FieldFunc& t_EField,
                                     const FieldFunc& t_BField,
                                     double t_mu,
                                     double t_wtau) : 
  EField_(t_EField), BField_(t_BField), mu_(t_mu), wtau_(t_wtau) {};

TVector3 EquationOfMotion::operator()(const TVector3& t_pos, double t_t)
{
  /*********************************
  * Equation of Motion is
  * dx/dt = mu/(1+wtau^2)*(E + wtau/|B|(E X B) + wtau^2 (E.B)B/B^2
  ***********************************/
  double factor1 = mu_/(1+wtau_*wtau_);
  auto E = this->EField_(t_pos); // very bad variable naming scheme
  auto B = this->BField_(t_pos); // but who cares
  TVector3 factor2 = E + wtau_/B.Mag()*(E.Cross(B)) + ((wtau_*wtau_*E.Dot(B))/(B.Mag2()))*B;
  return factor1*factor2;
}

TVector3 RK4Stepper(EquationOfMotion& t_eom, const TVector3& t_pos, double t_t, double t_dt)
{
  /********************
  * Runge-Kutta solver
  * y_n+1 = y_n + 1/6*(k1 + 2k2 + 2k3 + k4)
  * Reference: https://en.wikipedia.org/wiki/Runge%E2%80%93Kutta_methods
  *********************/
  TVector3 k1 = t_dt*t_eom(t_pos, t_t);
  TVector3 k2 = t_dt*t_eom(t_pos + 0.5*k1, t_t + t_dt/2);
  TVector3 k3 = t_dt*t_eom(t_pos + 0.5*k2, t_t + t_dt/2);
  TVector3 k4 = t_dt*t_eom(t_pos + k3, t_t + t_dt);

  return t_pos + 1/6.*(k1 + 2*k2 + 2*k3 + k4);
}

ElectronDrifter::ElectronDrifter(double t_dt, EquationOfMotion& t_eom) : dt_(t_dt), eom_(t_eom) {};

TVector3 ElectronDrifter::DriftFrom(const TVector3& t_pos)
{
  t_ = 0;
  TVector3 pos = t_pos;
  while(pos.Y() < ymax_)
  {
    double old_y = pos.Y();
    pos = RK4Stepper(eom_, pos, t_, dt_);
    if(pos.Y() < ymax_) t_ += dt_;
    else t_ += std::fabs(old_y/(pos.Y() - old_y)*dt_);
  }
  return pos;
};

double ElectronDrifter::GetDriftTime() { return t_; };



STSpaceChargeTask::STSpaceChargeTask()
:FairTask("STSpaceChargeTask"),
 fEventID(0),
 fIsDrift(kTRUE),
 fIsPersistence(kTRUE),
 fDispX(nullptr),
 fDispY(nullptr),
 fDispZ(nullptr)
{
  fIsPersistence = kFALSE;
  fLogger->Debug(MESSAGE_ORIGIN,"Defaul Constructor of STSpaceChargeTask");

  // Default value
  std::string vmc_dir = std::getenv("VMCWORKDIR");
  fEFieldFile = vmc_dir + "/input/E-field.root";
  fProj = "132Sn";
  fBeamRate = 3.14e-8;
}

STSpaceChargeTask::~STSpaceChargeTask()
{
  fLogger->Debug(MESSAGE_ORIGIN,"Destructor of STSpaceChargeTask");
  if(fEx) fEx->Delete();
  if(fEy) fEy->Delete();
  if(fEz) fEz->Delete();
}

void 
STSpaceChargeTask::SetParContainers()
{
  fLogger->Debug(MESSAGE_ORIGIN,"SetParContainers of STSpaceChargeTask");

  FairRunAna* ana = FairRunAna::Instance();
  FairRuntimeDb* rtdb = ana->GetRuntimeDb();
  fPar = (STDigiPar*) rtdb->getContainer("STDigiPar");
}

InitStatus 
STSpaceChargeTask::Init()
{
  fLogger->Debug(MESSAGE_ORIGIN,"Initilization of STSpaceChargeTask");

  FairRootManager* ioman = FairRootManager::Instance();

  fMCPointArray = (TClonesArray*) ioman->GetObject("STMCPoint");
  fDispMCPointArray = new TClonesArray("STMCPoint");
  ioman->Register("DispMCPoint","ST",fDispMCPointArray,fIsPersistence);

  if(fIsDrift)
  {
    if(fDispX && fDispY && fDispZ)
      fLogger->Info(MESSAGE_ORIGIN,"Using custom displacement map");
    else
    {
      if(!fBField) fLogger->Fatal(MESSAGE_ORIGIN, "B-field is not loaded");
      else fBField->Init();
      gErrorIgnoreLevel = kFatal;
      fLogger->Info(MESSAGE_ORIGIN, "Begin calculation of electron displacement map");
      this -> fSetEField();

      // Rewrite E and B field in a form that can be read by ElectronDrifter
      auto EFieldWrapper = [this](const TVector3& t_pos)
        {
          double x = t_pos.X();
          double y = t_pos.Y();
          double z = t_pos.Z();
          double ex = fEx->Interpolate(x, y, z);
          double ey = fEy->Interpolate(x, y, z);
          double ez = fEz->Interpolate(x, y, z);
          if(ey >= -10) ey = -127.7;
          return TVector3(ex, ey, ez);
        };

      auto BFieldWrapper = [this](const TVector3& t_pos)
        {
          double x = t_pos.X();
          double y = t_pos.Y();
          double z = t_pos.Z();
          return TVector3(fBField->GetBx(x,y,z)/10,
                          fBField->GetBy(x,y,z)/10,
                          fBField->GetBz(x,y,z)/10);
        };

      // mu= -4.252e4, wtau= -4 was found to reproduce E cross B result
      // still subjected to change, not finalized
      auto eom = EquationOfMotion(EFieldWrapper, BFieldWrapper, -4.252e4, -4); 
      const double dt = 2e-7; // seconds
      auto drifter = ElectronDrifter(dt, eom);

      // create displacement map with the following dimensions:
      // all dimensions are in cm

      double x_min = -50, x_max = 50;
      double y_min = -55, y_max = 0;
      double z_min = 0, z_max = 150;
      int binsx = 40, binsy = 40, binsz = 40;
      double assumed_drift_velocity = fPar->GetDriftVelocity();

      fDispX = new TH3D("shiftX","shiftX",binsx,x_min,x_max,binsy,y_min,y_max,binsz,z_min,z_max);
      fDispY = new TH3D("shiftY","shiftY",binsx,x_min,x_max,binsy,y_min,y_max,binsz,z_min,z_max);
      fDispZ = new TH3D("shiftZ","shiftZ",binsx,x_min,x_max,binsy,y_min,y_max,binsz,z_min,z_max);
       

      int counter = 1;
      int percentage = 0;
      std::cout << "STSpaceChargeTask Displacement map Calculation Percent completion: " << "\t";
      for(int idx = 1; idx <= binsx; ++idx)
       for(int idy = 1; idy <= binsy; ++idy)
         for(int idz = 1; idz <= binsz; ++idz)
         {
           double x = fDispX->GetXaxis()->GetBinCenter(idx);
           double y = fDispX->GetYaxis()->GetBinCenter(idy);
           double z = fDispX->GetZaxis()->GetBinCenter(idz);

           // initial position
           TVector3 pos(x, y, z);
           pos = drifter.DriftFrom(pos);

           fDispX->SetBinContent(idx, idy, idz, (pos.X()-x));
           fDispY->SetBinContent(idx, idy, idz, (pos.Y()+assumed_drift_velocity*drifter.GetDriftTime()));
           fDispZ->SetBinContent(idx, idy, idz, (pos.Z()-z));
           int new_progress =  int(counter/(double) (binsx*binsy*binsz)*100);
           if(new_progress != percentage)
           {
             percentage = new_progress;
             std::cout << "\b\b\b\b\b" << std::setw(3) << percentage << " %" << std::flush;
           }
           ++counter;
        }
      std::cout << std::endl;
    }
  } else fLogger->Info(MESSAGE_ORIGIN, "Space Chrage displacement is disabled");
  return kSUCCESS;
}

void STSpaceChargeTask::fSetEField()
{
  fLogger->Info(MESSAGE_ORIGIN,("Loading E-field solution from " + fEFieldFile + " for projectile " + fProj).c_str());
  TFile file(fEFieldFile.c_str());
  if(!file.IsOpen()) fLogger->Fatal(MESSAGE_ORIGIN, "E-field file cannot be opened!");
  auto homox = static_cast<TH3D*>(file.Get(("homo_" + fProj + "_Ex").c_str()));
  auto homoy = static_cast<TH3D*>(file.Get(("homo_" + fProj + "_Ey").c_str()));
  auto homoz = static_cast<TH3D*>(file.Get(("homo_" + fProj + "_Ez").c_str()));

  auto nohomox = static_cast<TH3D*>(file.Get(("nohomo_" + fProj + "_Ex").c_str()));
  auto nohomoy = static_cast<TH3D*>(file.Get(("nohomo_" + fProj + "_Ey").c_str()));
  auto nohomoz = static_cast<TH3D*>(file.Get(("nohomo_" + fProj + "_Ez").c_str()));

  fLogger->Info(MESSAGE_ORIGIN,TString::Format("E-field is calculated with beam rate = %e", fBeamRate));
  double factor = fBeamRate / 3.14e-8; //3.14e-8 is the beam rate of run 2841
  if(homox && homoy && homoz && nohomox && nohomoy && nohomoz)
  {
    homox->Add((nohomox->Scale(factor), nohomox));
    homoy->Add((nohomoy->Scale(factor), nohomoy));
    homoz->Add((nohomoz->Scale(factor), nohomoz));

    fEx = static_cast<TH3D*>(homox->Clone("shiftX"));
    fEy = static_cast<TH3D*>(homoy->Clone("shiftY"));
    fEz = static_cast<TH3D*>(homoz->Clone("shiftZ"));
    fEx -> SetDirectory(0); // disable memory management for this histogram
    fEy -> SetDirectory(0);
    fEz -> SetDirectory(0);
  }
  else fLogger->Fatal(MESSAGE_ORIGIN, "E-field cannot be loaded from files.");
}

void 
STSpaceChargeTask::Exec(Option_t* option)
{
  fLogger->Debug(MESSAGE_ORIGIN,"Exec of STSpaceChargeTask");

  if(!fDispMCPointArray) 
    fLogger->Fatal(MESSAGE_ORIGIN,"No Drifted MC Point Array!");
  fDispMCPointArray -> Delete();
  Int_t nMCPoints = fMCPointArray->GetEntries();

  /**
   * NOTE! that fMCPoint has unit of [cm] for length scale,
   * [GeV] for energy and [ns] for time.
   */
  for(Int_t iPoint=0; iPoint<nMCPoints; iPoint++) {
    fMCPoint = (STMCPoint*) fMCPointArray->At(iPoint);
    Int_t index = fDispMCPointArray->GetEntriesFast();
    STMCPoint *drifted_mc_point
      = new ((*fDispMCPointArray)[index])
        STMCPoint(*fMCPoint);
    double posx, posy, posz;
    if(fIsDrift)
    {
      this -> fSpaceChargeEffect(fMCPoint->GetX(), fMCPoint->GetY(), fMCPoint->GetZ(), posx, posy, posz);
      drifted_mc_point -> SetXYZ(posx, posy, posz);
    }
  }

  fLogger->Info(MESSAGE_ORIGIN, 
                Form("Event #%d : MC points (%d) found. The are dispaced due to space charge",
                     fEventID++, nMCPoints));


  return;
}

void STSpaceChargeTask::SetCustomMap(TH3D* dispx, TH3D* dispy, TH3D* dispz)
{ fDispX = dispx; fDispY = dispy; fDispZ = dispz; }
void STSpaceChargeTask::SetBField(STFieldMap* Bfield){ fBField = Bfield; }
void STSpaceChargeTask::SetEFieldSolution(const std::string& value)
{
  std::string vmc_dir(std::getenv("VMCWOKRDIR")); 
  fEFieldFile = vmc_dir + "/input/" + value; 
}

void STSpaceChargeTask::SetBeamRate(Double_t value){ fBeamRate = value; }
void STSpaceChargeTask::SetProjectile(const std::string& value){ fProj = value; }

void STSpaceChargeTask::SetPersistence(Bool_t value) { fIsPersistence = value; }
void STSpaceChargeTask::SetVerbose(Bool_t value) { fVerbose = value; }
void STSpaceChargeTask::SetElectronDrift(Bool_t value) { fIsDrift = value; }
void STSpaceChargeTask::fSpaceChargeEffect(double x, double y, double z, 
                       double& x_out, double& y_out, double& z_out)
{
  // remember the unit is now in cm
  x_out = x + fDispX->Interpolate(x, y, z);
  y_out = y + fDispY->Interpolate(x, y, z); 
  z_out = z + fDispZ->Interpolate(x, y, z);

}

void STSpaceChargeTask::ExportDisplacementMap(const std::string& value)
{
  TFile file(value.c_str(), "UPDATE");
  if(fDispX) fDispX->Write();
  if(fDispY) fDispY->Write();
  if(fDispZ) fDispZ->Write();
}

void STSpaceChargeTask::ExportEField(const std::string& value)
{
  TFile file(value.c_str(), "UPDATE");
  if(fEx) fEx->Write();
  if(fEy) fEy->Write();
  if(fEz) fEz->Write();
}
ClassImp(STSpaceChargeTask);