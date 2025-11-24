#include "generator.hh"


MyPrimaryGenerator::MyPrimaryGenerator()
{
    fParticleGun = new G4ParticleGun(1);
    //---------------------------------------------------------------------------------------
    G4ParticleTable *particleTable = G4ParticleTable::GetParticleTable();
    G4String particleName="proton";
    G4ParticleDefinition *particle = particleTable->FindParticle("proton");

    G4ThreeVector pos(0. ,0. ,0. );
    G4ThreeVector mom(0. ,0. ,1. );

    fParticleGun->SetParticlePosition(pos);
    fParticleGun->SetParticleMomentumDirection(mom);
    fParticleGun->SetParticleMomentum(100.*GeV);
    fParticleGun->SetParticleDefinition(particle);
//--------------------------------------------------------------------------------------//we added these here!
}
MyPrimaryGenerator::~MyPrimaryGenerator()
{
    delete fParticleGun;
}
void MyPrimaryGenerator::GeneratePrimaries(G4Event *anEvent)
{
     /* G4ParticleTable *particleTable = G4ParticleTable::GetParticleTable();
    G4String particleName="proton";
    G4ParticleDefinition *particle = particleTable->FindParticle("proton");

    G4ThreeVector pos(0. ,0. ,0. );
    G4ThreeVector mom(0. ,0. ,1. );

    fParticleGun->SetParticlePosition(pos);
    fParticleGun->SetParticleMomentumDirection(mom);
    fParticleGun->SetParticleMomentum(100.*GeV);
    fParticleGun->SetParticleDefinition(particle);*///---------->we have to delete all of these if we want to change the momentum and add it to the constructor


    fParticleGun->GeneratePrimaryVertex(anEvent);
}
