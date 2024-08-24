// framework_filesys_json.
#include <base64/base64.h>
#include <writer.h>
#include <stringbuffer.h>
#include "framework_filesys_json.h"

using namespace std;
using namespace PSAG_LOGGER;

namespace PSAG_FILESYS_JSON {

	PsagFilesysJson::PsagFilesysJson(const std::string instr, JsonModeType mode) {
		switch (mode) {
		case(InputJsonFileName): {
			ifstream ReadJsonFile(instr);
			// file status.
			if (!ReadJsonFile.is_open()) {
				StatusFlag = false;
				PushLogger(LogError, PSAG_FILESYS_JSON_LABEL, 
					"failed open,loader(json) file: %s", instr.c_str());
				return;
			}
			// read char file.
			string JsonDocString((istreambuf_iterator<char>(ReadJsonFile)), istreambuf_iterator<char>());
			ReadJsonFile.close();
			// read processing json.
			JsonObject.Parse(JsonDocString.c_str());

			PushLogger(LogInfo, PSAG_FILESYS_JSON_LABEL, 
				"loader(json) file: s% read_size: u%", instr.c_str(), JsonDocString.size());
			break;
		}
		case(InputJsonString): {
			// read processing json.
			JsonObject.Parse(instr.c_str());

			PushLogger(LogInfo, PSAG_FILESYS_JSON_LABEL, 
				"loader(json) data: s% read_size: u%", instr.c_str(), instr.size());
			break;
		}
		// write: invalid mode.
		default:
			PushLogger(LogError, PSAG_FILESYS_JSON_LABEL, "invalid mode, load json.");
		}
	}

	string PsagFilesysJson::SaveJsonString(const rapidjson::Document& doc) {
		rapidjson::StringBuffer StrBuffer;
		rapidjson::Writer<rapidjson::StringBuffer> WriterDoc(StrBuffer);

		// write doc => buffer => string.
		if (!doc.Accept(WriterDoc)) {
			PushLogger(LogError, PSAG_FILESYS_JSON_LABEL, "failed loader(json) write(doc).");
			return StrBuffer.GetString();
		}
		string ResultTemp = StrBuffer.GetString();;
		PushLogger(LogError, PSAG_FILESYS_JSON_LABEL, "loader(json) write(doc): u%", ResultTemp.size());
		return ResultTemp;
	}

	namespace BASE64_TOOL {

		string PsagBase64Encode(const string& strdata) {
			return base64_encode(reinterpret_cast<const unsigned char*>(strdata.c_str()), strdata.length());
		}
		string PsagBase64Decode(const string& strdata) {
			return base64_decode(strdata);
		}

		RawDataStream PsagStringToRawData(const string& strdata) {
			RawDataStream ResultTemp = {};
			ResultTemp.resize(strdata.size());
			for (char Cu8t : strdata)
				ResultTemp.push_back((uint8_t)Cu8t);
			return ResultTemp;
		}
		string PsagRawDataToString(const RawDataStream& bindata) {
			string ResultTemp = {};
			ResultTemp.resize(bindata.size());
			for (uint8_t U8tC : bindata)
				ResultTemp.push_back((char)U8tC);
			return ResultTemp;
		}

		RawDataStream PsagJsonStrToRawData(const string& json) { return PsagStringToRawData(PsagBase64Encode(json)); }
		string PsagRawDataToJsonStr(const RawDataStream& data) { return PsagBase64Decode(PsagRawDataToString(data)); }
	}
}