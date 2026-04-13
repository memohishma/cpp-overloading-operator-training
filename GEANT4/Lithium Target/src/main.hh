                                                            
#include <iostream>

#include "G4RunManager.hh"
#include "G4UImanager.hh"
#include "G4VisManager.hh"
#include "G4VisExecutive.hh"
#include "G4UIExecutive.hh"

#include "construction.hh"
#include "action.hh"
#include "physics.hh"
#include "generator.hh"

#include "G4VModularPhysicsList.hh"
#include "G4PhysListFactory.hh"

int main(int argc, char** argv)
{
   G4RunManager *runManager = new G4RunManager();

   runManager->SetUserInitialization(new MyDetector());
//   runManager->SetUserInitialization(new MyPhysicsList());
G4PhysListFactory factory;
G4VModularPhysicsList* physics = factory.GetReferencePhysList("QGSP_BIC_HP");
runManager->SetUserInitialization(physics);

    runManager->SetUserInitialization(new MyAction());

   runManager->Initialize();

   G4UIExecutive *ui = 0;

   if(argc == 1)
   {
   ui = new G4UIExecutive(argc, argv);
   }

   G4VisManager *visManager = new G4VisExecutive();
   visManager->Initialize();
   G4UImanager *UImanager = G4UImanager::GetUIpointer();

   if(ui)
   {
    UImanager->ApplyCommand("/control/execute vis.mac");
    ui->SessionStart();
    }
    else
    {
        G4String command = "/control/execute";
        G4String fileName = argv[1];
        UImanager->ApplyCommand(command+fileName);
    }
   return 0;
}

