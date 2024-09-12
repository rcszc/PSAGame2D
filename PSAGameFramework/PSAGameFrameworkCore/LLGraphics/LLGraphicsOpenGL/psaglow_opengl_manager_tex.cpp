// psaglow_opengl_manager_tex.
#include "psaglow_opengl.hpp"

namespace PSAG_OGL_MAG {
	// **************************************** Texture ****************************************
	// OpenGL 4.6 texture_cube, stb_image image,stb_image_write loader[.jpg.png]
	// Update: 2024_05_13. RCSZ
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
	StaticStrLABEL PSAG_OGLMAG_TEXTURE = "PSAG_OGL_TEXTURE";

	uint8_t* TextureSystemBase::ImageFileLoader(const std::string& file, int& width, int& height, int& channels) {
		uint8_t* ImageDataPointer = nullptr;

		if (std::filesystem::exists(file)) {
			// stbi_image loader.
			ImageDataPointer = stbi_load(file.c_str(), &width, &height, &channels, NULL);
			PsagLowLog(LogInfo, PSAG_OGLMAG_TEXTURE, "loader: image file: %s", file.c_str());
			return ImageDataPointer;
		}
		PsagLowLog(LogError, PSAG_OGLMAG_TEXTURE, "image_loader failed open file: %s", file.c_str());
		return nullptr;
	}

#define GEN_TEXTURE_COUNT 1
	bool TextureSystemBase::CreateBindTexture(PsagTexture& texture) {
		// opengl generate texture object.
		if (texture == OPENGL_INVALID_HANDEL) {
			// texture_type: 2d_array.
			glGenTextures(GEN_TEXTURE_COUNT, &texture);
			glBindTexture(GL_TEXTURE_2D_ARRAY, texture);

			PsagLowLog(LogInfo, PSAG_OGLMAG_TEXTURE, "texture.2d(array) object create.");
			return DEF_PSAGSTAT_SUCCESS;
		}
		// object duplicate creation.
		PsagLowLog(LogWarning, PSAG_OGLMAG_TEXTURE, "texture.2d(array) duplicate object create.");
		return DEF_PSAGSTAT_FAILED;
	}

	// nearest nighbor filtering.
	inline void FuncNearestNeighborFiltering(GLenum TYPE) {
		glTexParameteri(TYPE, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(TYPE, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	}
	// linear filtering.
	inline void FuncLinearFiltering(GLenum TYPE) {
		glTexParameteri(TYPE, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(TYPE, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}
	// anisotropic filtering.
	inline void FuncAnisotropicFiltering(GLenum TYPE) {
		GLfloat MaxAnisotropy;
		glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &MaxAnisotropy);
		glTexParameterf(TYPE, GL_TEXTURE_MAX_ANISOTROPY_EXT, MaxAnisotropy);
	}
	// generate mipmaps.
	inline void FuncGenerateMipmaps(GLenum TYPE) {
		glGenerateMipmap(TYPE);
	}

	inline void ConfigTextureFilter(TextureFilterMode filtering_flags, GLenum type) {
		// opengl texture3d config.
		if (filtering_flags & NearestNeighborFiltering) FuncNearestNeighborFiltering(type);
		if (filtering_flags & LinearFiltering)          FuncLinearFiltering         (type);
		if (filtering_flags & AnisotropicFiltering)     FuncAnisotropicFiltering    (type);
		if (filtering_flags & MipmapFiltering)          FuncGenerateMipmaps         (type);
	}

	inline void ConfigSurroundTex2DParams(GLenum MODE_TYPE) {
		glTexParameteri(MODE_TYPE, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(MODE_TYPE, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(MODE_TYPE, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(MODE_TYPE, GL_TEXTURE_WRAP_T, GL_REPEAT);
	}

	inline bool ConfigCreateTexture(
		uint32_t layers, uint32_t width, uint32_t height, uint32_t channels, const uint8_t* data, TextureFilterMode mode,
		uint32_t border = NULL, GLenum ModeType = GL_TEXTURE_2D_ARRAY
	) {
		// error texture2d(array) layers = 0.
		if (layers == NULL) return false;
		// error channels != 'DEF_IMG_CHANNEL_RGBA' and 'DEF_IMG_CHANNEL_RGB'
		if (channels > 4 || channels < 3) return false;

		GLenum InputFormat = GL_RGB;
		if (channels == 4) InputFormat = GL_RGBA;

		// opengl api, create texture_array. [GL_TEX_01]
		// internal_format = pixel_format. [20240704]
		// internal_format RGBA12 (HDR12), pixel_format RGBA8888. [20240717]
		if (ModeType == GL_TEXTURE_2D_ARRAY)
			glTexImage3D(ModeType, NULL, GL_RGBA12, width, height, layers, border, InputFormat, GL_UNSIGNED_BYTE, data);
		if (ModeType == GL_TEXTURE_2D)
			glTexImage2D(ModeType, NULL, GL_RGBA12, width, height, border, InputFormat, GL_UNSIGNED_BYTE, data);
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

		// config texture2d. surround,filter.
		ConfigSurroundTex2DParams(ModeType);
		ConfigTextureFilter(mode, ModeType);
		return true;
	}

	inline RawDataStream ConvertRawDataAhpla(const RawDataStream& rgb_data) {
		RawDataStream ConvertTemp = {};
		// rgb_pixel data => fill => rgba_pixel data, step = 3.
		for (size_t i = 0; i < rgb_data.size(); i += 3) {
			ConvertTemp.insert(ConvertTemp.end(), rgb_data.begin() + i, rgb_data.begin() + i + 3);
			ConvertTemp.push_back(0xff);
		}
		return ConvertTemp;
	}

	// **************************************** texture(array) oper_load ****************************************
	// [纹理数组]
	// PSA-V0.1.2 GL-TEX 标准. [GL_TEX_01]: UNISGNED_BYTES [GL_TEX_02]: RGBA8888, [GL_VERT_03]: AhplaChannelsFill.

	bool PsagTextureOGL::SetTextureParam(uint32_t width, uint32_t height, TextureFilterMode mode) {
		ColorTextureCreate.Width          = width;
		ColorTextureCreate.Height         = height;
		ColorTextureCreate.FilterModeType = mode;

		SetTextureAttrFlag = true;
		return DEF_PSAGSTAT_SUCCESS;
	}

	bool PsagTextureOGL::SetTextureSamplerCount(uint32_t count) {
		// device_gpu tmu units number(index).
		ColorTextureCreate.TextureSamplerCount = count;
		return DEF_PSAGSTAT_SUCCESS;
	}

	bool PsagTextureOGL::PsuhCreateTexEmpty(const TextureParam& param) {
		if (param.WidthScale < 0.0f || param.HeightScale < 0.0f) {
			PsagLowLog(LogError, PSAG_OGLMAG_TEXTURE, "texture push_create empty, scale_param < 0.0f");
			return DEF_PSAGSTAT_FAILED;
		}
		CreateDataIndex.push_back(SystemCreateTexture(RawDataStream(), param));
		return DEF_PSAGSTAT_SUCCESS;
	}

	bool PsagTextureOGL::PushCreateTexData(const TextureParam& param, uint32_t channels, const RawDataStream& data) {
		if (param.WidthScale < 0.0f || param.HeightScale < 0.0f) {
			PsagLowLog(LogError, PSAG_OGLMAG_TEXTURE, "texture push_create data, scale_param < 0.0f");
			return DEF_PSAGSTAT_FAILED;
		}
		// texture_load error tex_x * tex_y * channels != in_data bytes.
		if (size_t(ColorTextureCreate.Width * ColorTextureCreate.Height * channels) != data.size() ||
			data.empty()
		) {
			PsagLowLog(LogError, PSAG_OGLMAG_TEXTURE, "texture push_create data, data | channels err.");
			return DEF_PSAGSTAT_FAILED;
		}

		RawDataStream TempRawData = {};
		// channels != rgba => cvt => rgba_data, ahpla: 0xff.
		if (channels == DEF_IMG_CHANNEL_RGB) {
			TempRawData = ConvertRawDataAhpla(data);
			CreateDataIndex.push_back(SystemCreateTexture(TempRawData, param));
		}
		else
			CreateDataIndex.push_back(SystemCreateTexture(data, param));
		return DEF_PSAGSTAT_SUCCESS;
	}

	bool PsagTextureOGL::PushCreateTexLoader(const TextureParam& param, const std::string& file) {
		int Width = NULL, Height = NULL, Channels = NULL;
		uint8_t* TempData = ImageFileLoader(file, Width, Height, Channels);
		// failed stb_image load data.
		if (TempData == nullptr) {
			PsagLowLog(LogError, PSAG_OGLMAG_TEXTURE, "texture push_create loader, data = null.");
			return DEF_PSAGSTAT_FAILED;
		}
		// storage image raw_pixel_data.
		RawDataStream TempRawData = {};
		TempRawData.insert(TempRawData.begin(), TempData, TempData + size_t(Width * Height * Channels));
		// free stb_image load image memory.
		if (TempData != nullptr)
			stbi_image_free(TempData);
		// channels != rgba => cvt => rgba_data, ahpla: 0xff.
		if (Channels == DEF_IMG_CHANNEL_RGB)
			TempRawData = ConvertRawDataAhpla(TempRawData);

		CreateDataIndex.push_back(SystemCreateTexture(TempRawData, param));
		return DEF_PSAGSTAT_SUCCESS;
	}

	bool PsagTextureOGL::CreateTexture() {
		if (!SetTextureAttrFlag) {
			PsagLowLog(LogError, PSAG_OGLMAG_TEXTURE, "texture create, non set_texture param.");
			return DEF_PSAGSTAT_FAILED;
		}
		
		if (CreateBindTexture(ColorTextureCreate.Texture)) {
			// layers = push items.
			ColorTextureCreate.Layers   = (uint32_t)CreateDataIndex.size();
			ColorTextureCreate.Channels = DEF_IMG_CHANNEL_RGBA;

			if (ColorTextureCreate.Layers < 1) {
				PsagLowLog(LogError, PSAG_OGLMAG_TEXTURE, "texture create, param layers < 1.");
				return DEF_PSAGSTAT_FAILED;
			}

			if (!ConfigCreateTexture(
				ColorTextureCreate.Layers, ColorTextureCreate.Width, ColorTextureCreate.Height, ColorTextureCreate.Channels,
				nullptr,
				ColorTextureCreate.FilterModeType
			)) {
				PsagLowLog(LogError, PSAG_OGLMAG_TEXTURE, "texture create, opengl_api err.");
				return DEF_PSAGSTAT_FAILED;
			}

			for (size_t i = 0; i < ColorTextureCreate.Layers; ++i) {
				if (!CreateDataIndex[i].IamgeRawData.empty()) {
					// upload layer texture_data.
					glTexSubImage3D(GL_TEXTURE_2D_ARRAY, NULL,
						0, 0, (GLint)i, ColorTextureCreate.Width, ColorTextureCreate.Height, 1, GL_RGBA, GL_UNSIGNED_BYTE,
						CreateDataIndex[i].IamgeRawData.data()
					);
				}
				// load texture scale_params.
				ColorTextureCreate.LayersParams.push_back(CreateDataIndex[i].Param);
			}
			// unbind texture handle.
			glBindTexture(GL_TEXTURE_2D_ARRAY, NULL);

			PsagLowLog(LogInfo, PSAG_OGLMAG_TEXTURE, "texture create, succ: size: %u x %u, layers: %u.",
				ColorTextureCreate.Width, ColorTextureCreate.Height, ColorTextureCreate.Layers
			);
			// return flag & set resource flag.
			ReturnResourceFlag = DEFRES_FLAG_NORMAL;
			return DEF_PSAGSTAT_SUCCESS;
		}
		PsagLowLog(LogError, PSAG_OGLMAG_TEXTURE, "texture create, opengl_api err.");
		return DEF_PSAGSTAT_FAILED;
	}

	PsagTextureAttribute PsagTextureOGL::_MS_GETRES(ResourceFlag& flag) {
		flag = ReturnResourceFlag;
		return ColorTextureCreate;
	}

	// **************************************** texture2d depth oper_load ****************************************
	// [深度测试纹理]
	
	bool PsagTextureDepthOGL::CreateBindTextureDep(PsagTexture& texture) {
		// opengl generate texture object.
		if (texture == OPENGL_INVALID_HANDEL) {
			// texture_type: 2d.
			glGenTextures(GEN_TEXTURE_COUNT, &texture);
			glBindTexture(GL_TEXTURE_2D, texture);

			PsagLowLog(LogInfo, PSAG_OGLMAG_TEXTURE, "texture.2d(depth) object create.");
			return DEF_PSAGSTAT_SUCCESS;
		}
		// object duplicate creation.
		PsagLowLog(LogWarning, PSAG_OGLMAG_TEXTURE, "texture.2d(depth) duplicate object create.");
		return DEF_PSAGSTAT_FAILED;
	}

	bool PsagTextureDepthOGL::CreateDepthTexture(uint32_t width, uint32_t height, uint32_t sampler_count) {
		if (CreateBindTextureDep(DepthTextureCreate.Texture)) {
			// depth_texture params.
			DepthTextureCreate.TextureSamplerCount = sampler_count;

			DepthTextureCreate.Layers   = 1;
			DepthTextureCreate.Channels = 1;

			DepthTextureCreate.Width  = width;
			DepthTextureCreate.Height = height;

			if (DepthTextureCreate.TextureSamplerCount < 1) {
				PsagLowLog(LogError, PSAG_OGLMAG_TEXTURE, "texture_dep create, sampler_count err.");
				return DEF_PSAGSTAT_FAILED;
			}
			// config texture2d. surround,filter.
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

			glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
			// unbind texture handle.
			glBindTexture(GL_TEXTURE_2D, NULL);

			PsagLowLog(LogInfo, PSAG_OGLMAG_TEXTURE, "texture_dep create, succ: size: %u x %u",
				DepthTextureCreate.Width, DepthTextureCreate.Height
			);
			// return flag & set resource flag.
			ReturnResourceFlag = DEFRES_FLAG_NORMAL;
			return DEF_PSAGSTAT_SUCCESS;
		}
		PsagLowLog(LogError, PSAG_OGLMAG_TEXTURE, "texture_dep create, opengl_api err.");
		return DEF_PSAGSTAT_FAILED;
	}

	PsagTextureAttribute PsagTextureDepthOGL::_MS_GETRES(ResourceFlag& flag) {
		flag = ReturnResourceFlag;
		return DepthTextureCreate;
	}

	// **************************************** texture2d view oper_load ****************************************
	// [视图纹理(in-imgui)]
	// PSA-V0.1.2 GL-TEX-ATTR 标准. [GL_TEX_ATTR_02].

	bool PsagTextureViewOGL::CreateBindTextureView(PsagTexture& texture) {
		// opengl generate texture object.
		if (texture == OPENGL_INVALID_HANDEL) {
			// texture_type: 2d.
			glGenTextures(GEN_TEXTURE_COUNT, &texture);
			glBindTexture(GL_TEXTURE_2D, texture);

			PsagLowLog(LogInfo, PSAG_OGLMAG_TEXTURE, "texture.2d(view) object create.");
			return DEF_PSAGSTAT_SUCCESS;
		}
		// object duplicate creation.
		PsagLowLog(LogWarning, PSAG_OGLMAG_TEXTURE, "texture.2d(view) duplicate object create.");
		return DEF_PSAGSTAT_FAILED;
	}

	bool PsagTextureViewOGL::CreateTexViewEmpty(uint32_t width, uint32_t height, TextureFilterMode mode) {
		if (CreateBindTextureView(TextureViewCreate.Texture)) {
			// create texture2d view.
			if (!ConfigCreateTexture(1, width, height, DEF_IMG_CHANNEL_RGBA, nullptr, mode, NULL, GL_TEXTURE_2D)) {
				PsagLowLog(LogError, PSAG_OGLMAG_TEXTURE, "texture_view create, opengl_api err.");
				return DEF_PSAGSTAT_FAILED;
			}
			TextureViewCreate.Width    = width;
			TextureViewCreate.Height   = height;
			TextureViewCreate.Channels = DEF_IMG_CHANNEL_RGBA;

			PsagLowLog(LogInfo, PSAG_OGLMAG_TEXTURE, "texture_view create, succ: size: %u x %u", width, height);
			return DEF_PSAGSTAT_SUCCESS;
		}
		return DEF_PSAGSTAT_FAILED;
	}

	bool PsagTextureViewOGL::CreateTexViewData(const ImageRawData& image_data, TextureFilterMode mode) {
		if (CreateBindTextureView(TextureViewCreate.Texture)) {
			// check image pixel_data.
			if (image_data.ImagePixels.empty()) {
				PsagLowLog(LogError, PSAG_OGLMAG_TEXTURE, "texture_view create, image data_empty.");
				return DEF_PSAGSTAT_FAILED;
			}
			// create texture2d view.
			if (!ConfigCreateTexture(
				1, image_data.Width, image_data.Height, image_data.Channels, image_data.ImagePixels.data(), mode, 
				NULL, GL_TEXTURE_2D
			)) {
				PsagLowLog(LogError, PSAG_OGLMAG_TEXTURE, "texture_view create, opengl_api err.");
				return DEF_PSAGSTAT_FAILED;
			}
			TextureViewCreate.Width    = image_data.Width;
			TextureViewCreate.Height   = image_data.Height;
			TextureViewCreate.Channels = image_data.Channels;

			PsagLowLog(LogInfo, PSAG_OGLMAG_TEXTURE, "texture_view create, succ: size: %u x %u", image_data.Width, image_data.Height);
			return DEF_PSAGSTAT_SUCCESS;
		}
		return DEF_PSAGSTAT_FAILED;
	}

	PsagTextureView PsagTextureViewOGL::CreateTexture() {
		return TextureViewCreate;
	}
}