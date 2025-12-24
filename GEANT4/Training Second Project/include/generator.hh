  GNU nano 7.2                                                                          generator.hh                                                                                   
#ifndef GENERATOR_HH
#define GENERATOR_HH

#include "G4VUserPrimaryGeneratorAction.hh"
#include "G4SystemOfUnits.hh"
#include "G4ParticleTable.hh"
#include "G4ParticleGun.hh"
#include "G4ThreeVector.hh"

class MyGenerator : public G4VUserPrimaryGeneratorAction
{
   public:
        MyGenerator();
        ~MyGenerator();

        virtual void GeneratePrimaries(G4Event* anEvent) override ;


        G4ParticleGun *fParticleGun;

};
#endif


