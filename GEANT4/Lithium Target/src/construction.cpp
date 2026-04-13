                                                                                                                                   src/construction.cc                                                                                                                                               
#include "construction.hh"
#include "detector.hh"


#include "construction.hh"
#include "G4VisAttributes.hh"
#include "G4Colour.hh"

MyDetector::MyDetector()
{
   fMessenger = new G4GenericMessenger(this, "/detector/" , "DetectorConstruction");

   fMessenger->DeclareProperty("nCols",nCols, "Number of columns");
   fMessenger->DeclareProperty("nRows",nRows, "Number of rows");

  nCols = 100;
  nRows = 100;

}

MyDetector::~MyDetector()
{}
G4VPhysicalVolume *MyDetector::Construct()
{

  G4NistManager *nist = G4NistManager::Instance();

/*  G4Material *Li = new G4Material("Li",0.534*g/cm3,1);

  Li->AddElement(nist->FindOrBuildElement("Li"),1);*/
/////////////////////////////////////////////////////////////
  G4Isotope* Li7 = new G4Isotope("Li7", 3, 7, 7.016*g/mole);

  G4Element* elLi7 = new G4Element("EnrichedLithium7", "Li7", 1);
  elLi7->AddIsotope(Li7, 100.*perCent);

  G4Material* LiTarget = new G4Material("LiTarget", 0.534*g/cm3, 1);
  LiTarget->AddElement(elLi7, 1);

/////////////////////////////////////////////////////////////////
  //World Definition (Vaccum)
  G4double xWorld = 0.5*mm;
  G4double yWorld = 0.5*mm;
  G4double zWorld = 0.5*mm;

  G4Material *worldMat = nist->FindOrBuildMaterial("G4_Galactic");

  G4Box *solidWorld = new G4Box("solidWorld", xWorld, yWorld, zWorld);

  G4LogicalVolume *logicWorld = new G4LogicalVolume(solidWorld , worldMat , "logicWorld");

  G4VPhysicalVolume *physWorld = new G4PVPlacement(0, G4ThreeVector(0.,0.,0.),logicWorld,"physWorld",0, false, 0, true);

  //Material Definition of the pure lithium

   G4Box *solidLi = new G4Box("Li",0.4*mm ,0.4*mm ,0.1*mm);

   G4LogicalVolume *logicLi = new G4LogicalVolume(solidLi ,LiTarget ,"logicLi");
/*some master properities for the lithium slab*/

   G4VPhysicalVolume *physLi = new G4PVPlacement(0 , G4ThreeVector(0.,0.,0.025*mm), logicLi ,"physLi", logicWorld , false, 0, true);
 
  G4VisAttributes* LiVis = new G4VisAttributes(G4Colour(0.75, 0.75, 0.75));
  LiVis->SetForceSolid(true);
  logicLi->SetVisAttributes(LiVis);
  //Adding Detector

    G4Box *solidDetector = new G4Box("solidDetector",xWorld/nCols ,yWorld/nRows ,0.001*mm);

    logicDetector = new G4LogicalVolume(solidDetector, worldMat , "logicalDetector");

    for(G4int i = 0; i < nRows; i++)
    {
      for(G4int j = 0; j< nCols ; j++)
       {
          G4VPhysicalVolume *physDetector = new G4PVPlacement(0,G4ThreeVector(-0.5*mm+(i+0.5)*mm/nRows,-0.5*mm+(j+0.5)*mm/nCols,0.49*mm), logicDetector , "physDetector" , logicWorld , false , j+i*100, true);
       }
    }

  return physWorld;
}
void MyDetector::ConstructSDandField()
{
   MySensitiveDetector *sensDet = new MySensitiveDetector("SensitiveDetector");

   logicDetector->SetSensitiveDetector(sensDet);
}

