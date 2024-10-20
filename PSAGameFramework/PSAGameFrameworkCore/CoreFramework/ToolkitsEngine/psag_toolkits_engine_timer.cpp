// psag_toolkits_engine_timer.
#include "psag_toolkits_engine.h"

using namespace std;
using namespace PSAG_LOGGER;

namespace ToolkitsEngineTimerClock {

	bool GameCycleTimer::CycleTimerGetFlag() {
		// now_time >= target_time, flag => true.
		return chrono::system_clock::now() >= TimeTempTarget;
	}

	bool GameCycleTimer::CycleTimerClearReset(float time_delay) {
		if (time_delay <= 0.0f) {
			PushLogger(LogError, PSAGM_TOOLKITS_TIMER_LABEL, "cycle_time system: time_delay <= 0.0f");
			return false;
		}
		int64_t MicroTimeTemp = int64_t(time_delay * 1000.0f);
		TimeTempTarget = chrono::system_clock::now() + std::chrono::microseconds(MicroTimeTemp);
		return true;
	}

	float GameCycleTimer::GetTimeNowCount() {
		int64_t TimerCountI64 = 
			chrono::duration_cast<chrono::microseconds>(TimeTempTarget.time_since_epoch()).count();
		return (float)TimerCountI64 * 0.001f;
	}
}