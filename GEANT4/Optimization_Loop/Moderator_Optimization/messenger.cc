#include "messenger.hh"
#include "construction.hh"

MyDetectorMessenger::MyDetectorMessenger(MyDetectorConstruction* myDet)
: G4UImessenger(), fDetectorConstruction(myDet)
{
    fModThicknessCmd = new G4UIcmdWithADoubleAndUnit("/detector/setModeratorThickness", this);
    fModThicknessCmd->SetGuidance("Set the thickness of the moderator.");
    fModThicknessCmd->SetParameterName("thickness", false);
    fModThicknessCmd->SetUnitCategory("Length");
}

MyDetectorMessenger::~MyDetectorMessenger()
{
    delete fModThicknessCmd;
}

void MyDetectorMessenger::SetNewValue(G4UIcommand* command, G4String newValue)
{
    if (command == fModThicknessCmd) {
        fDetectorConstruction->SetModeratorThickness(fModThicknessCmd->GetNewDoubleValue(newValue));
    }
}
