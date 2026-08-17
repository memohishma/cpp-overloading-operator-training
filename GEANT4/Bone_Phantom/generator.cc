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
    G4double sigmaAngle = 10.0e-3; // التشتت الزاوي بمقدار 10 ميراد (0.010 راديان)
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
