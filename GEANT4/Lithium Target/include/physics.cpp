ifndef PHYSICS_HH
#define PHYSICS_HH

/*#include "G4VModularPhysicsList.hh"
#include "G4EmStandardPhysics.hh"
#include "G4DecayPhysics.hh"
#include "G4HadronElasticPhysicsHP.hh"
#include "QBBC.hh"
#include "G4HadronPhysicsQGSP_BIC_HP.hh"
#include "G4StoppingPhysics.hh"
#include "G4IonPhysics.hh"
#include "G4NeutronTrackingCut.hh"*/
#include "Shielding.hh"
class MyPhysicsList : public Shielding
{
        public:

        MyPhysicsList();
         ~MyPhysicsList();


};
#endif


