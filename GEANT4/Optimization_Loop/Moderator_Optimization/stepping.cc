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
            if (volumeFrom == "physTarget" && volumeTo == "physModerator") 
            {
                fRunAction->nTarget++;
                man->FillNtupleIColumn(4, 0, eventID);
                man->FillNtupleDColumn(4, 1, energy_eV);
                man->FillNtupleDColumn(4, 2, weight);
                man->AddNtupleRow(4);
            }
            else if (volumeFrom == "physModerator" && volumeTo == "physFastFilter") 
            {
                fRunAction->nModerator++;
                man->FillNtupleIColumn(5, 0, eventID);
                man->FillNtupleDColumn(5, 1, energy_eV);
                man->FillNtupleDColumn(5, 2, weight);
                man->AddNtupleRow(5);
            }
            else if (volumeFrom == "physFastFilter" && volumeTo == "physGammaFilter") 
            {
                fRunAction->nFastFilter++;
                man->FillNtupleIColumn(6, 0, eventID);
                man->FillNtupleDColumn(6, 1, energy_eV);
                man->FillNtupleDColumn(6, 2, weight);
                man->AddNtupleRow(6);
            }
            else if (volumeFrom == "physGammaFilter" && volumeTo == "physCollimator") 
            {
                fRunAction->nGammaFilter++;
                man->FillNtupleIColumn(7, 0, eventID);
                man->FillNtupleDColumn(7, 1, energy_eV);
                man->FillNtupleDColumn(7, 2, weight);
                man->AddNtupleRow(7);
            }
        }

        // الوصول للكاشف النهائي عند مخرج الـ BSA
        if (volumeTo == "physDetector")
        {
            G4ThreeVector momentumDir = track->GetMomentumDirection();
            G4double cosTheta = momentumDir.z(); 

            if (cosTheta > 0.0) 
            {
                G4double absCosTheta = std::abs(cosTheta);
                G4double fluxWeight = weight / absCosTheta;
                G4double currentWeight = weight; // No cosine correction

                if (particleName == "neutron") 
                {
                    fRunAction->nDetector++;

                    // --- 1. تجميع حسابات التيار (Surface Current / No Cosine) ---
                    if (energy_eV < 0.5) {
                        fRunAction->nThermalCurrentCount += currentWeight;
                    }
                    else if (energy_eV >= 0.5 && energy_eV <= 10000.0) {
                        fRunAction->nEpithermalCurrent += currentWeight;            
                    }
                    else if (energy_eV > 10000.0) {
                        fRunAction->nFastCurrent += currentWeight;
                    }

                    // --- 2. تجميع حسابات الفيض (Surface Flux / Cosine Corrected) ---
                    if (energy_eV < 0.5) {
                        fRunAction->nThermalFluxCount += fluxWeight;
                    }
                    else if (energy_eV >= 0.5 && energy_eV <= 10000.0) {
                        fRunAction->nEpithermalFlux += fluxWeight;            
                    }
                    else if (energy_eV > 10000.0) {
                        fRunAction->nFastFlux += fluxWeight;

                        // جداول كيرما النيوترونات السريعة (ICRU 63)
                        static const std::vector<G4double> e_n = {
                            0.011, 0.020, 0.036, 0.063, 0.082, 0.086, 0.090, 0.094, 0.098, 0.105,
                            0.115, 0.125, 0.135, 0.145, 0.155, 0.165, 0.175, 0.185, 0.195, 0.210,
                            0.230, 0.250, 0.270, 0.290, 0.310, 0.330, 0.350, 0.370, 0.390, 0.420,
                            0.460, 0.500, 0.540, 0.580, 0.620, 0.660, 0.700, 0.740, 0.780, 0.820,
                            0.860, 0.900, 0.940, 0.980, 1.050, 1.150, 1.250, 1.350, 1.450, 1.550,
                            1.650, 1.750, 1.850, 1.950, 2.100, 2.300, 2.500, 2.700, 2.900, 3.100,
                            3.300, 3.500, 3.700, 3.900, 4.200, 4.600, 5.000, 5.400, 5.800, 6.200,
                            6.600, 7.000, 7.400, 7.800, 8.200, 8.600, 9.000, 9.400, 9.800
                        };
     
                        static const std::vector<G4double> k_n = {
                            2.89e-13, 4.15e-13, 6.21e-13, 9.12e-13, 1.12e-12, 1.17e-12, 1.21e-12, 1.26e-12, 1.31e-12, 1.39e-12,
                            1.51e-12, 1.63e-12, 1.75e-12, 1.87e-12, 1.98e-12, 2.10e-12, 2.21e-12, 2.32e-12, 2.43e-12, 2.59e-12,
                            2.80e-12, 3.01e-12, 3.22e-12, 3.42e-12, 3.63e-12, 3.84e-12, 4.04e-12, 4.25e-12, 4.45e-12, 4.75e-12,
                            5.14e-12, 5.53e-12, 5.90e-12, 6.27e-12, 6.63e-12, 6.98e-12, 7.33e-12, 7.66e-12, 7.99e-12, 8.30e-12,
                            8.61e-12, 8.91e-12, 9.20e-12, 9.48e-12, 9.92e-12, 1.05e-11, 1.11e-11, 1.17e-11, 1.22e-11, 1.27e-11,
                            1.32e-11, 1.37e-11, 1.42e-11, 1.46e-11, 1.52e-11, 1.60e-11, 1.67e-11, 1.74e-11, 1.80e-11, 1.86e-11,
                            1.91e-11, 1.96e-11, 2.01e-11, 2.06e-11, 2.12e-11, 2.20e-11, 2.27e-11, 2.34e-11, 2.40e-11, 2.47e-11,
                            2.53e-11, 2.59e-11, 2.65e-11, 2.70e-11, 2.76e-11, 2.81e-11, 2.86e-11, 2.91e-11, 2.95e-11
                        };
    
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
                        fRunAction->dFastFluxAccumulated += fastKermaFactor * fluxWeight;
                        fRunAction->dFastCurrentAccumulated += fastKermaFactor * currentWeight;
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
                    fRunAction->nGammaFlux += fluxWeight;
                    fRunAction->nGammaCurrent += currentWeight;   
                   
                    // ICRU Report 57 لأشعة غاما
                    static const std::vector<G4double> e_g = {
                        0.010, 0.015, 0.020, 0.030, 0.040, 0.050, 0.060, 0.080, 0.100, 0.150,
                        0.200, 0.300, 0.400, 0.500, 0.600, 0.800, 1.000, 1.500, 2.000, 3.000,
                        4.000, 5.000, 6.000, 8.000, 10.000
                    };

                    static const std::vector<G4double> k_g = {
                        7.43e-12, 3.12e-12, 1.68e-12, 7.21e-13, 4.29e-13, 3.23e-13, 2.89e-13, 3.07e-13, 3.71e-13, 5.99e-13,
                        8.56e-13, 1.38e-12, 1.89e-12, 2.38e-12, 2.84e-12, 3.69e-12, 4.47e-12, 6.14e-12, 7.55e-12, 9.96e-12,
                        1.21e-11, 1.41e-11, 1.61e-11, 2.01e-11, 2.40e-11
                    };
                    
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
                    
                    fRunAction->dGammaFluxAccumulated += gammaKermaFactor * fluxWeight;
                    fRunAction->dGammaCurrentAccumulated += gammaKermaFactor * currentWeight;

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
