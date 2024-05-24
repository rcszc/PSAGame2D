// framework_filesys_json.

#ifndef _FRAMEWORK_FILESYS_JSON_H
#define _FRAMEWORK_FILESYS_JSON_H
#include <document.h>
// LLFilesys PSA-L.2 (=>logger).
#include "../LLLogger/framework_logger.hpp"

#define PSAG_FILESYS_JSON_LABEL "PSAG_FILESYS_JSON"

namespace PSAG_FILESYS_JSON {
	// .json str_data / .json str_name.
	enum JsonModeType {
		InputJsonFileName = 1 << 1,
		InputJsonString   = 1 << 2
	};

	class PsagFilesysJson {
	protected:
		rapidjson::Document JsonObject = {};
		bool StatusFlag = true;
	public:
		PsagFilesysJson(const std::string instr, JsonModeType mode);

		bool                 GetLoaderStatusFlag() { return StatusFlag;  }
		rapidjson::Document* GetLoaderJsonObj()    { return &JsonObject; }

		std::string SaveJsonString(const rapidjson::Document& doc);
	};
	// base64 encode,decode.
	namespace BASE64_TOOL {

		std::string PsagBase64Encode(const std::string& strdata);
		std::string PsagBase64Decode(const std::string& strdata);

		RawDataStream PsagStringToRawData(const std::string& strdata);
		std::string PsagRawDataToString(const RawDataStream& bindata);

		RawDataStream PsagJsonStrToRawData(const std::string& json);
		std::string PsagRawDataToJsonStr(const RawDataStream& data);
	}
}

#endif
