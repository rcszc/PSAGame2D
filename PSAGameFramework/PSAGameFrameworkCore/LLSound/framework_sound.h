// framework_sound.

#ifndef _FRAMEWORK_SOUND_H
#define _FRAMEWORK_SOUND_H
#include <bass.h>
// LLSound PSA - L.4 (=> logger).
#include "../LLLogger/framework_logger.hpp"

#define PSAG_SOUND_LABEL "PSAG_SOUND"

#define PSAG_BASS_CONFIG_DEVICE -1
#define PSAG_BASS_CONFIG_FREQ    44100

bool PSAG_FUNC_BASS_GLOBAL_INIT();
bool PSAG_FUNC_BASS_GLOBAL_FREE();

namespace PSAG_SOUND_PLAYER {
	// enable 3d_sound.
	struct Sound3DConfig {
		bool EnableFlag;
		Vector2T<float> SoundLimit; // x:min, y:max.
	};

	class PsagSoundHandleLoader {
	protected:
		HSTREAM BassStreamObject = {};
	public:
		bool SoundLoaderRawStream(const RawSoundStream& rawdata);
		bool SoundLoaderFile(const std::string& filename);

		HSTREAM _MS_GETRES();
	};

	class PsagSoundPlayerHandle {
	protected:
		HSTREAM BassStreamObject = {};
		bool Sound3DEnabelFlag = false;
	public:
		PsagSoundPlayerHandle(PsagSoundHandleLoader& loader, const Sound3DConfig& s3d = {});
		~PsagSoundPlayerHandle();

		bool PlayerSound();
		bool PauseSound();
		bool SetPlayerPosition(float second);

		bool PlayerEndedFlag();

		bool Set3DSoundListenerPosition(const Vector3T<float>& pos);
	};
}

// psag sound low_level, resource.
namespace PSAGSD_LOWLEVEL {
	// PSA - CVT.1 func.
	RawSoundStream SOUND_LLRES_CONVERT_FUNC(const RawDataStream& dataset);

	// PSAG framework lowlevel sound dataset.
	class PSAG_SOUND_LLRES {
	protected:
		std::unordered_map<std::string, RawSoundStream> ResourceRawShoudMap = {};
		std::mutex ResourceRawShoudMutex = {};

	public:
		~PSAG_SOUND_LLRES();

		RawSoundStream* ResourceFind(ResUnique key);
		bool ResourceStorage(ResUnique key, const RawSoundStream& res);
		bool ResourceDelete(ResUnique key);

		size_t GetResTotalSizeBytes();
	};
}

#endif