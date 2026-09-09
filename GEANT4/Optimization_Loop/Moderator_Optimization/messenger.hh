#ifndef MESSENGER_HH
#define MESSENGER_HH

#include "G4UImessenger.hh"
#include "G4UIcmdWithADoubleAndUnit.hh"

class MyDetectorConstruction;

class MyDetectorMessenger : public G4UImessenger
{
public:
    MyDetectorMessenger(MyDetectorConstruction* myDet);
    ~MyDetectorMessenger();
    
    virtual void SetNewValue(G4UIcommand* command, G4String newValue) override;

private:
    MyDetectorConstruction* fDetectorConstruction;
    G4UIcmdWithADoubleAndUnit* fModThicknessCmd;
};

#endif
