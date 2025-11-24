#include<iostream>
#include "G4RunManager.hh"
#include "G4UImanager.hh"
#include "G4VisManager.hh"
#include "G4VisExecutive.hh"
#include "G4UIExecutive.hh"
#include "G4PhysListFactory.hh"
#include "G4VModularPhysicsList.hh"

#include "construction.hh"
#include "physics.hh"
#include "action.hh"

int main(int argc, char** argv)
{
  G4RunManager *runManager = new G4RunManager();
  runManager->SetUserInitialization(new MyDetectorConstruction());

 G4PhysListFactory factory;
  G4VModularPhysicsList* physicsList = factory.GetReferencePhysList("FTFP_BERT");
  runManager->SetUserInitialization(new MyPhysicsList);
  runManager->SetUserInitialization(new MyActionInitialization);

  runManager->Initialize();

 // G4UIExecutive *ui = new G4UIExecutive(argc, argv);
 //we change the above into the bottom if sentence
  G4UIExecutive *ui = nullptr;

  if(argc == 1)
  {
     ui = new G4UIExecutive(argc, argv); 
  }


  G4VisManager *visManager = new G4VisExecutive();
  visManager->Initialize();

  G4UImanager *UImanager = G4UImanager::GetUIpointer();

  if(ui)
  {

  UImanager->ApplyCommand("/vis/open OGL");
  UImanager->ApplyCommand("/vis/viewer/set/viewpointvector 1 1 1");
  UImanager->ApplyCommand("/vis/drawVolume");
  UImanager->ApplyCommand("/vis/viewer/set/autoRefresh true");
  UImanager->ApplyCommand("/vis/scene/add/trajectories smooth");
  UImanager->ApplyCommand("/vis/scene/add/scale 20 cm");//new
  UImanager->ApplyCommand("/vis/scene/add/axes");       //new
  UImanager->ApplyCommand("/vis/scene/add/eventID");    //new
  UImanager->ApplyCommand("/vis/scene/endOfEventAction accumulate");

   //we repalce these  with one if we creat the vis.macro file//and dont forget to add it to the CmakeLists path later
   UImanager->ApplyCommand("/control/execute vis.mac");

   ui->SessionStart();
  }
  else
 {
    G4String command = "/control/execute ";
    G4String fileName = argv[1];
    UImanager->ApplyCommand(command+fileName);
 }


 return 0;
}
