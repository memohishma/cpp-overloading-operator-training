#ifndef ACTION_HH
#define ACTION_HH

#include "G4VUserActionInitialization.hh"

class MyAction : public G4VUserActionInitialization
{
   public:
     MyAction();
     ~MyAction();


    virtual void Build() const override;
    virtual void BuildForMaster() const override;


};
#endif

