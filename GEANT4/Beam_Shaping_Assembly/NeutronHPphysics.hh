#ifndef NeutronHPphysics_h
#define NeutronHPphysics_h 

#include "G4VPhysicsConstructor.hh"
#include "globals.hh"

class G4GenericMessenger;

class NeutronHPphysics : public G4VPhysicsConstructor
{
  public:
    NeutronHPphysics(const G4String& name = "neutronHP_Custom");
    ~NeutronHPphysics() override;

    void ConstructParticle() override {};
    void ConstructProcess() override;

  private:
    void DefineCommands();

    G4bool fThermal = true; // تفعيل التشتت الحراري تلقائياً لـ D2O و الـ BSA
    G4GenericMessenger* fMessenger = nullptr;
};

#endif
