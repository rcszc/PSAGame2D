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
		NotifyDataMode_None   = 0,
		NotifyDataMode_Object = 1 << 1,
		NotifyDataMode_Array  = 1 << 2
	};

	using RawDataStream = std::vector<uint8_t>;

	struct SerializeData {
		std::string   SerRawDataType = {};
		RawDataStream SerRawData     = {};
	};

	using UnamePair = std::pair<std::string, std::string>;
	struct NotifyInformation {
		UnamePair StationUniqueNamePair = {};

		size_t      NotifyUnqiueID  = NULL;
		std::string NotifyMessage   = {};
		uint16_t    NotifyCodeFlags = NULL;

		MSG_DATA_MODE NotifyDataMode  = NotifyDataMode_None;
		SerializeData NotifyDataBytes = {};

		inline void NotifyInfoClear() {
			NotifyUnqiueID  = NULL;
			NotifyMessage   = {};
			NotifyCodeFlags = NULL;
			NotifyDataMode  = {};
			NotifyDataBytes = {};
			// first: send, second: target.
			StationUniqueNamePair = {};
		}
	};

	class NotifySystem;
	class StationSystemInterface {
	protected:
		virtual void NotifySettingStatus() = 0;
		virtual void NotifyProcessReceive(const NotifyInformation& notify) = 0;
		virtual bool NotifyProcessSend() = 0;
	public:
		virtual std::string __STATION_REGISTER(
			NotifySystem* system_ptr
		) = 0;
		friend class NotifySystem;
	};

	class NotifySystem {
	private:
		std::mutex NotifyResourceMutex = {};
	protected:
		std::unordered_map<std::string, size_t> NotifyStationsIndex = {};
		std::vector<StationSystemInterface*>    NotifyStationsRef = {};

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
				LogError, PSAGM_TOOLKITS_NOTFIY_LABEL, "register object is nullptr."
			);
			return false;
		}

		bool __NOTIFY_SYSTEM_SEND(const std::string& rec_name, NotifyInformation* notify) {
			// check send station unique name.
			if (NotifyStationsIndex.find(rec_name) != NotifyStationsIndex.end()) {
				auto TargetStation = NotifyStationsRef[NotifyStationsIndex[rec_name]];
				// thread_safe => send.
				{
					std::lock_guard<std::mutex> MutexLock(NotifyResourceMutex);
					// alloc notify unique_id code.
					notify->NotifyUnqiueID = NotifyGenerateUnqiueID();
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

		bool StationStatusFlag = false;
		NotifyInformation ReceiveInformation = {};
		NotifyInformation SendInformation    = {};

		void NotifySettingStatus() override {
			StationStatusFlag = true;
		}
		// thread-safe context.
		void NotifyProcessReceive(const NotifyInformation& notify) override {
			// information data copy.
			ReceiveInformation.StationUniqueNamePair = notify.StationUniqueNamePair;
			// 'mutex' object non-copy.
			ReceiveInformation.NotifyUnqiueID  = notify.NotifyUnqiueID;
			ReceiveInformation.NotifyMessage   = notify.NotifyMessage;
			ReceiveInformation.NotifyCodeFlags = notify.NotifyCodeFlags;

			if (ReceiveInformation.NotifyDataMode != NotifyDataMode_None) {
				std::memcpy(
					ReceiveInformation.NotifyDataBytes.SerRawData.data(),
					notify.NotifyDataBytes.SerRawData.data(), notify.NotifyDataBytes.SerRawData.size()
				);
				// object info string copy.
				ReceiveInformation.NotifyDataBytes.SerRawDataType
					= notify.NotifyDataBytes.SerRawDataType;
			}
			std::pair<std::string, std::string> NamePair = notify.StationUniqueNamePair;
			PSAG_LOGGER::PushLogger(
				LogInfo, PSAGM_TOOLKITS_NOTFIY_LABEL, "station receive pair: %s/%s, data_size: %u bytes",
				NamePair.first.c_str(), NamePair.second.c_str(), notify.NotifyDataBytes.SerRawData.size()
			);
		}
		// send => notify_system ref.
		bool NotifyProcessSend() override {
			if (NotifySystemRef != nullptr) {
				// notify_system ref => send info.
				return NotifySystemRef->__NOTIFY_SYSTEM_SEND(
					SendInformation.StationUniqueNamePair.second,
					&SendInformation
				);
			}
			PSAG_LOGGER::PushLogger(
				LogError, PSAGM_TOOLKITS_NOTFIY_LABEL, "notify_system object ref is nullptr."
			);
			return false;
		}
	public:
		StationSystem(const std::string& u_name) : NotifySystemName(u_name) {}

		bool REC_InfoStatusGet() { return StationStatusFlag; }
		// clear cache & reset status flag.
		void REC_InfoStatusOK() {
			ReceiveInformation.NotifyInfoClear();
			StationStatusFlag = false;
		}

		SerializeData REC_GetInfoRawData()   const { return ReceiveInformation.NotifyDataBytes; }
		UnamePair     REC_GetInfoUnamePair() const { return ReceiveInformation.StationUniqueNamePair; }
		std::string   REC_GetInfoMessage()   const { return ReceiveInformation.NotifyMessage; }
		uint16_t      REC_GetInfoCode()      const { return ReceiveInformation.NotifyCodeFlags; }

		void SED_SetInfoTarget(const std::string& u_name) {
			if (u_name == NotifySystemName) {
				PSAG_LOGGER::PushLogger(
					LogWarning, PSAGM_TOOLKITS_NOTFIY_LABEL, "module send equals receive."
				);
				return;
			}
			SendInformation.StationUniqueNamePair.first = NotifySystemName;
			SendInformation.StationUniqueNamePair.second = u_name;
		}

		bool SED_SetInfoRawData(const SerializeData& data, MSG_DATA_MODE mode) {
			if (mode == NotifyDataMode_None) {
				PSAG_LOGGER::PushLogger(
					LogWarning, PSAGM_TOOLKITS_NOTFIY_LABEL,
					"info raw_data exist, flag setting failed."
				);
				return false;
			}
			SendInformation.NotifyDataMode  = mode;
			SendInformation.NotifyDataBytes = data;
			return true;
		}

		void SED_SetInfoMessage(const std::string& message) { SendInformation.NotifyMessage = message; }
		void SED_SetInfoCode(uint16_t code) { SendInformation.NotifyCodeFlags = code; }

		// send infomation & clear cache.
		bool SED_SendInformation() {
			bool SendFlag = NotifyProcessSend();
			SendInformation.NotifyInfoClear();
			return SendFlag;
		}

		std::string __STATION_REGISTER(NotifySystem* system_ptr) override {
			NotifySystemRef = system_ptr;
			PSAG_LOGGER::PushLogger(
				LogInfo, PSAGM_TOOLKITS_NOTFIY_LABEL, "station register n: %s, ptr %x",
				NotifySystemName.c_str(), NotifySystemRef
			);
			return NotifySystemName;
		}
	};
	
	// object serialize & deserialize.
	namespace NotifySER {

		template <typename T>
		SerializeData PsagSerialize(const T& data) {
			SerializeData ResultTemp;

			ResultTemp.SerRawDataType = typeid(T).name();
			// data => fmt bytes 'RawDataStream'.
			ResultTemp.SerRawData.resize(sizeof(T));
			std::memcpy(ResultTemp.SerRawData.data(), &data, sizeof(T));
			return ResultTemp;
		}

		template <typename T>
		bool PsagDeserialize(const SerializeData& data, T* deser_data) {
			// check data type.
			if (data.SerRawDataType != typeid(T).name()) {
				PSAG_LOGGER::PushLogger(
					LogError, PSAGM_TOOLKITS_NOTFIY_LABEL, "deser failed, types raw: %s, dat %s", 
					data.SerRawDataType.c_str(), typeid(T).name()
				);
				return false;
			}
			// check data size.
			if (data.SerRawData.size() != sizeof(T)) {
				PSAG_LOGGER::PushLogger(
					LogError, PSAGM_TOOLKITS_NOTFIY_LABEL, "deser failed, size raw: %u, dat %u", 
					data.SerRawData.size(), sizeof(T)
				);
				return false;
			}
			std::memcpy(deser_data, data.SerRawData.data(), sizeof(T));
			return true;
		}
	}
}

#endif