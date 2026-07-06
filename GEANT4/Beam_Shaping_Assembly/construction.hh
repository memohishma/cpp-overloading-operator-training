#ifndef CONSTRUCTION_HH
#define CONSTRUCTION_HH

#include "G4VUserDetectorConstruction.hh"
#include "G4VPhysicalVolume.hh"
#include "G4LogicalVolume.hh"
#include "G4Box.hh"
#include "G4Tubs.hh"
#include "G4NistManager.hh"
#include "G4SystemOfUnits.hh"
#include "detector.hh"
#include "G4MultiFunctionalDetector.hh"

class MyDetectorConstruction : public G4VUserDetectorConstruction
{
public:
    MyDetectorConstruction();
    ~MyDetectorConstruction();

    virtual G4VPhysicalVolume *Construct() override;
    virtual void ConstructSDandField() override;

private:

    G4MultiFunctionalDetector* bsaScorer;
    
    G4LogicalVolume *logicTarget;
    G4LogicalVolume *logicNickel;
    G4LogicalVolume *logicModerator;
    G4LogicalVolume *logicGammaFilter;
    G4LogicalVolume *logicThermalFilter;
    G4LogicalVolume *logicCollimator;
    G4LogicalVolume *logicReflector;
    G4LogicalVolume *logicDetector;
    
    
};

#endif
