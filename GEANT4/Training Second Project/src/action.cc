#include "action.hh"
#include "generator.hh"

MyAction::MyAction()
{}
MyAction::~MyAction()
{}

void MyAction::Build() const
{
        MyGenerator *generator = new MyGenerator();
        SetUserAction(generator);

}
void MyAction::BuildForMaster() const
{

}
