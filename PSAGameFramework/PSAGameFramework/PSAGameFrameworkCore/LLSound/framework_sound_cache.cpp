// framework_sound_cache.
#include "framework_sound.h"

using namespace std;
using namespace PSAG_LOGGER;

namespace PSAGSD_LOWLEVEL {
	RawSoundStream SOUND_LLRES_CONVERT_FUNC(const RawDataStream& dataset) {
		return RawSoundStream(dataset);
	}

	RawSoundStream* PSAG_SOUND_LLRES::ResourceFind(ResUnique key) {
		lock_guard<mutex> Lock(ResourceRawShoudMutex);
		return (ResourceRawShoudMap.find(key) != ResourceRawShoudMap.end()) ? &ResourceRawShoudMap[key] : nullptr;
	}

	bool PSAG_SOUND_LLRES::ResourceStorage(ResUnique key, const RawSoundStream& res) {
		lock_guard<mutex> Lock(ResourceRawShoudMutex);

		auto it = ResourceRawShoudMap.find(key);
		if (it != ResourceRawShoudMap.end()) {
			PushLogger(LogWarning, PSAG_SOUND_LABEL, "raw_sound: failed storage duplicate_key: %s", key.c_str());
			return false;
		}
		// resource size = empty ?
		if (!res.empty()) {
			ResourceRawShoudMap[key] = res;
			PushLogger(LogInfo, PSAG_SOUND_LABEL, "raw_sound: storage key: %s", key.c_str());
			return true;
		}
		PushLogger(LogWarning, PSAG_SOUND_LABEL, "raw_sound: failed storage, key: %s, data_empty.", key.c_str());
		return false;
	}

	bool PSAG_SOUND_LLRES::ResourceDelete(ResUnique key) {
		lock_guard<mutex> Lock(ResourceRawShoudMutex);

		auto it = ResourceRawShoudMap.find(key);
		if (it != ResourceRawShoudMap.end()) {

			// vector free => clear map_item.
			ResourceRawShoudMap.erase(it);

			PushLogger(LogInfo, PSAG_SOUND_LABEL, "raw_sound: delete key: %s", key.c_str());
			return true;
		}
		PushLogger(LogWarning, PSAG_SOUND_LABEL, "raw_sound: failed delete, not found key.");
		return false;
	}

	size_t PSAG_SOUND_LLRES::GetResTotalSizeBytes() {
		size_t TotalRawSoundData = {};
		for (const auto& PairTmp : ResourceRawShoudMap)
			TotalRawSoundData += PairTmp.second.size();
		return TotalRawSoundData;
	}

	PSAG_SOUND_LLRES::~PSAG_SOUND_LLRES() {
		size_t TotalSizeTemp = GetResTotalSizeBytes();
		PushLogger(LogTrace, PSAG_SOUND_LABEL, "free resource(raw_sound): %u bytes", TotalSizeTemp);
	}
}