// framework_sound.

#ifndef _FRAMEWORK_SOUND_H
#define _FRAMEWORK_SOUND_H
#include <al.h>
#include <alc.h>
// LLSound PSA - L.4 (=> logger).
#include "../LLLogger/framework_logger.hpp"

namespace PSAG_SOUND_PLAYER {
	StaticStrLABEL PSAG_SOUND_LABEL = "PSAG_SOUND";
	namespace system {
		class __PsagSoundDeviceHandle {
		private:
			static ALCdevice*  SoundDevicePtr;
			static ALCcontext* SoundContextPtr;
		protected:
			bool CreateSoundDevice();
			bool DeleteSoundDevice();
		};
	}

	class PsagSoundDataResource {
	protected:
		ALuint BufferObjectHandle = NULL;
		// create openal buffer_object handle.
		bool CreateBufHandle(ALuint& handle);
	public:
		PsagSoundDataResource(const RawSoundStream& rawdata);
		~PsagSoundDataResource();

		ALuint _MS_GETRES() {
			return BufferObjectHandle;
		}
	};

	class PsagSoundDataPlayer {
	protected:
		ALuint BufferObjectHandle = NULL;
		ALuint SourceObjectHandle = NULL;
		bool Sound3DEnabelFlag = false;
	public:
		PsagSoundDataPlayer(PsagSoundDataResource& loader);
		~PsagSoundDataPlayer();

		void SoundPlayer();
	    void SoundPause();
		void SetPlayerBegin();

		bool PlayerEndedFlag();

		void SoundSet3DPosition(const Vector3T<float>& position);   // 播放位置.
		void SoundSet3DDirection(const Vector3T<float>& direction); // 播放方向.
		void SoundSet3DGain(float gain); // 播放音频增益.

		// 用于模拟"Doppler"效应.
		void SoundSet3DVelocity(const Vector3T<float>& velocity);           // 播放速度.
		void SoundSet3DVelocityListener(const Vector3T<float>& listener_v); // 听者速度.
	};
}

// psag sound low_level, resource.
namespace PSAGSD_LOWLEVEL {
	StaticStrLABEL PSAG_SOUND_DATA_LABEL = "PSAG_SOUND_DATA";
	// PSA - CVT.1 func.
	RawSoundStream SOUND_LLRES_CONVERT_FUNC(const RawDataStream& dataset);

	// PSAG framework lowlevel sound dataset.
	class PSAG_SOUND_LLRES {
	protected:
		std::unordered_map<ResUnique, RawSoundStream> ResourceRawShoudMap = {};
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