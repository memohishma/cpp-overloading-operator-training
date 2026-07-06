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

//create a Run Action class, inheriting from GEANT4's generic Run Action class
class MyRunAction : public G4UserRunAction
{
    public:
        MyRunAction(); //constructor
        ~MyRunAction(); //destructor

        virtual void BeginOfRunAction(const G4Run*); //function to perform any action before each run
        virtual void EndOfRunAction(const G4Run*); //function to perform any action after each run

  // استبدلي الـ G4int القديمة بهذا التعريف الذكي:
    G4Accumulable<G4int> nTarget = 0;
    G4Accumulable<G4int> nNickel = 0;
    G4Accumulable<G4int> nModerator = 0;
    G4Accumulable<G4int> nGamma = 0;
    G4Accumulable<G4int> nThermal = 0;
    G4Accumulable<G4int> nCollimator = 0;
    G4Accumulable<G4int> nReflector = 0;
    G4Accumulable<G4int> nDetector = 0;
     
    G4Accumulable<G4int> nThermalFluxCount = 0; // تم تغيير الاسم هنا لمنع التكرار
    G4Accumulable<G4int> nEpithermal = 0;
    G4Accumulable<G4int> nFast = 0;
};

#endif
