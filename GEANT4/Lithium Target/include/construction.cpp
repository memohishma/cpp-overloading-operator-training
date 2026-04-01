ifndef CONSTRUCTION_HH
#define CONSTRUCTION_HH

#include "G4VPhysicalVolume.hh"
#include "G4SystemOfUnits.hh"
#include "G4VUserDetectorConstruction.hh"
#include "G4Box.hh"
#include "G4PVPlacement.hh"
#include "G4NistManager.hh"
#include "G4LogicalVolume.hh"

class MyDetector : public G4VUserDetectorConstruction
{
public:

    MyDetector();
    virtual ~MyDetector();

    virtual G4VPhysicalVolume *Construct();
};
#endif

