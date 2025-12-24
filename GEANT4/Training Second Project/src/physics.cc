#include "physics.hh"

MyPhysicsList::MyPhysicsList()
{
     RegisterPhysics(new G4EmStandardPhysics());
     RegisterPhysics(new G4DecayPhysics());
     RegisterPhysics(new G4RadioactiveDecayPhysics());
     RegisterPhysics(new G4HadronElasticPhysicsHP());
     RegisterPhysics(new G4HadronPhysicsQGSP_BIC_HP());
     RegisterPhysics(new G4NeutronTrackingCut());

}
MyPhysicsList::~MyPhysicsList()
{}

