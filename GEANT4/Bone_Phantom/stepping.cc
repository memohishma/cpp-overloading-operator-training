#include "stepping.hh"
#include "G4RunManager.hh"
#include "G4Track.hh"
#include "G4SystemOfUnits.hh"
#include "G4AnalysisManager.hh"
#include <cmath>

MySteppingAction::MySteppingAction(MyRunAction* runAction)
: G4UserSteppingAction(), fRunAction(runAction)
{}

MySteppingAction::~MySteppingAction()
{}

void MySteppingAction::UserSteppingAction(const G4Step* step)
{
    G4Track* track = step->GetTrack();
    G4String particleName = track->GetDefinition()->GetParticleName();

    // ----------------------------------------------------------------
    // 1. Energy Deposition Inside Bone Phantom (Dose Profile)
    // ----------------------------------------------------------------
    G4VPhysicalVolume* currentVolume = step->GetPreStepPoint()->GetTouchableHandle()->GetVolume();
    if (currentVolume && currentVolume->GetName() == "physBonePhantom")
    {
        G4double edep = step->GetTotalEnergyDeposit();
        if (edep > 0.0)
        {
            G4ThreeVector pos = step->GetPostStepPoint()->GetPosition();
            G4double zCenter = currentVolume->GetObjectTranslation().z();
            G4double zSurface = zCenter - (10.0 * cm); 
            G4double depth_cm = (pos.z() - zSurface) / CLHEP::cm;

            if (depth_cm >= 0.0)
            {
                G4int evt = G4RunManager::GetRunManager()->GetCurrentEvent()->GetEventID();
                G4double kineticEnergy_MeV = step->GetPreStepPoint()->GetKineticEnergy() / CLHEP::MeV;

                // 🖨️ طباعة نوع الجسيم (neutron, proton, gamma, e-, e+) وطاقته والعمق
                if (particleName == "neutron" || particleName == "proton" || 
                    particleName == "gamma"   || particleName == "e-" || particleName == "e+")
                {
                    G4cout << "==========> [INSIDE BONE PHANTOM] Event: " << evt 
                           << " | Particle: " << particleName 
                           << " | Energy = " << kineticEnergy_MeV << " MeV"
                           << " | Depth = " << depth_cm << " cm"
                           << " | Edep = " << edep / CLHEP::MeV << " MeV" << G4endl;
                }

                G4AnalysisManager* man = G4AnalysisManager::Instance();
                
                G4int particleTypeID = 2; // Default for Neutrons/Protons/Heavy ions
                if (particleName == "gamma") particleTypeID = 1;
                else if (particleName == "e-" || particleName == "e+") particleTypeID = 3;

                man->FillNtupleIColumn(4, 0, evt);
                man->FillNtupleDColumn(4, 1, edep / CLHEP::MeV);
                man->FillNtupleDColumn(4, 2, depth_cm);
                man->FillNtupleDColumn(4, 3, particleTypeID);
                man->AddNtupleRow(4);
            }
        }
    }

    // ----------------------------------------------------------------
    // 2. Beam Scoring at Boundaries
    // ----------------------------------------------------------------
    if (step->GetPostStepPoint()->GetStepStatus() == fGeomBoundary) 
    {
        if (!step->GetPreStepPoint()->GetTouchableHandle()->GetVolume() || 
            !step->GetPostStepPoint()->GetTouchableHandle()->GetVolume()) {
            return;
        }

        G4String volumeFrom = step->GetPreStepPoint()->GetTouchableHandle()->GetVolume()->GetName();
        G4String volumeTo   = step->GetPostStepPoint()->GetTouchableHandle()->GetVolume()->GetName();

        // Target to Moderator Interface
        if (particleName == "neutron" && volumeFrom == "physTarget" && volumeTo == "physModerator") 
        {
            if (track->GetCurrentStepNumber() <= 5) { 
                fRunAction->nModerator++;
                G4double energy_eV = step->GetPostStepPoint()->GetKineticEnergy() / CLHEP::eV;
                G4int evt = G4RunManager::GetRunManager()->GetCurrentEvent()->GetEventID();
                
                G4cout << "==========> [ENTERING MODERATOR] Event: " << evt 
                       << " | Energy = " << energy_eV << " eV (" << energy_eV/1e6 << " MeV)" << G4endl;

                G4AnalysisManager* man = G4AnalysisManager::Instance();
                man->FillNtupleIColumn(3, 0, evt);
                man->FillNtupleDColumn(3, 1, energy_eV);
                man->AddNtupleRow(3);
            }
        }

        // Entering Bone Phantom (Primary Incident Beam ONLY)
        if (volumeTo == "physBonePhantom")
        {
            G4ThreeVector momentumDir = track->GetMomentumDirection();
            G4double cosTheta = momentumDir.z();

            if (cosTheta > 0.0) 
            {
                G4double kineticEnergy = step->GetPostStepPoint()->GetKineticEnergy();
                G4double energy_eV  = kineticEnergy / CLHEP::eV;
                G4double energy_MeV = kineticEnergy / CLHEP::MeV;
                G4double weight     = track->GetWeight();

                G4int evt = G4RunManager::GetRunManager()->GetCurrentEvent()->GetEventID();
                G4ThreeVector pos = step->GetPostStepPoint()->GetPosition();
                G4double x_cm = pos.x() / CLHEP::cm;
                G4double y_cm = pos.y() / CLHEP::cm;
                G4double r_cm = std::sqrt(x_cm * x_cm + y_cm * y_cm);

                G4AnalysisManager* man = G4AnalysisManager::Instance();

                if (particleName == "neutron") 
                {
                    fRunAction->nDetector++; 
                    fRunAction->nDetectorCurrent += weight * cosTheta; 
                    fRunAction->nDetectorFlux    += weight;            

                    if (energy_eV < 0.5) {
                        fRunAction->nThermalFluxCount += weight;
                    }
                    else if (energy_eV >= 0.5 && energy_eV <= 10000.0) {
                        fRunAction->nEpithermal += weight;
                    }
                    else if (energy_eV > 10000.0) {
                        fRunAction->nFast += weight;

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
                        fRunAction->dFastAccumulated += fastKermaFactor * weight;
                    }

                    // 🖨️ طباعة النيوترونات عند السطح
                    G4cout << "==========> [ENTERING BONE SURFACE] Event: " << evt 
                           << " | Particle: Neutron | Energy = " << energy_MeV << " MeV" << G4endl;

                    man->FillNtupleIColumn(1, 0, evt);
                    man->FillNtupleDColumn(1, 1, energy_eV);    
                    man->FillNtupleDColumn(1, 2, cosTheta);     
                    man->FillNtupleDColumn(1, 3, x_cm);         
                    man->FillNtupleDColumn(1, 4, y_cm);         
                    man->FillNtupleDColumn(1, 5, r_cm);         
                    man->FillNtupleDColumn(1, 6, weight);   
                    man->AddNtupleRow(1);
                }
                else if (particleName == "gamma") 
                {
                    fRunAction->nGamma += weight; 

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
                    fRunAction->dGammaAccumulated += gammaKermaFactor * weight;

                    // 🖨️ طباعة غاما عند السطح
                    G4cout << "==========> [ENTERING BONE SURFACE] Event: " << evt 
                           << " | Particle: Gamma | Energy = " << energy_MeV << " MeV" << G4endl;

                    man->FillNtupleIColumn(2, 0, evt);
                    man->FillNtupleDColumn(2, 1, energy_MeV);  
                    man->FillNtupleDColumn(2, 2, x_cm);
                    man->FillNtupleDColumn(2, 3, y_cm);
                    man->FillNtupleDColumn(2, 4, weight);
                    man->AddNtupleRow(2);
                }
            }
        }
    }
}
