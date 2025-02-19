// psag_core_drivers.
#include "psag_core_drivers.h"

using namespace std;
using namespace PSAG_LOGGER;

namespace PsagMainEvent {

    void MainAsyncTask::AsyncEventStart() {
        for (const auto& EventItem : EventsArray)
            FutureEvent.push_back(async(launch::async, EventItem.TaskProcFunction));
        EventsArray.clear();
        EventsArray.shrink_to_fit();
        // async main drivers event(s).
        while (RunningFlag) {
            for (auto DriversEvent = FutureEvent.begin(); DriversEvent != FutureEvent.end();) {
                if (DriversEvent->wait_for(chrono::seconds(0)) == future_status::ready) {
                    // execution end.
                    EventsResults.push(DriversEvent->get());
                    DriversEvent = FutureEvent.erase(DriversEvent);
                }
                else
                    ++DriversEvent;
            }
            this_thread::sleep_for(chrono::milliseconds((long long)CYCLES_SLEEP_TIME));
            if (EventsArray.empty() && FutureEvent.empty()) {
                RunningFlag = false;
                PushLogger(LogTrace, PSAGM_MAIN_EVENT_LABEL, "async_events execution end.");
            }
        }
    }

    void MainAsyncTask::TasksStart() {
        RunningFlag = true;
        PushLogger(LogInfo, PSAGM_MAIN_EVENT_LABEL, "async_events executing...");
        // framework global async events start.
        AsyncEventStart();
    }

    void MainAsyncTask::AsyncTaskADD(string name, function<int()> event) {
        DriversEvent CreateTaskEvent = {};
        CreateTaskEvent.TaskProcFunction = event;

        CreateTaskEvent.EventUniqueID = SystemGenUID.PsagGenUniqueKey();
        CreateTaskEvent.EventName     = name;
        // event task => events(array).
        EventsArray.push_back(CreateTaskEvent);
        PushLogger(LogTrace, PSAGM_MAIN_EVENT_LABEL, "async_event add item_id: %s", name.c_str());
    }
}

namespace PsagFrameworkStart {
    namespace System {
        CoreMidFrameworkBase* GLOBAL_FMC_OBJECT = nullptr;
    }
    void CorePsagMainStart::StartCoreObjectsRes(FrameworkSysVersion version) {
        PushLogger(LogInfo, PSAGM_DRIVE_START_LABEL, "core_start objects(frm) total: only");
        PushLogger(LogInfo, PSAGM_DRIVE_START_LABEL, "core_start objects(net) total: %u", CoreNetworkObj.size());
        PushLogger(LogInfo, PSAGM_DRIVE_START_LABEL, "core_start objects(ext) total: %u", CoreExtModuleObj.size());

        // create event_task(func_object).
        function<int()> FuncObjTemp = [&]() {
            bool ExitFlag = false;
            // framework init_func.
            CoreFramewokObj->CoreFrameworkInit(version);
            while (!ExitFlag) {
                ExitFlag = !CoreFramewokObj->CoreFrameworkEvent();
            }
            bool ResultTemp = CoreFramewokObj->CoreFrameworkCloseFree() == true ?
                PSAGM_FLAG_FRAMEWORK : PSAGM_FLAG_FRAMEWORK + 1;
            delete CoreFramewokObj;
            CoreFramewokObj = nullptr;
            return ResultTemp;
        };
        System::GLOBAL_FMC_OBJECT = CoreFramewokObj;
        StartEventsLoop->AsyncTaskADD(to_string((size_t)&FuncObjTemp), FuncObjTemp);

        // 'CoreNetworkObj' =add=> event_task.
        for (const auto& ObjectPointer : CoreNetworkObj) {
            // create event_task(func_object).
            function<int()> FuncObjTemp = [&]() {
                bool ExitFlag = false;
                // init_func network.
                ObjectPointer->CoreFrameworkInit();
                while (!ExitFlag) {
                    ExitFlag = !ObjectPointer->CoreFrameworkEvent();
                    ObjectPointer->CoreFrameworkDataEvent();
                }
                bool ResultTemp = ObjectPointer->CoreFrameworkCloseFree() == true ?
                    PSAGM_FLAG_NETWORK : PSAGM_FLAG_NETWORK + 1;
                delete ObjectPointer;
                return ResultTemp;
            };
            StartEventsLoop->AsyncTaskADD(to_string((size_t)&FuncObjTemp), FuncObjTemp);
        }

        // 'CoreExtModuleObj' =add=> event_task.
        for (const auto& ObjectPointer : CoreExtModuleObj) {
            // create event_task(func_object).
            function<int()> FuncObjTemp = [&]() {
                bool ExitFlag = false;
                // init_func extern_module.
                ObjectPointer->CoreFrameworkInit(version);
                while (!ExitFlag) {
                    ExitFlag = !ObjectPointer->CoreFrameworkEvent();
                    ObjectPointer->CoreFrameworkDataEvent();
                }
                bool ResultTemp = ObjectPointer->CoreFrameworkCloseFree() == true ?
                    PSAGM_FLAG_EXTMODULE : PSAGM_FLAG_EXTMODULE + 1;
                delete ObjectPointer;
                return ResultTemp;
            };
            StartEventsLoop->AsyncTaskADD(to_string((size_t)&FuncObjTemp), FuncObjTemp);
        }
        // start all event tasks.
        StartEventsLoop->TasksStart();
    }

    void CorePsagMainStart::StartPsaGame(FrameworkSysVersion version, uint32_t cyctime) {
        StartEventsLoop = new PsagMainEvent::MainAsyncTask(cyctime);
        if (StartEventsLoop == nullptr) return;
        StartCoreObjectsRes(version);
    }

    int CorePsagMainStart::FreeFramework() {
        const auto& Results = StartEventsLoop->GetTasksResults();
        while (!Results->empty()) {
            // ...process. 20240426.
            PushLogger(LogInfo, PSAGM_DRIVE_START_LABEL, "free framework ret_code: %i", Results->front());
            Results->pop();
        }
        if (StartEventsLoop == nullptr) return -2;
        delete StartEventsLoop;
        return NULL;
    }

    void CorePsagMainStart::UniqueFrameworkObj(CoreMidFrameworkBase* objptr) {
        CoreFramewokObj = objptr;
    }
    // check register object ptr.
    template<typename TOBJ>
    inline bool ADDOBJECT_TMP(vector<TOBJ*>& vecidx, TOBJ* object) {
        if (object == nullptr) return false;
        vecidx.push_back(object);
        return true;
    }
    bool CorePsagMainStart::AddItemNetworkObj(CoreMidFrameworkNetBase* objptr) { 
        // main async run_event add: module.net comp.
        return ADDOBJECT_TMP(CoreNetworkObj, objptr);   
    }
    bool CorePsagMainStart::AddItemExtModuleObj(CoreMidExtModuleBase* objptr)  { 
        // main async run_event add: module.ext comp.
        return ADDOBJECT_TMP(CoreExtModuleObj, objptr); 
    }
}