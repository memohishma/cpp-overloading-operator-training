 GNU nano 7.2                                                                          generator.cc                                                                                   
#include "generator.hh"

MyGenerator::MyGenerator()
{
        fParticleGun = new G4ParticleGun(1);

        /*G4ParticleTable *particleTable = G4ParticleTable::GetParticleTable();
        G4String particleName="neutron";
        G4ParticleDefinition *particle = particleTable->FindParticle("neutron");

        fParticleGun->SetParticleDefinition(particle);
*/
}
MyGenerator::~MyGenerator()
{
        delete fParticleGun;
}
void MyGenerator::GeneratePrimaries(G4Event *anEvent)
{
        G4ParticleTable *particleTable = G4ParticleTable::GetParticleTable();
        G4String particleName="neutron";
        G4ParticleDefinition *particle = particleTable->FindParticle("neutron");

        G4ThreeVector pos(0. ,0. ,-1.*cm);
        G4ThreeVector mom(0. ,0. , 1.);

        fParticleGun->SetParticlePosition(pos);
        fParticleGun->SetParticleMomentumDirection(mom);
        fParticleGun->SetParticleMomentum(10.*MeV);
        fParticleGun->SetParticleDefinition(particle);

        fParticleGun->GeneratePrimaryVertex(anEvent);

}
