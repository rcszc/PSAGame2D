// psag_toolkits_engine_notify. RCSZ. [middle_level_engine]
// toolkits(module) notfiy engine. [20241228]
// only-header 全局通用广播系统.

#ifndef __PSAG_TOOLKITS_ENGINE_NOTIFY_H
#define __PSAG_TOOLKITS_ENGINE_NOTIFY_H

#define ENABLE_LOWMODULE_AUDIO
#include "../psag_lowlevel_support.h"

namespace ToolkitsEngineNotify {
	StaticStrLABEL PSAGM_TOOLKITS_NOTFIY_LABEL = "PSAG_TOOL_NOTFIY";

	enum MSG_DATA_MODE {
		NotifyDataMode_None         = 0,
		NotifyDataMode_ArrayInt32   = 1 << 1,
		NotifyDataMode_ArrayFloat32 = 1 << 2,
		NotifyDataMode_ArraySource  = 1 << 3,
	};

	using UnamePair = std::pair<std::string, std::string>;
	struct NotifyInformation {
		UnamePair  StationUniqueNamePair = {};
		std::mutex StationDataMutex      = {};

		size_t      NotifyUnqiueID  = NULL;
		std::string NotifyMessage   = {};
		uint16_t    NotifyCodeFlags = NULL;

		MSG_DATA_MODE NotifyDataMode  = NotifyDataMode_None;
		RawDataStream NotifyDataArray = {};

		inline void NotifyInfoClear() {
			NotifyUnqiueID  = NULL;
			NotifyMessage   = {};
			NotifyCodeFlags = NULL;
			NotifyDataMode  = {};
			NotifyDataArray = {};
			// first: send, second: target.
			StationUniqueNamePair = {};
		}
	};

	class NotifySystem;
	class StationSystemInterface {
	protected:
		virtual void NotifySettingStatus() = 0;
		virtual void NotifyProcessReceive(const NotifyInformation& notify) = 0;
		virtual void NotifyProcessSend() = 0;
	public:
		virtual std::string __STATION_REGISTER(
			NotifySystem* system_ptr
		) = 0;
		friend class NotifySystem;
	};

	class NotifySystem {
	protected:
		std::unordered_map<std::string, size_t> NotifyStationsIndex = {};
		std::vector<StationSystemInterface*>    NotifyStationsRef   = {};

		std::string NotifyTagName = {};

		size_t NotifyGenerateUnqiueID() {
			// ns_time count gen uniuqe_id. RCSZ 20241228.
			return (size_t)std::chrono::duration_cast<std::chrono::nanoseconds>(
				std::chrono::steady_clock::now().time_since_epoch()
			).count();
		}
	public:
		NotifySystem(const std::string& tag_name) : NotifyTagName(tag_name) {};

		bool RegisterStation(StationSystemInterface* ptr) {
			if (ptr != nullptr) {
				NotifyStationsRef.push_back(ptr);
				// register index => ref hash_map.
				NotifyStationsIndex[ptr->__STATION_REGISTER(this)] 
					= NotifyStationsRef.size() - 1;
				return true;
			}
			PSAG_LOGGER::PushLogger(
				LogError, PSAGM_TOOLKITS_NOTFIY_LABEL, 
				"register object is nullptr."
			);
			return false;
		}

		bool __NOTIFY_SYSTEM_SEND(const std::string& rec_name, NotifyInformation* notify) {
			// check send station unique name.
			if (NotifyStationsIndex.find(rec_name) != NotifyStationsIndex.end()) {
				auto TargetStation = NotifyStationsRef[NotifyStationsIndex[rec_name]];
				// thread_safe => send.
				{
					std::lock_guard<std::mutex> MutexLock(notify->StationDataMutex);
					// set uid & set target u_name.
					notify->NotifyUnqiueID = NotifyGenerateUnqiueID();
					notify->StationUniqueNamePair.second = rec_name;
					// info send to target => set status.
					TargetStation->NotifyProcessReceive(*notify);
					TargetStation->NotifySettingStatus();
				}
				return true;
			}
			PSAG_LOGGER::PushLogger(
				LogWarning, PSAGM_TOOLKITS_NOTFIY_LABEL, 
				"not found station name: %s", rec_name.c_str()
			);
			return false;
		}
	};

	class StationSystem :public StationSystemInterface {
	protected:
		std::string   NotifySystemName = {};
		NotifySystem* NotifySystemRef  = nullptr;

		NotifyInformation ReceiveInformation = {};
		bool StationStatusFlag = false;

		void NotifySettingStatus() override {
			StationStatusFlag = true;
		}
		// thread-safe context.
		void NotifyProcessReceive(const NotifyInformation& notify) override {
			// information data copy.
			ReceiveInformation.StationUniqueNamePair = notify.StationUniqueNamePair;
			ReceiveInformation.NotifyUnqiueID  = notify.NotifyUnqiueID;
			ReceiveInformation.NotifyMessage   = notify.NotifyMessage;
			ReceiveInformation.NotifyCodeFlags = notify.NotifyCodeFlags;

			if (ReceiveInformation.NotifyDataMode != NotifyDataMode_None) {
				std::memcpy(
					ReceiveInformation.NotifyDataArray.data(),
					notify.NotifyDataArray.data(), notify.NotifyDataArray.size()
				);
			}
			std::pair<std::string, std::string> NamePair = notify.StationUniqueNamePair;
			PSAG_LOGGER::PushLogger(
				LogInfo, PSAGM_TOOLKITS_NOTFIY_LABEL, "station receive pair: %s/%s, data_size: %u bytes",
				NamePair.first.c_str(), NamePair.second.c_str(), notify.NotifyDataArray.size()
			);
		}
		// send => notify_system ref.
		void NotifyProcessSend() override {

		}
	public:
		StationSystem(const std::string& u_name) : NotifySystemName(u_name) {};

		bool REC_InfoStatusGet() { return StationStatusFlag; }
		void REC_InfoStatusOK () {
			ReceiveInformation.NotifyInfoClear();
			StationStatusFlag = false;
		}

		RawDataStream REC_GetInfoRawData()   const { return ReceiveInformation.NotifyDataArray; }
		UnamePair     REC_GetInfoUnamePair() const { return ReceiveInformation.StationUniqueNamePair; }
		std::string   REC_GetInfoMessage()   const { return ReceiveInformation.NotifyMessage; }
		uint16_t      REC_GetInfoCode()      const { return ReceiveInformation.NotifyCodeFlags; }

		std::string __STATION_REGISTER(NotifySystem* system_ptr) override {
			NotifySystemRef = system_ptr;
			PSAG_LOGGER::PushLogger(
				LogInfo, PSAGM_TOOLKITS_NOTFIY_LABEL, "station register n: %s, ptr %x",
				NotifySystemName.c_str(), NotifySystemRef
			);
			return NotifySystemName;
		}
	};
}

#endif