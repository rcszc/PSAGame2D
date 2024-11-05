// framework_channels_string.
#include "../framework_filesys_loader.hpp"

using namespace std;
using namespace PSAG_LOGGER;

namespace DecoderChannels {

	void StringDecodeChannel::__SystemRawSource(RawDataStream* data) {
		if (data == nullptr) {
			PushLogger(LogError, PSAG_DEC_CHANNEL_STRING, "decoder data is nullptr.");
			return;
		}
		RawSourceDataRef = data;
	}

	bool StringDecodeChannel::__SystemRawDecoding() {
		if (RawSourceDataRef->empty())
			return false;
		DecoderOutput.assign(RawSourceDataRef->begin(), RawSourceDataRef->end());
		return true;
	}

	string StringDecodeChannel::GetStringOutput() {
		return DecoderOutput;
	}
}

namespace EncoderChannels {

	void StringEncodeChannel::__SystemRawSource(RawDataStream* data) {
		if (data == nullptr) {
			PushLogger(LogError, PSAG_ENC_CHANNEL_STRING, "encoder data is nullptr.");
			return;
		}
		RawSourceDataRef = data;
	}

	bool StringEncodeChannel::__SystemRawEncoding() {
		if (RawSourceDataRef->empty())
			return false;
		RawSourceDataRef->assign(EncoderInput.begin(), EncoderInput.end());
		return true;
	}

	void StringEncodeChannel::StringDataInput(const string& data) {
		EncoderInput = data;
	}
}