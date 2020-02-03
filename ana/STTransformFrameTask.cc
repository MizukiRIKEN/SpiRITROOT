#include "STTransformFrameTask.hh"

// FAIRROOT classes
#include "FairRootManager.h"
#include "FairRunOnline.h"
#include "FairRun.h"
#include "FairRunAna.h"
#include "FairRuntimeDb.h"

// ROOT classes
#include "TDatabasePDG.h"

#include <iostream>

ClassImp(STTransformFrameTask);

STTransformFrameTask::STTransformFrameTask() : fTargetMass(0), fDoRotation(false)
{ 
  fLogger = FairLogger::GetLogger(); 
  fCMVector = new STVectorVec3();
  fFragRapidity = new STVectorF();
  fBeamRapidity = new STVectorF();
}

STTransformFrameTask::~STTransformFrameTask()
{}

InitStatus STTransformFrameTask::Init()
{
  FairRootManager *ioMan = FairRootManager::Instance();
  if (ioMan == 0) {
    fLogger -> Error(MESSAGE_ORIGIN, "Cannot find RootManager!");
    return kERROR;
  }

  fPDG = (STVectorI*) ioMan -> GetObject("PDG");
  fData = (TClonesArray*) ioMan -> GetObject("STData");

  ioMan -> Register("CMVector", "ST", fCMVector, fIsPersistence);
  ioMan -> Register("FragRapidity", "ST", fFragRapidity, fIsPersistence);
  fBeamRapidity -> fElements.push_back(0);
  ioMan -> Register("BeamRapidity", "ST", fBeamRapidity, fIsPersistence);
  return kSUCCESS;
}

void STTransformFrameTask::SetParContainers()
{
  FairRunAna *run = FairRunAna::Instance();
  if (!run)
    fLogger -> Fatal(MESSAGE_ORIGIN, "No analysis run!");

  FairRuntimeDb *db = run -> GetRuntimeDb();
  if (!db)
    fLogger -> Fatal(MESSAGE_ORIGIN, "No runtime database!");

  fPar = (STDigiPar *) db -> getContainer("STDigiPar");
  if (!fPar)
    fLogger -> Fatal(MESSAGE_ORIGIN, "Cannot find STDigiPar!");
}

void STTransformFrameTask::Exec(Option_t *opt)
{
  fCMVector -> fElements.clear();
  fFragRapidity -> fElements.clear();

  auto data = (STData*) fData -> At(0);

  int beamMass = (data -> aoq)*(data -> z) + 0.5;
  double energyPerN = data -> beamEnergyTargetPlane;
  double EBeam = energyPerN*beamMass + beamMass*fNucleonMass;
  double PBeam = sqrt(EBeam*EBeam - beamMass*beamMass*fNucleonMass*fNucleonMass);
  TLorentzVector LV(0,0,PBeam,EBeam);
  double beta = PBeam/(LV.Gamma()*beamMass*fNucleonMass + fTargetMass*fNucleonMass);
  auto vBeam = TVector3(0,0,-beta);
  fBeamRapidity -> fElements[0] = LV.Rapidity();

  // beam rotation
  TVector3 beamDirection(TMath::Tan(data -> proja/1000.), TMath::Tan(data ->projb/1000.),1.);
  beamDirection = beamDirection.Unit();
  auto rotationAxis = beamDirection.Cross(TVector3(0,0,1));
  auto rotationAngle = beamDirection.Angle(TVector3(0,0,1));

  int npart = data -> multiplicity;
  for(int part = 0; part < npart; ++part)
  {
    if(auto particle = TDatabasePDG::Instance()->GetParticle(fPDG -> fElements[part]))
    {
      int ParticleZ = particle -> Charge()/3; // TParticlePDG define charge in units of |e|/3, probably to accomodate quarks
      double ParticleMass = particle -> Mass()*1000; // GeV to MeV

      auto mom = data -> vaMom[part]*ParticleZ;
      if(fDoRotation) mom.Rotate(rotationAngle, rotationAxis);

      TLorentzVector pCM(mom.x(), mom.y(), mom.z(), sqrt(mom.Mag2() + ParticleMass*ParticleMass));
      pCM.Boost(vBeam);
      fCMVector -> fElements.emplace_back(pCM.Px(), pCM.Py(), pCM.Pz());
      fFragRapidity -> fElements.push_back(pCM.Rapidity()); 
    }
    else 
    {
      fCMVector -> fElements.emplace_back(0,0,0);
      fFragRapidity -> fElements.push_back(0);
    }
  }
}

void STTransformFrameTask::SetPersistence(Bool_t value)                                              { fIsPersistence = value; }
void STTransformFrameTask::SetTargetMass(int tgMass)                                                 { fTargetMass = tgMass; }
void STTransformFrameTask::SetDoRotation(bool doRotate)                                              { fDoRotation = true; }