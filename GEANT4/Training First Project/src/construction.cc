  GNU nano 7.2                                                                                                                                                                                                                                                                                                            src/construction.cc *                                                                                                                                                                                                                                                                                                                    
#include "construction.hh"

MyDetectorConstruction::MyDetectorConstruction()
{
   fMessenger = new G4GenericMessenger(this, "/detector/","DetectorConstruction");

   fMessenger->DeclareProperty("nCols", nCols, "Number of Columns");
   fMessenger->DeclareProperty("nRows", nRows, "Number of Rows");
   fMessenger->DeclareProperty("isCherenkov", isCherenkov, "Toggle Cherenkov setup");
   fMessenger->DeclareProperty("isScintillator", isScintillator, "Toggle Scintillator setup");

    nCols = 100;
    nRows = 100;

    DefineMaterial();

    xWorld = 0.5*m;
    yWorld = 0.5*m;
    zWorld = 0.5*m;

    isCherenkov = false;
    isScintillator = true;
}

MyDetectorConstruction::~MyDetectorConstruction()
{}

void MyDetectorConstruction::DefineMaterial()
{
    G4NistManager *nist = G4NistManager::Instance();

    SiO2 = new G4Material("SiO2", 2.201*g/cm3,2);
    SiO2->AddElement(nist->FindOrBuildElement("Si"),1);
    SiO2->AddElement(nist->FindOrBuildElement("O"),2);

    H2O = new G4Material("H2O", 1.000*g/cm3,2);
    H2O->AddElement(nist->FindOrBuildElement("H"),2);
    H2O->AddElement(nist->FindOrBuildElement("O"),1);

    C = nist->FindOrBuildElement("C");

    Aerogel = new G4Material("Aerogel", 0.200*g/cm3, 3);
    Aerogel->AddMaterial(SiO2, 62.5*perCent);
    Aerogel->AddMaterial(H2O, 37.4*perCent);
    Aerogel->AddElement(C, 0.1*perCent);

    WorldMat = nist->FindOrBuildMaterial("G4_AIR");

    G4double energy[2] = {1.239841939*eV/0.9,1.239841939*eV/0.2,};
    G4double rindexAerogel[2] = {1.1, 1.1};
    G4double rindexWorld[2] = {1.0, 1.0};
    G4double rindexNaI[2] = {1.78, 1.78};


    G4MaterialPropertiesTable *mptAerogel = new G4MaterialPropertiesTable();
    mptAerogel->AddProperty("RINDEX", energy,rindexAerogel,2);

    G4MaterialPropertiesTable *mptWorld = new G4MaterialPropertiesTable();
    mptWorld->AddProperty("RINDEX", energy,rindexWorld,2);

    Aerogel->SetMaterialPropertiesTable(mptAerogel);

    //WorldMat->SetMaterialPropertiesTable(mptWorld);

    Na = nist->FindOrBuildElement("Na");
     I = nist->FindOrBuildElement("I");

    NaI = new G4Material("NaI" , 3.67*g/cm3 ,2);

    NaI->AddElement(Na , 1);
    NaI->AddElement(I , 1);

    G4double fraction[2] = {1.0,1.0};

    G4MaterialPropertiesTable *mptNaI = new G4MaterialPropertiesTable();

    mptNaI->AddProperty("RINDEX",energy,rindexNaI,2,true);
    mptNaI->AddProperty("FASTCOMPONENT",energy,fraction,2,true);
    mptNaI->AddConstProperty("SCINTILLATIONYIELD",38./keV,true);
    mptNaI->AddConstProperty("RESOLUTIONSCALE",1.0,true);
    mptNaI->AddConstProperty("FASTTIMECONSTANT",250*ns,true);
    mptNaI->AddConstProperty("YIELDRATIO",1.,true);

    NaI->SetMaterialPropertiesTable(mptNaI);

    WorldMat->SetMaterialPropertiesTable(mptWorld);
     }

    void MyDetectorConstruction::ConstructScintillator()
    {
        solidScintillator = new G4Box("solidScintillator",5*cm ,5*cm, 6*cm);

        logicScintillator = new G4LogicalVolume(solidScintillator, NaI,"logicScintillator");

        solidDetector = new G4Box("solidDetector" ,1*cm, 5*cm, 6*cm);

        logicDetector = new G4LogicalVolume(solidDetector, WorldMat, "logicDetector");

        fScoringVolume = logicScintillator;

        for(G4int i = 0 ; i<6 ; i++)
        {
           for(G4int j = 0; j<16 ; j++)
            {
            G4Rotate3D rotZ(j*22.5*deg , G4ThreeVector(0,0,1));
            G4Translate3D transXScint(G4ThreeVector(5./tan(22.5/2*deg)*cm + 5.*cm ,0.*cm ,-40.*cm + i*15*cm));
            G4Translate3D transXDet(G4ThreeVector(5./tan(22.5/2*deg)*cm + 6.*cm + 5.*cm ,0.*cm ,-40.*cm + i*15*cm));

            G4Transform3D transformScint = (rotZ)*(transXScint);
            G4Transform3D transformDet = (rotZ)*(transXDet);

            physScintillator = new G4PVPlacement(transformScint ,logicScintillator , "physScintillator",logicWorld, false,0,true);
            physDetector = new G4PVPlacement(transformDet ,logicDetector , "physDetector",logicWorld, false,0,true);

            }
        }
    }

void MyDetectorConstruction::ConstructCherenkov()
    {
         // G4Box *solidRadiator = new G4Box("solidRadiator",0.4*m,0.4*m, 0.01*m);
        solidRadiator = new G4Box("solidRadiator",0.4*m,0.4*m, 0.01*m);

        logicRadiator = new G4LogicalVolume(solidRadiator,Aerogel,"logicRadiator");

        fScoringVolume = logicRadiator;

        physRadiator = new G4PVPlacement(0, G4ThreeVector(0. ,0. ,0.25*m),logicRadiator,"physRadiator",logicWorld, false,0,true);

        solidDetector = new G4Box("solidDetector",xWorld/nRows,yWorld/nCols,0.01*m);

        logicDetector = new G4LogicalVolume(solidDetector,WorldMat, "logicalDetector");


    for(G4int i=0; i< nRows ; i++)
    {
       for(G4int j=0; j< nCols ;j++)
       {
        physDetector = new G4PVPlacement(0,G4ThreeVector(-0.5*m+(i+0.5)*m/nRows,-0.5*m+(j+0.5)*m/nCols,0.49*m), logicDetector ,"physDetector",logicWorld, false ,j+i*nCols,true);
       }
     }

     }
 G4VPhysicalVolume *MyDetectorConstruction::Construct()
      {
             solidWorld = new G4Box("solidWorld",xWorld,yWorld,zWorld);

             logicWorld = new G4LogicalVolume(solidWorld, WorldMat,"logicWorld");

             physWorld = new G4PVPlacement(0,G4ThreeVector(0. , 0. , 0. ),logicWorld,"physWorld",0,false,0,true);

    if(isCherenkov)
        ConstructCherenkov();

    if(isScintillator)
        ConstructScintillator();

   return physWorld;

    }

    void MyDetectorConstruction::ConstructSDandFields()
    {
      MySensitiveDetector *sensDet = new MySensitiveDetector("SensitiveDetector");

      if(logicDetector != NULL)
          logicDetector->SetSensitiveDetector(sensDet);
    }


