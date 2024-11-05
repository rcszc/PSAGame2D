// framework_channels_json.
#include <stringbuffer.h>
#include <writer.h>
#include "../framework_filesys_loader.hpp"

using namespace std;
using namespace PSAG_LOGGER;

namespace DecoderChannels {

	void JsonDecodeChannel::__SystemRawSource(RawDataStream* data) {
		if (data == nullptr) {
			PushLogger(LogError, PSAG_DEC_CHANNEL_STRING, "decoder data is nullptr.");
			return;
		}
		RawSourceDataRef = data;
	}

	bool JsonDecodeChannel::__SystemRawDecoding() {
		if (RawSourceDataRef->empty())
			return false;
		// raw_data => string => json_data.
		string ConvertRawDataToString(RawSourceDataRef->begin(), RawSourceDataRef->end());
		DecodeOutput.Parse(ConvertRawDataToString.c_str());
		return true;
	}
	
	rapidjson::Document* JsonDecodeChannel::GetDocumentOutput() {
		return &DecodeOutput;
	}

	rapidjson::Document JsonDecodeChannel::StringDecodeDocument(const string& str_data) {
		rapidjson::Document DecoderOutputTemp = {};
		string DecoderStrTemp = str_data;
		DecoderOutputTemp.Parse(DecoderStrTemp.c_str());
		return DecoderOutputTemp;
	}
}

namespace EncoderChannels {

	void JsonEncodeChannel::__SystemRawSource(RawDataStream* data) {
		if (data == nullptr) {
			PushLogger(LogError, PSAG_ENC_CHANNEL_STRING, "encoder data is nullptr.");
			return;
		}
		RawSourceDataRef = data;
	}

	bool JsonEncodeChannel::__SystemRawEncoding() {
		rapidjson::StringBuffer StrBuffer = {};
		rapidjson::Writer<rapidjson::StringBuffer> WriterDoc(StrBuffer);

		// write doc => buffer => string.
		if (!EncoderInput->Accept(WriterDoc)) {
			PushLogger(LogError, PSAG_ENC_CHANNEL_JSON, "failed loader(json) write(doc).");
			return false;
		}
		RawSourceDataRef->assign(StrBuffer.GetString(), StrBuffer.GetString() + StrBuffer.GetSize());
		PushLogger(LogInfo, PSAG_ENC_CHANNEL_JSON, "loader(json) write(doc): u%", 
			RawSourceDataRef->size()
		);
		return true;
	}

	void JsonEncodeChannel::SetDocumentInput(rapidjson::Document* data) {
		EncoderInput = data;
	}
}