#ifndef CONSTRUCTION_HH
#define CONSTRUCTION_HH

#include "G4VUserDetectorConstruction.hh"
#include "G4VPhysicalVolume.hh"
#include "G4Box.hh"
#include "G4PVPlacement.hh"
#include "G4NistManager.hh"
#include "G4LogicalVolume.hh"
#include "G4SystemOfUnits.hh"
#include "G4VPhysicalVolume.hh"
#include "G4PVPlacement.hh"
#include "G4GenericMessenger.hh"

#include "detector.hh"
class MyDetectorConstruction : public G4VUserDetectorConstruction
{
 public:
      MyDetectorConstruction();
     ~MyDetectorConstruction();

      G4LogicalVolume *GetScoringVolume() const { return fScoringVolume; }

        virtual G4VPhysicalVolume *Construct();
        virtual void ConstructSDandFields();

   private:
    G4LogicalVolume *logicDetector;

    G4int nCols, nRows;

    G4Box *solidWorld, *solidRadiator , *solidDetector;
    G4LogicalVolume *logicWorld, *logicRadiator;
    G4VPhysicalVolume *physWorld ,*physRadiator ,*physDetector;

    G4GenericMessenger *fMessenger;

    G4LogicalVolume *fScoringVolume;

    G4Material *SiO2, *H2O, *Aerogel, *WorldMat;
    G4Element *C;

   void DefineMaterial();
};
#endif


