#include "detector.hh"
#include "runaction.hh"
#include "G4RunManager.hh"

MySensitiveDetector::MySensitiveDetector(G4String name) : G4VSensitiveDetector(name)
{}

MySensitiveDetector::~MySensitiveDetector()
{}

G4bool MySensitiveDetector::ProcessHits(G4Step *aStep, G4TouchableHistory *R0hist)
{
    // تم نقل آلية التسجيل بشكل كامل وحصري إلى SteppingAction 
    // لمنع التكرار والحصول على ملفات CSV نظيفة ودقيقة جداً
    return true; 
}


