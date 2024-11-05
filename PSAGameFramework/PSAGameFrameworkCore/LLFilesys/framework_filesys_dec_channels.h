// framework_filesys_dec_channels. 2024_10_31 RCSZ.
// file data decode channels header.

#ifndef __FRAMEWORK_FILESYS_DEC_CHANNELS_H
#define __FRAMEWORK_FILESYS_DEC_CHANNELS_H
#include <document.h>

namespace PSAG_FILESYS_CH_BASE {
	// decode channels interface.
	class PsagFilesysDecodeBase {
	public:
		virtual void __SystemRawSource(RawDataStream* data) = 0;
		virtual bool __SystemRawDecoding() = 0;
	};

	// encode channels interface.
	class PsagFilesysEncodeBase {
	public:
		virtual void __SystemRawSource(RawDataStream* data) = 0;
		virtual bool __SystemRawEecoding() = 0;
	};
}

namespace DecoderChannels {
	StaticStrLABEL PSAG_DEC_CHANNEL_STRING = "PSAG_FILESYS_DCH_STRING";
	StaticStrLABEL PSAG_DEC_CHANNEL_JSON   = "PSAG_FILESYS_DCH_JSON";

	class StringDecodeChannel :public PSAG_FILESYS_CH_BASE::PsagFilesysDecodeBase {
	protected:
		RawDataStream* RawSourceDataRef = nullptr;
		std::string DecoderOutput = {};
	public:
		void __SystemRawSource(RawDataStream* data);
		bool __SystemRawDecoding();

		std::string GetStringOutput();
	};

	class JsonDecodeChannel :public PSAG_FILESYS_CH_BASE::PsagFilesysDecodeBase {
	protected:
		RawDataStream* RawSourceDataRef = nullptr;
		rapidjson::Document DecodeOutput = {};
	public:
		void __SystemRawSource(RawDataStream* data);
		bool __SystemRawDecoding();

		rapidjson::Document* GetDocumentOutput();
		// non string_data cache, copy_oper.
		rapidjson::Document StringDecodeDocument(const std::string& str_data);
	};
}

namespace EncoderChannels {
	StaticStrLABEL PSAG_ENC_CHANNEL_STRING = "PSAG_FILESYS_ECH_STRING";
	StaticStrLABEL PSAG_ENC_CHANNEL_JSON   = "PSAG_FILESYS_ECH_JSON";

	class StringEncodeChannel :public PSAG_FILESYS_CH_BASE::PsagFilesysEncodeBase {
	protected:
		RawDataStream* RawSourceDataRef = nullptr;
		std::string EncoderInput = {};
	public:
		void __SystemRawSource(RawDataStream* data);
		bool __SystemRawEncoding();

		void StringDataInput(const std::string& data);
	};

	class JsonEncodeChannel :public PSAG_FILESYS_CH_BASE::PsagFilesysEncodeBase {
	protected:
		RawDataStream* RawSourceDataRef = nullptr;
		rapidjson::Document* EncoderInput = nullptr;
	public:
		void __SystemRawSource(RawDataStream* data);
		bool __SystemRawEncoding();

		void SetDocumentInput(rapidjson::Document* data);
	};
}

#endif