#include "construction.hh"

MyDetectorConstruction::MyDetectorConstruction()
: logicTarget(nullptr), logicDetector(nullptr)
{}

MyDetectorConstruction::~MyDetectorConstruction()
{}

G4VPhysicalVolume *MyDetectorConstruction::Construct()
{
    G4NistManager *nist = G4NistManager::Instance();

    G4Material *vacuum = nist->BuildMaterialWithNewDensity("vacuum", "G4_AIR", (0.011 / (287 * 300))); 
    G4Material *Li = nist->FindOrBuildMaterial("G4_Li");
    G4Material *aluminum = nist->FindOrBuildMaterial("G4_Al");

    G4double LWorld = 20*cm;
    G4double targetX = 6.*cm;     // نصف الطول
    G4double targetY = 6.*cm;     // نصف العرض
    G4double targetZ = 200.*um;    // نصف السُمك (الكلي 100 ميكرومتر)

    G4ThreeVector xyzTarget(0., 0., targetZ);
    G4ThreeVector xyzDetector(0., 0., 6.1*cm);

    G4Box *solidWorld = new G4Box("solidWorld", LWorld, LWorld, LWorld);
    G4LogicalVolume *logicWorld = new G4LogicalVolume(solidWorld, vacuum, "logicWorld");
    G4VPhysicalVolume *physWorld = new G4PVPlacement(0, G4ThreeVector(0., 0., 0.), logicWorld, "physWorld", 0, false, 0, true);

    G4Box *solidTarget = new G4Box("solidTarget", targetX, targetY, targetZ);
    logicTarget = new G4LogicalVolume(solidTarget, Li, "logicTarget");
    new G4PVPlacement(0, xyzTarget, logicTarget, "physTarget", logicWorld, false, 0, true);

    G4Tubs *solidBF3Detector = new G4Tubs("solidBF3Detector", 0.*mm, 6*cm, 6.*cm, 0.*deg, 360.*deg);
    logicDetector = new G4LogicalVolume(solidBF3Detector, aluminum, "LogicDetector");
    new G4PVPlacement(0, xyzDetector, logicDetector, "physDetector", logicWorld, false, 0, true);

    return physWorld;
}

void MyDetectorConstruction::ConstructSDandField()
{
    MySensitiveDetector *targetDet = new MySensitiveDetector("TargetDetector");
    if(logicTarget) logicTarget->SetSensitiveDetector(targetDet);
    
    MySensitiveDetector *neutronDet = new MySensitiveDetector("NeutronDetector");
    if(logicDetector) logicDetector->SetSensitiveDetector(neutronDet);
}
