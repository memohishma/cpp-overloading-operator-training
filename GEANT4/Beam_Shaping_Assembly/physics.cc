#include "physics.hh"
#include "NeutronHPphysics.hh" // استدعاء الهيدر الجديد

// الحزم القياسية المستقرة
#include "G4EmStandardPhysics.hh"
#include "G4DecayPhysics.hh"
#include "G4RadioactiveDecayPhysics.hh"
#include "G4HadronPhysicsQGSP_BIC_AllHP.hh"


#include "G4BaryonConstructor.hh"
#include "G4BosonConstructor.hh"
#include "G4IonConstructor.hh"
#include "G4LeptonConstructor.hh"
#include "G4MesonConstructor.hh"
#include "G4ShortLivedConstructor.hh"


#include "G4SystemOfUnits.hh"
#include "G4UnitsTable.hh"


MyPhysicsList::MyPhysicsList()
{
    // تسجيل الفيزياء الكهرومغناطيسية والاضمحلال
    SetVerboseLevel(1);

   new G4UnitDefinition("mm2/g", "mm2/g", "Surface/Mass", mm2 / g);
  new G4UnitDefinition("um2/mg", "um2/mg", "Surface/Mass", um * um / mg);

    RegisterPhysics(new G4EmStandardPhysics()); 
    RegisterPhysics(new G4DecayPhysics()); 
    RegisterPhysics(new G4RadioactiveDecayPhysics()); 
    
    // تسجيل حزمة الهادرونات الشاملة
    RegisterPhysics(new G4HadronPhysicsQGSP_BIC_AllHP()); 

    // 🔒 صمام الأمان الثاني: تسجيل الموديل الحراري هنا يضمن تفعيله برمجياً بالوقت القياسي الصحيح
    RegisterPhysics(new NeutronHPphysics("neutronHP_Custom")); 
}

MyPhysicsList::~MyPhysicsList()
{} //destructor (no code required inside)

void MyPhysicsList::ConstructParticle() 
{
  G4BosonConstructor pBosonConstructor;
  pBosonConstructor.ConstructParticle();

  G4LeptonConstructor pLeptonConstructor;
  pLeptonConstructor.ConstructParticle();

  G4MesonConstructor pMesonConstructor;
  pMesonConstructor.ConstructParticle();

  G4BaryonConstructor pBaryonConstructor;
  pBaryonConstructor.ConstructParticle();

  G4IonConstructor pIonConstructor;
  pIonConstructor.ConstructParticle();

  G4ShortLivedConstructor pShortLivedConstructor;
  pShortLivedConstructor.ConstructParticle();
}

void MyPhysicsList::SetCuts()
{
  SetCutValue(0.1 * um, "proton");
  SetCutValue(0.1 * um, "neutron");
  SetCutValue(0.1 * um, "gamma");
}
