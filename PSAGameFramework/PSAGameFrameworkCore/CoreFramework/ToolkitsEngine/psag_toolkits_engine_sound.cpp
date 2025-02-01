// psag_toolkits_engine_sound.
#include "psag_toolkits_engine.h"

using namespace std;
using namespace PSAG_LOGGER;

namespace ToolkitsEngineSound {

	GameAudioSound::GameAudioSound(const RawAudioStream& data) {
		if (data.empty()) {
			PushLogger(LogError, PSAGM_TOOLKITS_SOUND, "audio stream_data empty!");
			return;
		}
		PSAG_SYS_GENERATE_KEY GenResourceID;
		// create key => storage.
		RawStreamResource = GenResourceID.PsagGenUniqueKey();
		AudioResource->ResourceStorage(RawStreamResource, data);

		// load resource => create player.
		PsagLow::PsagSupAudioData* LoaderResource = new PsagLow::PsagSupAudioData(data);
		AudioPlayer = new PsagLow::PsagSupAudioDataPlayer(LoaderResource);

		PushLogger(LogInfo, PSAGM_TOOLKITS_SOUND, "audio player create.");
	}

	GameAudioSound::~GameAudioSound() {
		delete AudioPlayer;
		AudioResource->ResourceDelete(RawStreamResource);
	}

	PsagLow::PsagSupAudioDataPlayer* GameAudioSound::AudioPlayerOperate() {
		return AudioPlayer;
	}
}
