#include "stepping.hh"
#include "G4RunManager.hh"
#include "G4Track.hh"
#include "G4SystemOfUnits.hh"
#include <set> // مكتبة لمنع التكرار باستخدام الـ IDs

MySteppingAction::MySteppingAction(MyRunAction* runAction)
: G4UserSteppingAction(), fRunAction(runAction)
{}

MySteppingAction::~MySteppingAction()
{}

void MySteppingAction::UserSteppingAction(const G4Step* step)
{
    // 1. الحصول على بيانات الجسيم الحالي دون قيود مبكرة
    G4Track* track = step->GetTrack();
    G4String particleName = track->GetDefinition()->GetParticleName();

    // 2. التأكد من أن الجسيم يعبر حداً فاصلاً بين حجمين هندسيين (Boundary)
    if (step->GetPostStepPoint()->GetStepStatus() == fGeomBoundary) 
    {
        // التحقق من وجود الحجم الفيزيائي قبل وبعد الخطوة لتجنب الـ Crash
        if (!step->GetPreStepPoint()->GetTouchableHandle()->GetVolume() || 
            !step->GetPostStepPoint()->GetTouchableHandle()->GetVolume()) {
            return;
        }

        G4String volumeFrom = step->GetPreStepPoint()->GetTouchableHandle()->GetVolume()->GetName();
        G4String volumeTo = step->GetPostStepPoint()->GetTouchableHandle()->GetVolume()->GetName();

        G4double kineticEnergy = step->GetPostStepPoint()->GetKineticEnergy();
        G4double energy_eV = kineticEnergy / CLHEP::eV;
        G4double energy_MeV = kineticEnergy / CLHEP::MeV; // 🌟 تم تعريفه لحسابات كيرما الدقيقة

        // ----------------------------------------------------------------
        // أ) تتبع النيوترونات فقط عبر المكونات الداخلية للـ BSA
        // ----------------------------------------------------------------
        if (particleName == "neutron") 
        {
            // 1. من الهدف إلى المهدئ
            if (volumeFrom == "physTarget" && volumeTo == "physModerator") 
            {
                if (track->GetCurrentStepNumber() <= 5) { 
                    fRunAction->nModerator++;
                    G4cout << "-> [Energy AFTER TARGET / Entering Moderator] = " << energy_eV << " eV" << G4endl;          
                }
            }
            // 2. من المهدئ إلى فلتر غاما (هنا مكان جملتكِ الصحيح والمضمون)
            else if (volumeFrom == "physModerator" && volumeTo == "physGammaFilter") 
            {
                //G4cout << "-> [Energy AFTER MODERATOR / Entering GammaFilter] = " << energy_eV << " eV" << G4endl;
            }
            // 3. من فلتر غاما إلى الكوليماتور
            else if (volumeFrom == "physGammaFilter" && volumeTo == "physCollimator") 
            {
                fRunAction->nCollimator++;
            }
        }
        // ----------------------------------------------------------------
        // ب) 🌟 السطح الفاصل الحرج المشترك (عند واجهة الكاشف) 🌟
        // ----------------------------------------------------------------
// ----------------------------------------------------------------
        // ب) 🌟 السطح الفاصل الحرج المشترك (عند واجهة الكاشف) 🌟
        // ----------------------------------------------------------------
        if (volumeTo == "physDetector" || volumeTo == "physDetector_PV" || 
           (volumeFrom == "physCollimator" && volumeTo == "physDetector"))
        {
            // 📐 1. حساب زاوية السقوط الفيزيائية الحقيقية بالنسبة للمحور العمودي (نفترض أنه محور Z)
            G4ThreeVector momentumDir = track->GetMomentumDirection();
            G4double cosTheta = momentumDir.z(); // جيب تمام الزاوية مع المحور Z العمودي على السطح

            // حماية الكود من الانقسام على صفر أو الزوايا الميتة
            if (cosTheta <= 0.0) { cosTheta = 1.0e-4; } 
            if (cosTheta > 1.0)  { cosTheta = 1.0; }

            // الوزن الفيزيائي لحساب الفيض الحقيقي (1 / cosTheta)
            G4double fluxWeight = 1.0 / cosTheta;

            // 🎯 أولاً: إذا كان الجسيم نيوتروناً
            if (particleName == "neutron") 
            {
                // زيادة عداد التيار (Current J) بمقدار ثابت = 1
                fRunAction->nDetector++; 

                // زيادة عدادات الفيض (Flux) الموزونة بالزاوية الحقيقية لمنع الثبات على 1
                if (energy_eV < 0.5) {
                    fRunAction->nThermalFluxCount += fluxWeight * track->GetWeight();
                }
                else if (energy_eV >= 0.5 && energy_eV <= 10000.0) {
                    fRunAction->nEpithermal += fluxWeight * track->GetWeight();
                }
                else if (energy_eV > 10000.0) {
                    fRunAction->nFast += fluxWeight * track->GetWeight();

                    // حساب كيرما النيوترونات السريعة الحقيقي
                    static const std::vector<G4double> e_n = {0.01, 0.02, 0.05, 0.1, 0.2, 0.5, 1.0, 2.0, 5.0, 10.0};
                    static const std::vector<G4double> k_n = {1.8e-12, 3.2e-12, 7.0e-12, 1.2e-11, 1.9e-11, 2.8e-11, 3.8e-11, 4.3e-11, 4.8e-11, 5.2e-11};
                    G4double fastKermaFactor = 0.0;
                    if (energy_MeV <= e_n.front()) { fastKermaFactor = k_n.front(); } 
                    else if (energy_MeV >= e_n.back()) { fastKermaFactor = k_n.back(); } 
                    else {
                        for (size_t i = 0; i < e_n.size() - 1; ++i) {
                            if (energy_MeV >= e_n[i] && energy_MeV <= e_n[i+1]) {
                                G4double fraction = (energy_MeV - e_n[i]) / (e_n[i+1] - e_n[i]);
                                fastKermaFactor = k_n[i] + fraction * (k_n[i+1] - k_n[i]);
                                break;
                            }
                        }
                    }
                    fRunAction->dFastAccumulated += fastKermaFactor * track->GetWeight();
                }

                track->SetTrackStatus(fStopAndKill); 
            }
            
            // 🎯 ثانياً: إذا كان الجسيم فوتون غاما
            else if (particleName == "gamma") 
            {
                // غاما هنا تُعامل بوزن الفيض الزاوي أيضاً
                fRunAction->nGamma += fluxWeight * track->GetWeight(); 
                
                static const std::vector<G4double> e_g = {0.01, 0.03, 0.05, 0.1, 0.2, 0.5, 1.0, 2.0, 5.0, 10.0};
                static const std::vector<G4double> k_g = {4.8e-12, 2.2e-12, 1.6e-12, 2.2e-12, 3.0e-12, 4.2e-12, 5.8e-12, 8.2e-12, 1.4e-11, 2.2e-11};
                G4double gammaKermaFactor = 0.0;
                if (energy_MeV <= e_g.front()) { gammaKermaFactor = k_g.front(); } 
                else if (energy_MeV >= e_g.back()) { gammaKermaFactor = k_g.back(); } 
                else {
                    for (size_t i = 0; i < e_g.size() - 1; ++i) {
                        if (energy_MeV >= e_g[i] && energy_MeV <= e_g[i+1]) {
                            G4double fraction = (energy_MeV - e_g[i]) / (e_g[i+1] - e_g[i]);
                            gammaKermaFactor = k_g[i] + fraction * (k_g[i+1] - k_g[i]);
                            break;
                        }
                    }
                }
                fRunAction->dGammaAccumulated += gammaKermaFactor * track->GetWeight();
                
                track->SetTrackStatus(fStopAndKill); 
            }
        }
    }
}
