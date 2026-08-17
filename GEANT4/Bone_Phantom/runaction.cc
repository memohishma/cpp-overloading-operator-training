#include "runaction.hh"
#include <fstream>
#include <vector>
#include "G4AccumulableManager.hh"
#include "G4Run.hh"
#include "G4Event.hh"
#include "G4AnalysisManager.hh"
#include "G4SystemOfUnits.hh"
#include "G4Threading.hh"

MyRunAction::MyRunAction()
{
    G4AnalysisManager* man = G4AnalysisManager::Instance();
    man->SetDefaultFileType("csv"); 

    // Ntuple 0: Target
    man->CreateNtuple("Target", "Target"); 
    man->CreateNtupleIColumn("fEvent"); 
    man->CreateNtupleDColumn("PreStepEnergy_keV"); 
    man->CreateNtupleDColumn("PostStepEnergy_keV"); 
    man->CreateNtupleDColumn("fX_m");
    man->CreateNtupleDColumn("fY_m");
    man->CreateNtupleDColumn("fZ_m");
    man->FinishNtuple(0); 

    // Ntuple 1: BSA Output Neutrons
    man->CreateNtuple("Detector", "BSA_Output_Neutrons"); 
    man->CreateNtupleIColumn("fEvent");              
    man->CreateNtupleDColumn("Energy_eV");           
    man->CreateNtupleDColumn("CosTheta");            
    man->CreateNtupleDColumn("fX_cm");               
    man->CreateNtupleDColumn("fY_cm");               
    man->CreateNtupleDColumn("R_cm");                
    man->CreateNtupleDColumn("Weight");              
    man->FinishNtuple(1); 

    // Ntuple 2: BSA Output Gamma
    man->CreateNtuple("GammaOutput", "BSA_Output_Gamma");
    man->CreateNtupleIColumn("fEvent");              
    man->CreateNtupleDColumn("Energy_MeV");          
    man->CreateNtupleDColumn("fX_cm");               
    man->CreateNtupleDColumn("fY_cm");               
    man->CreateNtupleDColumn("Weight");              
    man->FinishNtuple(2);

    // Ntuple 3: Target Interface
    man->CreateNtuple("TargetInterface", "Target_Moderator_Interface");
    man->CreateNtupleIColumn("fEvent");
    man->CreateNtupleDColumn("Energy_eV");
    man->FinishNtuple(3);

    // Ntuple 4: Bone Phantom Dose Profile
    man->CreateNtuple("BoneDose", "Bone_Phantom_Dose");
    man->CreateNtupleIColumn("fEvent");
    man->CreateNtupleDColumn("Edep_MeV");
    man->CreateNtupleDColumn("Depth_cm");
    man->CreateNtupleDColumn("ParticleType"); 
    man->FinishNtuple(4);

    // تسجيل العدادات
    G4AccumulableManager* accumulableManager = G4AccumulableManager::Instance();
    accumulableManager->Register(nTarget);
    accumulableManager->Register(nModerator);
    accumulableManager->Register(nFastFilter);
    accumulableManager->Register(nGamma);
    accumulableManager->Register(nCollimator);
    accumulableManager->Register(nReflector);
    accumulableManager->Register(nDetector);
    
    accumulableManager->Register(nDetectorCurrent);
    accumulableManager->Register(nDetectorFlux);

    accumulableManager->Register(dFastAccumulated);
    accumulableManager->Register(dGammaAccumulated);

    accumulableManager->Register(nThermalFluxCount);
    accumulableManager->Register(nEpithermal);
    accumulableManager->Register(nFast);
}

MyRunAction::~MyRunAction()
{} 

void MyRunAction::BeginOfRunAction(const G4Run*)
{
    G4AccumulableManager* accumulableManager = G4AccumulableManager::Instance();
    accumulableManager->Reset();

    G4AnalysisManager* man = G4AnalysisManager::Instance();
    man->OpenFile("output.csv");
}

void MyRunAction::EndOfRunAction(const G4Run* aRun)
{
    G4AnalysisManager* man = G4AnalysisManager::Instance();
    man->Write();
    man->CloseFile();

    G4AccumulableManager::Instance()->Merge();

    if (G4Threading::IsMasterThread()) 
    {
        G4cout << "\n------------------- Run Complete! -------------------" << G4endl;

        G4double masterThermalWeight    = nThermalFluxCount.GetValue();
        G4double masterEpithermalWeight = nEpithermal.GetValue();
        G4double masterFastWeight       = nFast.GetValue();
        G4double masterGammaWeight      = nGamma.GetValue();

        G4double radius = 6.0 * CLHEP::cm; 
        G4double A_cm2 = (CLHEP::pi * radius * radius) / (CLHEP::cm * CLHEP::cm);
        G4double N_p = aRun->GetNumberOfEventToBeProcessed(); 

        G4double fluxThermal    = masterThermalWeight / (A_cm2 * N_p); 
        G4double fluxEpithermal = masterEpithermalWeight / (A_cm2 * N_p);
        G4double fluxFast       = masterFastWeight / (A_cm2 * N_p);
        G4double fluxGamma      = masterGammaWeight / (A_cm2 * N_p); 

        G4double beamCurrent = 30.0e-3; 
        G4double protonCharge = 1.602176634e-19; 
        G4double protonsPerSecond = beamCurrent / protonCharge; 

        G4double realFluxThermal    = fluxThermal * protonsPerSecond;
        G4double realFluxEpithermal = fluxEpithermal * protonsPerSecond;
        G4double realFluxFast       = fluxFast * protonsPerSecond;
        G4double realFluxGamma      = fluxGamma * protonsPerSecond;

        G4double totalCurrent = nDetectorCurrent.GetValue();
        G4double totalFlux    = nDetectorFlux.GetValue();
        G4double beamDirectionality = (totalFlux > 0.0) ? (totalCurrent / totalFlux) : 0.0;

        G4double doseFastPerEpithermal  = (masterEpithermalWeight > 0.0) ? (dFastAccumulated.GetValue() / masterEpithermalWeight) : 0.0; 
        G4double doseGammaPerEpithermal = (masterEpithermalWeight > 0.0) ? (dGammaAccumulated.GetValue() / masterEpithermalWeight) : 0.0;

        G4double ratioEpithermalFast    = (realFluxFast > 0.0) ? (realFluxEpithermal / realFluxFast) : 0.0;
        G4double ratioThermalEpithermal = (realFluxEpithermal > 0.0) ? (realFluxThermal / realFluxEpithermal) : 0.0;

        G4cout << "\n=====================================================" << G4endl;
        G4cout << "      IAEA BNCT BEAM QUALITY RECOMMENDATIONS METRIC   " << G4endl;
        G4cout << "=====================================================" << G4endl;
        G4cout << std::scientific; 
        
        G4cout << "1. Epithermal Flux (Target: > 1e9 n/cm^2.s) -> Value: " << realFluxEpithermal << " n/cm^2.s" << G4endl;
        
        G4cout << std::defaultfloat;
        G4cout << "2. Phi_epithermal / Phi_Fast     (Target: > 20)   -> Value: " << ratioEpithermalFast << G4endl;
        G4cout << "3. Phi_thermal / Phi_epithermal  (Target: < 0.05) -> Value: " << ratioThermalEpithermal << G4endl;
        
        G4cout << std::scientific;
        G4cout << "4. D_fast / Phi_epithermal  (Target: < 2e-13) -> Value: " << doseFastPerEpithermal << " Gy.cm^2" << G4endl;
        G4cout << "5. D_gamma / Phi_epithermal (Target: < 2e-13) -> Value: " << doseGammaPerEpithermal << " Gy.cm^2" << G4endl;
        
        G4cout << std::defaultfloat;
        G4cout << "6. Beam Directionality (J / Phi) (Target: > 0.7) -> Value: " << beamDirectionality << G4endl;
        G4cout << "=====================================================\n" << G4endl;
    }
}
