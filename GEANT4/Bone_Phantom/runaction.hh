#ifndef RUNACTION_HH
#define RUNACTION_HH

#include <iostream>
#include <fstream>

#include "G4UserRunAction.hh"
#include "G4AnalysisManager.hh"
#include "G4RootAnalysisManager.hh"
#include "G4RunManager.hh"
#include "G4Threading.hh"
#include "G4Accumulable.hh"

class G4Run;

class MyRunAction : public G4UserRunAction
{
public:
    MyRunAction();
    ~MyRunAction();

    virtual void BeginOfRunAction(const G4Run*);
    virtual void EndOfRunAction(const G4Run*);

    // Accumulables للعدادات والجرعات
    G4Accumulable<G4int> nTarget = 0;
    G4Accumulable<G4int> nModerator = 0;
    G4Accumulable<G4int> nFastFilter = 0;
    G4Accumulable<G4int> nGamma = 0;
    G4Accumulable<G4int> nCollimator = 0;
    G4Accumulable<G4int> nReflector = 0;
    G4Accumulable<G4int> nDetector = 0;

    // متغيّرات دقيقة لحساب اتجاهية الحزمة J/Phi
    G4Accumulable<G4double> nDetectorCurrent = 0.0; // J component (Weighted by cosTheta)
    G4Accumulable<G4double> nDetectorFlux    = 0.0; // Phi component (Weighted)

    G4Accumulable<G4double> dFastAccumulated = 0.0;
    G4Accumulable<G4double> dGammaAccumulated = 0.0;

    G4Accumulable<G4double> nThermalFluxCount = 0.0; // أوزان معدلة لسرعة وحساب الـ Flux
    G4Accumulable<G4double> nEpithermal = 0.0;
    G4Accumulable<G4double> nFast = 0.0;
};

#endif
