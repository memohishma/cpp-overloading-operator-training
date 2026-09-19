#include "runaction.hh"
#include <fstream>
#include <vector>
#include "G4AccumulableManager.hh"
#include "G4Run.hh"
#include "G4ScoringManager.hh"
#include "G4SDManager.hh"
#include "G4THitsMap.hh"
#include "G4Event.hh"
#include "G4AnalysisManager.hh"

MyRunAction::MyRunAction()
{
    G4AnalysisManager* man = G4AnalysisManager::Instance();
    man->SetDefaultFileType("csv");
    man->SetNtupleMerging(true);

    // Ntuple 0: Target
    man->CreateNtuple("Target", "Target"); 
    man->CreateNtupleIColumn("fEvent"); 
    man->CreateNtupleDColumn("PreStepEnergy_keV"); 
    man->CreateNtupleDColumn("PostStepEnergy_keV"); 
    man->CreateNtupleDColumn("fX_m");
    man->CreateNtupleDColumn("fY_m");
    man->CreateNtupleDColumn("fZ_m");
    man->FinishNtuple(0); 

    // Ntuple 1: Detector (BSA_Output_Neutrons) -> ID = 1
    man->CreateNtuple("Detector", "BSA_Output_Neutrons"); 
    man->CreateNtupleIColumn("fEvent");                     
    man->CreateNtupleDColumn("Energy_eV");           
    man->CreateNtupleDColumn("CosTheta");              
    man->CreateNtupleDColumn("fX_cm");                   
    man->CreateNtupleDColumn("fY_cm");                   
    man->CreateNtupleDColumn("R_cm");                    
    man->CreateNtupleDColumn("FluxWeight");            
    man->FinishNtuple(1); 

    // Ntuple 2: GammaOutput -> ID = 2
    man->CreateNtuple("GammaOutput", "BSA_Output_Gamma");
    man->CreateNtupleIColumn("fEvent");                     
    man->CreateNtupleDColumn("Energy_MeV");            
    man->CreateNtupleDColumn("fX_cm");                   
    man->CreateNtupleDColumn("fY_cm");                   
    man->CreateNtupleDColumn("FluxWeight");            
    man->FinishNtuple(2);

    // Ntuple 3: TargetInterface -> ID = 3
    man->CreateNtuple("TargetInterface", "Target_Moderator_Interface");
    man->CreateNtupleIColumn("fEvent");
    man->CreateNtupleDColumn("Energy_eV");
    man->FinishNtuple(3);

    man->CreateNtuple("Target_Exit", "Neutrons_After_Target");
    man->CreateNtupleIColumn("fEvent");
    man->CreateNtupleDColumn("Energy_eV");
    man->CreateNtupleDColumn("Weight");
    man->FinishNtuple(4);

    man->CreateNtuple("Moderator_Exit", "Neutrons_After_Moderator");
    man->CreateNtupleIColumn("fEvent");
    man->CreateNtupleDColumn("Energy_eV");
    man->CreateNtupleDColumn("Weight");
    man->FinishNtuple(5);

    man->CreateNtuple("FastFilter_Exit", "Neutrons_After_FastFilter");
    man->CreateNtupleIColumn("fEvent");
    man->CreateNtupleDColumn("Energy_eV");
    man->CreateNtupleDColumn("Weight");
    man->FinishNtuple(6);

    man->CreateNtuple("GammaFilter_Exit", "Neutrons_After_GammaFilter");
    man->CreateNtupleIColumn("fEvent");
    man->CreateNtupleDColumn("Energy_eV");
    man->CreateNtupleDColumn("Weight");
    man->FinishNtuple(7);

    // تسجيل العدادات التراكمية (Accumulables)
    G4AccumulableManager* accumulableManager = G4AccumulableManager::Instance();
    accumulableManager->Register(nTarget);
    accumulableManager->Register(nModerator);
    accumulableManager->Register(nFastFilter);
    accumulableManager->Register(nGammaFilter);
    accumulableManager->Register(nCollimator);
    accumulableManager->Register(nReflector);
    accumulableManager->Register(nDetector);
    
    accumulableManager->Register(dFastFluxAccumulated);
    accumulableManager->Register(dFastCurrentAccumulated);
    accumulableManager->Register(dGammaFluxAccumulated);
    accumulableManager->Register(dGammaCurrentAccumulated);

    accumulableManager->Register(nThermalFluxCount);
    accumulableManager->Register(nEpithermalFlux);
    accumulableManager->Register(nFastFlux);

    accumulableManager->Register(nThermalCurrentCount);
    accumulableManager->Register(nEpithermalCurrent);
    accumulableManager->Register(nFastCurrent);

    accumulableManager->Register(nGammaFlux);
    accumulableManager->Register(nGammaCurrent);
}

MyRunAction::~MyRunAction()
{} 

void MyRunAction::BeginOfRunAction(const G4Run*)
{
    G4AccumulableManager* accumulableManager = G4AccumulableManager::Instance();
    accumulableManager->Reset();

    G4AnalysisManager* man = G4AnalysisManager::Instance();
    man->OpenFile("output.root");
}

void MyRunAction::EndOfRunAction(const G4Run* aRun)
{
    G4AnalysisManager* man = G4AnalysisManager::Instance();
    man->Write();
    man->CloseFile();

    G4AccumulableManager::Instance()->Merge();

    if (G4Threading::IsMasterThread()) 
    {
        G4cout << "------------------- Run Complete! -------------------" << G4endl;

        G4double countTarget     = nTarget.GetValue();
        G4double countModerator  = nModerator.GetValue();
        G4double countFastFilt   = nFastFilter.GetValue();
        G4double countGammaFilt  = nGammaFilter.GetValue();
        G4double numDetectorNeutrons = nDetector.GetValue();  

        // 1. استخراج قيم التيار (No-Cosine)
        G4double masterThermalCurrent     = nThermalCurrentCount.GetValue();
        G4double masterEpithermalCurrent  = nEpithermalCurrent.GetValue();
        G4double masterFastCurrent        = nFastCurrent.GetValue();
        G4double masterGammaCurrent       = nGammaCurrent.GetValue();

        // 2. استخراج قيم الفيض (Cosine-Corrected) للاستخدامات الداخلية وملف الفيض
        G4double masterThermalFlux        = nThermalFluxCount.GetValue();
        G4double masterEpithermalFlux     = nEpithermalFlux.GetValue();
        G4double masterFastFlux           = nFastFlux.GetValue();
        G4double masterGammaFlux          = nGammaFlux.GetValue();

        G4cout << "\n=====================================================" << G4endl;
        G4cout << "         NEUTRON COUNTS AFTER EACH COMPONENT         " << G4endl;
        G4cout << "=====================================================" << G4endl;
        G4cout << "Neutrons exiting Target (toward moderator) : " << countTarget << G4endl;
        G4cout << "Neutrons exiting Moderator   : " << countModerator << G4endl;
        G4cout << "Neutrons exiting Fast Filter : " << countFastFilt << G4endl;
        G4cout << "Neutrons exiting Gamma Filter: " << countGammaFilt << G4endl;
        G4cout << "Total Detector Crossings     : " << numDetectorNeutrons << G4endl;
        G4cout << "=====================================================\n" << G4endl;

        G4cout << "\n=====================================================" << G4endl;
        G4cout << "               🛑 DEBUG COUNTS (CURRENT) 🛑           " << G4endl;
        G4cout << "=====================================================" << G4endl;
        G4cout << "Raw Thermal Surface-Current Sum    (< 0.5 eV)   : " << masterThermalCurrent << G4endl;
        G4cout << "Raw Epithermal Surface-Current Sum(0.5eV-10keV): " << masterEpithermalCurrent << G4endl;
        G4cout << "Raw Fast Surface-Current Sum       (> 10 keV)   : " << masterFastCurrent << G4endl;
        G4cout << "Raw Gamma Surface-Current Sum      (At Boundary): " << masterGammaCurrent << G4endl;
        G4cout << "Raw Detector Crossing Count                     : " << numDetectorNeutrons << G4endl;
        G4cout << "=====================================================\n" << G4endl;

        G4double radius = 7.0 * CLHEP::cm; 
        G4double A_cm2 = (CLHEP::pi * radius * radius) / (CLHEP::cm * CLHEP::cm);
        G4double N_p = aRun->GetNumberOfEventToBeProcessed(); 

        // حسابات التيار المطلقة (per primary)
        G4double curThermal    = masterThermalCurrent / (A_cm2 * N_p); 
        G4double curEpithermal = masterEpithermalCurrent / (A_cm2 * N_p);
        G4double curFast       = masterFastCurrent / (A_cm2 * N_p);
        G4double curGamma      = masterGammaCurrent / (A_cm2 * N_p); 

        // حسابات الفيض المطلقة (per primary) - تُستخدم لملف الـ Flux CSV
        G4double fluxThermal    = masterThermalFlux / (A_cm2 * N_p); 
        G4double fluxEpithermal = masterEpithermalFlux / (A_cm2 * N_p);
        G4double fluxFast       = masterFastFlux / (A_cm2 * N_p);
        G4double fluxGamma      = masterGammaFlux / (A_cm2 * N_p); 

        G4cout << "=====================================================" << G4endl;
        G4cout << "       FINAL PARTICLE CURRENT AT BSA OUTPUT (NO-COS)   " << G4endl;
        G4cout << "=====================================================" << G4endl;
        G4cout << std::scientific; 
        G4cout << "Thermal Current (<0.5 eV)        : " << curThermal    << " n/cm^2.primary" << G4endl;
        G4cout << "Epithermal Current (0.5eV-10keV) : " << curEpithermal << " n/cm^2.primary" << G4endl;
        G4cout << "Fast Current (>10 keV)           : " << curFast       << " n/cm^2.primary" << G4endl;
        G4cout << "-----------------------------------------------------" << G4endl;
        G4cout << "Gamma Contamination Current      : " << curGamma      << " photons/cm^2.primary" << G4endl;
        G4cout << "=====================================================" << G4endl;

        G4double beamCurrent = 30.0e-3; 
        G4double protonCharge = 1.602176634e-19; 
        G4double protonsPerSecond = beamCurrent / protonCharge; 

        // القيم الحقيقية عند 30 mA (للتيار)
        G4double realCurThermal    = curThermal * protonsPerSecond;
        G4double realCurEpithermal = curEpithermal * protonsPerSecond;
        G4double realCurFast       = curFast * protonsPerSecond;
        G4double realCurGamma      = curGamma * protonsPerSecond;

        // القيم الحقيقية عند 30 mA (للفيض) - لملف الـ Flux CSV
        G4double realFluxThermal    = fluxThermal * protonsPerSecond;
        G4double realFluxEpithermal = fluxEpithermal * protonsPerSecond;
        G4double realFluxFast       = fluxFast * protonsPerSecond;
        G4double realFluxGamma      = fluxGamma * protonsPerSecond;

        G4cout << G4endl;
        G4cout << "=====================================================" << G4endl;
        G4cout << "    REAL ABSOLUTE CURRENT AT 30 mA BEAM CURRENT (NO-COS)" << G4endl;
        G4cout << "=====================================================" << G4endl;
        G4cout << "Real Thermal Current            : " << realCurThermal    << " n/cm^2.s" << G4endl;
        G4cout << "Real Epithermal Current (BNCT)  : " << realCurEpithermal << " n/cm^2.s" << G4endl;
        G4cout << "Real Fast Current               : " << realCurFast       << " n/cm^2.s" << G4endl;
        G4cout << "-----------------------------------------------------" << G4endl;
        G4cout << "Real Gamma Contamination Current: " << realCurGamma      << " photons/cm^2.s" << G4endl;
        G4cout << "=====================================================" << G4endl;

        G4double ratioThermalEpithermal = (masterEpithermalCurrent > 0.0) ? (realCurThermal / realCurEpithermal) : 0.0;
        
        // الجرعات تعتمد على التيار
        G4double doseFastPerEpithermal  = (masterEpithermalCurrent > 0.0) ? (dFastCurrentAccumulated.GetValue() / masterEpithermalCurrent) : 0.0; 
        G4double doseGammaPerEpithermal = (masterEpithermalCurrent > 0.0) ? (dGammaCurrentAccumulated.GetValue() / masterEpithermalCurrent) : 0.0;

        // الجرعات للفيض (لملف الـ Flux CSV)
        G4double doseFastPerPhiEpithermal  = (masterEpithermalFlux > 0.0) ? (dFastFluxAccumulated.GetValue() / masterEpithermalFlux) : 0.0; 
        G4double doseGammaPerPhiEpithermal = (masterEpithermalFlux > 0.0) ? (dGammaFluxAccumulated.GetValue() / masterEpithermalFlux) : 0.0;

        // الاتجاهية (J / Phi_epi)
        G4double directionality = (masterEpithermalFlux > 0.0) ? (masterEpithermalCurrent / masterEpithermalFlux) : 0.0;

        G4cout << G4endl;
        G4cout << "=====================================================" << G4endl;
        G4cout << "    IAEA BNCT BEAM QUALITY RECOMMENDATIONS METRIC (CURRENT)" << G4endl;
        G4cout << "=====================================================" << G4endl;
        G4cout << std::defaultfloat; 
        
        G4cout << "1. Epithermal Current (IAEA Target: > 5 e+8 n/cm^2.s) -> Value: " << realCurEpithermal << G4endl;
        
        if (realCurFast > 0.0)
        {
            G4double ratioEpithermalFast = realCurEpithermal / realCurFast;
            G4cout << "2. J_epithermal / J_fast         (IAEA Target: Recommended)-> Value: " << ratioEpithermalFast << G4endl;
        }
        else
        {
            G4cout << "2. J_epithermal / J_fast         (IAEA Target: Recommended)-> Value: undefined (J_fast = 0)" << G4endl;
        }

        G4cout << "3. J_thermal / J_epithermal      (IAEA Target: < 0.05)    -> Value: " << ratioThermalEpithermal << G4endl;
        
        G4cout << std::scientific;
        G4cout << "4. D_fast / J_epithermal    (IAEA Target: < 7e-13)     -> Value: " << doseFastPerEpithermal  << " Gy.cm^2" << G4endl;
        G4cout << "5. D_gamma / J_epithermal   (IAEA Target: < 2e-13)     -> Value: " << doseGammaPerEpithermal << " Gy.cm^2" << G4endl;
        
        G4cout << std::defaultfloat;
        G4cout << "6. Beam Directionality (J / Phi_epi) (IAEA Target: > 0.7)-> Value: " << directionality << G4endl;
        G4cout << "=====================================================" << G4endl;

        G4double moderatorThickness = 0.0; // قم بتعديلها لاحقاً إذا كانت جزءاً من حلقة (Loop)

        // --- 1. حفظ نتائج التيار في ملف moderator_sweep_surface_current.csv ---
        {
            std::ofstream file1("moderator_sweep_surface_current.csv", std::ios::app);
            file1.seekp(0, std::ios::end);
            if (file1.tellp() == 0) {
                file1 << "ModeratorThickness,ThermalCurrent,EpithermalCurrent,FastCurrent,J_epi_per_J_fast,J_thermal_per_J_epi,D_fast_per_J_epi,D_gamma_per_J_epi\n";
            }
            file1 << moderatorThickness << ","
                  << realCurThermal << ","
                  << realCurEpithermal << ","
                  << realCurFast << ","
                  << (realCurFast > 0.0 ? realCurEpithermal / realCurFast : 0.0) << ","
                  << ratioThermalEpithermal << ","
                  << doseFastPerEpithermal << ","
                  << doseGammaPerEpithermal << "\n";
            file1.close();
        }

        // --- 2. حفظ نتائج الفيض في ملف moderator_sweep_surface_flux.csv ---
        {
            std::ofstream file2("moderator_sweep_surface_flux.csv", std::ios::app);
            file2.seekp(0, std::ios::end);
            if (file2.tellp() == 0) {
                file2 << "ModeratorThickness,ThermalFlux,EpithermalFlux,FastFlux,Phi_epi_per_Phi_fast,Phi_thermal_per_Phi_epi,D_fast_per_Phi_epi,D_gamma_per_Phi_epi\n";
            }
            file2 << moderatorThickness << ","
                  << realFluxThermal << ","
                  << realFluxEpithermal << ","
                  << realFluxFast << ","
                  << (realFluxFast > 0.0 ? realFluxEpithermal / realFluxFast : 0.0) << ","
                  << (realFluxEpithermal > 0.0 ? realFluxThermal / realFluxEpithermal : 0.0) << ","
                  << doseFastPerPhiEpithermal << ","
                  << doseGammaPerPhiEpithermal << "\n";
            file2.close();
        }
    }
}
