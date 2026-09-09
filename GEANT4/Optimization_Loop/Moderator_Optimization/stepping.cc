#include "stepping.hh"
#include "G4RunManager.hh"
#include "G4Track.hh"
#include "G4SystemOfUnits.hh"
#include "G4AnalysisManager.hh"
#include <cmath>
#include <vector>

MySteppingAction::MySteppingAction(MyRunAction* runAction)
: G4UserSteppingAction(), fRunAction(runAction)
{}

MySteppingAction::~MySteppingAction()
{}

void MySteppingAction::UserSteppingAction(const G4Step* step)
{
    G4Track* track = step->GetTrack();
    
    G4String particleName = track->GetDefinition()->GetParticleName();

    if (step->GetPostStepPoint()->GetStepStatus() == fGeomBoundary) 
    {
        if (!step->GetPreStepPoint()->GetTouchableHandle()->GetVolume() || 
            !step->GetPostStepPoint()->GetTouchableHandle()->GetVolume()) {
            return;
        }

        G4String volumeFrom = step->GetPreStepPoint()->GetTouchableHandle()->GetVolume()->GetName();
        G4String volumeTo = step->GetPostStepPoint()->GetTouchableHandle()->GetVolume()->GetName();

        G4double kineticEnergy = step->GetPostStepPoint()->GetKineticEnergy();
        G4double energy_eV = kineticEnergy / CLHEP::eV;
        G4double energy_MeV = kineticEnergy / CLHEP::MeV;
        G4double weight = track->GetWeight();
        G4int eventID = G4RunManager::GetRunManager()->GetCurrentEvent()->GetEventID();

        G4AnalysisManager* man = G4AnalysisManager::Instance();

        if (particleName == "neutron") 
        {
            // 1. Target -> Moderator
            if (volumeFrom == "physTarget" && volumeTo == "physModerator") 
            {
                fRunAction->nTarget++;
                man->FillNtupleIColumn(4, 0, eventID);
                man->FillNtupleDColumn(4, 1, energy_eV);
                man->FillNtupleDColumn(4, 2, weight);
                man->AddNtupleRow(4);
            }
            // 2. Moderator -> FastFilter
            else if (volumeFrom == "physModerator" && volumeTo == "physFastFilter") 
            {
                fRunAction->nModerator++;
                man->FillNtupleIColumn(5, 0, eventID);
                man->FillNtupleDColumn(5, 1, energy_eV);
                man->FillNtupleDColumn(5, 2, weight);
                man->AddNtupleRow(5);
            }
            // 3. FastFilter -> GammaFilter
            else if (volumeFrom == "physFastFilter" && volumeTo == "physGammaFilter") 
            {
                fRunAction->nFastFilter++;
                man->FillNtupleIColumn(6, 0, eventID);
                man->FillNtupleDColumn(6, 1, energy_eV);
                man->FillNtupleDColumn(6, 2, weight);
                man->AddNtupleRow(6);
            }
            // 4. GammaFilter -> Collimator
            else if (volumeFrom == "physGammaFilter" && volumeTo == "physCollimator") 
            {
                fRunAction->nGammaFilter++;
                man->FillNtupleIColumn(7, 0, eventID);
                man->FillNtupleDColumn(7, 1, energy_eV);
                man->FillNtupleDColumn(7, 2, weight);
                man->AddNtupleRow(7);
            }
        }

        // 5. الوصول للكاشف النهائي عند مخرج الـ BSA
        if (volumeTo == "physDetector")
        {
            G4ThreeVector momentumDir = track->GetMomentumDirection();
            G4double cosTheta = momentumDir.z(); 

            if (cosTheta > 0.0) 
            {
                G4double fluxWeight = weight / cosTheta;

                if (particleName == "neutron") 
                {
                    fRunAction->nDetector++;

                    if (energy_eV < 0.5) {
                        fRunAction->nThermalFluxCount += fluxWeight;
                    }
                    else if (energy_eV >= 0.5 && energy_eV <= 10000.0) {
                        fRunAction->nEpithermal += fluxWeight;            
                        fRunAction->nCurrentEpithermal += weight;            
                    }
                    else if (energy_eV > 10000.0) {
                        fRunAction->nFast += fluxWeight;

                        // حساب كيرما النيوترونات السريعة للنيوترونات السريعة فقط (> 10 keV)
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

                    // Ntuple 1: Detector Neutrons
                    man->FillNtupleIColumn(1, 0, eventID);
                    man->FillNtupleDColumn(1, 1, energy_eV);
                    man->FillNtupleDColumn(1, 2, cosTheta);
                    
                    G4double x_cm = track->GetPosition().x() / CLHEP::cm;
                    G4double y_cm = track->GetPosition().y() / CLHEP::cm;
                    G4double r_cm = std::sqrt(x_cm * x_cm + y_cm * y_cm);

                    man->FillNtupleDColumn(1, 3, x_cm);
                    man->FillNtupleDColumn(1, 4, y_cm);
                    man->FillNtupleDColumn(1, 5, r_cm);
                    man->FillNtupleDColumn(1, 6, fluxWeight);

                    man->AddNtupleRow(1);

                    track->SetTrackStatus(fStopAndKill); 
                }
                else if (particleName == "gamma") 
                {
                    fRunAction->nGamma += fluxWeight;    
                    
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
                    
                    // تم تفعيل السطر التالي لحساب وتجميع كيرما أشعة غاما
                    fRunAction->dGammaAccumulated += gammaKermaFactor * track->GetWeight();

                    // Ntuple 2: Gamma Output
                    man->FillNtupleIColumn(2, 0, eventID);
                    man->FillNtupleDColumn(2, 1, energy_MeV);
                    man->FillNtupleDColumn(2, 2, track->GetPosition().x() / CLHEP::cm);
                    man->FillNtupleDColumn(2, 3, track->GetPosition().y() / CLHEP::cm);
                    man->FillNtupleDColumn(2, 4, fluxWeight);
                    man->AddNtupleRow(2);

                    track->SetTrackStatus(fStopAndKill); 
                }
            }
        }
    }
}

