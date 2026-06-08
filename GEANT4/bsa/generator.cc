#include "generator.hh"

MyPrimaryGenerator::MyPrimaryGenerator()
{
    //=========================================================================================================================
    //constructor (called when an object from the MyPrimaryGenerator class is created)
    //=========================================================================================================================

    fParticleGun = new G4ParticleGun(1); //create a new Particle Gun in the Primatry Generator constructor
    //the Particle Gun constructor, G4ParticleGun(1) takes 1 argument, the number of primary particles per event
    //THIS SHOULD STAY AS ONE PARTICLE PER EVENT!!
}

MyPrimaryGenerator::~MyPrimaryGenerator()
{
    //=========================================================================================================================
    //destructor (called when an object from the MyPrimaryGenerator class is deleted)
    //=========================================================================================================================

    delete fParticleGun; //put away your particle gun when you're done with it
}

void MyPrimaryGenerator::GeneratePrimaries(G4Event *anEvent)
{
    //=========================================================================================================================
    //this function describes and generates primary particles. This is how the incident beam of protons is defined
    //within this function, the: type, source position, energy, and direction of the incident particle is defined
    //the particle gun is then called to "shoot" the defined particle
    //=========================================================================================================================

    //get the particle type:
    G4ParticleTable *particleTable = G4ParticleTable::GetParticleTable(); //get the GEANT4 table of default particles
    G4String particleName = "proton"; //the name of the particle we want
    G4ParticleDefinition *particle = particleTable->FindParticle(particleName); //get the particle from the particle table
    // G4ParticleDefinition is a class that describes all of the properties of a particle in it's attributes (mass, charge, energy, etc.)

    //define the source:
    // here we wil define the starting position, as well as the 'Raster System' of the Tandetron Accelerator
    // the 'Raster System' takes the thin beam of protons, and sweeps it out into a square that lands on the target
    G4double LToTarget = 20.0 * cm; //distance from source position to target
    G4double LRaster = 2.5 * cm; //full side length of the square proton beam landing on target

    G4double thetaMax = std::atan(LRaster / LToTarget); //maximum deflection angle allowed in the Raster system
    G4double thetaX = thetaMax * (CLHEP::HepRandom::getTheEngine()->flat() - 0.5); //choose a random angle from Z axis in X direction
    G4double thetaY = thetaMax * (CLHEP::HepRandom::getTheEngine()->flat() - 0.5); //choose a random angle from Z axis in Y direction

    G4ThreeVector pos(0., 0., -LToTarget); //Source position of the proton beam
    G4ThreeVector mom(std::tan(thetaX), std::tan(thetaY), 1.); //Momentum direction of the  proton beam
    
    
        
    //G4ThreeVector mom(0, 0, 1.); //proton beam point source momentum direction (use this to get a perfectly straight beam on target)

    //shoot the chosen particle, from the chosen position, in the chosen direction
    fParticleGun->SetParticlePosition(pos); //set the beam source position 
    fParticleGun->SetParticleMomentumDirection(mom); //set the beam direction
    fParticleGun->SetParticleEnergy(2.2 *MeV); //set the energy of each particle (to be set in the UI)
    fParticleGun->SetParticleDefinition(particle); //set the particle type to the particle definition object above

    fParticleGun->GeneratePrimaryVertex(anEvent); //Shoot the particle to start this event!
}

//==================================================================================================================//
//Beam Info//
#include "generator.hh"
#include "CLHEP/Random/RandGauss.h"

MyPrimaryGenerator::MyPrimaryGenerator()
{
    fParticleGun = new G4ParticleGun(1); // طلقة واحدة لكل إيفنت لحساب العائد بدقة
}

MyPrimaryGenerator::~MyPrimaryGenerator()
{
    delete fParticleGun; 
}

void MyPrimaryGenerator::GeneratePrimaries(G4Event *anEvent)
{
    // 1. جلب تعريف جسيم البروتون من الجدول
    G4ParticleTable *particleTable = G4ParticleTable::GetParticleTable();
    G4String particleName = "proton";
    G4ParticleDefinition *particle = particleTable->FindParticle(particleName);

    // 2. إعدادات المسافة والأبعاد الأصلية
    G4double LToTarget = 20.0 * cm; // المسافة الآمنة التي حددناها سابقاً قبل شريحة الليثيوم

    // 3. [تعديل الـ Beam Spot الدائري عشوائياً]
    G4double beamRadius = 5.0 * mm; // نصف قطر البقعة الحركية (تعدل إلى cm إن رغبتِ)
    G4double r = beamRadius * std::sqrt(CLHEP::HepRandom::getTheEngine()->flat()); // توزيع مساحي عادل
    G4double phi = 2.0 * CLHEP::pi * CLHEP::pi * CLHEP::HepRandom::getTheEngine()->flat();
    
    G4double posX = r * std::cos(phi);
    G4double posY = r * std::sin(phi);
    G4ThreeVector pos(posX, posY, -LToTarget); // موقع انطلاق الحزمة الدائري الموزع

    // 4. [تعديل التشتت الزاوي الغاوسي - Divergence]
    G4double sigmaAngle = 10.0e-3; // التشتت الزاوي بمقدار 5 ميراد (0.005 راديان)
    G4double thetaX = G4RandGauss::shoot(0.0, sigmaAngle);
    G4double thetaY = G4RandGauss::shoot(0.0, sigmaAngle);
    
    // اتجاه حركة الحزمة للأمام مع تشتت خفيف جداً غاوسي الأطراف
    G4ThreeVector mom(std::tan(thetaX), std::tan(thetaY), 1.0); 

    // 5. [تعديل طاقة الحزمة لتكون غاوسية بالكامل]
    G4double meanEnergy = 2.2 * MeV;  // المتوسط
    G4double sigmaEnergy = 0.2 * MeV; // الـ Sigma
    G4double energy = G4RandGauss::shoot(meanEnergy, sigmaEnergy);

    // 6. شحن طلقات البندقية الفيزيائية
    fParticleGun->SetParticlePosition(pos);
    fParticleGun->SetParticleMomentumDirection(mom);
    fParticleGun->SetParticleEnergy(energy);
    fParticleGun->SetParticleDefinition(particle);

    fParticleGun->GeneratePrimaryVertex(anEvent); // إطلاق البروتون غاوسي الخصائص
}
//=========================================================================//
//the last update
#include "generator.hh"
#include "CLHEP/Random/RandGauss.h"

MyPrimaryGenerator::MyPrimaryGenerator()
{
    fParticleGun = new G4ParticleGun(1); // طلقة واحدة لكل إيفنت لحساب العائد بدقة
}

MyPrimaryGenerator::~MyPrimaryGenerator()
{
    delete fParticleGun; 
}

void MyPrimaryGenerator::GeneratePrimaries(G4Event *anEvent)
{
    // 1. جلب تعريف جسيم البروتون من الجدول
    G4ParticleTable *particleTable = G4ParticleTable::GetParticleTable();
    G4String particleName = "proton";
    G4ParticleDefinition *particle = particleTable->FindParticle(particleName);

    // 2. إعدادات المسافة والأبعاد الأصلية
    G4double LToTarget = 60.0 * cm; // المسافة الآمنة التي حددناها سابقاً قبل شريحة الليثيوم

    // 3. [تعديل الـ Beam Spot الدائري عشوائياً]
/*G4double phi =
2.0 * CLHEP::pi * CLHEP::pi *
rando*/

    G4double beamRadius = 5.0 * mm; // نصف قطر البقعة الحركية (تعدل إلى cm إن رغبتِ)
    G4double r = beamRadius * std::sqrt(CLHEP::HepRandom::getTheEngine()->flat()); // توزيع مساحي عادل
    G4double phi = 2.0 * CLHEP::pi * CLHEP::HepRandom::getTheEngine()->flat();
    
    G4double posX = r * std::cos(phi);
    G4double posY = r * std::sin(phi);
    G4ThreeVector pos(posX, posY, -LToTarget); // موقع انطلاق الحزمة الدائري الموزع

    // 4. [تعديل التشتت الزاوي الغاوسي - Divergence]
    G4double sigmaAngle = 10.0e-3; // التشتت الزاوي بمقدار 5 ميراد (0.005 راديان)
    G4double thetaX = G4RandGauss::shoot(0.0, sigmaAngle);
    G4double thetaY = G4RandGauss::shoot(0.0, sigmaAngle);
    
    // اتجاه حركة الحزمة للأمام مع تشتت خفيف جداً غاوسي الأطراف
    G4ThreeVector mom(std::tan(thetaX), std::tan(thetaY), 1.0); 

    // 5. [تعديل طاقة الحزمة لتكون غاوسية بالكامل]
    G4double meanEnergy = 2.2 * MeV;  // المتوسط
    G4double sigmaEnergy = 0.2 * MeV; // الـ Sigma
    G4double energy = G4RandGauss::shoot(meanEnergy, sigmaEnergy);

    // 6. شحن طلقات البندقية الفيزيائية
    fParticleGun->SetParticlePosition(pos);
    fParticleGun->SetParticleMomentumDirection(mom);
    fParticleGun->SetParticleEnergy(energy);
    fParticleGun->SetParticleDefinition(particle);

    fParticleGun->GeneratePrimaryVertex(anEvent); // إطلاق البروتون غاوسي الخصائص
}
