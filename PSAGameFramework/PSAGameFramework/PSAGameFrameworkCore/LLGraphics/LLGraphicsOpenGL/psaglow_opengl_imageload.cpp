// psaglow_opengl_imageload.
#include "psaglow_opengl.hpp"

namespace PSAG_OGL_IMG {
	// libray: stb_image,stb_image_write
#include "stb_image.h"
	// "sprintf" => "sprintf_s"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define __STDC_LIB_EXT1__
#include "stb_image_write.h"

	inline bool STB_IMAGE_WRITE_JPG(const std::string& file, const ImageRawData& raw, float qua) {
		return (bool)stbi_write_jpg(file.c_str(), raw.Width, raw.Height, raw.Channels, raw.ImagePixels.data(), int(qua * 100.0f));
	}
	inline bool STB_IMAGE_WRITE_PNG(const std::string& file, const ImageRawData& raw, float qua) {
		return (bool)stbi_write_png(file.c_str(), raw.Width, raw.Height, raw.Channels, raw.ImagePixels.data(), int(qua * 100.0f));
	}
	inline bool STB_IMAGE_WRITE_BMP(const std::string& file, const ImageRawData& raw) {
		return (bool)stbi_write_bmp(file.c_str(), raw.Width, raw.Height, raw.Channels, raw.ImagePixels.data());
	}

	bool PsagIOImageFileSTB::WriteImageFile(const ImageRawData& rawdata, const std::string& file, SaveImageMode mode, float quality) {
		std::filesystem::path SaveFilePath(file);
		std::filesystem::path DirectoryPath = SaveFilePath.parent_path();

		auto ImageWriteLogProcess = [&](bool flag, const char* label, const char* file) {
			// process stbi_write return_flag.
			if (flag) PsagLowLog(LogInfo, PSAG_OGLIMG_LABEL, "success save(%s) image_file: %s", label, file);
			else      PsagLowLog(LogWarning, PSAG_OGLIMG_LABEL, "failed save(%s) image_file: %s", label, file);
		};

		if (!std::filesystem::is_directory(DirectoryPath)) {
			PsagLowLog(LogError, PSAG_OGLIMG_LABEL, "invalid directory, save image_file: %s", DirectoryPath.string().c_str());
			return false;
		}

		switch (mode) {
		case(ImageJPG): {
			bool StatusFlag = STB_IMAGE_WRITE_JPG(file, rawdata, quality);
			ImageWriteLogProcess(StatusFlag, ".jpg", file.c_str());
			return StatusFlag;
		}
		case(ImagePNG): {
			bool StatusFlag = STB_IMAGE_WRITE_PNG(file, rawdata, quality);
			ImageWriteLogProcess(StatusFlag, ".png", file.c_str());
			return StatusFlag;
		}
		case(ImageBMP): {
			bool StatusFlag = STB_IMAGE_WRITE_BMP(file, rawdata);
			ImageWriteLogProcess(StatusFlag, ".bmp", file.c_str());
			return StatusFlag;
		}
		// write: invalid mode.
		default: {
			PsagLowLog(LogError, PSAG_OGLIMG_LABEL, "invalid mode, save image.");
			return false;
		}}
	}

	ImageRawData PsagIOImageFileSTB::ReadImageFile(std::string file) {
		ImageRawData ReturnRawImageData = {};

		if (std::filesystem::exists(file)) {
			// stbi_image loader.
			int Width = NULL, Height = NULL, Channels = NULL;
			uint8_t* RawDataPtr = stbi_load(file.c_str(), &Width, &Height, &Channels, NULL);;

			// stbi_load return nullptr: failed load.
			if (RawDataPtr == nullptr) {
				PsagLowLog(LogError, PSAG_OGLIMG_LABEL, "failed load, raw_dataptr = nullptr.", file.c_str());
				return ReturnRawImageData;
			}

			PsagLowLog(LogInfo, PSAG_OGLIMG_LABEL, "read image: image file: %s         ", file.c_str());
			PsagLowLog(LogInfo, PSAG_OGLIMG_LABEL, "read image: image size: %d x %d    ", Width, Height);
			PsagLowLog(LogInfo, PSAG_OGLIMG_LABEL, "read image: image color_channel: %d", Channels);

			ReturnRawImageData.Channels = Channels;
			ReturnRawImageData.Width = Width;
			ReturnRawImageData.Height = Height;
			// copy raw_image memory.
			ReturnRawImageData.ImagePixels.insert(
				ReturnRawImageData.ImagePixels.begin(),
				RawDataPtr,
				RawDataPtr + (size_t)Width * (size_t)Height * (size_t)Channels
			);
			// free loader memory.
			stbi_image_free(RawDataPtr);
		}
		else
			PsagLowLog(LogError, PSAG_OGLIMG_LABEL, "failed open, read image_file: %s", file.c_str());
		return ReturnRawImageData;
	}

	inline void StbiExtWriteToMemory(void* context, void* data, int size) {
		RawDataStream* OutputBuffer = reinterpret_cast<RawDataStream*>(context);
		uint8_t* ImageData = reinterpret_cast<uint8_t*>(data);
		OutputBuffer->insert(OutputBuffer->end(), ImageData, ImageData + size);
	}

	RawDataStream PsagIOImageRawDataSTB::EncodeImageRawData(const ImageRawData& rawdata, SaveImageMode mode, float quality) {
		RawDataStream ResultRawDataTemp = {};
		// raw_data rotate != mem_data.
		stbi_flip_vertically_on_write(true);

		switch (mode) {
		case(ImageJPG): {
			RawDataStream InputBuffer = rawdata.ImagePixels;
			uint8_t* RawDataBuffer = nullptr;
			int EncodeSize = rawdata.Width * rawdata.Height * rawdata.Channels;

			stbi_write_jpg_to_func(
				&StbiExtWriteToMemory, InputBuffer.data(), rawdata.Width, rawdata.Height, rawdata.Channels,
				&RawDataBuffer, int(quality * 100.0f)
			);
			
			if (RawDataBuffer == nullptr) {
				PsagLowLog(LogError, PSAG_OGLIMG_LABEL, "failed encode jpg, data = nullptr.");
				return ResultRawDataTemp;
			}
			ResultRawDataTemp.resize(EncodeSize);
			// buffer => temp, free buffer.
			ResultRawDataTemp.assign(RawDataBuffer, RawDataBuffer + ResultRawDataTemp.size());
			stbi_image_free(RawDataBuffer);

			PsagLowLog(LogInfo, PSAG_OGLIMG_LABEL, "success encode jpg, size: %d bytes.", EncodeSize);
			return ResultRawDataTemp;
		}
		case(ImagePNG): {
			int EncodeSize = NULL;
			uint8_t* RawDataBuffer = stbi_write_png_to_mem(
				rawdata.ImagePixels.data(),
				rawdata.Width * rawdata.Channels, rawdata.Width, rawdata.Height, rawdata.Channels,
				&EncodeSize
			);

			if (RawDataBuffer == nullptr) {
				PsagLowLog(LogError, PSAG_OGLIMG_LABEL, "failed encode png, data = nullptr.");
				return ResultRawDataTemp;
			}
			ResultRawDataTemp.resize(EncodeSize);
			// buffer => temp, free buffer.
			ResultRawDataTemp.assign(RawDataBuffer, RawDataBuffer + ResultRawDataTemp.size());
			stbi_image_free(RawDataBuffer);

			PsagLowLog(LogInfo, PSAG_OGLIMG_LABEL, "success encode png, size: %d bytes.", EncodeSize);
			return ResultRawDataTemp;
		}
		// write: invalid mode.
		default: {
			PsagLowLog(LogError, PSAG_OGLIMG_LABEL, "invalid mode, encode image.");
			return ResultRawDataTemp;
		}}
	}

	ImageRawData PsagIOImageRawDataSTB::DecodeImageRawData(const RawDataStream& rawdata) {
		ImageRawData ResultRawDataTemp = {};

		int Width = NULL, Height = NULL, Channels = NULL;
		uint8_t* DataPtr = stbi_load_from_memory(rawdata.data(), (int)rawdata.size(), &Width, &Height, &Channels, NULL);

		if (DataPtr == nullptr) {
			PsagLowLog(LogError, PSAG_OGLIMG_LABEL, "failed decode image, data = nullptr.");
			return ResultRawDataTemp;
		}

		ResultRawDataTemp.ImagePixels.resize(Width * Height * Channels);
		ResultRawDataTemp.ImagePixels.assign(DataPtr, DataPtr + ResultRawDataTemp.ImagePixels.size());
		ResultRawDataTemp.Width    = Width;
		ResultRawDataTemp.Height   = Height;
		ResultRawDataTemp.Channels = Channels;

		stbi_image_free(DataPtr);
		PsagLowLog(LogInfo, PSAG_OGLIMG_LABEL, "success decode image: size: %d x %d, color_channel: %d",
			Width, Height, Channels);
		return ResultRawDataTemp;
	}
}