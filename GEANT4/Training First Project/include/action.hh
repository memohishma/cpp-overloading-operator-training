#ifndef ACTION_HH
#define ACTION_HH

#include "G4VUserActionInitialization.hh"
#include "generator.hh"
class MyActionInitialization : public G4VUserActionInitializa>
{
    public:
     MyActionInitialization();
     ~MyActionInitialization();

     virtual void Build() const;


};
#endif
