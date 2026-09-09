#include "construction.hh"
#include "messenger.hh"
#include "G4PVPlacement.hh"  
#include "G4SubtractionSolid.hh" 
#include "G4Cons.hh"             
#include "G4Tubs.hh"             
#include "G4VisAttributes.hh"    
#include "G4Colour.hh"           
#include "G4SDManager.hh" 
#include "G4UserLimits.hh"
#include "G4MultiFunctionalDetector.hh"
#include "G4VPrimitiveScorer.hh"
#include "G4RunManager.hh"
#include "G4PSDoseDeposit.hh" 
#include "G4SDParticleFilter.hh"
#include "G4NistManager.hh"
#include "G4Box.hh"
#include "G4Isotope.hh"
#include "G4Element.hh"
#include "G4Material.hh"
#include "G4UIcmdWithADoubleAndUnit.hh"



MyDetectorConstruction::MyDetectorConstruction()
: logicTarget(nullptr), logicModerator(nullptr), logicFastFilter(nullptr), logicGammaFilter(nullptr),  
  logicCollimator(nullptr), logicReflector(nullptr), logicDetector(nullptr),
  fModeratorThickness(20.0 * cm) 
{
    fMessenger = new MyDetectorMessenger(this);
}

MyDetectorConstruction::~MyDetectorConstruction()
{
    delete fMessenger;
}

G4VPhysicalVolume *MyDetectorConstruction::Construct()
{
    G4NistManager *nist = G4NistManager::Instance();

    G4Material* vacuum = nist->FindOrBuildMaterial("G4_Galactic");
    
    G4Isotope* Li7 = new G4Isotope("Li7", 3, 7, 7.016*g/mole);
    G4Element* elLi7 = new G4Element("Lithium-7", "Li7", 1);
    elLi7->AddIsotope(Li7, 1.0);
    
    G4double liDensity = 0.534 * g/cm3; 
    G4Material* targetLi7 = new G4Material("Target_Li7", liDensity, 1);
    targetLi7->AddElement(elLi7, 1);

    G4Element* elB = nist->FindOrBuildElement("B");
    G4Element* elF = nist->FindOrBuildElement("F");
    G4Material* BF3Gas = new G4Material("BF3Gas", 0.00276*g/cm3, 2);
    BF3Gas->AddElement(elB, 1); 
    BF3Gas->AddElement(elF, 3); 

    G4Material *nickel = nist->FindOrBuildMaterial("G4_Ni");  
    G4Material *lead = nist->FindOrBuildMaterial("G4_Pb");                 
    G4Material *bismuth = nist->FindOrBuildMaterial("G4_Bi");              
    G4Material *MgF2 = nist->FindOrBuildMaterial("G4_MAGNESIUM_FLUORIDE");
    G4Material *tungsten = nist->FindOrBuildMaterial("G4_W"); 

    G4double LWorld = 80*cm;  
    
    G4double targetX = 3.*cm;     
    G4double targetY = 3.*cm;     
    G4double targetZ = (80 / 2.0) *um;    

    G4double bsaX = 25.*cm;                
    G4double bsaY = 25.*cm;                
    
    G4double hzModerator = fModeratorThickness / 2.0;
    
    G4cout << "\n*** CONSTRUCTING MODERATOR: total thickness = "
       << fModeratorThickness / cm
       << " cm ***\n" << G4endl;
       
    G4double reflectorThickness = 25.*cm;
    G4double hzFastFilter = (0.7 / 2.0)*cm;
    G4double hzGammaFilter = (3.0 / 2.0)*cm;
    G4double hzCollimator = (5.0 / 2.0)*cm;  

    G4double beamPipeRadius = 3.0 * cm;         
    G4double pbSleeveThickness = 3.0 * cm;     

    G4double currentZ = 0.*cm;

    currentZ += targetZ;
    G4ThreeVector xyzTarget(0., 0., currentZ);

    currentZ += targetZ + hzModerator;
    G4ThreeVector xyzModerator(0., 0., currentZ);

    currentZ += hzModerator + hzFastFilter;
    G4ThreeVector xyzFastFilter(0., 0., currentZ);

    currentZ += hzFastFilter + hzGammaFilter;
    G4ThreeVector xyzGammaFilter(0., 0., currentZ);

    currentZ += hzGammaFilter + hzCollimator;
    G4ThreeVector xyzCollimator(0., 0., currentZ);

    G4double fullBsaLength = 2.0*hzModerator + 2.0*hzFastFilter + 2.0*hzGammaFilter + 2.0*hzCollimator;
    G4double halfBsaLength = fullBsaLength / 2.0;

    G4double bsaStartZ = xyzModerator.z() - hzModerator;
    G4double bsaEndZ   = xyzCollimator.z() + hzCollimator;
    G4double bsaCenterZ = 0.5 * (bsaStartZ + bsaEndZ);

    G4double reflectorExtra = 1.0 * cm;
    G4double reflectorBottomZ = (xyzTarget.z() - targetZ) - 20.0 * cm; 
    G4double reflectorTopZ    = bsaEndZ + reflectorExtra;

    G4double reflectorHalfLength = 0.5 * (reflectorTopZ - reflectorBottomZ);
    G4double reflectorCenterZ    = 0.5 * (reflectorTopZ + reflectorBottomZ);

    G4Box *solidWorld = new G4Box("solidWorld", LWorld, LWorld, LWorld);
    G4LogicalVolume *logicWorld = new G4LogicalVolume(solidWorld, vacuum, "logicWorld");
    G4VPhysicalVolume *physWorld = new G4PVPlacement(0, G4ThreeVector(0., 0., 0.), logicWorld, "physWorld", 0, false, 0, true);

    G4VisAttributes *worldVis = new G4VisAttributes(G4Colour(0.8, 0.8, 0.8, 0.05)); 
    worldVis->SetForceSolid(true); 
    logicWorld->SetVisAttributes(worldVis);

    G4Box *solidTarget = new G4Box("solidTarget", targetX, targetY, targetZ);
    logicTarget = new G4LogicalVolume(solidTarget, targetLi7, "logicTarget");
    new G4PVPlacement(0, xyzTarget, logicTarget, "physTarget", logicWorld, false, 0, true);

    G4VisAttributes *targetVis = new G4VisAttributes(G4Colour(1.0, 0.5, 0.0, 0.8));
    targetVis->SetForceSolid(true);
    logicTarget->SetVisAttributes(targetVis);

    G4Box *solidFastFilter = new G4Box("solidFastFilter", bsaX, bsaY, hzFastFilter);
    logicFastFilter = new G4LogicalVolume(solidFastFilter, nickel, "logicFastFilter");
    new G4PVPlacement(0, xyzFastFilter, logicFastFilter, "physFastFilter", logicWorld, false, 0, true);

    G4VisAttributes *niVis = new G4VisAttributes(G4Colour(0.4, 0.8, 0.5, 0.5));
    niVis->SetForceSolid(true);
    logicFastFilter->SetVisAttributes(niVis);

    G4Box *solidModerator = new G4Box("solidModerator", bsaX, bsaY, hzModerator);
    logicModerator = new G4LogicalVolume(solidModerator, MgF2, "logicModerator");
    new G4PVPlacement(0, xyzModerator, logicModerator, "physModerator", logicWorld, false, 0, true);
    
    G4VisAttributes *modVis = new G4VisAttributes(G4Colour(0.2, 0.5, 1.0, 0.5));
    modVis->SetForceSolid(true);
    logicModerator->SetVisAttributes(modVis);

    G4Box *solidGammaFilter = new G4Box("solidGammaFilter", bsaX, bsaY, hzGammaFilter);
    logicGammaFilter = new G4LogicalVolume(solidGammaFilter, bismuth, "logicGammaFilter");
    new G4PVPlacement(0, xyzGammaFilter, logicGammaFilter, "physGammaFilter", logicWorld, false, 0, true);

    G4VisAttributes *gammaVis = new G4VisAttributes(G4Colour(1.0, 0.6, 0.8, 0.5));
    gammaVis->SetForceSolid(true);
    logicGammaFilter->SetVisAttributes(gammaVis);

    G4Box *solidCollimatorOuterBox = new G4Box("solidCollimatorOuterBox", bsaX, bsaY, hzCollimator);
    G4double Rmin1_tunnel = 0.*mm;     
    G4double Rmax1_tunnel = 9.0*cm;     
    G4double Rmin2_tunnel = 0.*mm;     
    G4double Rmax2_tunnel = 7.0*cm;     
    
    G4Cons *solidCollimatorInnerCone = new G4Cons("solidCollimatorInnerCone", Rmin1_tunnel, Rmax1_tunnel, Rmin2_tunnel, Rmax2_tunnel, hzCollimator + 1.*mm, 0.*deg, 360.*deg);
    G4SubtractionSolid *solidCollimator = new G4SubtractionSolid("solidCollimator", solidCollimatorOuterBox, solidCollimatorInnerCone);
    logicCollimator = new G4LogicalVolume(solidCollimator, tungsten, "logicCollimator");
    new G4PVPlacement(0, xyzCollimator, logicCollimator, "physCollimator", logicWorld, false, 0, true);

    G4VisAttributes *collVis = new G4VisAttributes(G4Colour(1.0, 0.8, 0.1, 0.6));
    collVis->SetForceSolid(true);
    logicCollimator->SetVisAttributes(collVis);

    G4double pipeBeforeStartZ = -LWorld;
    G4double pipeBeforeEndZ = reflectorBottomZ;
    G4double pipeBeforeHalfZ = (pipeBeforeEndZ - pipeBeforeStartZ) / 2.0;
    G4double pipeBeforeCenterZ = pipeBeforeStartZ + pipeBeforeHalfZ;
    G4ThreeVector xyzPipeBefore(0., 0., pipeBeforeCenterZ);

    G4Tubs *solidPipeBefore = new G4Tubs("solidPipeBefore", 0.*cm, beamPipeRadius, pipeBeforeHalfZ, 0.*deg, 360.*deg);
    G4LogicalVolume *logicPipeBefore = new G4LogicalVolume(solidPipeBefore, vacuum, "logicPipeBefore");
    new G4PVPlacement(0, xyzPipeBefore, logicPipeBefore, "physPipeBefore", logicWorld, false, 0, true);

    G4VisAttributes *pipeBeforeVis = new G4VisAttributes(G4Colour(0.6, 0.6, 0.6, 0.6));
    pipeBeforeVis->SetForceSolid(true);
    logicPipeBefore->SetVisAttributes(pipeBeforeVis);

    G4double pipeInsideStartZ = reflectorBottomZ;
    G4double pipeInsideEndZ = xyzTarget.z() - targetZ;
    G4double pipeInsideHalfZ = (pipeInsideEndZ - pipeInsideStartZ) / 2.0;
    G4double pipeInsideCenterZ = pipeInsideStartZ + pipeInsideHalfZ;
    G4ThreeVector xyzPipeInside(0., 0., pipeInsideCenterZ);

    G4Tubs *solidPipeInside = new G4Tubs("solidPipeInside", 0.*cm, beamPipeRadius, pipeInsideHalfZ, 0.*deg, 360.*deg);
    G4LogicalVolume *logicPipeInside = new G4LogicalVolume(solidPipeInside, vacuum, "logicPipeInside");
    new G4PVPlacement(0, xyzPipeInside, logicPipeInside, "physPipeInside", logicWorld, false, 0, true);

    G4VisAttributes *pipeInsideVis = new G4VisAttributes(G4Colour(0.3, 0.8, 1.0, 0.4));
    pipeInsideVis->SetForceSolid(true);
    logicPipeInside->SetVisAttributes(pipeInsideVis);

    G4Tubs *solidPbSleeve = new G4Tubs("solidPbSleeve", beamPipeRadius, beamPipeRadius + pbSleeveThickness, pipeInsideHalfZ, 0.*deg, 360.*deg);
    G4LogicalVolume *logicPbSleeve = new G4LogicalVolume(solidPbSleeve, lead, "logicPbSleeve");
    new G4PVPlacement(0, xyzPipeInside, logicPbSleeve, "physPbSleeve", logicWorld, false, 0, true);

    G4VisAttributes *sleeveVis = new G4VisAttributes(G4Colour(0.3, 0.1, 0.4, 0.6));
    sleeveVis->SetForceSolid(true);
    logicPbSleeve->SetVisAttributes(sleeveVis);

    G4Box *solidOuterReflector = new G4Box("solidOuterReflector", 
                                            bsaX + reflectorThickness, 
                                            bsaY + reflectorThickness, 
                                            reflectorHalfLength);

    G4Box *solidInnerBsaSpace = new G4Box("solidInnerBsaSpace", bsaX, bsaY, halfBsaLength + 0.1*cm); 
    G4ThreeVector bsaSubPos(0., 0., bsaCenterZ - reflectorCenterZ);

    G4double intersectStartZ = reflectorBottomZ;
    G4double intersectEndZ = xyzTarget.z() - targetZ;
    G4double intersectLength = intersectEndZ - intersectStartZ;
    G4double intersectHalfZ = intersectLength / 2.0;
    G4double intersectCenterZ = intersectStartZ + intersectHalfZ;

    G4Tubs *solidBeamChannelSpace = new G4Tubs("solidBeamChannelSpace", 0.*cm, beamPipeRadius + pbSleeveThickness + 0.1*cm, intersectHalfZ + 0.1*cm, 0.*deg, 360.*deg);
    G4ThreeVector beamChannelSubPos(0., 0., intersectCenterZ - reflectorCenterZ);

    G4double targetClearance = 0.5 * cm; 
    G4Box *solidTargetSpace = new G4Box("solidTargetSpace", 
                                        targetX + targetClearance, 
                                        targetY + targetClearance, 
                                        targetZ + targetClearance);
    G4ThreeVector targetSubPos(0., 0., xyzTarget.z() - reflectorCenterZ);

    G4SubtractionSolid *interRef1 = new G4SubtractionSolid("interRef1", solidOuterReflector, solidInnerBsaSpace, 0, bsaSubPos);
    G4SubtractionSolid *interRef2 = new G4SubtractionSolid("interRef2", interRef1, solidBeamChannelSpace, 0, beamChannelSubPos);
    G4SubtractionSolid *solidReflector = new G4SubtractionSolid("solidReflector", interRef2, solidTargetSpace, 0, targetSubPos);

    logicReflector = new G4LogicalVolume(solidReflector, lead, "logicReflector");
    new G4PVPlacement(0, G4ThreeVector(0., 0., reflectorCenterZ), logicReflector, "physReflector", logicWorld, false, 0, true);

    G4VisAttributes *refVis = new G4VisAttributes(G4Colour(0.6, 0.2, 0.8, 0.3));
    refVis->SetForceSolid(true);
    logicReflector->SetVisAttributes(refVis);

    G4double detectorRadius = 7.0 * cm;  
    G4double detectorHalfLength = 1.0 * cm; 

    G4Tubs *solidBF3Detector = new G4Tubs("solidBF3Detector", 0.*mm, detectorRadius, detectorHalfLength, 0.*deg, 360.*deg);
    logicDetector = new G4LogicalVolume(solidBF3Detector, BF3Gas, "LogicDetector");
    
    G4double detectorZ = xyzCollimator.z() + hzCollimator + detectorHalfLength;
    G4ThreeVector xyzDetector(0., 0., detectorZ);
    
    new G4PVPlacement(0, xyzDetector, logicDetector, "physDetector", logicWorld, false, 0, true);

    G4VisAttributes *detVis = new G4VisAttributes(G4Colour(0.1, 0.8, 0.3, 0.5));
    detVis->SetForceSolid(true);
    logicDetector->SetVisAttributes(detVis);
    
    G4Region* targetRegion = new G4Region("TargetRegion");
    targetRegion->AddRootLogicalVolume(logicTarget);
    G4UserLimits* stepLimit = new G4UserLimits(1.0 * um);
    targetRegion->SetUserLimits(stepLimit); 

    G4Region* bsaRegion = new G4Region("BSARegion");
    bsaRegion->AddRootLogicalVolume(logicFastFilter);
    bsaRegion->AddRootLogicalVolume(logicModerator);
    bsaRegion->AddRootLogicalVolume(logicGammaFilter);
    
    G4Region* reflectorRegion = new G4Region("ReflectorRegion");
    reflectorRegion->AddRootLogicalVolume(logicReflector);  
    reflectorRegion->AddRootLogicalVolume(logicCollimator);
    reflectorRegion->AddRootLogicalVolume(logicPbSleeve);
  
    return physWorld;
}

void MyDetectorConstruction::ConstructSDandField()
{
    G4SDManager* sdMan = G4SDManager::GetSDMpointer();
    
    MySensitiveDetector *bsaSensitiveDetector = new MySensitiveDetector("BsaSD");
    sdMan->AddNewDetector(bsaSensitiveDetector);

    if(logicTarget)     logicTarget->SetSensitiveDetector(bsaSensitiveDetector);
    if(logicFastFilter) logicFastFilter->SetSensitiveDetector(bsaSensitiveDetector);
    if(logicModerator)  logicModerator->SetSensitiveDetector(bsaSensitiveDetector);
    if(logicGammaFilter) logicGammaFilter->SetSensitiveDetector(bsaSensitiveDetector);
    if(logicCollimator) logicCollimator->SetSensitiveDetector(bsaSensitiveDetector);
    if(logicReflector)  logicReflector->SetSensitiveDetector(bsaSensitiveDetector);

    G4MultiFunctionalDetector* detectorScorer = new G4MultiFunctionalDetector("DetectorScorer");
    sdMan->AddNewDetector(detectorScorer);

    G4VPrimitiveScorer* gammaDose = new G4PSDoseDeposit("GammaDose");
    G4SDParticleFilter* gammaFilter = new G4SDParticleFilter("gammaFilter");
    gammaFilter->add("gamma");
    gammaDose->SetFilter(gammaFilter);
    detectorScorer->RegisterPrimitive(gammaDose);

    if(logicDetector) {
        logicDetector->SetSensitiveDetector(detectorScorer);
    }
}

void MyDetectorConstruction::SetModeratorThickness(G4double val)
{
    fModeratorThickness = val;

    G4cout << "\n========================================\n";
    G4cout << "MODERATOR THICKNESS SET TO: "
           << fModeratorThickness / cm
           << " cm\n";
    G4cout << "========================================\n"
           << G4endl;

    G4RunManager::GetRunManager()->GeometryHasBeenModified();
}


