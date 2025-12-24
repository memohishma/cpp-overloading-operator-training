ifndef PHYSICS_HH
#define PHYSICS_HH

#include "G4VModularPhysicsList.hh"

//EM
#include "G4EmStandardPhysics.hh"
//Decays
#include "G4DecayPhysics.hh"
#include "G4RadioactiveDecayPhysics.hh"
//Hadronic(Neutrons)
#include "G4HadronElasticPhysicsHP.hh"
#include "G4HadronPhysicsQGSP_BIC_HP.hh"
#include "G4NeutronTrackingCut.hh"

class MyPhysicsList : public G4VModularPhysicsList
{
    public:
     MyPhysicsList();
     ~MyPhysicsList();



};
#endif


