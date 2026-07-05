//lITHIUM TARGET ONLY
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

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//ADDING THE BSA COMPONENTS


#include "construction.hh"
#include "G4SubtractionSolid.hh" // مكتبة الطرح الهندسي للأشكال المركبة
#include "G4Cons.hh"             // مكتبة المخروط المصمت للتجويف
#include "G4VisAttributes.hh"    
#include "G4Colour.hh"           

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

    // مواد الـ BSA
    G4Material *nickel = nist->FindOrBuildMaterial("G4_Ni");            
    G4Material *teflon = nist->FindOrBuildMaterial("G4_TEFLON");        
    G4Material *lead = nist->FindOrBuildMaterial("G4_Pb");              
    G4Material *bismuth = nist->FindOrBuildMaterial("G4_Bi");          
    G4Material *cadmium = nist->FindOrBuildMaterial("G4_Cd");          

    // حجم العالم (World)
    G4double LWorld = 60*cm; 
    
    // أبعاد الهدف الأصلي
    G4double targetX = 6.*cm;     
    G4double targetY = 6.*cm;     
    G4double targetZ = 70.*um;    

    // أبعاد الـ BSA بناءً على القيم المثالية المحددة في البحث
    G4double bsaX = 25.*cm;              
    G4double bsaY = 25.*cm;              
    
    // السماكات المثالية (نصف الأبعاد للجينت 4)
    G4double hzNickel = 0.75*cm;         // الكلي 1.5 سم
    G4double hzModerator = 10.*cm;       // الكلي 20 سم
    G4double hzGammaFilter = 1.5*cm;     // الكلي 3 سم
    G4double hzThermalFilter = 0.8*cm;   // الكلي 1.6 سم
    G4double hzCollimator = 2.0*cm;      // الكلي 4 سم
    
    G4double reflectorThickness = 20.*cm;

    // حساب المتغير الديناميكي للمواقع على محور Z لترتيب الطبقات تلقائياً
    G4double currentZ = 0.*cm;

    // إحداثيات الهدف
    currentZ += targetZ;
    G4ThreeVector xyzTarget(0., 0., currentZ);

    // موقع مرشح النيوترونات السريعة (Nickel)
    currentZ += targetZ + hzNickel;
    G4ThreeVector xyzNickel(0., 0., currentZ);

    // موقع المهدئ (Teflon)
    currentZ += hzNickel + hzModerator;
    G4ThreeVector xyzModerator(0., 0., currentZ);

    // موقع مصفاة غاما (Bismuth)
    currentZ += hzModerator + hzGammaFilter;
    G4ThreeVector xyzGammaFilter(0., 0., currentZ);

    // موقع مصفاة النيوترونات الحرارية (Cadmium)
    currentZ += hzGammaFilter + hzThermalFilter;
    G4ThreeVector xyzThermalFilter(0., 0., currentZ);

    // موقع الموازئ (Collimator Box)
    currentZ += hzThermalFilter + hzCollimator;
    G4ThreeVector xyzCollimator(0., 0., currentZ);

    // تحديد موقع الكاشف خلف نهاية الموازئ بـ 6 سم
    currentZ += hzCollimator + 6.*cm; 
    G4ThreeVector xyzDetector(0., 0., currentZ);

    // بناء حجم العالم (World)
    G4Box *solidWorld = new G4Box("solidWorld", LWorld, LWorld, LWorld);
    G4LogicalVolume *logicWorld = new G4LogicalVolume(solidWorld, vacuum, "logicWorld");
    G4VPhysicalVolume *physWorld = new G4PVPlacement(0, G4ThreeVector(0., 0., 0.), logicWorld, "physWorld", 0, false, 0, true);

    G4VisAttributes *worldVis = new G4VisAttributes(G4Colour(0.8, 0.8, 0.8, 0.1)); 
    worldVis->SetForceSolid(true); 
    logicWorld->SetVisAttributes(worldVis);

    // أنبوب الحزمة والدرع الأسطواني المحيط بالبروتونات
    G4double pipeInnerRadius = 3.0*cm;   
    G4double pipeOuterRadius = 3.5*cm;   
    G4double pipeLengthZ = (LWorld + (xyzTarget.z() - targetZ)) / 2.0; 
    G4double zPosPipe = -LWorld + pipeLengthZ; 
    G4ThreeVector xyzBeamPipe(0., 0., zPosPipe);

    G4Tubs *solidBeamPipe = new G4Tubs("solidBeamPipe", pipeInnerRadius, pipeOuterRadius, pipeLengthZ, 0.*deg, 360.*deg);
    G4LogicalVolume *logicBeamPipe = new G4LogicalVolume(solidBeamPipe, aluminum, "logicBeamPipe");
    new G4PVPlacement(0, xyzBeamPipe, logicBeamPipe, "physBeamPipe", logicWorld, false, 0, true);

    G4VisAttributes *pipeVis = new G4VisAttributes(G4Colour(0.4, 0.4, 0.4, 0.5)); 
    pipeVis->SetForceSolid(true);
    logicBeamPipe->SetVisAttributes(pipeVis);

    // بناء شريحة الليثيوم الأصلية
    G4Box *solidTarget = new G4Box("solidTarget", targetX, targetY, targetZ);
    logicTarget = new G4LogicalVolume(solidTarget, Li, "logicTarget");
    new G4PVPlacement(0, xyzTarget, logicTarget, "physTarget", logicWorld, false, 0, true);

    G4VisAttributes *targetVis = new G4VisAttributes(G4Colour(1.0, 0.4, 0.0, 0.5)); 
    targetVis->SetForceSolid(true);
    logicTarget->SetVisAttributes(targetVis);

    // بناء وتلوين صندوق النيكل (Nickel Box)
    G4Box *solidNickel = new G4Box("solidNickel", bsaX, bsaY, hzNickel);
    G4LogicalVolume *logicNickel = new G4LogicalVolume(solidNickel, nickel, "logicNickel");
    new G4PVPlacement(0, xyzNickel, logicNickel, "physNickel", logicWorld, false, 0, true);

    G4VisAttributes *niVis = new G4VisAttributes(G4Colour(0.6, 0.4, 0.2, 0.4)); 
    niVis->SetForceSolid(true);
    logicNickel->SetVisAttributes(niVis);

    // أ- المهدئ المربع (Teflon Box)
    G4Box *solidModerator = new G4Box("solidModerator", bsaX, bsaY, hzModerator);
    G4LogicalVolume *logicModerator = new G4LogicalVolume(solidModerator, teflon, "logicModerator");
    new G4PVPlacement(0, xyzModerator, logicModerator, "physModerator", logicWorld, false, 0, true);

    G4VisAttributes *modVis = new G4VisAttributes(G4Colour(0.0, 0.6, 1.0, 0.3)); 
    modVis->SetForceSolid(true);
    logicModerator->SetVisAttributes(modVis);

    // ب- مصفاة غاما المربعة (Bismuth Box)
    G4Box *solidGammaFilter = new G4Box("solidGammaFilter", bsaX, bsaY, hzGammaFilter);
    G4LogicalVolume *logicGammaFilter = new G4LogicalVolume(solidGammaFilter, bismuth, "logicGammaFilter");
    new G4PVPlacement(0, xyzGammaFilter, logicGammaFilter, "physGammaFilter", logicWorld, false, 0, true);

    G4VisAttributes *gammaVis = new G4VisAttributes(G4Colour(0.0, 0.8, 0.2, 0.4)); 
    gammaVis->SetForceSolid(true);
    logicGammaFilter->SetVisAttributes(gammaVis);

    // ج- مصفاة النيوترونات الحرارية المربعة (Cadmium Box)
    G4Box *solidThermalFilter = new G4Box("solidThermalFilter", bsaX, bsaY, hzThermalFilter);
    G4LogicalVolume *logicThermalFilter = new G4LogicalVolume(solidThermalFilter, cadmium, "logicThermalFilter");
    new G4PVPlacement(0, xyzThermalFilter, logicThermalFilter, "physThermalFilter", logicWorld, false, 0, true);

    G4VisAttributes *thermalVis = new G4VisAttributes(G4Colour(1.0, 0.9, 0.0, 0.4)); 
    thermalVis->SetForceSolid(true);
    logicThermalFilter->SetVisAttributes(thermalVis);

    // ==========================================
    // [التعديل هنا]: الموازئ كصندوق مربع مفرغ مخروطياً من المنتصف (Box with Cone Subtraction)
    G4Box *solidCollimatorOuterBox = new G4Box("solidCollimatorOuterBox", bsaX, bsaY, hzCollimator);
    
    // تعريف المخروط المصمت الذي سيتم طرحه من الصندوق لإنشاء النفق المخروطي الداخلي
    G4double Rmin1_tunnel = 0.*mm;     // نصف القطر الداخلي للمخروط نفسه دائماً 0 لأنه مصمت
    G4double Rmax1_tunnel = 5.0*cm;    // نصف القطر الخارجي للمخروط عند البداية (فتحة الدخول الواسعة جهة الفلاتر)
    G4double Rmin2_tunnel = 0.*mm;     
    G4double Rmax2_tunnel = 2.5*cm;    // نصف القطر الخارجي للمخروط عند النهاية (فتحة الخروج الضيقة جهة الكاشف)
    
    G4Cons *solidCollimatorInnerCone = new G4Cons("solidCollimatorInnerCone", Rmin1_tunnel, Rmax1_tunnel, Rmin2_tunnel, Rmax2_tunnel, hzCollimator + 1.*mm, 0.*deg, 360.*deg);
    
    // طرح المخروط من الصندوق
    G4SubtractionSolid *solidCollimator = new G4SubtractionSolid("solidCollimator", solidCollimatorOuterBox, solidCollimatorInnerCone);
    
    G4LogicalVolume *logicCollimator = new G4LogicalVolume(solidCollimator, aluminum, "logicCollimator");
    new G4PVPlacement(0, xyzCollimator, logicCollimator, "physCollimator", logicWorld, false, 0, true);

    G4VisAttributes *collVis = new G4VisAttributes(G4Colour(1.0, 0.2, 0.2, 0.4)); // لون أحمر شفاف
    collVis->SetForceSolid(true);
    logicCollimator->SetVisAttributes(collVis);
    // ==========================================

    // د- العاكس المربع الخارجي (Lead Reflector Box)
    G4double totalBsaLength = hzNickel + hzModerator + hzGammaFilter + hzThermalFilter + hzCollimator;
    G4ThreeVector xyzReflector(0., 0., xyzNickel.z() - hzNickel + totalBsaLength);

    G4VSolid *solidOuterReflector = new G4Box("solidOuterReflector", bsaX + reflectorThickness, bsaY + reflectorThickness, totalBsaLength);
    G4VSolid *solidInnerReflectorSpace = new G4Box("solidInnerReflectorSpace", bsaX, bsaY, totalBsaLength + 1.*mm); 
    
    G4SubtractionSolid *solidReflector = new G4SubtractionSolid("solidReflector", solidOuterReflector, solidInnerReflectorSpace);
    G4LogicalVolume *logicReflector = new G4LogicalVolume(solidReflector, lead, "logicReflector");
    new G4PVPlacement(0, xyzReflector, logicReflector, "physReflector", logicWorld, false, 0, true);

    G4VisAttributes *refVis = new G4VisAttributes(G4Colour(0.7, 0.4, 0.7, 0.3)); 
    refVis->SetForceSolid(true);
    logicReflector->SetVisAttributes(refVis);

    // بناء كاشف الـ BF3 الأسطواني الأصلي
    G4Tubs *solidBF3Detector = new G4Tubs("solidBF3Detector", 0.*mm, 6*cm, 6.*cm, 0.*deg, 360.*deg);
    logicDetector = new G4LogicalVolume(solidBF3Detector, aluminum, "LogicDetector");
    new G4PVPlacement(0, xyzDetector, logicDetector, "physDetector", logicWorld, false, 0, true);

    G4VisAttributes *detVis = new G4VisAttributes(G4Colour(0.5, 0.5, 0.5, 0.5)); 
    detVis->SetForceSolid(true);
    logicDetector->SetVisAttributes(detVis);

    return physWorld;
}

void MyDetectorConstruction::ConstructSDandField()
{
    MySensitiveDetector *targetDet = new MySensitiveDetector("TargetDetector");
    if(logicTarget) logicTarget->SetSensitiveDetector(targetDet);
    
    MySensitiveDetector *neutronDet = new MySensitiveDetector("NeutronDetector");
    if(logicDetector) logicDetector->SetSensitiveDetector(neutronDet);
}
//==========================================================================================================//
//The last update
#include "construction.hh"
#include "G4PVPlacement.hh"  
#include "G4SubtractionSolid.hh" 
#include "G4Cons.hh"             
#include "G4VisAttributes.hh"    
#include "G4Colour.hh"           
#include "G4SDManager.hh" 
#include "G4UserLimits.hh"

MyDetectorConstruction::MyDetectorConstruction()
: logicTarget(nullptr), logicNickel(nullptr), logicModerator(nullptr),
  logicGammaFilter(nullptr), logicThermalFilter(nullptr), logicCollimator(nullptr),
  logicReflector(nullptr), logicDetector(nullptr)
{}

MyDetectorConstruction::~MyDetectorConstruction()
{}

G4VPhysicalVolume *MyDetectorConstruction::Construct()
{
// تعريف منطقة للهدف لضمان دقة التفاعل

    G4NistManager *nist = G4NistManager::Instance();

    G4Material *vacuum = nist->BuildMaterialWithNewDensity("vacuum", "G4_AIR", (0.011 / (287 * 300))); 
    G4Material *Li = nist->FindOrBuildMaterial("G4_Li");
    G4Material *aluminum = nist->FindOrBuildMaterial("G4_Al");

    // مواد الـ BSA 
    G4Material *nickel = nist->FindOrBuildMaterial("G4_Ni");            
    G4Material *lead = nist->FindOrBuildMaterial("G4_Pb");              
    G4Material *bismuth = nist->FindOrBuildMaterial("G4_Bi");          
    G4Material *cadmium = nist->FindOrBuildMaterial("G4_Cd");          

    // مادة المهدئ الجديد فلوريد المغنيسيوم (MgF2)
    G4Material *MgF2 = nist->FindOrBuildMaterial("G4_MAGNESIUM_FLUORIDE");

    // حجم العالم (World)
    G4double LWorld = 60*cm; 
    
    // أبعاد الهدف الأصلي
    G4double targetX = 6.*cm;     
    G4double targetY = 6.*cm;     
    G4double targetZ = 70.*um;    

    // أبعاد الـ BSA
    G4double bsaX = 25.*cm;              
    G4double bsaY = 25.*cm;              
    
    // السماكات (نصف الأبعاد للجينت 4)
    G4double hzNickel = 0.75*cm;         
    G4double hzModerator = 10.*cm;       // سماكة المهدئ MgF2 الكلية 20 سم
    G4double hzGammaFilter = 1.5*cm;     
    G4double hzThermalFilter = 0.8*cm;   
    G4double hzCollimator = 4.0*cm;      
    G4double reflectorThickness = 10.*cm;

    // حساب مواقع الطبقات تلقائياً على محور Z
    G4double currentZ = 0.*cm;

    currentZ += targetZ;
    G4ThreeVector xyzTarget(0., 0., currentZ);

    currentZ += targetZ + hzNickel;
    G4ThreeVector xyzNickel(0., 0., currentZ);

    currentZ += hzNickel + hzModerator;
    G4ThreeVector xyzModerator(0., 0., currentZ);

    currentZ += hzModerator + hzGammaFilter;
    G4ThreeVector xyzGammaFilter(0., 0., currentZ);

    currentZ += hzGammaFilter + hzThermalFilter;
    G4ThreeVector xyzThermalFilter(0., 0., currentZ);

    currentZ += hzThermalFilter + hzCollimator;
    G4ThreeVector xyzCollimator(0., 0., currentZ);

    currentZ += hzCollimator + 6.*cm; 
    G4ThreeVector xyzDetector(0., 0., currentZ);

    // بناء حجم العالم (World)
    G4Box *solidWorld = new G4Box("solidWorld", LWorld, LWorld, LWorld);
    G4LogicalVolume *logicWorld = new G4LogicalVolume(solidWorld, vacuum, "logicWorld");
    G4VPhysicalVolume *physWorld = new G4PVPlacement(0, G4ThreeVector(0., 0., 0.), logicWorld, "physWorld", 0, false, 0, true);

    G4VisAttributes *worldVis = new G4VisAttributes(G4Colour(0.8, 0.8, 0.8, 0.1)); 
    worldVis->SetForceSolid(true); 
    logicWorld->SetVisAttributes(worldVis);

    // أنبوب الحزمة الألومنيوم
    G4double pipeInnerRadius = 3.0*cm;   
    G4double pipeOuterRadius = 3.5*cm;   
    G4double pipeLengthZ = (LWorld + (xyzTarget.z() - targetZ)) / 2.0; 
    G4double zPosPipe = -LWorld + pipeLengthZ; 
    G4ThreeVector xyzBeamPipe(0., 0., zPosPipe);

    G4Tubs *solidBeamPipe = new G4Tubs("solidBeamPipe", pipeInnerRadius, pipeOuterRadius, pipeLengthZ, 0.*deg, 360.*deg);
    G4LogicalVolume *logicBeamPipe = new G4LogicalVolume(solidBeamPipe, aluminum, "logicBeamPipe");
    new G4PVPlacement(0, xyzBeamPipe, logicBeamPipe, "physBeamPipe", logicWorld, false, 0, true);

    G4VisAttributes *pipeVis = new G4VisAttributes(G4Colour(0.4, 0.4, 0.4,0.5)); 
    pipeVis->SetForceSolid(true);
    logicBeamPipe->SetVisAttributes(pipeVis);

    // بناء شريحة الليثيوم (Target)
    G4Box *solidTarget = new G4Box("solidTarget", targetX, targetY, targetZ);
    logicTarget = new G4LogicalVolume(solidTarget, Li, "logicTarget");
    new G4PVPlacement(0, xyzTarget, logicTarget, "physTarget", logicWorld, false, 0, true);

    G4VisAttributes *targetVis = new G4VisAttributes(G4Colour(1.0, 0.4, 0.0, 0.5)); 
    targetVis->SetForceSolid(true);
    logicTarget->SetVisAttributes(targetVis);

    // مرشح النيكل (Nickel Box)
    G4Box *solidNickel = new G4Box("solidNickel", bsaX, bsaY, hzNickel);
    logicNickel = new G4LogicalVolume(solidNickel, nickel, "logicNickel");
    new G4PVPlacement(0, xyzNickel, logicNickel, "physNickel", logicWorld, false, 0, true);

    G4VisAttributes *niVis = new G4VisAttributes(G4Colour(0.6, 0.4, 0.2, 0.4)); 
    niVis->SetForceSolid(true);
    logicNickel->SetVisAttributes(niVis);

    // المهدئ المربع (MgF2 Box)
    G4Box *solidModerator = new G4Box("solidModerator", bsaX, bsaY, hzModerator);
    logicModerator = new G4LogicalVolume(solidModerator, MgF2, "logicModerator");
    new G4PVPlacement(0, xyzModerator, logicModerator, "physModerator", logicWorld, false, 0, true);

    G4VisAttributes *modVis = new G4VisAttributes(G4Colour(0.0, 0.6, 1.0, 0.3)); 
    modVis->SetForceSolid(true);
    logicModerator->SetVisAttributes(modVis);

    // مصفاة غاما (Bismuth Box)
    G4Box *solidGammaFilter = new G4Box("solidGammaFilter", bsaX, bsaY, hzGammaFilter);
    logicGammaFilter = new G4LogicalVolume(solidGammaFilter, bismuth, "logicGammaFilter");
    new G4PVPlacement(0, xyzGammaFilter, logicGammaFilter, "physGammaFilter", logicWorld, false, 0, true);

    G4VisAttributes *gammaVis = new G4VisAttributes(G4Colour(0.0, 0.8, 0.2, 0.4)); 
    gammaVis->SetForceSolid(true);
    logicGammaFilter->SetVisAttributes(gammaVis);

    // مصفاة النيوترونات الحرارية (Cadmium Box)
    G4Box *solidThermalFilter = new G4Box("solidThermalFilter", bsaX, bsaY, hzThermalFilter);
    logicThermalFilter = new G4LogicalVolume(solidThermalFilter, cadmium, "logicThermalFilter");
    new G4PVPlacement(0, xyzThermalFilter, logicThermalFilter, "physThermalFilter", logicWorld, false, 0, true);

    G4VisAttributes *thermalVis = new G4VisAttributes(G4Colour(1.0, 0.9, 0.0, 0.4)); 
    thermalVis->SetForceSolid(true);
    logicThermalFilter->SetVisAttributes(thermalVis);

    // الموازئ (Collimator Box) المفرغ مخروطياً
    G4Box *solidCollimatorOuterBox = new G4Box("solidCollimatorOuterBox", bsaX, bsaY, hzCollimator);
    
    G4double Rmin1_tunnel = 0.*mm;     
    G4double Rmax1_tunnel = 9.0*cm;    
    G4double Rmin2_tunnel = 0.*mm;     
    G4double Rmax2_tunnel = 6.0*cm;    
    
    G4Cons *solidCollimatorInnerCone = new G4Cons("solidCollimatorInnerCone", Rmin1_tunnel, Rmax1_tunnel, Rmin2_tunnel, Rmax2_tunnel, hzCollimator + 1.*mm, 0.*deg, 360.*deg);
    
    G4SubtractionSolid *solidCollimator = new G4SubtractionSolid("solidCollimator", solidCollimatorOuterBox, solidCollimatorInnerCone);
    logicCollimator = new G4LogicalVolume(solidCollimator, aluminum, "logicCollimator");
    new G4PVPlacement(0, xyzCollimator, logicCollimator, "physCollimator", logicWorld, false, 0, true);

    G4VisAttributes *collVis = new G4VisAttributes(G4Colour(1.0, 0.2, 0.2, 0.4)); 
    collVis->SetForceSolid(true);
    logicCollimator->SetVisAttributes(collVis);

    // العاكس المربع الخارجي (Lead Reflector Box)
    G4double totalBsaLength = hzNickel + hzModerator + hzGammaFilter + hzThermalFilter + hzCollimator;
    G4ThreeVector xyzReflector(0., 0., xyzNickel.z() - hzNickel + totalBsaLength);

    G4VSolid *solidOuterReflector = new G4Box("solidOuterReflector", bsaX + reflectorThickness, bsaY + reflectorThickness, totalBsaLength);
    G4VSolid *solidInnerReflectorSpace = new G4Box("solidInnerReflectorSpace", bsaX, bsaY, totalBsaLength + 1.*mm); 
    
    G4SubtractionSolid *solidReflector = new G4SubtractionSolid("solidReflector", solidOuterReflector, solidInnerReflectorSpace);
    logicReflector = new G4LogicalVolume(solidReflector, lead, "logicReflector");
    new G4PVPlacement(0, xyzReflector, logicReflector, "physReflector", logicWorld, false, 0, true);

    G4VisAttributes *refVis = new G4VisAttributes(G4Colour(0.7, 0.4, 0.7, 0.3)); 
    refVis->SetForceSolid(true);
    logicReflector->SetVisAttributes(refVis);

    // بناء كاشف الـ BF3 الأسطواني
    G4Tubs *solidBF3Detector = new G4Tubs("solidBF3Detector", 0.*mm, 6*cm, 6.*cm, 0.*deg, 360.*deg);
    logicDetector = new G4LogicalVolume(solidBF3Detector, aluminum, "LogicDetector");
    new G4PVPlacement(0, xyzDetector, logicDetector, "physDetector", logicWorld, false, 0, true);

    G4VisAttributes *detVis = new G4VisAttributes(G4Colour(0.5, 0.5, 0.5, 0.5)); 
    detVis->SetForceSolid(true);
    logicDetector->SetVisAttributes(detVis);
    
    
G4Region* targetRegion = new G4Region("TargetRegion");
targetRegion->AddRootLogicalVolume(logicTarget);

G4UserLimits* stepLimit = new G4UserLimits(1.0 * um); // الحد الأقصى للخطوة 1 ميكرون
logicTarget->SetUserLimits(stepLimit);


    return physWorld;
}

void MyDetectorConstruction::ConstructSDandField()
{
    // إنشاء مدير الكواشف
    G4SDManager* sdMan = G4SDManager::GetSDMpointer();
    
    // إنشاء الكاشف (مثال: ننشئ كاشفاً واحداً عاماً كما فعلتِ)
    MySensitiveDetector *bsaSensitiveDetector = new MySensitiveDetector("BsaSD");
    sdMan->AddNewDetector(bsaSensitiveDetector);

    // ربط الكواشف مع التحقق من وجود الحجم (LogicVolume)
    if(logicTarget)        logicTarget->SetSensitiveDetector(bsaSensitiveDetector);
    if(logicNickel)        logicNickel->SetSensitiveDetector(bsaSensitiveDetector);
    if(logicModerator)     logicModerator->SetSensitiveDetector(bsaSensitiveDetector);
    if(logicGammaFilter)   logicGammaFilter->SetSensitiveDetector(bsaSensitiveDetector);
    if(logicThermalFilter) logicThermalFilter->SetSensitiveDetector(bsaSensitiveDetector);
    if(logicCollimator)    logicCollimator->SetSensitiveDetector(bsaSensitiveDetector);
    if(logicReflector)     logicReflector->SetSensitiveDetector(bsaSensitiveDetector);
    if(logicDetector)      logicDetector->SetSensitiveDetector(bsaSensitiveDetector);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//My Current code
#include "construction.hh"
#include "G4PVPlacement.hh"  
#include "G4SubtractionSolid.hh" 
#include "G4Cons.hh"             
#include "G4VisAttributes.hh"    
#include "G4Colour.hh"           
#include "G4SDManager.hh" 
#include "G4UserLimits.hh"
#include "G4MultiFunctionalDetector.hh"
#include "G4VPrimitiveScorer.hh"
#include "G4PSTrackLength.hh"
#include "G4SDManager.hh"

class MyBasicSD : public G4VSensitiveDetector {
public:
    MyBasicSD(G4String name) : G4VSensitiveDetector(name) {}
    G4bool ProcessHits(G4Step* step, G4TouchableHistory*) override {
        G4double edep = step->GetTotalEnergyDeposit();
        // هنا يمكنك إضافة G4cout للتأكد من عمل الكاشف
        // G4cout << "Hit detected! Energy: " << edep << G4endl; 
        return edep > 0;
    }
};

MyDetectorConstruction::MyDetectorConstruction()
: logicTarget(nullptr), logicNickel(nullptr), logicModerator(nullptr),
  logicGammaFilter(nullptr), logicThermalFilter(nullptr), logicCollimator(nullptr),
  logicReflector(nullptr), logicDetector(nullptr)
{}

MyDetectorConstruction::~MyDetectorConstruction()
{}

G4VPhysicalVolume *MyDetectorConstruction::Construct()
{
// تعريف منطقة للهدف لضمان دقة التفاعل

    G4NistManager *nist = G4NistManager::Instance();

    G4Material* vacuum = nist->FindOrBuildMaterial("G4_Galactic");
   // G4Material *vacuum = nist->BuildMaterialWithNewDensity("vacuum", "G4_AIR", (0.011 / (287 * 300))); 
    G4Material *Li = nist->FindOrBuildMaterial("G4_Li");
    G4Material *aluminum = nist->FindOrBuildMaterial("G4_Al");
    G4Material* BF3Gas = nist->FindOrBuildMaterial("G4_BF3");

    // مواد الـ BSA 
    G4Material *nickel = nist->FindOrBuildMaterial("G4_Ni");            
    G4Material *lead = nist->FindOrBuildMaterial("G4_Pb");              
    G4Material *bismuth = nist->FindOrBuildMaterial("G4_Bi");          
    G4Material *cadmium = nist->FindOrBuildMaterial("G4_Cd");          

    // مادة المهدئ الجديد فلوريد المغنيسيوم (MgF2)
    G4Material *MgF2 = nist->FindOrBuildMaterial("G4_MAGNESIUM_FLUORIDE");

    // حجم العالم (World)
    G4double LWorld = 60*cm; 
    
    // أبعاد الهدف الأصلي
    G4double targetX = 6.*cm;     
    G4double targetY = 6.*cm;     
    G4double targetZ = 70.*um;    

    // أبعاد الـ BSA
    G4double bsaX = 25.*cm;              
    G4double bsaY = 25.*cm;              
    
    // السماكات (نصف الأبعاد للجينت 4)
    G4double hzNickel = 0.75*cm;         
    G4double hzModerator = 10.*cm;       // سماكة المهدئ MgF2 الكلية 20 سم
    G4double hzGammaFilter = 1.5*cm;     
    G4double hzThermalFilter = 0.8*cm;   
    G4double hzCollimator = 2.0*cm;      
    
    G4double reflectorThickness = 20.*cm;

    // حساب مواقع الطبقات تلقائياً على محور Z
    G4double currentZ = 0.*cm;

    currentZ += targetZ;
    G4ThreeVector xyzTarget(0., 0., currentZ);

    currentZ += targetZ + hzNickel;
    G4ThreeVector xyzNickel(0., 0., currentZ);

    currentZ += hzNickel + hzModerator;
    G4ThreeVector xyzModerator(0., 0., currentZ);

    currentZ += hzModerator + hzGammaFilter;
    G4ThreeVector xyzGammaFilter(0., 0., currentZ);

    currentZ += hzGammaFilter + hzThermalFilter;
    G4ThreeVector xyzThermalFilter(0., 0., currentZ);

    currentZ += hzThermalFilter + hzCollimator;
    G4ThreeVector xyzCollimator(0., 0., currentZ);

    currentZ += hzCollimator + 6.*cm; 
    G4ThreeVector xyzDetector(0., 0., currentZ);

    // بناء حجم العالم (World)
    G4Box *solidWorld = new G4Box("solidWorld", LWorld, LWorld, LWorld);
    G4LogicalVolume *logicWorld = new G4LogicalVolume(solidWorld, vacuum, "logicWorld");
    G4VPhysicalVolume *physWorld = new G4PVPlacement(0, G4ThreeVector(0., 0., 0.), logicWorld, "physWorld", 0, false, 0, true);

    G4VisAttributes *worldVis = new G4VisAttributes(G4Colour(0.8, 0.8, 0.8, 0.1)); 
    worldVis->SetForceSolid(true); 
    logicWorld->SetVisAttributes(worldVis);

    // أنبوب الحزمة الألومنيوم
/*    G4double pipeInnerRadius = 3.0*cm;   
    G4double pipeOuterRadius = 3.5*cm;   
    G4double pipeLengthZ = (LWorld + (xyzTarget.z() - targetZ)) / 2.0; 
    G4double zPosPipe = -LWorld + pipeLengthZ; 
    G4ThreeVector xyzBeamPipe(0., 0., zPosPipe);

    G4Tubs *solidBeamPipe = new G4Tubs("solidBeamPipe", pipeInnerRadius, pipeOuterRadius, pipeLengthZ, 0.*deg, 360.*deg);
    G4LogicalVolume *logicBeamPipe = new G4LogicalVolume(solidBeamPipe, aluminum, "logicBeamPipe");
    new G4PVPlacement(0, xyzBeamPipe, logicBeamPipe, "physBeamPipe", logicWorld, false, 0, true);

    G4VisAttributes *pipeVis = new G4VisAttributes(G4Colour(0.4, 0.4, 0.4,0.5)); 
    pipeVis->SetForceSolid(true);
    logicBeamPipe->SetVisAttributes(pipeVis);
*/
    // بناء شريحة الليثيوم (Target)

    G4Box *solidTarget = new G4Box("solidTarget", targetX, targetY, targetZ);
    logicTarget = new G4LogicalVolume(solidTarget, Li, "logicTarget");
    new G4PVPlacement(0, xyzTarget, logicTarget, "physTarget", logicWorld, false, 0, true);

    G4VisAttributes *targetVis = new G4VisAttributes(G4Colour(1.0, 0.4, 0.0, 0.5)); 
    targetVis->SetForceSolid(true);
    logicTarget->SetVisAttributes(targetVis);

    // مرشح النيكل (Nickel Box)
    G4Box *solidNickel = new G4Box("solidNickel", bsaX, bsaY, hzNickel);
    logicNickel = new G4LogicalVolume(solidNickel, nickel, "logicNickel");
    new G4PVPlacement(0, xyzNickel, logicNickel, "physNickel", logicWorld, false, 0, true);

    G4VisAttributes *niVis = new G4VisAttributes(G4Colour(0.6, 0.4, 0.2, 0.4)); 
    niVis->SetForceSolid(true);
    logicNickel->SetVisAttributes(niVis);

    // المهدئ المربع (MgF2 Box)
    G4Box *solidModerator = new G4Box("solidModerator", bsaX, bsaY, hzModerator);
    logicModerator = new G4LogicalVolume(solidModerator, MgF2, "logicModerator");
    new G4PVPlacement(0, xyzModerator, logicModerator, "physModerator", logicWorld, false, 0, true);

    G4VisAttributes *modVis = new G4VisAttributes(G4Colour(0.0, 0.6, 1.0, 0.3)); 
    modVis->SetForceSolid(true);
    logicModerator->SetVisAttributes(modVis);

    // مصفاة غاما (Bismuth Box)
    G4Box *solidGammaFilter = new G4Box("solidGammaFilter", bsaX, bsaY, hzGammaFilter);
    logicGammaFilter = new G4LogicalVolume(solidGammaFilter, bismuth, "logicGammaFilter");
    new G4PVPlacement(0, xyzGammaFilter, logicGammaFilter, "physGammaFilter", logicWorld, false, 0, true);

    G4VisAttributes *gammaVis = new G4VisAttributes(G4Colour(0.0, 0.8, 0.2, 0.4)); 
    gammaVis->SetForceSolid(true);
    logicGammaFilter->SetVisAttributes(gammaVis);

    // مصفاة النيوترونات الحرارية (Cadmium Box)
    G4Box *solidThermalFilter = new G4Box("solidThermalFilter", bsaX, bsaY, hzThermalFilter);
    logicThermalFilter = new G4LogicalVolume(solidThermalFilter, cadmium, "logicThermalFilter");
    new G4PVPlacement(0, xyzThermalFilter, logicThermalFilter, "physThermalFilter", logicWorld, false, 0, true);

    G4VisAttributes *thermalVis = new G4VisAttributes(G4Colour(1.0, 0.9, 0.0, 0.4)); 
    thermalVis->SetForceSolid(true);
    logicThermalFilter->SetVisAttributes(thermalVis);

    // الموازئ (Collimator Box) المفرغ مخروطياً
    G4Box *solidCollimatorOuterBox = new G4Box("solidCollimatorOuterBox", bsaX, bsaY, hzCollimator);
    
    G4double Rmin1_tunnel = 0.*mm;     
    G4double Rmax1_tunnel = 9.0*cm;    
    G4double Rmin2_tunnel = 0.*mm;     
    G4double Rmax2_tunnel = 6.0*cm;    
    
    G4Cons *solidCollimatorInnerCone = new G4Cons("solidCollimatorInnerCone", Rmin1_tunnel, Rmax1_tunnel, Rmin2_tunnel, Rmax2_tunnel, hzCollimator + 1.*mm, 0.*deg, 360.*deg);
    
    G4SubtractionSolid *solidCollimator = new G4SubtractionSolid("solidCollimator", solidCollimatorOuterBox, solidCollimatorInnerCone);
    logicCollimator = new G4LogicalVolume(solidCollimator, aluminum, "logicCollimator");
    new G4PVPlacement(0, xyzCollimator, logicCollimator, "physCollimator", logicWorld, false, 0, true);

    G4VisAttributes *collVis = new G4VisAttributes(G4Colour(1.0, 0.2, 0.2, 0.4)); 
    collVis->SetForceSolid(true);
    logicCollimator->SetVisAttributes(collVis);

    // العاكس المربع الخارجي (Lead Reflector Box)
    G4double totalBsaLength = hzNickel + hzModerator + hzGammaFilter + hzThermalFilter + hzCollimator;
    G4ThreeVector xyzReflector(0., 0., xyzNickel.z() - hzNickel + totalBsaLength);

    G4VSolid *solidOuterReflector = new G4Box("solidOuterReflector", bsaX + reflectorThickness, bsaY + reflectorThickness, totalBsaLength);
    G4VSolid *solidInnerReflectorSpace = new G4Box("solidInnerReflectorSpace", bsaX, bsaY, totalBsaLength + 1.*mm); 
    
    G4SubtractionSolid *solidReflector = new G4SubtractionSolid("solidReflector", solidOuterReflector, solidInnerReflectorSpace);
    logicReflector = new G4LogicalVolume(solidReflector, lead, "logicReflector");
    new G4PVPlacement(0, xyzReflector, logicReflector, "physReflector", logicWorld, false, 0, true);

    G4VisAttributes *refVis = new G4VisAttributes(G4Colour(0.7, 0.4, 0.7, 0.3)); 
    refVis->SetForceSolid(true);
    logicReflector->SetVisAttributes(refVis);

    // بناء كاشف الـ BF3 الأسطواني
    G4Tubs *solidBF3Detector = new G4Tubs("solidBF3Detector", 0.*mm, 6*cm, 6.*cm, 0.*deg, 360.*deg);
    logicDetector = new G4LogicalVolume(solidBF3Detector, aluminum, "LogicDetector");
    new G4PVPlacement(0, xyzDetector, logicDetector, "physDetector", logicWorld, false, 0, true);

    G4VisAttributes *detVis = new G4VisAttributes(G4Colour(0.5, 0.5, 0.5, 0.5)); 
    detVis->SetForceSolid(true);
    logicDetector->SetVisAttributes(detVis);
    
    
   /* G4Region* targetRegion = new G4Region("TargetRegion");
    targetRegion->AddRootLogicalVolume(logicTarget);

    G4UserLimits* stepLimit = new G4UserLimits(1.0 * um); // الحد الأقصى للخطوة 1 ميكرون
    logicTarget->SetUserLimits(stepLimit);*/

// 1. تعريف الـ Region للهدف (لتقليل الخطوة - Step Limit)
    G4Region* targetRegion = new G4Region("TargetRegion");
    targetRegion->AddRootLogicalVolume(logicTarget);
    G4UserLimits* stepLimit = new G4UserLimits(1.0 * um);
    targetRegion->SetUserLimits(stepLimit); // تطبيق الـ Limits على المنطقة

    // 2. تعريف الـ Region للـ BSA (مفيد جداً للـ Biasing لاحقاً)
    G4Region* bsaRegion = new G4Region("BSARegion");
    bsaRegion->AddRootLogicalVolume(logicModerator);
    bsaRegion->AddRootLogicalVolume(logicNickel);
    bsaRegion->AddRootLogicalVolume(logicGammaFilter);
    bsaRegion->AddRootLogicalVolume(logicThermalFilter);
    
    // 3. تعريف الـ Region للـ Reflector
    G4Region* reflectorRegion = new G4Region("ReflectorRegion");
    reflectorRegion->AddRootLogicalVolume(logicReflector); 
    // يمكنكِ إضافة الـ Collimator هنا أيضاً إذا كان محيطاً بالـ Reflector
    reflectorRegion->AddRootLogicalVolume(logicCollimator);
  
    return physWorld;
}

void MyDetectorConstruction::ConstructSDandField()
{
  // إنشاء مدير الكواشف
    G4SDManager* sdMan = G4SDManager::GetSDMpointer();
    
    // إنشاء الكاشف (مثال: ننشئ كاشفاً واحداً عاماً كما فعلتِ)
    MySensitiveDetector *bsaSensitiveDetector = new MySensitiveDetector("BsaSD");
    sdMan->AddNewDetector(bsaSensitiveDetector);

    // ربط الكواشف مع التحقق من وجود الحجم (LogicVolume)
    if(logicTarget)        logicTarget->SetSensitiveDetector(bsaSensitiveDetector);
    if(logicNickel)        logicNickel->SetSensitiveDetector(bsaSensitiveDetector);
    if(logicModerator)     logicModerator->SetSensitiveDetector(bsaSensitiveDetector);
    if(logicGammaFilter)   logicGammaFilter->SetSensitiveDetector(bsaSensitiveDetector);
    if(logicThermalFilter) logicThermalFilter->SetSensitiveDetector(bsaSensitiveDetector);
    if(logicCollimator)    logicCollimator->SetSensitiveDetector(bsaSensitiveDetector);
    if(logicReflector)     logicReflector->SetSensitiveDetector(bsaSensitiveDetector);
    if(logicDetector)      logicDetector->SetSensitiveDetector(bsaSensitiveDetector);
}
