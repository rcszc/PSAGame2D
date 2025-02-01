// framework_sound.

#ifndef _FRAMEWORK_SOUND_H
#define _FRAMEWORK_SOUND_H
#include <al.h>
#include <alc.h>
// LLSound PSA - L.4 (=> logger).
#include "../LLLogger/framework_logger.hpp"

namespace PSAG_AUDIO_PLAYER {
	StaticStrLABEL PSAG_AUDIO_LABEL = "PSAG_AUDIO";

#define OPENAL_INVALID_HANDEL (ALuint)NULL
	namespace system {
		class PsagAudioSystemDevice {
		private:
			static ALCdevice*  HDPTR_OPENAL_DEVICE;
			static ALCcontext* HDPTR_OPENAL_CONTEXT;
		protected:
			bool CreateSoundDevice();
			bool DeleteSoundDevice();
		};
	}

	class PsagAudioDataResource {
	protected:
		ALuint HANDLE_AUDIO_BUFFER = OPENAL_INVALID_HANDEL;
		// create openal buffer_object handle.
		bool CreateBufferHandle(ALuint& handle);
	public:
		PsagAudioDataResource(const RawAudioStream& rawdata);
		~PsagAudioDataResource();

		ALuint _MS_GETRES() {
			return HANDLE_AUDIO_BUFFER;
		}
	};

	class PsagAudioDataPlayer {
	protected:
		PsagAudioDataResource* ResourceLoader = nullptr;

		ALuint HANDLE_AUDIO_BUFFER = OPENAL_INVALID_HANDEL;
		ALuint HANDLE_AUDIO_OBJECT = OPENAL_INVALID_HANDEL;

		bool Sound3DEnabelFlag = false;
	public:
		PsagAudioDataPlayer(PsagAudioDataResource* loader);
		~PsagAudioDataPlayer();

		void SoundPlayer();
	    void SoundPause();
		void SoundBeginPosition();

		bool PlayerEndedFlag();

		void SoundSet3DPosition (const Vector3T<float>& position);  // 播放位置.
		void SoundSet3DDirection(const Vector3T<float>& direction); // 播放方向.
		void SoundSet3DGain     (float gain); // 播放音频增益.

		// 用于模拟"Doppler"效应.
		void SoundSet3DVelocity(const Vector3T<float>& velocity);           // 播放速度.
		void SoundSet3DVelocityListener(const Vector3T<float>& listener_v); // 听者速度.
	};
}

// psag audio low_level, resource.
namespace PSAGSD_LOWLEVEL {
	StaticStrLABEL PSAG_AUDIO_DATA_LABEL = "PSAG_AUDIO_DATA";
	// PSA - CVT.1 func.
	RawAudioStream AUDIO_LLRES_CONVERT_FUNC(const RawDataStream& dataset);

	// psag framework lowlevel audio dataset.
	class PsagResAudioSourceData {
	protected:
		std::unordered_map<ResUnique, RawAudioStream> ResourceRawAudioMap = {};
		std::mutex ResourceRawAudioMutex = {};
	public:
		~PsagResAudioSourceData();

		RawAudioStream* ResourceFind(ResUnique key);
		bool ResourceStorage(ResUnique key, const RawAudioStream& res);
		bool ResourceDelete(ResUnique key);

		size_t GetResTotalSizeBytes();
	};

	class PSAG_AUDIO_LLRES {
	protected:
		static PsagResAudioSourceData* AudioResource;

		void LowLevelResourceCreate();
		bool LowLevelResourceFree();
	};
}

#endif