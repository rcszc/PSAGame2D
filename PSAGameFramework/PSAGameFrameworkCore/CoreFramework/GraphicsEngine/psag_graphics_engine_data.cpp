// psag_graphics_engine_data.
#include "psag_graphics_engine.h"

using namespace std;
using namespace PSAG_LOGGER;

namespace IMAGE_TOOLS {
	// fmt image size. (ZERO_FILL)
	void IMAGE_TOOL_FILL(
		vector<uint8_t>& image, const Vector2T<uint32_t>& src_size, const Vector2T<uint32_t>& fill_size, uint32_t channels
	) {
		if (src_size.vector_x == fill_size.vector_x && src_size.vector_y == fill_size.vector_y)
			return;
		vector<uint8_t> FillImageTemp(fill_size.vector_x * fill_size.vector_y * channels, 0x00);
		// image raw_pixel fill.
		for (uint32_t y = 0; y < src_size.vector_y; ++y)
			for (uint32_t x = 0; x < src_size.vector_x; ++x)
				for (uint32_t c = 0; c < channels; ++c)
					FillImageTemp[(y * fill_size.vector_x + x) * channels + c] = image[(y * src_size.vector_x + x) * channels + c];
		// src => fill(dst).
		image = move(FillImageTemp);
	}

	// fmt image channels(rgb) => channels(rgba).
	void IMAGE_TOOL_CHANNELS(vector<uint8_t>& image) {
		RawDataStream ConvertChannelsTemp = {};
		// rgb_pixel data => fill => rgba_pixel data, step = 3.
		for (size_t i = 0; i < image.size(); i += 3) {
			ConvertChannelsTemp.insert(ConvertChannelsTemp.end(), image.begin() + i, image.begin() + i + 3);
			ConvertChannelsTemp.push_back(0xff);
		}
		image = move(ConvertChannelsTemp);
	}
}

namespace GraphicsEngineDataset {
	// GLOBAL STATE: 静态顶点组(大小), 动态顶点组(大小), 在线虚拟贴图数量(已加载帖图).
	atomic<size_t> GLEngineDataSTATE::DataBytesStaticVertex  = NULL;
	atomic<size_t> GLEngineDataSTATE::DataBytesDynamicVertex = NULL;
	atomic<size_t> GLEngineDataSTATE::DataBytesOnlineTexture = NULL;

	PsagLow::PsagSupGraphicsOper::PsagRender::PsagOpenGLApiRenderOper
		GLEngineStcVertexData::ShaderRender = {};

	ResUnique GLEngineStcVertexData::VertexAttribute = {};
	ResUnique GLEngineStcVertexData::VertexBuffer = {};

	unordered_map<ResUnique, VABO_DATASET_INFO> GLEngineStcVertexData::IndexItems = {};
	ResUnique GLEngineStcVertexData::SystemPresetRectangle = {};

	mutex GLEngineStcVertexData::DatasetResMutex = {};

	bool GLEngineStcVertexData::VerStcDataItemAlloc(ResUnique rukey, const vector<float>& data) {
		VABO_DATASET_INFO AllocVertInfo = {};

		size_t DatasetSizeTemp = NULL;
		// enable [alloc] TCS, TCS: 线程临界区.
		lock_guard<mutex> Lock(DatasetResMutex);

		auto it = IndexItems.find(rukey);
		if (it != IndexItems.end()) {
			PushLogger(LogWarning, PSAGM_GLENGINE_DATA_LABEL, "ver_data(stc): failed alloc duplicate_key: %u", rukey);
			return false;
		}
		auto VertexAttrib = LLRES_VertexBuffers->ExtResourceMapping(VertexBuffer);
		// 映射GPU显存 => 重写数据 => 上传GPU显存.
		vector<float> DatasetTemp = ShaderRender.ReadVertexDatasetFP32(VertexAttrib->DataBuffer);

		AllocVertInfo.DatasetOffsetLength = DatasetTemp.size();
		AllocVertInfo.DatasetLength       = data.size();
		// push_back add dataset.
		DatasetTemp.insert(DatasetTemp.begin(), data.begin(), data.end());
		// cpu =upload=> gpu memory.
		ShaderRender.UploadVertexDataset(VertexAttrib, DatasetTemp.data(), DatasetTemp.size() * sizeof(float), GL_STATIC_DRAW);
		// update temp,global_state.
		DatasetSizeTemp       = DatasetTemp.size();
		DataBytesStaticVertex = DatasetTemp.size();

		IndexItems[rukey] = AllocVertInfo;

		PushLogger(LogInfo, PSAGM_GLENGINE_DATA_LABEL, "ver_data(stc) upload gpu_data: %u bytes", DatasetSizeTemp);
		PushLogger(LogInfo, PSAGM_GLENGINE_DATA_LABEL, "ver_data(stc) item: alloc key: %u", rukey);
		return true;
	}

	bool GLEngineStcVertexData::VerStcDataItemFree(ResUnique rukey) {
		{ // enable [free] TCS.
			lock_guard<mutex> Lock(DatasetResMutex);

			auto it = IndexItems.find(rukey);
			if (it != IndexItems.end()) {
				IndexItems.erase(it); // delete info_item.
				auto VertexAttrib = LLRES_VertexBuffers->ExtResourceMapping(VertexBuffer);
				// 映射GPU显存 => 重写信息 => 重写数据 => 上传GPU显存.
				vector<float> DatasetTemp = ShaderRender.ReadVertexDatasetFP32(VertexAttrib->DataBuffer);
				vector<float> DatasetRewriting = {};

				size_t DataPointerCount = NULL;
				for (auto& STCIT : IndexItems) {
					// 重写信息(属性) & 数据.
					size_t PreviousCount = DatasetRewriting.size();
					DataPointerCount += STCIT.second.DatasetLength;

					DatasetRewriting.insert(
						DatasetRewriting.begin(),
						DatasetTemp.data() + STCIT.second.DatasetOffsetLength,
						DatasetTemp.data() + STCIT.second.DatasetOffsetLength + STCIT.second.DatasetLength
					);
					STCIT.second.DatasetOffsetLength = PreviousCount;
					STCIT.second.DatasetLength = DataPointerCount;
				}
				// cpu =upload=> gpu memory.
				ShaderRender.UploadVertexDataset(VertexAttrib, DatasetRewriting.data(), DatasetRewriting.size() * sizeof(float), GL_STATIC_DRAW);
				// update global_state: stc.
				DataBytesStaticVertex = DatasetTemp.size();

				PushLogger(LogInfo, PSAGM_GLENGINE_DATA_LABEL, "ver_data(stc) upload gpu_data: %u bytes", DatasetRewriting.size());
				PushLogger(LogInfo, PSAGM_GLENGINE_DATA_LABEL, "ver_data(stc) item: delete key: %u", rukey);
				return true;
			}
		}
		PushLogger(LogWarning, PSAGM_GLENGINE_DATA_LABEL, "ver_data(stc) item: failed delete, not found key.");
		return false;
	}

	bool GLEngineStcVertexData::VerStcOperFrameDraw(ResUnique rukey) {
		// enable [draw_cmd] TCS.
		lock_guard<mutex> Lock(DatasetResMutex);

		if (IndexItems.find(rukey) == IndexItems.end())
			return false;

		// draw vertex: length,offset.
		ShaderRender.DrawVertexGroupSeg(
			LLRES_VertexBuffers->ResourceFind(VertexBuffer),
			IndexItems[rukey].DatasetLength / (FS_VERTEX_LENGTH), IndexItems[rukey].DatasetOffsetLength / (FS_VERTEX_LENGTH)
		);
		return true;
	}

	void GLEngineStcVertexData::StaticVertexDataObjectCreate() {
		PSAG_SYSGEN_TIME_KEY GenResourceID;
		PsagLow::PsagSupGraphicsOper::PsagGraphicsModel VertexProcess;

		auto VerBufferTemp = VertexProcess.CreateVertexBuffer();
		// fixed vertex attribute object. [stc]
		if (LLRES_VertexAttributes->ResourceFind(VertexAttribute) == OPENGL_INVALID_HANDEL) {
			VertexAttribute = GenResourceID.PsagGenTimeKey();
			LLRES_VertexAttributes->ResourceStorage(VertexAttribute, VertexProcess.CreateVertexAttribute(0, 0));
		}

		// create rendering rect.
		if (VertexProcess.CreateStaticModel(LLRES_VertexAttributes->ResourceFind(VertexAttribute), VerBufferTemp, nullptr, NULL)) {
			VertexBuffer = GenResourceID.PsagGenTimeKey();
			LLRES_VertexBuffers->ResourceStorage(VertexBuffer, &VertexProcess);
		}

		// alloc system preset.
		SystemPresetRectangle = GenResourceID.PsagGenTimeKey();
		vector<float> DataTemp = {};
		DataTemp = PSAG_OGL_MAG::ShaderTemplateRectDep(0.0f);
		VerStcDataItemAlloc(SystemPresetRectangle, DataTemp);

		PushLogger(LogTrace, PSAGM_GLENGINE_DATA_LABEL, "static vertex data manager_create.");
	}

	void GLEngineStcVertexData::StaticVertexDataObjectDelete() {
		// opengl free vao,vbo handle.
		LLRES_VertexAttributes->ResourceDelete(VertexAttribute);
		LLRES_VertexBuffers->ResourceDelete(VertexBuffer);
	}

	PsagLow::PsagSupGraphicsOper::PsagRender::PsagOpenGLApiRenderOper
		GLEngineDyVertexData::ShaderRender = {};

	ResUnique GLEngineDyVertexData::VertexAttribute = {};
	ResUnique GLEngineDyVertexData::VertexBuffer    = {};

	unordered_map<ResUnique, VABO_DATASET_INFO> GLEngineDyVertexData::IndexItems = {};
	vector<float> GLEngineDyVertexData::VertexRawDataset = {};

	bool GLEngineDyVertexData::GLOBAL_UPDATE_FLAG = false;

	mutex GLEngineDyVertexData::DatasetResMutex = {};

	bool GLEngineDyVertexData::VerDyDataItemAlloc(ResUnique rukey) {
		// enable [alloc] TCS.
		lock_guard<mutex> Lock(DatasetResMutex);

		auto it = IndexItems.find(rukey);
		if (it != IndexItems.end()) {
			PushLogger(LogWarning, PSAGM_GLENGINE_DATA_LABEL, "ver_data(dy): failed alloc duplicate_key: %u", rukey);
			return false;
		}
		IndexItems[rukey] = VABO_DATASET_INFO();
		PushLogger(LogInfo, PSAGM_GLENGINE_DATA_LABEL, "ver_data(dy) item: alloc key: %u", rukey);
		return true;
	}

	bool GLEngineDyVertexData::VerDyDataItemFree(ResUnique rukey) {
		// enable [free] TCS.
		lock_guard<mutex> Lock(DatasetResMutex);

		auto MapIt = IndexItems.find(rukey);
		if (MapIt == IndexItems.end())
			return false;

		// erase data.
		const auto EraseParam = &MapIt->second;
		VertexRawDataset.erase(
			VertexRawDataset.begin() + EraseParam->DatasetOffsetLength,
			VertexRawDataset.begin() + EraseParam->DatasetOffsetLength + EraseParam->DatasetLength
		);
		IndexItems.erase(MapIt);
		PushLogger(LogInfo, PSAGM_GLENGINE_DATA_LABEL, "ver_data(dy) item: delete key: %u", rukey);

		PushLogger(LogWarning, PSAGM_GLENGINE_DATA_LABEL, "ver_data(dy) item: failed delete, not found key.");
		return true;
	}

	bool GLEngineDyVertexData::VerDyOperFramePushData(ResUnique rukey, const vector<float>& data) {
		// enable [free] TCS.
		lock_guard<mutex> Lock(DatasetResMutex);

		if (IndexItems.find(rukey) == IndexItems.end())
			return false;

		// offset => length => push back data.
		IndexItems[rukey].DatasetOffsetLength = VertexRawDataset.size();
		IndexItems[rukey].DatasetLength = data.size();
		VertexRawDataset.insert(VertexRawDataset.end(), data.data(), data.data() + data.size());

		// data => update_gpu flag: [待更新状态].
		GLOBAL_UPDATE_FLAG = true;
		return true;
	}

	bool GLEngineDyVertexData::VerDyOperFrameDraw(ResUnique rukey) {
		// enable [draw] TCS.
		lock_guard<mutex> Lock(DatasetResMutex);

		if (IndexItems.find(rukey) == IndexItems.end())
			return false;

		// vertex dataset [待更新] => update_dataset.
		if (GLOBAL_UPDATE_FLAG) UpdateVertexDyDataset();
		// update_data => gpu draw_command.
		// draw vertex: length,offset.
		ShaderRender.DrawVertexGroupSeg(
			LLRES_VertexBuffers->ResourceFind(VertexBuffer),
			IndexItems[rukey].DatasetLength / (FS_VERTEX_LENGTH), IndexItems[rukey].DatasetOffsetLength / (FS_VERTEX_LENGTH)
		);
		return true;
	}

	void GLEngineDyVertexData::SystemFrameUpdateNewState() {
		// enable [new_state] TCS.
		lock_guard<mutex> Lock(DatasetResMutex);

		// clear dataset => clear data_info.
		VertexRawDataset.clear();
		for (auto it = IndexItems.begin(); it != IndexItems.end(); ++it)
			it->second = VABO_DATASET_INFO();
	}

	void GLEngineDyVertexData::UpdateVertexDyDataset() {
		// vertex buffer update_dataset, cpu => gpu.
		ShaderRender.UploadVertexDataset(
			LLRES_VertexBuffers->ExtResourceMapping(VertexBuffer), VertexRawDataset.data(), VertexRawDataset.size() * sizeof(float)
		);
		// update global_state: dy.
		DataBytesDynamicVertex = VertexRawDataset.size();
		// set_flag: 已更新状态.
		GLOBAL_UPDATE_FLAG = false;
	}

	void GLEngineDyVertexData::VertexDataObjectCreate() {
		PSAG_SYSGEN_TIME_KEY GenResourceID;
		PsagLow::PsagSupGraphicsOper::PsagGraphicsModel VertexProcess;

		auto VerBufferTemp = VertexProcess.CreateVertexBuffer();
		// fixed vertex attribute object. [dy]
		if (LLRES_VertexAttributes->ResourceFind(VertexAttribute) == OPENGL_INVALID_HANDEL) {
			VertexAttribute = GenResourceID.PsagGenTimeKey();
			LLRES_VertexAttributes->ResourceStorage(VertexAttribute, VertexProcess.CreateVertexAttribute(0, 0));
		}

		if (VertexProcess.CreateDynamicModel(LLRES_VertexAttributes->ResourceFind(VertexAttribute), VerBufferTemp, nullptr, NULL)) {
			VertexBuffer = GenResourceID.PsagGenTimeKey();
			LLRES_VertexBuffers->ResourceStorage(VertexBuffer, &VertexProcess);
		}
		PushLogger(LogTrace, PSAGM_GLENGINE_DATA_LABEL, "dynamic vertex data manager_create.");
	}

	void GLEngineDyVertexData::VertexDataObjectDelete() {
		// opengl free vao,vbo handle.
		LLRES_VertexAttributes->ResourceDelete(VertexAttribute);
		LLRES_VertexBuffers->ResourceDelete(VertexBuffer);
	}

	namespace TextureLayerAlloc {
		// find array vacancy index.
		inline bool FindArrayStateflag(std::vector<bool>& data, uint32_t& sampler_count) {
			bool IDLEresources = false;
			// texture mapping units switch_flag.
			for (size_t i = 0; i < data.size(); ++i) {
				if (data[i] == false) {
					data[i] = true;
					IDLEresources = true;
					sampler_count = (uint32_t)i;
					break;
				}
			}
			return IDLEresources;
		}

		// layers => alloc => layer_count.
		uint32_t TEX_LAYER_ALLOC::AllocLayerCount() {
			uint32_t ReturnTmuCount = NULL;
			bool FindIdelFlag = FindArrayStateflag(TexArrayStateFlag, ReturnTmuCount);

			if (!FindIdelFlag) {
				PushLogger(LogWarning, PSAGM_GLENGINE_DATA_LABEL, "tex_layer_sys failed alloc_count idle = 0.");
				return NULL;
			}
			PushLogger(LogInfo, PSAGM_GLENGINE_DATA_LABEL, "tex_layer_sys alloc_count: %u", ReturnTmuCount);
			return ReturnTmuCount;
		}

		// layer_count => free => layers.
		void TEX_LAYER_ALLOC::FreeLayerCount(uint32_t count) {
			if (count >= TexArrayStateFlag.size())
				PushLogger(LogError, PSAGM_GLENGINE_DATA_LABEL, "tex_layer_sys failed delete_count(space) fc >= max.");
			else {
				TexArrayStateFlag[(size_t)count] = false;
				PushLogger(LogInfo, PSAGM_GLENGINE_DATA_LABEL, "tex_layer_sys delete_count(space): %u", count);
			}
		}
	}

	PsagLow::PsagSupGraphicsOper::PsagRender::PsagOpenGLApiRenderOper GLEngineSmpTextureData::ShaderRender = {};
	PsagLow::PsagSupGraphicsOper::PsagGraphicsUniform GLEngineSmpTextureData::ShaderUniform = {};

	SamplerTextures GLEngineSmpTextureData::TexturesSize1X = {};
	SamplerTextures GLEngineSmpTextureData::TexturesSize2X = {};
	SamplerTextures GLEngineSmpTextureData::TexturesSize4X = {};
	SamplerTextures GLEngineSmpTextureData::TexturesSize8X = {};

	unordered_map<ResUnique, VirTextureParam> GLEngineSmpTextureData::TexIndexItems = {};

	uint32_t GLEngineSmpTextureData::CheckResolutionType(const Vector2T<uint32_t>& size) {
		uint32_t ResolutionType = NULL;
		auto CheckSizeFunc = [&](const Vector2T<uint32_t>& size, const Vector2T<uint32_t>& base) {
			return bool(size.vector_x <= base.vector_x && size.vector_y <= base.vector_y);
		};
		// type optimum resolution.
		if (CheckSizeFunc(size, TexturesSize8X.TextureResolution)) ResolutionType = 1;
		if (CheckSizeFunc(size, TexturesSize4X.TextureResolution)) ResolutionType = 2;
		if (CheckSizeFunc(size, TexturesSize2X.TextureResolution)) ResolutionType = 3;
		if (CheckSizeFunc(size, TexturesSize1X.TextureResolution)) ResolutionType = 4;
		return ResolutionType;
	}

	VirTextureParam* GLEngineSmpTextureData::FindTexIndexItems(ResUnique rukey) {
		// unqiue number != 0(null).
		if (rukey == NULL) return nullptr;
		return (TexIndexItems.find(rukey) != TexIndexItems.end()) ? &TexIndexItems[rukey] : nullptr;
	}

	bool GLEngineSmpTextureData::VirTextureItemAlloc(ResUnique rukey, const ImageRawData& image) {
		SamplerTextures* TextureIndex = nullptr;
		const char* TexResLabel = "NULL";
		
		uint32_t SizeType = CheckResolutionType(Vector2T<uint32_t>(image.Width, image.Height));
		switch (SizeType) {
		case(1): { TexResLabel = "1x/1"; TextureIndex = &TexturesSize8X; break; }
		case(2): { TexResLabel = "2x/1"; TextureIndex = &TexturesSize4X; break; }
		case(3): { TexResLabel = "4x/1"; TextureIndex = &TexturesSize2X; break; }
		case(4): { TexResLabel = "8x/1"; TextureIndex = &TexturesSize1X; break; }
		}
		if (SizeType == NULL || TextureIndex == nullptr) {
			PushLogger(LogError, PSAGM_GLENGINE_DATA_LABEL, "vir_texture item: not_size.");
			return false;
		}

		auto it = TexIndexItems.find(rukey);
		if (it != TexIndexItems.end()) {
			PushLogger(LogWarning, PSAGM_GLENGINE_DATA_LABEL, "vir_texture: failed alloc duplicate_key: %u", rukey);
			return false;
		}
		// 纹理渲染裁剪.
		Vector2T<float> SmpCropping = Vector2T<float>(
			(float)image.Width  / (float)TextureIndex->TextureResolution.vector_x,
			(float)image.Height / (float)TextureIndex->TextureResolution.vector_y
		);

		// err: texture count >= max.
		if (TextureIndex->ArraySize.vector_y >= TextureIndex->ArraySize.vector_x) {
			PushLogger(LogWarning, PSAGM_GLENGINE_DATA_LABEL, "vir_texture: failed alloc items_max.");
			return false;
		}
		// texture count + 1.
		++TextureIndex->ArraySize.vector_y;

		ImageRawData ImgDataTemp = image;
		// alloc virtual texture layer.
		uint32_t VirTextureLayer = TextureIndex->LayerAllotter->AllocLayerCount();

		if (!ImgDataTemp.ImagePixels.empty()) {
			// pixels data process.
			// 格式化填充纹理数据. src => fmt_size.
			IMAGE_TOOLS::IMAGE_TOOL_FILL(
				ImgDataTemp.ImagePixels,
				Vector2T<uint32_t>(ImgDataTemp.Width, ImgDataTemp.Height),
				TextureIndex->TextureResolution,
				ImgDataTemp.Channels
			);
			// AHPLA 通道填充.
			if (ImgDataTemp.Channels == DEF_IMG_CHANNEL_RGB)
				IMAGE_TOOLS::IMAGE_TOOL_CHANNELS(ImgDataTemp.ImagePixels);

			PushLogger(LogInfo, PSAGM_GLENGINE_DATA_LABEL, "vir_texture: src: %u x %u, fmt_fill: %u x %u",
				ImgDataTemp.Width, ImgDataTemp.Height, TextureIndex->TextureResolution.vector_x, TextureIndex->TextureResolution.vector_y
			);
			// upload texture layer_data, rgba(default).
			ShaderRender.UploadTextureLayer(
				LLRES_Textures->ResourceFind(TextureIndex->TextureArrayIndex).Texture,
				VirTextureLayer,
				TextureIndex->TextureResolution,
				ImgDataTemp.ImagePixels.data()
			);
		}

		// set_param => storage.
		TexIndexItems[rukey] = VirTextureParam(
			SizeType,
			TextureIndex->TextureArrayIndex,
			VirTextureLayer,
			SmpCropping,
			Vector2T<float>((float)image.Width, (float)image.Height)
		);

		PushLogger(LogInfo, PSAGM_GLENGINE_DATA_LABEL, "vir_texture item: alloc key: %u, cropping: %.2f x %.2f, res: %s",
			rukey, SmpCropping.vector_x, SmpCropping.vector_y, TexResLabel
		);
		// add texture count.
		++DataBytesOnlineTexture;
		return true;
	}

	bool GLEngineSmpTextureData::VirTextureItemAllocEmpty(ResUnique rukey, const Vector2T<uint32_t>& size) {
		ImageRawData EmptyImageTemp = {};
		EmptyImageTemp.ImagePixels = {};
		// set virtual resolution.
		EmptyImageTemp.Width = size.vector_x; EmptyImageTemp.Height = size.vector_y;
		EmptyImageTemp.Channels = DEF_IMG_CHANNEL_RGBA;
		// alloc.
		if (!VirTextureItemAlloc(rukey, EmptyImageTemp))
			return false;
		// add texture count.
		++DataBytesOnlineTexture;
		return true;
	}

	bool GLEngineSmpTextureData::VirTextureItemFree(ResUnique rukey) {
		SamplerTextures* TextureIndex = nullptr;
		
		VirTextureParam* TexParam = FindTexIndexItems(rukey);
		if (TexParam == nullptr) {
			PushLogger(LogError, PSAGM_GLENGINE_DATA_LABEL, "vir_texture item: failed delete, not found key.");
			return false;
		}
		switch (TexParam->ResolutionType) {
		case(1): { TextureIndex = &TexturesSize8X; break; }
		case(2): { TextureIndex = &TexturesSize4X; break; }
		case(3): { TextureIndex = &TexturesSize2X; break; }
		case(4): { TextureIndex = &TexturesSize1X; break; }
		}
		if (TextureIndex == nullptr)
			return false;

		auto it = TexIndexItems.find(rukey);
		if (it != TexIndexItems.end()) {
			// erase item param.
			it->second = VirTextureParam();
			TexIndexItems.erase(it);
			// free virtual texture layer.
			TextureIndex->LayerAllotter->FreeLayerCount(it->second.SampleLayers);
			// texture count - 1.
			--TextureIndex->ArraySize.vector_y;

			PushLogger(LogInfo, PSAGM_GLENGINE_DATA_LABEL, "vir_texture item: delete key: %u", rukey);
			// sub texture count;
			--DataBytesOnlineTexture;
			return true;
		}
		return false;
	}

	bool GLEngineSmpTextureData::VirTextureExist(ResUnique rukey) {
		return FindTexIndexItems(rukey) == nullptr ? false : true;
	}

	bool GLEngineSmpTextureData::VirTextureItemDraw(ResUnique rukey, PsagShader shader, const VirTextureUniformName& uniform_name) {
		// find virtual texture item_idx.
		auto TexItemTemp = FindTexIndexItems(rukey);
		if (TexItemTemp != nullptr) {
			// find tex => bind context => sampler => uniform(s).
			auto TexResourceTemp = LLRES_Textures->ResourceFind(TexItemTemp->Texture);
#if PSAG_DEBUG_MODE
			if (TexResourceTemp.Texture == OPENGL_INVALID_HANDEL)
				return false;
#endif
			ShaderRender.RenderBindTexture(TexResourceTemp);

			// bind texture context => sampler(tmu) count.
			ShaderUniform.UniformInteger(shader, uniform_name.TexParamSampler, TexResourceTemp.TextureSamplerCount);
			// shader uniform params.
			ShaderUniform.UniformInteger(shader, uniform_name.TexParamLayer,    (int32_t)TexItemTemp->SampleLayers);
			ShaderUniform.UniformVec2   (shader, uniform_name.TexParamCropping, TexItemTemp->SampleCropping);
			ShaderUniform.UniformVec2   (shader, uniform_name.TexParamSize,     TexItemTemp->SampleSize);
			return true;
		}
		return false;
	}

	bool GLEngineSmpTextureData::VirTextureItemIndex(ResUnique rukey, PsagTexture& texture, uint32_t layer_index) {
		// find virtual texture item_idx.
		auto TexItemTemp = FindTexIndexItems(rukey);
		if (TexItemTemp != nullptr) {
			auto TexResourceTemp = LLRES_Textures->ResourceFind(TexItemTemp->Texture);
#if PSAG_DEBUG_MODE
			if (TexResourceTemp.Texture == OPENGL_INVALID_HANDEL)
				return false;
#endif
			// texture handle & layer index.
			texture     = TexResourceTemp.Texture;
			layer_index = TexItemTemp->SampleLayers;
			return true;
		}
		return false;
	}

	void GLEngineSmpTextureData::VirtualTextureDataObjectCreate(Vector2T<uint32_t> base_size, const VirTexturesGenParams& params) {
		if (base_size.vector_x < 512 || base_size.vector_y < 512)
			PushLogger(LogWarning, PSAGM_GLENGINE_DATA_LABEL, "vir_create texture, base_size > 512.");

		base_size.vector_x = base_size.vector_x < 512 ? 512 : base_size.vector_x;
		base_size.vector_y = base_size.vector_y < 512 ? 512 : base_size.vector_y;

		TexturesSize8X.TextureResolution = base_size;
		TexturesSize4X.TextureResolution = Vector2T<uint32_t>(base_size.vector_x / 2, base_size.vector_y / 2);
		TexturesSize2X.TextureResolution = Vector2T<uint32_t>(base_size.vector_x / 4, base_size.vector_y / 4);
		TexturesSize1X.TextureResolution = Vector2T<uint32_t>(base_size.vector_x / 8, base_size.vector_y / 8);

		TexturesSize8X.ArraySize.vector_x = (uint32_t)params.Tex8Xnum;
		TexturesSize4X.ArraySize.vector_x = (uint32_t)params.Tex4Xnum;
		TexturesSize2X.ArraySize.vector_x = (uint32_t)params.Tex2Xnum;
		TexturesSize1X.ArraySize.vector_x = (uint32_t)params.Tex1Xnum;

		// 创建纹理数组(层)分配器.
		TexturesSize8X.LayerAllotter = new TextureLayerAlloc::TEX_LAYER_ALLOC(params.Tex8Xnum);
		TexturesSize4X.LayerAllotter = new TextureLayerAlloc::TEX_LAYER_ALLOC(params.Tex4Xnum);
		TexturesSize2X.LayerAllotter = new TextureLayerAlloc::TEX_LAYER_ALLOC(params.Tex2Xnum);
		TexturesSize1X.LayerAllotter = new TextureLayerAlloc::TEX_LAYER_ALLOC(params.Tex1Xnum);

		PsagLow::PsagSupGraphicsOper::PsagGraphicsTexture TextureCreate8X;
		PsagLow::PsagSupGraphicsOper::PsagGraphicsTexture TextureCreate4X;
		PsagLow::PsagSupGraphicsOper::PsagGraphicsTexture TextureCreate2X;
		PsagLow::PsagSupGraphicsOper::PsagGraphicsTexture TextureCreate1X;

		// global textures filter mode.
		TextureFilterMode TexMode = LinearFiltering | MipmapFiltering;
		TextureCreate8X.SetTextureParam(TexturesSize8X.TextureResolution.vector_x, TexturesSize8X.TextureResolution.vector_y, TexMode);
		TextureCreate4X.SetTextureParam(TexturesSize4X.TextureResolution.vector_x, TexturesSize4X.TextureResolution.vector_y, TexMode);
		TextureCreate2X.SetTextureParam(TexturesSize2X.TextureResolution.vector_x, TexturesSize2X.TextureResolution.vector_y, TexMode);
		TextureCreate1X.SetTextureParam(TexturesSize1X.TextureResolution.vector_x, TexturesSize1X.TextureResolution.vector_y, TexMode);

		// create textures.
		for (size_t i = 0; i < params.Tex8Xnum; ++i) 
			TextureCreate8X.PsuhCreateTexEmpty(TextureParam(1.0f, 1.0f));
		for (size_t i = 0; i < params.Tex4Xnum; ++i)
			TextureCreate4X.PsuhCreateTexEmpty(TextureParam(1.0f, 1.0f));
		for (size_t i = 0; i < params.Tex2Xnum; ++i)
			TextureCreate2X.PsuhCreateTexEmpty(TextureParam(1.0f, 1.0f));
		for (size_t i = 0; i < params.Tex1Xnum; ++i) 
			TextureCreate1X.PsuhCreateTexEmpty(TextureParam(1.0f, 1.0f));

		TextureCreate8X.SetTextureSamplerCount(LLRES_Samplers->AllocTmuCount());
		TextureCreate4X.SetTextureSamplerCount(LLRES_Samplers->AllocTmuCount());
		TextureCreate2X.SetTextureSamplerCount(LLRES_Samplers->AllocTmuCount());
		TextureCreate1X.SetTextureSamplerCount(LLRES_Samplers->AllocTmuCount());

		bool CreateErrorFlag = false;
		CreateErrorFlag |= !TextureCreate8X.CreateTexture();
		CreateErrorFlag |= !TextureCreate4X.CreateTexture();
		CreateErrorFlag |= !TextureCreate2X.CreateTexture();
		CreateErrorFlag |= !TextureCreate1X.CreateTexture();

		// generate unique_id.
		PSAG_SYSGEN_TIME_KEY GenResourceID;
		if (!CreateErrorFlag) {
			// create key => storage res.
			TexturesSize8X.TextureArrayIndex = GenResourceID.PsagGenTimeKey();
			LLRES_Textures->ResourceStorage(TexturesSize8X.TextureArrayIndex, &TextureCreate8X);

			TexturesSize4X.TextureArrayIndex = GenResourceID.PsagGenTimeKey();
			LLRES_Textures->ResourceStorage(TexturesSize4X.TextureArrayIndex, &TextureCreate4X);

			TexturesSize2X.TextureArrayIndex = GenResourceID.PsagGenTimeKey();
			LLRES_Textures->ResourceStorage(TexturesSize2X.TextureArrayIndex, &TextureCreate2X);

			TexturesSize1X.TextureArrayIndex = GenResourceID.PsagGenTimeKey();
			LLRES_Textures->ResourceStorage(TexturesSize1X.TextureArrayIndex, &TextureCreate1X);
		}

		if (CreateErrorFlag)
			PushLogger(LogError, PSAGM_GLENGINE_DATA_LABEL, "vir_create texture, graphics_low err.");

		auto PrintSizeFunc = [&](const Vector2T<uint32_t>& size) {
			return string(to_string(size.vector_x) + " x " + to_string(size.vector_y));
		};
		PushLogger(LogTrace, PSAGM_GLENGINE_DATA_LABEL, "vir_create texture[8x] size: %s", PrintSizeFunc(TexturesSize8X.TextureResolution).c_str());
		PushLogger(LogTrace, PSAGM_GLENGINE_DATA_LABEL, "vir_create texture[4x] size: %s", PrintSizeFunc(TexturesSize4X.TextureResolution).c_str());
		PushLogger(LogTrace, PSAGM_GLENGINE_DATA_LABEL, "vir_create texture[2x] size: %s", PrintSizeFunc(TexturesSize2X.TextureResolution).c_str());
		PushLogger(LogTrace, PSAGM_GLENGINE_DATA_LABEL, "vir_create texture[1x] size: %s", PrintSizeFunc(TexturesSize1X.TextureResolution).c_str());
	}

	void GLEngineSmpTextureData::VirtualTextureDataObjectDelete() {
		// free sampler & free textures.
		LLRES_Samplers->FreeTmuCount(LLRES_Textures->ResourceFind(TexturesSize8X.TextureArrayIndex).TextureSamplerCount);
		LLRES_Samplers->FreeTmuCount(LLRES_Textures->ResourceFind(TexturesSize4X.TextureArrayIndex).TextureSamplerCount);
		LLRES_Samplers->FreeTmuCount(LLRES_Textures->ResourceFind(TexturesSize2X.TextureArrayIndex).TextureSamplerCount);
		LLRES_Samplers->FreeTmuCount(LLRES_Textures->ResourceFind(TexturesSize1X.TextureArrayIndex).TextureSamplerCount);

		LLRES_Textures->ResourceDelete(TexturesSize8X.TextureArrayIndex);
		LLRES_Textures->ResourceDelete(TexturesSize4X.TextureArrayIndex);
		LLRES_Textures->ResourceDelete(TexturesSize2X.TextureArrayIndex);
		LLRES_Textures->ResourceDelete(TexturesSize1X.TextureArrayIndex);

		if (TexturesSize8X.LayerAllotter == nullptr) PushLogger(LogError, PSAGM_GLENGINE_DATA_LABEL, "vir_create texture[8x] allotter free_err.");
		delete TexturesSize8X.LayerAllotter;
		if (TexturesSize4X.LayerAllotter == nullptr) PushLogger(LogError, PSAGM_GLENGINE_DATA_LABEL, "vir_create texture[4x] allotter free_err.");
		delete TexturesSize4X.LayerAllotter;
		if (TexturesSize2X.LayerAllotter == nullptr) PushLogger(LogError, PSAGM_GLENGINE_DATA_LABEL, "vir_create texture[2x] allotter free_err.");
		delete TexturesSize2X.LayerAllotter;
		if (TexturesSize1X.LayerAllotter == nullptr) PushLogger(LogError, PSAGM_GLENGINE_DATA_LABEL, "vir_create texture[1x] allotter free_err.");
		delete TexturesSize1X.LayerAllotter;
	}
}