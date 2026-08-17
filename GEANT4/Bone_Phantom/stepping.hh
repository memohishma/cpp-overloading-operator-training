#ifndef STEPPING_HH
#define STEPPING_HH

#include "G4UserSteppingAction.hh"
#include "G4Step.hh"
#include "runaction.hh"

class MySteppingAction : public G4UserSteppingAction
{
public:
    MySteppingAction(MyRunAction* runAction);
    ~MySteppingAction();

    virtual void UserSteppingAction(const G4Step*);

private:
    MyRunAction* fRunAction;
};

#endif
