// psag_mainevent_support.
#include "psag_mainevent_support.h"

using namespace std;
using namespace PSAG_LOGGER;

namespace PsagMainEvent {

    void MainAsyncTask::AsyncEventStart() {
        for (const auto& EventItem : EventsArray)
            FutureEvent.push_back(async(launch::async, EventItem.EventTask));
        EventsArray.clear();
        EventsArray.shrink_to_fit();

        while (RunningFlag) {
            for (auto Event = FutureEvent.begin(); Event != FutureEvent.end();) {
                if (Event->wait_for(chrono::seconds(NULL)) == future_status::ready) {
                    // execution end.
                    EventsResults.push(Event->get());
                    Event = FutureEvent.erase(Event);
                }
                else
                    ++Event;
            }
            this_thread::sleep_for(chrono::milliseconds((long long)CyclesSleepTime));
            if (EventsArray.empty()) {
                RunningFlag = false;
                PushLogger(LogTrace, PSAGM_MAIN_EVENT_LABEL, "async_events execution end.");
            }
        }
    }

    void MainAsyncTask::TasksStart() {
        RunningFlag = true;
        AsyncEventStart();
        PushLogger(LogInfo, PSAGM_MAIN_EVENT_LABEL, "async_events executing...");
    }

    void MainAsyncTask::AsyncTaskAdd(string name, function<int()> event) {
        Event CreateTaskEvent = {};

        CreateTaskEvent.EventUniqueID = SystemGenUID.PsagGenTimeKey();
        CreateTaskEvent.EventName     = name;
        CreateTaskEvent.EventTask     = event;

        EventsArray.push_back(CreateTaskEvent);
        PushLogger(LogTrace, PSAGM_MAIN_EVENT_LABEL, "async_event add item_id: %s", name.c_str());
    }
}

namespace PsagFrameworkStart {
    namespace System {
        CoreMidFrameworkBase* GLOBAL_FMC_OBJECT = nullptr;
    }
    void CorePsagMainStart::StartCoreObjectsRes(FrameworkSysVersion version) {
        PushLogger(LogInfo, PSAGM_FRAME_START_LABEL, "core_start objects(frm) total: only");
        PushLogger(LogInfo, PSAGM_FRAME_START_LABEL, "core_start objects(net) total: %u", CoreNetworkObj.size());
        PushLogger(LogInfo, PSAGM_FRAME_START_LABEL, "core_start objects(ext) total: %u", CoreExtModuleObj.size());

        // create event_task(func_object).
        function<int()> FuncObjTemp = [&]() {
            bool ExitFlag = false;
            // framework init_func.
            CoreFramewokObj->CoreFrameworkInit(version);
            while (!ExitFlag) {
                ExitFlag = !CoreFramewokObj->CoreFrameworkEvent();
            }
            bool ResultTemp = CoreFramewokObj->CoreFrameworkCloseFree() == true ?
                PSAGM_FLAG_FRAMEWORK :
                PSAGM_FLAG_FRAMEWORK + 1;
            delete CoreFramewokObj;
            return ResultTemp;
        };
        System::GLOBAL_FMC_OBJECT = CoreFramewokObj;
        StartEventsLoop->AsyncTaskAdd(to_string((size_t)&FuncObjTemp), FuncObjTemp);

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
                    PSAGM_FLAG_NETWORK :
                    PSAGM_FLAG_NETWORK + 1;
                delete ObjectPointer;
                return ResultTemp;
            };
            StartEventsLoop->AsyncTaskAdd(to_string((size_t)&FuncObjTemp), FuncObjTemp);
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
                    PSAGM_FLAG_EXTMODULE :
                    PSAGM_FLAG_EXTMODULE + 1;
                delete ObjectPointer;
                return ResultTemp;
            };
            StartEventsLoop->AsyncTaskAdd(to_string((size_t)&FuncObjTemp), FuncObjTemp);
        }
        // start all event_tasks.
        StartEventsLoop->TasksStart();
    }

    void CorePsagMainStart::StartPsaGame(FrameworkSysVersion version, uint32_t cyctiem) {
        StartEventsLoop = new PsagMainEvent::MainAsyncTask(cyctiem);
        if (StartEventsLoop == nullptr)
            return;
        StartCoreObjectsRes(version);
    }

    int CorePsagMainStart::FreeFramework() {
        const auto& Results = StartEventsLoop->GetTasksResults();
        while (!Results->empty()) {
            // ...process. 20240426.
            PushLogger(LogInfo, PSAGM_FRAME_START_LABEL, "free framework ret_code: %i", Results->front());
            Results->pop();
        }
        if (StartEventsLoop == nullptr)
            return -2;
        delete StartEventsLoop;
        return NULL;
    }

    template<typename TOBJ>
    inline bool ADDOBJECT_TMP(vector<TOBJ*>& vecidx, TOBJ* object) {
        if (object == nullptr)
            return false;
        vecidx.push_back(object);
        return true;
    }

    void CorePsagMainStart::UniqueFrameworkObj(CoreMidFrameworkBase* objptr)   { CoreFramewokObj = objptr;  }
    bool CorePsagMainStart::AddItemNetworkObj(CoreMidFrameworkNetBase* objptr) { return ADDOBJECT_TMP(CoreNetworkObj, objptr);   }
    bool CorePsagMainStart::AddItemExtModuleObj(CoreMidExtModuleBase* objptr)  { return ADDOBJECT_TMP(CoreExtModuleObj, objptr); }
}