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
#include "G4MultiFunctionalDetector.hh"
#include "G4RunManager.hh"

#include "G4GeometryManager.hh"
#include "G4PhysicalVolumeStore.hh"
#include "G4LogicalVolumeStore.hh"
#include "G4SolidStore.hh"

#include "G4PSDoseDeposit.hh" // لحساب الجرعة الممتصة المودعة من غاما
#include "G4PSCellFlux.hh"    // أو لحساب الفيض (تشتت الجسيمات داخل الحجم)
#include "G4SDParticleWithEnergyFilter.hh"
#include "G4SDParticleFilter.hh"

class MyBasicSD : public G4VSensitiveDetector {
public:
    MyBasicSD(G4String name) : G4VSensitiveDetector(name) {}
    G4bool ProcessHits(G4Step* step, G4TouchableHistory*) override {
        G4double edep = step->GetTotalEnergyDeposit();
        return edep > 0;
    }
};

MyDetectorConstruction::MyDetectorConstruction()
: logicTarget(nullptr), logicModerator(nullptr),logicFastFilter(nullptr), logicGammaFilter(nullptr), 
  logicCollimator(nullptr), logicReflector(nullptr), logicDetector(nullptr)
{
    // ملاحظة: تم إزالة مؤشرات logicNickel و logicThermalFilter من هنا
}

MyDetectorConstruction::~MyDetectorConstruction()
{}

G4VPhysicalVolume *MyDetectorConstruction::Construct()
{

   
    G4NistManager *nist = G4NistManager::Instance();

    G4Material* vacuum = nist->FindOrBuildMaterial("G4_Galactic");
    G4Material *Li = nist->FindOrBuildMaterial("G4_Li");
    G4Material *aluminum = nist->FindOrBuildMaterial("G4_Al");
   // --- تعريف غاز BF3 يدويًا لتجنب الـ nullptr ---
    G4Element* elB = nist->FindOrBuildElement("B");
    G4Element* elF = nist->FindOrBuildElement("F");
    
    G4Material *icruBone = nist->FindOrBuildMaterial("G4_BONE_COMPACT_ICRU");
    
    // كثافة غاز BF3 عند الظروف القياسية تقريبًا 0.00276 g/cm3
    G4Material* BF3Gas = new G4Material("BF3Gas", 0.00276*g/cm3, 2);
    BF3Gas->AddElement(elB, 1); // ذرة بورون واحدة
    BF3Gas->AddElement(elF, 3); // ثلاث ذرات فلور


    // مواد الـ BSA المتبقية والمادة الجديدة للموازئ
    G4Material *nickel = nist->FindOrBuildMaterial("G4_Ni"); 
    G4Material *lead = nist->FindOrBuildMaterial("G4_Pb");              
    G4Material *bismuth = nist->FindOrBuildMaterial("G4_Bi");          
    G4Material *MgF2 = nist->FindOrBuildMaterial("G4_MAGNESIUM_FLUORIDE");
    G4Material *tungsten = nist->FindOrBuildMaterial("G4_W"); // إضافة التنجستن للموازئ

    // حجم العالم (World)
    G4double LWorld = 80. *cm; 
    
    // أبعاد الهدف الأصلي
    G4double targetX = 6.*cm;     
    G4double targetY = 6.*cm;     
    G4double targetZ = 70.*um;    

    // أبعاد الـ BSA
    G4double bsaX = 25.*cm;              
    G4double bsaY = 25.*cm;              
    
    // السماكات المتبقية (نصف الأبعاد للجينت 4) 
   
    G4double hzModerator = (33.0 / 2.0) * cm; 
    G4double reflectorThickness = 25.*cm;
    G4double hzFastFilter = (0.5 / 2.0)* cm;
    G4double hzGammaFilter = (3.0 / 2.0)*cm;     
    G4double hzCollimator = (3 / 2.0)*cm;      


        // أبعاد أسطوانة شبح العظم (Bone Phantom Cylinder)
    G4double boneRadius = 15.0 * cm;           // نصف قطر الأسطوانة
    G4double hzBonePhantom = (20.0 / 2.0) * cm; // نصف ارتفاع أسطوانة العظم (20 cm إجمالي)
    
    
    // الحساب التلقائي المحدث للمواقع على محور Z بعد حذف النيكل والكادميوم
    G4double currentZ = 0.*cm;

    // 1. موقع شريحة الليثيوم (Target)
    currentZ += targetZ;
    G4ThreeVector xyzTarget(0., 0., currentZ);

    // 2. موقع المهدئ (MgF2) - يأتي مباشرة بعد الليثيوم
    currentZ += targetZ + hzModerator;
    G4ThreeVector xyzModerator(0., 0., currentZ);

    // 2.5 موقع مرشح النيوترونات السريعة (النيكل) - جديد
    currentZ += hzModerator + hzFastFilter;
    G4ThreeVector xyzFastFilter(0., 0., currentZ);

    // 3. موقع مصفاة غاما (البزموت) - يتزحزح للأمام
    currentZ += hzFastFilter + hzGammaFilter;
    G4ThreeVector xyzGammaFilter(0., 0., currentZ);

    // 4. موقع الموازئ التنجستن 
    currentZ += hzGammaFilter + hzCollimator;
    G4ThreeVector xyzCollimator(0., 0., currentZ);

    // 5. موقع الكاشف خلف الموازئ
   currentZ += hzCollimator + 0.1 * cm + hzBonePhantom; 
    G4ThreeVector xyzBonePhantom(0., 0., currentZ);
  
// بناء حجم العالم (World)
    G4Box *solidWorld = new G4Box("solidWorld", LWorld, LWorld, LWorld);
    G4LogicalVolume *logicWorld = new G4LogicalVolume(solidWorld, vacuum, "logicWorld");
    G4VPhysicalVolume *physWorld = new G4PVPlacement(0, G4ThreeVector(0., 0., 0.), logicWorld, "physWorld", 0, false, 0, true);

    G4VisAttributes *worldVis = new G4VisAttributes(G4Colour(0.8, 0.8, 0.8, 0.1)); 
    worldVis->SetForceSolid(true); 
    logicWorld->SetVisAttributes(worldVis);

    // 1. Target
    G4Box *solidTarget = new G4Box("solidTarget", targetX, targetY, targetZ);
    logicTarget = new G4LogicalVolume(solidTarget, Li, "logicTarget");
    new G4PVPlacement(0, xyzTarget, logicTarget, "physTarget", logicWorld, false, 0, true);

    G4VisAttributes *targetVis = new G4VisAttributes(G4Colour(1.0, 0.4, 0.0, 0.5)); 
    targetVis->SetForceSolid(true);
    logicTarget->SetVisAttributes(targetVis);

    // 2. Moderator (MgF2)
    G4Box *solidModerator = new G4Box("solidModerator", bsaX, bsaY, hzModerator);
    logicModerator = new G4LogicalVolume(solidModerator, MgF2, "logicModerator");
    new G4PVPlacement(0, xyzModerator, logicModerator, "physModerator", logicWorld, false, 0, true);
    
    G4VisAttributes *modVis = new G4VisAttributes(G4Colour(0.0, 0.6, 1.0, 0.3)); 
    modVis->SetForceSolid(true);
    logicModerator->SetVisAttributes(modVis);

    // 3. Fast Filter (Nickel)
    G4Box *solidFastFilter = new G4Box("solidFastFilter", bsaX, bsaY, hzFastFilter);
    logicFastFilter = new G4LogicalVolume(solidFastFilter, nickel, "logicFastFilter");
    new G4PVPlacement(0, xyzFastFilter, logicFastFilter, "physFastFilter", logicWorld, false, 0, true);

    G4VisAttributes *niVis = new G4VisAttributes(G4Colour(0.9, 0.7, 0.2, 0.5));
    niVis->SetForceSolid(true);
    logicFastFilter->SetVisAttributes(niVis);

    // 4. Gamma Filter (Bismuth)
    G4Box *solidGammaFilter = new G4Box("solidGammaFilter", bsaX, bsaY, hzGammaFilter);
    logicGammaFilter = new G4LogicalVolume(solidGammaFilter, bismuth, "logicGammaFilter");
    new G4PVPlacement(0, xyzGammaFilter, logicGammaFilter, "physGammaFilter", logicWorld, false, 0, true);

    G4VisAttributes *gammaVis = new G4VisAttributes(G4Colour(0.0, 0.8, 0.2, 0.4)); 
    gammaVis->SetForceSolid(true);
    logicGammaFilter->SetVisAttributes(gammaVis);

    // 5. Collimator (Tungsten)
    G4Box *solidCollimatorOuterBox = new G4Box("solidCollimatorOuterBox", bsaX, bsaY, hzCollimator);
    G4double Rmin1_tunnel = 0.*mm, Rmax1_tunnel = 18.0*cm; 
    G4double Rmin2_tunnel = 0.*mm, Rmax2_tunnel = 14.0*cm; 
    
    G4Cons *solidCollimatorInnerCone = new G4Cons("solidCollimatorInnerCone", Rmin1_tunnel, Rmax1_tunnel, Rmin2_tunnel, Rmax2_tunnel, hzCollimator + 1.*mm, 0.*deg, 360.*deg);
    G4SubtractionSolid *solidCollimator = new G4SubtractionSolid("solidCollimator", solidCollimatorOuterBox, solidCollimatorInnerCone);
    
    logicCollimator = new G4LogicalVolume(solidCollimator, tungsten, "logicCollimator");
    new G4PVPlacement(0, xyzCollimator, logicCollimator, "physCollimator", logicWorld, false, 0, true);

    G4VisAttributes *collVis = new G4VisAttributes(G4Colour(0.3, 0.3, 0.3, 0.6));
    collVis->SetForceSolid(true);
    logicCollimator->SetVisAttributes(collVis);

    // 6. Reflector (Lead)
    G4double totalBsaLength = hzModerator + hzFastFilter + hzGammaFilter + hzCollimator;
    G4ThreeVector xyzReflector(0., 0., xyzModerator.z() - hzModerator + totalBsaLength);

    G4VSolid *solidOuterReflector = new G4Box("solidOuterReflector", bsaX + reflectorThickness, bsaY + reflectorThickness, totalBsaLength);
    G4VSolid *solidInnerReflectorSpace = new G4Box("solidInnerReflectorSpace", bsaX, bsaY, totalBsaLength + 1.*mm); 
    G4SubtractionSolid *solidReflector = new G4SubtractionSolid("solidReflector", solidOuterReflector, solidInnerReflectorSpace);
    
    logicReflector = new G4LogicalVolume(solidReflector, lead, "logicReflector");
    new G4PVPlacement(0, xyzReflector, logicReflector, "physReflector", logicWorld, false, 0, true);

    G4VisAttributes *refVis = new G4VisAttributes(G4Colour(0.7, 0.4, 0.7, 0.3)); 
    refVis->SetForceSolid(true);
    logicReflector->SetVisAttributes(refVis);

    // =========================================================================
    // 7. بناء أسطوانة شبح العظم (ICRU Bone Cylinder Phantom)
    // =========================================================================
    G4Tubs *solidBonePhantom = new G4Tubs("solidBonePhantom", 0.*cm, boneRadius, hzBonePhantom, 0.*deg, 360.*deg);
    logicBonePhantom = new G4LogicalVolume(solidBonePhantom, icruBone, "logicBonePhantom");
    new G4PVPlacement(0, xyzBonePhantom, logicBonePhantom, "physBonePhantom", logicWorld, false, 0, true);

    // إعطاء أسطوانة العظم لوناً عاجياً/أبيض مميزاً للتمييز البصري
    G4VisAttributes *boneVis = new G4VisAttributes(G4Colour(0.95, 0.95, 0.85, 0.7)); 
    boneVis->SetForceSolid(true);
    logicBonePhantom->SetVisAttributes(boneVis);

    // Regions
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
  
    return physWorld;
}

void MyDetectorConstruction::ConstructSDandField()
{
    G4SDManager* sdMan = G4SDManager::GetSDMpointer();
    
    MySensitiveDetector *bsaSensitiveDetector = new MySensitiveDetector("BsaSD");
    sdMan->AddNewDetector(bsaSensitiveDetector);

    // ربط الكواشف مع المكونات الحالية فقط (تمت إزالة النيكل والكادميوم لحمايته من الـ Crash)
    if(logicTarget)        logicTarget->SetSensitiveDetector(bsaSensitiveDetector);
    if(logicFastFilter)    logicFastFilter->SetSensitiveDetector(bsaSensitiveDetector);
    if(logicModerator)     logicModerator->SetSensitiveDetector(bsaSensitiveDetector);
    if(logicGammaFilter)   logicGammaFilter->SetSensitiveDetector(bsaSensitiveDetector);
    if(logicCollimator)    logicCollimator->SetSensitiveDetector(bsaSensitiveDetector);
    if(logicReflector)     logicReflector->SetSensitiveDetector(bsaSensitiveDetector);
    //if(logicDetector)      logicDetector->SetSensitiveDetector(bsaSensitiveDetector);
// =========================================================================
    // إنشاء كاشف الجرعة الممتصة داخل شبح العظم (Bone Phantom Scorer)
    // =========================================================================
    G4MultiFunctionalDetector* boneScorer = new G4MultiFunctionalDetector("BoneScorer");
    sdMan->AddNewDetector(boneScorer);

    // تسجيل الجرعة الممتصة المودعة (Dose Deposit)
    G4VPrimitiveScorer* gammaDose = new G4PSDoseDeposit("GammaDose");
    G4SDParticleFilter* gammaFilter = new G4SDParticleFilter("gammaFilter");
    gammaFilter->add("gamma"); 
    gammaDose->SetFilter(gammaFilter);
    boneScorer->RegisterPrimitive(gammaDose);

    // ربط الـ Scorer بأصل حجم العظم المنطقي
    if(logicBonePhantom) logicBonePhantom->SetSensitiveDetector(boneScorer);
}
