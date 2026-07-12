#include "runaction.hh"
#include <fstream>
#include <vector>
#include "G4AccumulableManager.hh"
#include "G4Run.hh"

#include "G4ScoringManager.hh"
#include "G4SDManager.hh"
#include "G4THitsMap.hh"
#include "G4Event.hh"

MyRunAction::MyRunAction()
{
    //=========================================================================================================================
    // Constructor
    //=========================================================================================================================
    G4AnalysisManager* man = G4AnalysisManager::Instance();

    // Tally neutrons produced in Li target
    man->CreateNtuple("Target", "Target"); 
    man->CreateNtupleIColumn("fEvent"); 
    man->CreateNtupleDColumn("PreStepEnergy (keV)"); 
    man->CreateNtupleDColumn("PostStepEnergy (keV)"); 
    man->CreateNtupleDColumn("fX (m)");
    man->CreateNtupleDColumn("fY (m)");
    man->CreateNtupleDColumn("fZ (m)");
    man->FinishNtuple(0); 

    // Tally neutrons in the detector
    man->CreateNtuple("Detector", "Detector"); 
    man->CreateNtupleIColumn("fEvent"); 
    man->CreateNtupleDColumn("PreStepEnergy (keV)");
    man->CreateNtupleDColumn("PostStepEnergy (keV)");
    man->CreateNtupleDColumn("fX (m)");
    man->CreateNtupleDColumn("fY (m)");
    man->CreateNtupleDColumn("fZ (m)");
    man->FinishNtuple(1); 
    
    // تسجيل العدادات داخل مدير الـ Accumulables
    // تسجيل العدادات داخل مدير الـ Accumulables بالطريقة الحديثة
    G4AccumulableManager* accumulableManager = G4AccumulableManager::Instance();
    accumulableManager->Register(nTarget);
    accumulableManager->Register(nModerator);
    accumulableManager->Register(nFastFilter);
    accumulableManager->Register(nGamma);
    accumulableManager->Register(nCollimator);
    accumulableManager->Register(nReflector);
    accumulableManager->Register(nDetector);
    
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
    // 1. كتابة وحفظ البيانات المجمعة الافتراضية
    G4AnalysisManager* man = G4AnalysisManager::Instance();
    man->Write();
    man->CloseFile();

    // 2. دمج العدادات عبر جميع الـ Threads (خطوة جوهرية في الـ Multithreading)
    G4AccumulableManager::Instance()->Merge();

    // 3. الحسابات والتحليل والطباعة داخل الخيط الرئيسي فقط (Master Thread)
    if (G4Threading::IsMasterThread()) 
    {
        G4cout << "------------------- Run Complete! -------------------" << G4endl;

        // جلب القيم الخام مباشرة من الـ Accumulables بعد الدمج
        G4int masterThermalCount    = nThermalFluxCount.GetValue();
        G4int masterEpithermalCount = nEpithermal.GetValue();
        G4int masterFastCount       = nFast.GetValue();
        G4int numDetectorNeutrons   = nDetector.GetValue(); 
        G4int numTargetNeutrons     = nModerator.GetValue(); // النيوترونات الصافية الخارجة من الهدف

        // ====================================================================
         // 4. طباعة التقارير الأساسية
        G4cout << "Total number of Neutrons produced in target: " << numTargetNeutrons << G4endl;
        G4cout << "Total number of Neutrons reaching the detector: " << numDetectorNeutrons << G4endl;
        G4cout << "-----------------------------------------------------" << G4endl;

        // 🛑 لوحة الـ DEBUG الخام المطلوبة لحسم مشكلة النسب
        // ====================================================================
        G4cout << "\n=====================================================" << G4endl;
        G4cout << "                   🛑 DEBUG COUNTS 🛑                 " << G4endl;
        G4cout << "=====================================================" << G4endl;
        G4cout << "Raw Thermal Count     (< 0.5 eV)     : " << masterThermalCount << G4endl;
        G4cout << "Raw Epithermal Count  (0.5eV - 10keV): " << masterEpithermalCount << G4endl;
        G4cout << "Raw Fast Count        (> 10 keV)     : " << masterFastCount << G4endl;
        G4cout << "Raw Gamma Count       (At Boundary)  : " << nGamma.GetValue() << G4endl;
        G4cout << "Raw Detector Neutrons (Total Unique) : " << numDetectorNeutrons << G4endl;
        G4cout << "=====================================================\n" << G4endl;

        // 4. الحسابات الهندسية للتدفق (Flux) - يجب وضعها هنا قبل الطباعة!
        G4double radius = 6.0 * CLHEP::cm; 
        G4double A_cm2 = (CLHEP::pi * radius * radius) / (CLHEP::cm * CLHEP::cm);
        G4double N_p = aRun->GetNumberOfEventToBeProcessed(); 

        // تعريف وحساب قيم الـ Flux لكل نوع
        G4double fluxThermal    = (G4double)masterThermalCount / (A_cm2 * N_p); 
        G4double fluxEpithermal = (G4double)masterEpithermalCount / (A_cm2 * N_p);
        G4double fluxFast       = (G4double)masterFastCount / (A_cm2 * N_p);
        
        // 🌟 تصحيح الـ Gamma Scoring: نعتمد على الـ Flux العددي المباشر من الحدود
        G4double fluxGamma      = (G4double)nGamma.GetValue() / (A_cm2 * N_p); 

        G4cout << "=====================================================" << G4endl;
        G4cout << "            FINAL PARTICLE FLUX AT BSA OUTPUT        " << G4endl;
        G4cout << "=====================================================" << G4endl;
        G4cout << std::scientific; 
        G4cout << "Thermal Flux (<0.5 eV)        : " << fluxThermal    << " n/cm^2.primary" << G4endl;
        G4cout << "Epithermal Flux (0.5eV-10keV) : " << fluxEpithermal << " n/cm^2.primary" << G4endl;
        G4cout << "Fast Flux (>10 keV)           : " << fluxFast       << " n/cm^2.primary" << G4endl;
        G4cout << "-----------------------------------------------------" << G4endl;
        G4cout << "Gamma Contamination Flux      : " << fluxGamma      << " photons/cm^2.primary" << G4endl;
        G4cout << "=====================================================" << G4endl;

        // 5. حساب الـ Real Flux الفعلي لتيار 20mA
        G4double beamCurrent = 20.0e-3; 
        G4double protonCharge = 1.602176634e-19; 
        G4double protonsPerSecond = beamCurrent / protonCharge; 

        G4double realFluxThermal    = fluxThermal * protonsPerSecond;
        G4double realFluxEpithermal = fluxEpithermal * protonsPerSecond;
        G4double realFluxFast       = fluxFast * protonsPerSecond;
        G4double realFluxGamma      = fluxGamma * protonsPerSecond;

        G4cout << G4endl;
        G4cout << "=====================================================" << G4endl;
        G4cout << "       REAL ABSOLUTE FLUX AT 20 mA BEAM CURRENT      " << G4endl;
        G4cout << "=====================================================" << G4endl;
        G4cout << "Real Thermal Flux            : " << realFluxThermal    << " n/cm^2.s" << G4endl;
        G4cout << "Real Epithermal Flux (BNCT)  : " << realFluxEpithermal << " n/cm^2.s" << G4endl;
        G4cout << "Real Fast Flux               : " << realFluxFast       << " n/cm^2.s" << G4endl;
        G4cout << "-----------------------------------------------------" << G4endl;
        G4cout << "Real Gamma Contamination Flux: " << realFluxGamma      << " photons/cm^2.s" << G4endl;
        G4cout << "=====================================================" << G4endl;

        // 6. 🌟 لوحة معايير الجودة المصححة للـ IAEA 🌟
        G4double ratioEpithermalFast    = (realFluxFast > 0.0) ? (realFluxEpithermal / realFluxFast) : 0.0;
        
        // النسبة المصححة فيزيائياً: نريد التدفق الحراري مقسوماً على فوق الحراري (Target < 0.05)
        G4double ratioThermalEpithermal = (realFluxEpithermal > 0.0) ? (realFluxThermal / realFluxEpithermal) : 0.0;
        
        // 🌟 [التعديل الجديد]: حساب الجرعات الحقيقية الدقيقة بناءً على قيم كيرما التراكمية الصافية
        // نقوم بقسمة الجرعة المتراكمة الإجمالية (الموزونة بالوزن وطاقة الجسيم) على إجمالي عدد النيوترونات فوق الحرارية المكتشفة
        G4double totalRealEpithermalCount = (G4double)masterEpithermalCount;

        G4double doseFastPerEpithermal  = (totalRealEpithermalCount > 0.0) ? (dFastAccumulated.GetValue() / totalRealEpithermalCount) : 0.0; 
        G4double doseGammaPerEpithermal = (totalRealEpithermalCount > 0.0) ? (dGammaAccumulated.GetValue() / totalRealEpithermalCount) : 0.0;

        // حساب الاتجاهية الإشعاعية
        G4double totalRealNeutronFlux = realFluxThermal + realFluxEpithermal + realFluxFast;
        G4double realNeutronCurrentDensity = (G4double)numDetectorNeutrons / (A_cm2 * (N_p / protonsPerSecond));
        G4double currentToFluxRatio = (totalRealNeutronFlux > 0.0) ? (realNeutronCurrentDensity / totalRealNeutronFlux) : 0.0;

        G4cout << G4endl;
        G4cout << "=====================================================" << G4endl;
        G4cout << "     IAEA BNCT BEAM QUALITY RECOMMENDATIONS METRIC   " << G4endl;
        G4cout << "=====================================================" << G4endl;
        G4cout << std::defaultfloat; 
        
        G4cout << "1. Epithermal Flux (IAEA Target: > 1e9 n/cm^2.s) -> Value: " << realFluxEpithermal << G4endl;
        G4cout << "2. Phi_epithermal / Phi_Fast     (IAEA Target: > 20)  -> Value: " << ratioEpithermalFast << G4endl; // تم تعديل المسمى النصي ليطابق المتغير بدقة
        G4cout << "3. Phi_thermal / Phi_epithermal  (IAEA Target: < 0.05)-> Value: " << ratioThermalEpithermal << G4endl;
        
        G4cout << std::scientific;
        G4cout << "4. D_fast / Phi_epithermal  (IAEA Target: < 2e-13)   -> Value: " << doseFastPerEpithermal << " Gy.cm^2" << G4endl;
        G4cout << "5. D_gamma / Phi_epithermal (IAEA Target: < 2e-13)   -> Value: " << doseGammaPerEpithermal << " Gy.cm^2" << G4endl;
        
        G4cout << std::defaultfloat;
        G4cout << "6. Beam Directionality (J / Phi) (IAEA Target: > 0.7)-> Value: " << currentToFluxRatio << G4endl;
        G4cout << "=====================================================" << G4endl;
    }
}

