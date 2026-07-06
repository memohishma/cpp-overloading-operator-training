#ifndef PHYSICS_HH
#define PHYSICS_HH 1

#include "G4VModularPhysicsList.hh"
#include "globals.hh"

class MyPhysicsList : public G4VModularPhysicsList
{
public:
    MyPhysicsList();
    virtual ~MyPhysicsList();

    virtual void ConstructParticle() override; 
    virtual void SetCuts() override;
};

#endif
