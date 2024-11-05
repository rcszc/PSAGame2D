// framework_sound_cache.
#include "framework_sound.h"

using namespace std;
using namespace PSAG_LOGGER;

namespace PSAGSD_LOWLEVEL {
	RawAudioStream AUDIO_LLRES_CONVERT_FUNC(const RawDataStream& dataset) {
		return RawAudioStream(dataset);
	}

	RawAudioStream* PsagResAudioSourceData::ResourceFind(ResUnique key) {
		unique_lock<mutex> Lock(ResourceRawAudioMutex);
		return (ResourceRawAudioMap.find(key) != ResourceRawAudioMap.end()) ? &ResourceRawAudioMap[key] : nullptr;
	}

	bool PsagResAudioSourceData::ResourceStorage(ResUnique key, const RawAudioStream& res) {
		unique_lock<mutex> Lock(ResourceRawAudioMutex);

		auto it = ResourceRawAudioMap.find(key);
		if (it != ResourceRawAudioMap.end()) {
			PushLogger(LogWarning, PSAG_AUDIO_DATA_LABEL, "raw_audio: failed storage duplicate_key: %u", key);
			return false;
		}
		// resource size = empty ?
		if (!res.empty()) {
			ResourceRawAudioMap[key] = res;
			PushLogger(LogInfo, PSAG_AUDIO_DATA_LABEL, "raw_audio: storage key: %u", key);
			return true;
		}
		PushLogger(LogWarning, PSAG_AUDIO_DATA_LABEL, "raw_audio: failed storage, key: %u, data_empty.", key);
		return false;
	}

	bool PsagResAudioSourceData::ResourceDelete(ResUnique key) {
		unique_lock<mutex> Lock(ResourceRawAudioMutex);

		auto it = ResourceRawAudioMap.find(key);
		if (it != ResourceRawAudioMap.end()) {

			ResourceRawAudioMap.erase(it);
			PushLogger(LogInfo, PSAG_AUDIO_DATA_LABEL, "raw_audio: delete key: %u", key);
			return true;
		}
		PushLogger(LogWarning, PSAG_AUDIO_DATA_LABEL, "raw_audio: failed delete, not found key.");
		return false;
	}

	size_t PsagResAudioSourceData::GetResTotalSizeBytes() {
		size_t TotalRawSoundData = {};
		for (const auto& PairTemp : ResourceRawAudioMap)
			TotalRawSoundData += PairTemp.second.size();
		return TotalRawSoundData;
	}

	PsagResAudioSourceData::~PsagResAudioSourceData() {
		size_t TotalSizeTemp = GetResTotalSizeBytes();
		PushLogger(LogTrace, PSAG_AUDIO_DATA_LABEL, "free resource(raw_audio): %u bytes", TotalSizeTemp);
	}

	PsagResAudioSourceData* PSAG_AUDIO_LLRES::AudioResource = nullptr;

	void PSAG_AUDIO_LLRES::LowLevelResourceCreate() {
		AudioResource = new PsagResAudioSourceData();
		PushLogger(LogInfo, PSAG_AUDIO_DATA_LABEL, "LLRES static_object() create.");
	}

	bool PSAG_AUDIO_LLRES::LowLevelResourceFree() {
		if (AudioResource == nullptr) {
			PushLogger(LogError, PSAG_AUDIO_DATA_LABEL, "LLRES FREE_RES %s = NULLPTR.", "AUDIO");
			return false;
		}
		delete AudioResource;
		PushLogger(LogInfo, PSAG_AUDIO_DATA_LABEL, "LLRES static_object() delete.");
		return true;
	}
}