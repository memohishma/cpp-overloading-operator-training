#include "stepping.hh"
#include "G4RunManager.hh"
#include "G4Track.hh"
#include "G4SystemOfUnits.hh" // أضفنا هذا السطر لضمان تعريف CLHEP::eV بشكل سليم

MySteppingAction::MySteppingAction(MyRunAction* runAction)
: G4UserSteppingAction(), fRunAction(runAction)
{}

MySteppingAction::~MySteppingAction()
{}
void MySteppingAction::UserSteppingAction(const G4Step* step)
{
    // الحصول على الجسيم الحالي
    G4Track* track = step->GetTrack();
    G4String particleName = track->GetDefinition()->GetParticleName();

    // نحن نهتم فقط بالنيوترونات
    if (particleName != "neutron") return;

    // التأكد من أن النيوترون يعبر حداً فاصلاً بين حجمين هندسيين (Boundary)
    if (step->GetPostStepPoint()->GetStepStatus() == fGeomBoundary) 
    {
        // التحقق من وجود الحجم الفيزيائي قبل وبعد الخطوة لتجنب أي انهيارات برمجية (Null Pointer Check)
        if (!step->GetPreStepPoint()->GetTouchableHandle()->GetVolume() || 
            !step->GetPostStepPoint()->GetTouchableHandle()->GetVolume()) {
            return;
        }

        // اسم المكون الذي خرج منه النيوترون والمنظومة التي دخل إليها الآن
        G4String volumeFrom = step->GetPreStepPoint()->GetTouchableHandle()->GetVolume()->GetName();
        G4String volumeTo = step->GetPostStepPoint()->GetTouchableHandle()->GetVolume()->GetName();

        // 1. الانتقال من الهدف (Target) إلى المهدئ (Moderator) مباشرة (حسب ترتيبك الجديد)
        if (volumeFrom == "physTarget" && volumeTo == "physModerator") {
            fRunAction->nModerator++;
        }
        // 2. الانتقال من المهدئ (Moderator) إلى فلتر الغاما (GammaFilter)
        else if (volumeFrom == "physModerator" && volumeTo == "physGammaFilter")
{
    fRunAction->nGamma++;

    G4double kineticEnergy =
        step->GetPreStepPoint()->GetKineticEnergy();

    G4double energy_eV = kineticEnergy / CLHEP::eV;

    G4cout << "After Moderator = "
           << energy_eV
           << " eV" << G4endl;
}
        // 3. الانتقال من فلتر الغاما (GammaFilter) إلى الموجه/الموازئ (Collimator) مباشرة
        else if (volumeFrom == "physGammaFilter" && volumeTo == "physCollimator") {
            fRunAction->nCollimator++;
        }
        
        // =========================================================
        // ======= التعديل المرن والآمن لالتقاط بيانات الكاشف =======
        // =========================================================
        // 4. بمجرد أن يلمس الجسيم الكاشف النهائي (physDetector) القادم من الموازئ
        if (volumeTo == "physDetector") 
        {
            // الحفاظ على العداد العام للكاشف
            fRunAction->nDetector++;

            // الاعتماد على طاقة الدخول (PreStepEnergy) لتوحيد الحسابات مع الـ CSV
            G4double kineticEnergy = step->GetPreStepPoint()->GetKineticEnergy(); 
            G4double energy_eV = kineticEnergy / CLHEP::eV;

	    G4cout << "Neutron entered detector with energy = "
            << energy_eV << " eV" << G4endl;
            // التصنيف الطاقي الصارم والآمن داخل الـ Accumulables
            if (energy_eV < 0.5) {
                fRunAction->nThermalFluxCount++;
            }
            else if (energy_eV >= 0.5 && energy_eV <= 10000.0) {
                fRunAction->nEpithermal++;
            }
            else if (energy_eV > 10000.0) {
                fRunAction->nFast++;
            }
        }
    }
}
