#include "construction.hh"

#include "G4NistManager.hh"
#include "G4Material.hh"
#include "G4Box.hh"
#include "G4LogicalVolume.hh"
#include "G4PVPlacement.hh"
#include "G4VPhysicalVolume.hh"
#include "G4SystemOfUnits.hh"

MyDetector::MyDetector()
{}
MyDetector::~MyDetector()
{}
G4VPhysicalVolume *MyDetector::Construct()
{
    G4NistManager *nist = G4NistManager::Instance();
    //Material Definition of Polyethelyne
    G4Material *HDPE = new G4Material("HDPE",0.94*g/cm3,2);
    HDPE->AddElement(nist->FindOrBuildElement("C"),1);
    HDPE->AddElement(nist->FindOrBuildElement("H"),2);

    //Material Definition of Air
    G4Material *worldMat = nist->FindOrBuildMaterial("G4_AIR");

    //World Definition
    G4Box *solidWorld = new G4Box("solidWorld",0.5*m,0.5*m,0.5*m);

    G4LogicalVolume *logicWorld = new G4LogicalVolume(solidWorld,worldMat,"logicWorld");

    G4VPhysicalVolume *physWorld = new G4PVPlacement(0 , G4ThreeVector(0.,0.,0.), logicWorld ,"physWorld", 0, false, 0, true);

    G4Box *solidPE = new G4Box("solidPE",0.5*m ,0.5*m ,0.01*m);

    G4LogicalVolume *logicPE = new G4LogicalVolume(solidPE ,HDPE,"logicPE");

    G4VPhysicalVolume *physPE = new G4PVPlacement(0 , G4ThreeVector(0.,0.,0.25*m), logicPE ,"physPE", logicWorld , false, 0, true);


    return physWorld;
}
