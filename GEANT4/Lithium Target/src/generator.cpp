#include "generator.hh"

MyGenerator::MyGenerator()
{
        fParticleGun = new G4ParticleGun(1);

        G4ParticleTable *particleTable = G4ParticleTable::GetParticleTable();
        G4String particleName="proton";
        G4ParticleDefinition *particle = particleTable->FindParticle("proton>

        G4ThreeVector pos(0.,0.,-1.*mm);
        G4ThreeVector mom(0.,0.,1.);

        fParticleGun->SetParticlePosition(pos);
        fParticleGun->SetParticleMomentumDirection(mom);
        fParticleGun->SetParticleEnergy(3.0*MeV);
        fParticleGun->SetParticleDefinition(particle);


}
MyGenerator::~MyGenerator()
{
        delete fParticleGun;
}
void MyGenerator::GeneratePrimaries(G4Event *anEvent)
{

        fParticleGun->GeneratePrimaryVertex(anEvent);

}



