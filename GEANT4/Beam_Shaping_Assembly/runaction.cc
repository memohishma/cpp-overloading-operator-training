#include "runaction.hh"
#include <fstream>
#include <vector>
#include "G4AccumulableManager.hh"
#include "G4Run.hh"

MyRunAction::MyRunAction()
{
    //=========================================================================================================================
    //constructor (called when an object from the MyRunAction class is created)
    //=========================================================================================================================

    //get an instance of the G4AnalysisManager, which collects data and writes it to file for later
    G4AnalysisManager* man = G4AnalysisManager::Instance();

    //Tally neutrons produced in Li target
    man->CreateNtuple("Target", "Target"); //create an Ntuple to hold information we want in the target
    man->CreateNtupleIColumn("fEvent"); //create an I (interger) column to hold the event number 
    man->CreateNtupleDColumn("PreStepEnergy (keV)"); //create a D (double) column to hold the pre-step energy
    man->CreateNtupleDColumn("PostStepEnergy (keV)"); //...
    man->CreateNtupleDColumn("fX (m)");
    man->CreateNtupleDColumn("fY (m)");
    man->CreateNtupleDColumn("fZ (m)");
    man->FinishNtuple(0); //finish the Ntuple, and give it the ID of 0

    //Tally neutrons in the detector
    man->CreateNtuple("Detector", "Detector"); //create an Ntuple to hold information we want in the BF3 detector
    man->CreateNtupleIColumn("fEvent"); //same deal as above
    man->CreateNtupleDColumn("PreStepEnergy (keV)");
    man->CreateNtupleDColumn("PostStepEnergy (keV)");
    man->CreateNtupleDColumn("fX (m)");
    man->CreateNtupleDColumn("fY (m)");
    man->CreateNtupleDColumn("fZ (m)");
    man->FinishNtuple(1); //finish the Ntuple, and give it the ID of 1
    
    // تسجيل العدادات داخل مدير الـ Accumulables
    G4AccumulableManager* accumulableManager = G4AccumulableManager::Instance();
    accumulableManager->RegisterAccumulable(nTarget);
    accumulableManager->RegisterAccumulable(nModerator);
    accumulableManager->RegisterAccumulable(nGamma);
    accumulableManager->RegisterAccumulable(nCollimator);
    accumulableManager->RegisterAccumulable(nReflector);
    accumulableManager->RegisterAccumulable(nDetector);
    
    accumulableManager->RegisterAccumulable(nThermalFluxCount);
    accumulableManager->RegisterAccumulable(nEpithermal);
    accumulableManager->RegisterAccumulable(nFast);
}

MyRunAction::~MyRunAction()
{} //destructor (no additional action is required => empty)

void MyRunAction::BeginOfRunAction(const G4Run*)
{
    // ======= التعديل الجوهري هنا =======
    // تصفير العدادات لجميع الخيوط قبل بدء المحاكاة الفعلية لضمان التقاط بيانات الـ SteppingAction
    G4AccumulableManager* accumulableManager = G4AccumulableManager::Instance();
    accumulableManager->Reset();

    // فتح ملفات الـ CSV الخاصة بالـ Ntuples لكل خيط
    G4AnalysisManager* man = G4AnalysisManager::Instance();
    man->OpenFile("output.csv");
}
void MyRunAction::EndOfRunAction(const G4Run* aRun)
{
    // 1. كتابة وحفظ البيانات المجمعة في ملفات الـ CSV لكافة الخيوط
    G4AnalysisManager* man = G4AnalysisManager::Instance();
    man->Write();
    man->CloseFile();

    // 2. دمج العدادات العادية
    G4AccumulableManager::Instance()->Merge();

    // 3. القراءة والتحليل داخل الخيط الرئيسي فقط (Master Thread)
if (G4Threading::IsMasterThread()) 
    {
        G4cout << "------------------- Run Complete! -------------------" << G4endl;

        G4int numTargetNeutrons = 0;
        G4int numDetectorNeutrons = 0;
        
        // عدادات الفرز الطاقي داخل الـ Master Thread
        G4int masterThermalCount = 0;
        G4int masterEpithermalCount = 0;
        G4int masterFastCount = 0;

        for (G4int i=0; i<G4Threading::GetNumberOfRunningWorkerThreads(); i++)
        {
            std::stringstream strThreadID;
            strThreadID << i;
            G4String targetFileName = "output_nt_Target_t" + strThreadID.str() + ".csv";
            G4String detectorFileName = "output_nt_Detector_t" + strThreadID.str() + ".csv";

            // [قراءة ملف الهدف]
            std::vector<G4bool> targetEvtList;
            G4String targetLine;
            std::ifstream targetFile(targetFileName);
            if (targetFile.is_open()) {
                for (G4int j=0; j<10; j++) getline(targetFile, targetLine); // تخطي الكومنتات
                while (getline(targetFile, targetLine)) {
                    if(targetLine.empty()) continue;
                    G4int evt = std::stoi(targetLine.substr(0, targetLine.find(',')));
                    while (evt >= targetEvtList.size()) targetEvtList.push_back(false);
                    targetEvtList[evt] = true;
                }
            }
            for (G4bool val : targetEvtList) { if (val) numTargetNeutrons++; }

            // [قراءة ملف الكاشف المطور للفرز الطاقي الحقيقي]
            std::vector<G4bool> detectorEvtList;
            G4String detectorLine;
            std::ifstream detectorFile(detectorFileName);
            if (detectorFile.is_open()) {
                for (G4int j=0; j<10; j++) getline(detectorFile, detectorLine); // تخطي الكومنتات
                
                while (getline(detectorFile, detectorLine)) 
                {
                    if(detectorLine.empty()) continue;
                    
                    // تقطيع السطر للحصول على رقم الحدث والطاقة
                    std::stringstream ss(detectorLine);
                    G4String item;
                    std::vector<G4String> row;
                    while (std::getline(ss, item, ',')) {
                        row.push_back(item);
                    }

                    if(row.size() < 3) continue; // التأكد من وجود الأعمدة المطلوبة

                    G4int evt = std::stoi(row[0]);
                    G4double energy_keV = std::stod(row[1]); 
                    G4double energy_eV = energy_keV * 1000.0; // تحويلها إلى eV

                    while (evt >= detectorEvtList.size()) detectorEvtList.push_back(false);
                    detectorEvtList[evt] = true;

                    // الفرز الطاقي المباشر والآمن هنا:
                    if (energy_eV < 0.5) {
                        masterThermalCount++;
                    }
                    else if (energy_eV >= 0.5 && energy_eV <= 10000.0) {
                        masterEpithermalCount++;
                    }
                    else if (energy_eV > 10000.0) {
                        masterFastCount++;
                    }
                }
            }
            for (G4bool val : detectorEvtList) { if (val) numDetectorNeutrons++; }
        }

        // 4. طباعة التقارير الأساسية المحدثة متوافقة مع الهندسة الجديدة
        G4cout << "Total number of Neutrons produced in target: " << numTargetNeutrons << G4endl;
        G4cout << "Total number of Neutrons reaching the detector: " << numDetectorNeutrons << G4endl;
        G4cout << "-----------------------------------------------------" << G4endl;
        
        G4cout << "Neutrons Beam Evolution via BSA components (Boundary Crossings):" << G4endl;
        G4cout << "Target     -> Moderator  : " << nModerator.GetValue() << G4endl;
        G4cout << "Moderator  -> Gamma      : " << nGamma.GetValue() << G4endl;
        G4cout << "Gamma      -> Collimator : " << nCollimator.GetValue() << G4endl;
        G4cout << "Collimator -> Detector   : " << nDetector.GetValue() << G4endl;
        G4cout << "-----------------------------------------------------" << G4endl;
        // 5. حساب الـ Flux الحقيقي من واقع الملفات المقروءة للـ Master
        G4double radius = 10.0 * CLHEP::cm; 
        G4double A_cm2 = (CLHEP::pi * radius * radius) / (CLHEP::cm * CLHEP::cm);
        G4double N_p = aRun->GetNumberOfEventToBeProcessed(); 

        G4double fluxThermal    = (G4double)masterThermalCount / (A_cm2 * N_p); 
        G4double fluxEpithermal = (G4double)masterEpithermalCount / (A_cm2 * N_p);
        G4double fluxFast       = (G4double)masterFastCount / (A_cm2 * N_p);

        G4cout << "=====================================================" << G4endl;
        G4cout << "        FINAL NEUTRON FLUX AT BSA OUTPUT             " << G4endl;
        G4cout << "=====================================================" << G4endl;
        G4cout << std::scientific; 
        G4cout << "Thermal Flux (<0.5 eV)       : " << fluxThermal    << " n/cm^2.primary" << G4endl;
        G4cout << "Epithermal Flux (0.5eV-10keV): " << fluxEpithermal << " n/cm^2.primary" << G4endl;
        G4cout << "Fast Flux (>10 keV)          : " << fluxFast       << " n/cm^2.primary" << G4endl;
        G4cout << std::defaultfloat; 
        G4cout << "=====================================================" << G4endl;
 
    }
}
