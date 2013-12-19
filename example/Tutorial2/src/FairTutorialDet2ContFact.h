#ifndef FAIRTUTORIALDET2CONTFACT_H
#define FAIRTUTORIALDET2CONTFACT_H

#include "FairContFact.h"

class FairContainer;

class FairTutorialDet2ContFact : public FairContFact
{
  private:
    void setAllContainers();
  public:
    FairTutorialDet2ContFact();
    ~FairTutorialDet2ContFact() {}
    FairParSet* createContainer(FairContainer*);
    ClassDef( FairTutorialDet2ContFact,0) // Factory for all MyDet parameter containers
};

#endif  /* !FAIRTUTORIALDETCONTFACT_H */
