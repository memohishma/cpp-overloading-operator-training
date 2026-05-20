#include "physics.hh"
/*
#include "G4HadronElasticProcess.hh" 
#include "G4HadronInelasticProcess.hh" 
#include "G4NeutronCaptureProcess.hh" 
#include "G4NeutronFissionProcess.hh" 
#include "G4ParticleHPCapture.hh" 
#include "G4ParticleHPCaptureData.hh" 
#include "G4ParticleHPElastic.hh" 
#include "G4ParticleHPElasticData.hh" 
#include "G4ParticleHPFission.hh" 
#include "G4ParticleHPFissionData.hh" 
#include "G4ParticleHPInelastic.hh" 
#include "G4ParticleHPInelasticData.hh" 
#include "G4ParticleHPThermalScattering.hh" 
#include "G4ParticleHPThermalScatteringData.hh" 
#include "G4SystemOfUnits.hh"  

#include "G4HadronElasticPhysicsHP.hh"   // ← هذا هو الدرس من المنشور
#include "G4IonPhysicsPHP.hh"
*/
MyPhysicsList::MyPhysicsList()
{
    //=========================================================================================================================
    //constructor (called when a MyPhysicsList object is created)
    //this funtion adds all of the physical processes to be modeled to the physics list
    //=========================================================================================================================

    RegisterPhysics(new G4EmStandardPhysics()); //electromagnetic standard physics
    RegisterPhysics(new G4DecayPhysics()); //decay physics
    RegisterPhysics(new G4RadioactiveDecayPhysics()); //radioactive decay physics
    RegisterPhysics(new G4HadronPhysicsQGSP_BIC_AllHP()); //hadronic physics model

  //  RegisterPhysics(new G4HadronElasticPhysicsHP());      
  //  RegisterPhysics(new G4IonPhysicsPHP());
}

MyPhysicsList::~MyPhysicsList()
{} //destructor (no code required inside)
