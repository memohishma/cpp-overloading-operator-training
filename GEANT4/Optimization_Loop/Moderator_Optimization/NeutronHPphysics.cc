#include "NeutronHPphysics.hh"
#include "G4GenericMessenger.hh"
#include "G4ParticleDefinition.hh"
#include "G4ProcessManager.hh"
#include "G4ProcessTable.hh"
#include "G4Neutron.hh"

// Processes
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

NeutronHPphysics::NeutronHPphysics(const G4String& name) : G4VPhysicsConstructor(name)
{
    DefineCommands();
}

NeutronHPphysics::~NeutronHPphysics()
{
    delete fMessenger;
}

void NeutronHPphysics::ConstructProcess()
{
    G4ParticleDefinition* neutron = G4Neutron::Neutron();
    G4ProcessManager* pManager = neutron->GetProcessManager();

    // 🛑 صمام الأمان الأول لمنع الـ Segmentation Fault داخل الـ Worker Threads
    if (!pManager) return;

    // تنظيف العمليات القديمة إن وجدت بأمان
    G4VProcess* process = nullptr;
    
    process = pManager->GetProcess("hadElastic");
    if (process) pManager->RemoveProcess(process);
    
    process = pManager->GetProcess("neutronInelastic");
    if (process) pManager->RemoveProcess(process);
    
    process = pManager->GetProcess("nCapture");
    if (process) pManager->RemoveProcess(process);
    
    process = pManager->GetProcess("nFission");
    if (process) pManager->RemoveProcess(process);

    // 1. إعادة بناء عملية التشتت المرن (Elastic) مع النموذج الحراري S(alpha, beta)
    G4HadronElasticProcess* process1 = new G4HadronElasticProcess();
    pManager->AddDiscreteProcess(process1);
    
    G4ParticleHPElastic* model1a = new G4ParticleHPElastic();
    process1->RegisterMe(model1a);
    process1->AddDataSet(new G4ParticleHPElasticData());

    if (fThermal) {
        model1a->SetMinEnergy(4 * eV); // ترك المجال تحت 4 إلكترون فولت للموديل الحراري
        G4ParticleHPThermalScattering* model1b = new G4ParticleHPThermalScattering();
        process1->RegisterMe(model1b);
        process1->AddDataSet(new G4ParticleHPThermalScatteringData());
    }

    // 2. عملية التشتت غير المرن (Inelastic)
    G4HadronInelasticProcess* process2 = new G4HadronInelasticProcess("neutronInelastic", G4Neutron::Definition());
    pManager->AddDiscreteProcess(process2);
    process2->AddDataSet(new G4ParticleHPInelasticData());
    G4ParticleHPInelastic* model2 = new G4ParticleHPInelastic();
    process2->RegisterMe(model2);

    // 3. عملية الالتقاط النيوتروني (Capture) الهامة جداً لحسابات الفلاتر والجرعات
    G4NeutronCaptureProcess* process3 = new G4NeutronCaptureProcess();
    pManager->AddDiscreteProcess(process3);
    process3->AddDataSet(new G4ParticleHPCaptureData());
    G4ParticleHPCapture* model3 = new G4ParticleHPCapture();
    process3->RegisterMe(model3);

    // 4. عملية الانشطار (Fission)
    G4NeutronFissionProcess* process4 = new G4NeutronFissionProcess();
    pManager->AddDiscreteProcess(process4);
    process4->AddDataSet(new G4ParticleHPFissionData());
    G4ParticleHPFission* model4 = new G4ParticleHPFission();
    process4->RegisterMe(model4);
}

void NeutronHPphysics::DefineCommands()
{
    fMessenger = new G4GenericMessenger(this, "/testhadr/phys/", "physics list commands");
    auto& thermalCmd = fMessenger->DeclareProperty("thermalScattering", fThermal);
    thermalCmd.SetGuidance("set thermal scattering model");
    thermalCmd.SetParameterName("thermal", false);
    thermalCmd.SetStates(G4State_PreInit);
}
