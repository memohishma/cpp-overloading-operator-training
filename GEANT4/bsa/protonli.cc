//WITHOUT VISIULAIZATION

#include "protonli.hh"
#include "G4VisExecutive.hh" // تم إضافتها لضمان التوافق إذا تم تفعيل الـ Vis لاحقاً

int main(int argc, char** argv)
{
    #ifdef G4MULTITHREADED // run GEANT4 in multithreaded mode if available
        G4MTRunManager* runManager = new G4MTRunManager();
        // NumThreads يجب أن تكون معرفة في protonli.hh، يمكنكِ وضع رقم ثابت مثل 4 أو 8 هنا إذا حدث خطأ
        runManager->SetNumberOfThreads(NumThreads); 
    #else
        G4RunManager *runManager = new G4RunManager();
    #endif

    runManager->SetUserInitialization(new MyDetectorConstruction()); // initialize Geometry
    runManager->SetUserInitialization(new MyPhysicsList());           // initialize physics
    runManager->SetUserInitialization(new MyActionInitialization()); // initialize user actions

    runManager->Initialize(); // initialize the RunManager

    G4UImanager *UImanager = G4UImanager::GetUIpointer();

    // ====================================================================================
    // تعديل طور التشغيل التلقائي باستخدام ملف run.mac (Batch Mode)
    // ====================================================================================
    
    // 1. تفعيل أوامر الفيزياء الخاصة بالنيوترونات عالية الدقة لمنع تشويه الحسابات
    UImanager->ApplyCommand("/process/had/particle_hp/do_not_adjust_final_state true");

    // 2. أمر قراءة وتنفيذ ملف الماكرو run.mac تلقائياً
    G4String command = "/control/execute run.mac";
    UImanager->ApplyCommand(command);

    // ====================================================================================

    // تنظيف الذاكرة وإغلاق البرنامج تلقائياً بعد انتهاء المحاكاة وحفظ ملفات الـ CSV
    delete runManager;
    return 0;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
//WITH VISIULAZATION

#include "protonli.hh"

int main(int argc, char** argv)
{
    #ifdef G4MULTITHREADED //run GEANT4 in multithreaded mode if available
        G4MTRunManager* runManager = new G4MTRunManager();
    #else
        G4RunManager *runManager = new G4RunManager();
    #endif

    runManager->SetUserInitialization(new MyDetectorConstruction()); //initilize Geometry
    runManager->SetUserInitialization(new MyPhysicsList()); //initilaize physics (electromagnetic)
    runManager->SetUserInitialization(new MyActionInitialization()); //intilize user actions

    runManager->SetNumberOfThreads(NumThreads); //set the simulation to use the ideal number of threads
    runManager->Initialize(); //initilize the RunManager

    G4UIExecutive *ui = new G4UIExecutive(argc, argv); //set up UI
    G4UImanager *UImanager = G4UImanager::GetUIpointer();

    if (runWithVis)
    {
        G4VisManager *visManager = new G4VisExecutive(); //initilize the geometry visualization window
        visManager->Initialize();

        //initilize UI with commands 
        //UImanager->ApplyCommand("/process/had/particle_hp/skip_missing_isotopes true");
        UImanager->ApplyCommand("/process/had/particle_hp/do_not_adjust_final_state true");

        UImanager->ApplyCommand("/vis/open OGL"); //open the geometry viewer window
        UImanager->ApplyCommand("/vis/viewer/set/viewpointVector 1 0.2 0.2"); //set the perspective
        UImanager->ApplyCommand("/vis/drawVolume"); //draw the simulation geometry
        UImanager->ApplyCommand("/vis/viewer/set/autoRefresh true"); //autorefresh viewer
        UImanager->ApplyCommand("/vis/scene/add/trajectories"); //draw particle trajectories
        UImanager->ApplyCommand("/vis/scene/endOfEventAction accumulate -1"); //accumulate all events before displaying
        UImanager->ApplyCommand("/vis/multithreading/maxEventQueueSize -1"); //queue all events
        UImanager->ApplyCommand("/vis/modeling/trajectories/create/drawByParticleID true"); //draw trajectories in colour by particle type
    }
    else
    {
        //initilize UI with some commands
        //UImanager->ApplyCommand("/process/had/particle_hp/skip_missing_isotopes true");
        UImanager->ApplyCommand("/process/had/particle_hp/do_not_adjust_final_state true");
    }

    ui->SessionStart(); //Start the interactive UI
    return 0;
}
