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

class MyRunAction : public G4UserRunAction
{
    public:
        MyRunAction();
        ~MyRunAction();

        virtual void BeginOfRunAction(const G4Run*);
        virtual void EndOfRunAction(const G4Run*);

        G4Accumulable<G4int> nTarget = 0;
        G4Accumulable<G4int> nFastFilter = 0;
        G4Accumulable<G4int> nModerator = 0;
        G4Accumulable<G4int> nCollimator = 0;
        G4Accumulable<G4int> nReflector = 0;
        G4Accumulable<G4int> nDetector = 0;
         
        // عدادات الكيرما المنفصلة (Flux vs Current)
        G4Accumulable<G4double> dFastFluxAccumulated = 0;
        G4Accumulable<G4double> dFastCurrentAccumulated = 0;
        G4Accumulable<G4double> dGammaFluxAccumulated = 0;
        G4Accumulable<G4double> dGammaCurrentAccumulated = 0;

        // عدادات الفيض (Surface Flux / Cosine Corrected)
        G4Accumulable<G4int> nThermalFluxCount = 0;
        G4Accumulable<G4int> nEpithermalFlux = 0;
        G4Accumulable<G4int> nFastFlux = 0;
        G4Accumulable<G4int> nGammaFlux = 0;

        // عدادات التيار (Surface Current / No Cosine)
        G4Accumulable<G4int> nThermalCurrentCount = 0;
        G4Accumulable<G4int> nEpithermalCurrent = 0;
        G4Accumulable<G4int> nFastCurrent = 0;
        G4Accumulable<G4int> nGammaCurrent = 0;
};

#endif
