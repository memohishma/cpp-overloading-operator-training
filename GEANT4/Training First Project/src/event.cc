#include "event.hh"

#include "run.hh"
#include "G4AnalysisManager.hh"
#include "G4Event.hh"

MyEventAction::MyEventAction()
{}
MyEventAction::~MyEventAction()
{}

void MyEventAction::BeginOfEventAction(const G4Event* event)
{}

void MyEventAction::EndOfEventAction(const G4Event* event)
{
     G4AnalysisManager *man = G4AnalysisManager::Instance();

     man->FillNtupleIColumn(0,event->GetEventID());
       man->FillNtupleDColumn(1, 10.5 + event->GetEventID());      // fX
       man->FillNtupleDColumn(2, 20.3 + event->GetEventID()*0.1);  // fY
       man->FillNtupleDColumn(3, 30.7 + event->GetEventID()*0.05); // fZ

     man->AddNtupleRow();


}
