#pragma once 

#include "TObject.h"

class STDriftedElectron : public TObject
{
  public :

    /** Default constructor **/
    STDriftedElectron();

    STDriftedElectron(Double_t x,
                        Double_t z,
                        Double_t zWire,
                        Double_t time,
                        Int_t    gain);

    /** Default destructor **/
    ~STDriftedElectron();

    //Getters
    Double_t GetX();
    Double_t GetZ();
       Int_t GetZWire();
    Double_t GetTime();
       Int_t GetGain();

    //Setters
    void SetIndex(Int_t index);
    
  private :
    Double_t fX;     /// x position [mm]
    Double_t fZ;     /// z position [mm]
    Int_t    fZWire; /// z position of wire where electron is absorbed [mm]
    Double_t fTime;  /// arrival time on wire plane [ns]
    Int_t    fGain;  /// amound of gain in wire plane

    Int_t    fIndex; /// position of STDriftedElectron in "TClonesArray"

  ClassDef(STDriftedElectron, 1)
};
