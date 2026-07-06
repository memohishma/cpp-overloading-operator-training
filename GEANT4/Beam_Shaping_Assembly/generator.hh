#ifndef GENERATOR_HH
#define GENERATOR_HH

#include "G4VUserPrimaryGeneratorAction.hh"
#include "G4ParticleGun.hh"
#include "G4SystemOfUnits.hh"
#include "G4ParticleTable.hh"
#include "Randomize.hh"
#include <CLHEP/Random/Randomize.h>
#include <cmath>

class G4Event;

class MyPrimaryGenerator : public G4VUserPrimaryGeneratorAction
{
    public:
        MyPrimaryGenerator(); 
        ~MyPrimaryGenerator(); 

        virtual void GeneratePrimaries(G4Event* anEvent) override; 

    private:
        G4ParticleGun *fParticleGun; 
};

#endif
