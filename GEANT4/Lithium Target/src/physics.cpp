  GNU nano 7.2                            physics.cc                                      
#include "physics.hh"

MyPhysicsList::MyPhysicsList()
{
        RegisterPhysics(new G4EmStandardPhysics());
        RegisterPhysics(new G4DecayPhysics());
        RegisterPhysics(new G4HadronElasticPhysicsHP());
        RegisterPhysics(new G4HadronPhysicsQGSP_BIC_HP());
        RegisterPhysics(new G4StoppingPhysics());
        RegisterPhysics(new G4IonPhysics());
        RegisterPhysics(new G4NeutronTrackingCut());

}
MyPhysicsList::~MyPhysicsList()
{}
