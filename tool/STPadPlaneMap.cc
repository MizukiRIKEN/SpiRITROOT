#include "STPadPlaneMap.hh"
#include <iostream>
using namespace std;

void STPadHitContainer::Clear()
{
  fHitArray.clear();
}

void STPadHitContainer::AddNeighbor(STPadHitContainer *neighbor)
{
  fNeighbors.push_back(neighbor);
}

pads_t *STPadHitContainer::GetNeighbors() { return &fNeighbors; }

void STPadHitContainer::AddHit(STHit *hit)
{
  fHitArray.push_back(hit);
}

void STPadHitContainer::GetHits(hits_t *array)
{
  for (auto hit : fHitArray)
    array -> push_back(hit);
}

STHit *STPadHitContainer::PullOutNextHit()
{
  if (fHitArray.size() == 0)
    return nullptr;

  STHit *hit = fHitArray.back();
  fHitArray.pop_back();
  return hit;
}

STHit *STPadHitContainer::PullOutNextFreeHit()
{
  Int_t numHits = fHitArray.size();
  if (numHits == 0)
    return nullptr;

  for (Int_t i = 0; i < numHits; i++) {
    Int_t idx = numHits-i-1;
    STHit *hit = fHitArray[idx];
    if (hit -> GetNumTrackCands() == 0) {
      fHitArray.erase(fHitArray.begin()+idx);
      return hit;
    }
  }

  return nullptr;
}


void STPadHitContainer::PullOutHits(hits_t *array)
{
  auto numHits = fHitArray.size();
  for (Int_t iHit = 0; iHit < numHits; iHit++) 
  {
    STHit *hit = fHitArray.back();
    fHitArray.pop_back();
    array -> push_back(hit);
  }
}

ClassImp(STPadPlaneMap)

STPadPlaneMap::STPadPlaneMap()
{
  for (Int_t row = 0; row < 108; row++)
    for (Int_t layer = 0; layer < 112; layer++) 
      fPadMap[row][layer] = new STPadHitContainer();

  Int_t drow[]   = {0, 0, 1, -1};
  Int_t dlayer[] = {-1, 1, 0, 0};

  for (Int_t row = 0; row < 108; row++) {
    for (Int_t layer = 0; layer < 112; layer++) {

      for (Int_t i = 0; i < 4; i++) {
        Int_t rowNb = row + drow[i];
        Int_t layerNb = layer + dlayer[i];

        if (rowNb < 0 || rowNb >= 108 || layerNb < 0 || layerNb >= 112 )
          continue;

        fPadMap[row][layer] -> AddNeighbor(fPadMap[rowNb][layerNb]);
      }

    }
  }
}

void STPadPlaneMap::Clear()
{
  for (Int_t row = 0; row < 108; row++)
    for (Int_t layer = 0; layer < 112; layer++) 
      fPadMap[row][layer] -> Clear();

  fNextRow = 107;
  fNextLayer = 90;
  fNextFreeRow = 107;
  fNextFreeLayer = 90;
}

void STPadPlaneMap::AddHit(STHit *hit)
{
  fPadMap[hit->GetRow()][hit->GetLayer()] -> AddHit(hit);
}

void STPadPlaneMap::AddHits(hits_t *hits)
{
  for (auto hit : *hits)
    fPadMap[hit->GetRow()][hit->GetLayer()] -> AddHit(hit);
}

STHit *STPadPlaneMap::PullOutNextHit()
{
  if (fNextRow == 0 && fNextLayer == 0)
    return nullptr;

  STHit *hit = fPadMap[fNextRow][fNextLayer] -> PullOutNextHit();

  if (hit == nullptr) {
    if (fNextRow == 0) {
      fNextLayer--;
      fNextRow = 107;
    } else
      fNextRow--;

    return PullOutNextHit();
  }
  else
    return hit;
}

STHit *STPadPlaneMap::PullOutNextFreeHit()
{
  if (fNextFreeRow == 0 && fNextFreeLayer == 0)
    return nullptr;

  STHit *hit = fPadMap[fNextFreeRow][fNextFreeLayer] -> PullOutNextFreeHit();

  if (hit == nullptr) {
    if (fNextFreeRow == 0) {
      fNextFreeLayer--;
      fNextFreeRow = 107;
    } else
      fNextFreeRow--;

    return PullOutNextFreeHit();
  }
  else
    return hit;
}

void STPadPlaneMap::PullOutNeighborHits(hits_t *hits, hits_t *array, Int_t level)
{
  for (auto hit : *hits)
  {
    auto pad = fPadMap[hit->GetRow()][hit->GetLayer()];
    auto neighbors = pad -> GetNeighbors();
    while (level > 0)
    {
      pads_t neighborsSum;
      for (auto neighbor : *neighbors) {
        auto neighbors2 = pad -> GetNeighbors();
        for (auto neighbor2 : *neighbors2)
          neighborsSum.push_back(neighbor2);
      }
      for (auto neighbor2 : neighborsSum)
        neighbors -> push_back(neighbor2);
      level--;
    }
    for (auto neighbor : *neighbors)
    {
      neighbor -> PullOutHits(array);
    }
  }
}

void STPadPlaneMap::PullOutNeighborHits(STHit *hit, hits_t *array)
{
  auto pad = fPadMap[hit->GetRow()][hit->GetLayer()];
  auto neighbors = pad -> GetNeighbors();
  for (auto neighbor : *neighbors)
  {
    neighbor -> PullOutHits(array);
  }
}

void STPadPlaneMap::PullOutNeighborHits(TVector3 pos, Double_t range, hits_t *array)
{
  Int_t row = CalculateRow(pos.X());
  Int_t layer = CalculateLayer(pos.Z());
  Int_t rowRange = (range+4)/8;
  Int_t layerRange = (range+6)/12;

  for (auto iRow = -rowRange; iRow <= rowRange; iRow++) {
    for (auto iLayer = -layerRange; iLayer <= layerRange; iLayer++) {
      Int_t rowNb = row+iRow;
      Int_t layerNb = layer+iLayer;
      if (rowNb < 0 || rowNb >= 108 || layerNb < 0 || layerNb >= 112 )
        continue;
      fPadMap[rowNb][layerNb] -> PullOutHits(array);
    }
  }
}

void STPadPlaneMap::PullOutPadHits(Int_t row, Int_t layer, hits_t *array)
{
  if (row < 0 || row >= 108 || layer< 0 || layer>= 112 )
    return;

  fPadMap[row][layer] -> PullOutHits(array);
}

void STPadPlaneMap::PullOutRowHits(Int_t row, hits_t *array)
{
  if (row < 0 || row >= 108)
    return;

  for (Int_t layer = 0; layer < 112; layer++) 
    fPadMap[row][layer] -> PullOutHits(array);
}

void STPadPlaneMap::PullOutLayerHits(Int_t layer, hits_t *array)
{
  if (layer< 0 || layer>= 112 )
    return;

  for (Int_t row = 0; row < 108; row++) 
    fPadMap[row][layer] -> PullOutHits(array);
}

void STPadPlaneMap::GetNeighborHits(STHit *hit, hits_t *array)
{
  auto pad = fPadMap[hit->GetRow()][hit->GetLayer()];
  auto neighbors = pad -> GetNeighbors();
  for (auto neighbor : *neighbors)
    neighbor -> GetHits(array);
}

void STPadPlaneMap::GetRowHits(Int_t row, hits_t *array)
{
  if (row < 0 || row >= 108)
    return;

  for (Int_t layer = 0; layer < 112; layer++) 
    fPadMap[row][layer] -> GetHits(array);
}

void STPadPlaneMap::GetLayerHits(Int_t layer, hits_t *array)
{
  if (layer< 0 || layer>= 112 )
    return;

  for (Int_t row = 0; row < 108; row++) 
    fPadMap[row][layer] -> GetHits(array);
}

Int_t STPadPlaneMap::CalculateRow(Double_t x)   { return Int_t((x+432)/8); }
Int_t STPadPlaneMap::CalculateLayer(Double_t z) { return Int_t(z/12); }
Double_t STPadPlaneMap::CalculateX(Int_t row)   { return (row+0.5)*8.-432.; }
Double_t STPadPlaneMap::CalculateZ(Int_t layer) { return (layer+0.5)*12.; }
