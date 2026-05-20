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
