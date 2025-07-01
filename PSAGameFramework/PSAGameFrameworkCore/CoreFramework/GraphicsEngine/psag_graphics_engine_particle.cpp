// psag_graphics_engine_particle.
#include "psag_graphics_engine.h"

using namespace std;
using namespace PSAG_LOGGER;
using namespace GraphicsShaderCode;

#define DEG_TO_RAD(degrees) ((degrees) * 3.14159265 / 180.0)

#define PTCMS_SIN(deg) (float)sin(DEG_TO_RAD(deg))
#define PTCMS_COS(deg) (float)cos(DEG_TO_RAD(deg))

constexpr size_t ParticleVertAtt = 12;
constexpr size_t ParticleVertLen = 72;

// std: 2d_framework const z(layer). [20240824]
constexpr float STD_PARTICLES_Z = 75.0f;

inline float RandomTimeSeedFP32(float value_min, float value_max) {
	// seed: time(microseconds) => MT19937.
	mt19937 MtGen((uint32_t)chrono::duration_cast<chrono::microseconds>(
		chrono::system_clock::now().time_since_epoch()
	).count());
	uniform_real_distribution<float> Distr(value_min, value_max);
	return Distr(MtGen);
}

inline void ParticleDispPosition(
	GraphicsEngineParticle::ParticleAttributes& PARTC, 
	const Vector2T<float>& pos, const Vector3T<float>& offset
) {
	// position dispersion.
	PARTC.ParticlePosition.vector_x = RandomTimeSeedFP32(pos.vector_x, pos.vector_y) + offset.vector_x;
	PARTC.ParticlePosition.vector_y = RandomTimeSeedFP32(pos.vector_x, pos.vector_y) + offset.vector_y;
	PARTC.ParticlePosition.vector_z = STD_PARTICLES_Z + offset.vector_z;
}

inline void ParticleDispVector(
	GraphicsEngineParticle::ParticleAttributes& PARTC, Vector2T<float>& vec
) {
	// vector dispersion.
	PARTC.ParticleVector.vector_x = RandomTimeSeedFP32(vec.vector_x, vec.vector_y);
	PARTC.ParticleVector.vector_y = RandomTimeSeedFP32(vec.vector_x, vec.vector_y);
	PARTC.ParticleVector.vector_z = 0.0f;
}

inline void ParticleColorsys(
	GraphicsEngineParticle::ParticleAttributes& PARTC, 
	const Vector3T<Vector2T<float>>& color_system, bool gray_enable
) {
	float ColorAhpla = float(rand() % (1500 - 500 + 1) + 500) / 1000.0f;
	if (gray_enable) {
		float ColorGray = RandomTimeSeedFP32(color_system.vector_x.vector_x, color_system.vector_x.vector_y);
		PARTC.ParticleColor = Vector4T<float>(ColorGray, ColorGray, ColorGray, ColorAhpla);
		return;
	}
	PARTC.ParticleColor.vector_x = RandomTimeSeedFP32(color_system.vector_x.vector_x, color_system.vector_x.vector_y);
	PARTC.ParticleColor.vector_y = RandomTimeSeedFP32(color_system.vector_y.vector_x, color_system.vector_y.vector_y);
	PARTC.ParticleColor.vector_z = RandomTimeSeedFP32(color_system.vector_z.vector_x, color_system.vector_z.vector_y);
	PARTC.ParticleColor.vector_w = ColorAhpla;
}

// normal.x = particles.life value.
inline vector<float> ParticleBaseElement(
	const Vector3T<float>& position, const Vector4T<float>& color, float size, float life
) {
	vector<float> DefaultParticleRect = {
		-size + position.vector_x, -size + position.vector_y, 0.0f + position.vector_z,
		0.0f + color.vector_x, 0.0f + color.vector_y, 0.0f + color.vector_z, 0.0f + color.vector_w, 0.0f,0.0f, life,0.0f,0.0f,
		size + position.vector_x, -size + position.vector_y, 0.0f + position.vector_z,
		0.0f + color.vector_x, 0.0f + color.vector_y, 0.0f + color.vector_z, 0.0f + color.vector_w, 1.0f,0.0f, life,0.0f,0.0f,
		size + position.vector_x,  size + position.vector_y, 0.0f + position.vector_z,
		0.0f + color.vector_x, 0.0f + color.vector_y, 0.0f + color.vector_z, 0.0f + color.vector_w, 1.0f,1.0f, life,0.0f,0.0f,

		-size + position.vector_x, -size + position.vector_y, 0.0f + position.vector_z,
		0.0f + color.vector_x, 0.0f + color.vector_y, 0.0f + color.vector_z, 0.0f + color.vector_w, 0.0f,0.0f, life,0.0f,0.0f,
		size + position.vector_x,  size + position.vector_y, 0.0f + position.vector_z,
		0.0f + color.vector_x, 0.0f + color.vector_y, 0.0f + color.vector_z, 0.0f + color.vector_w, 1.0f,1.0f, life,0.0f,0.0f,
		-size + position.vector_x,  size + position.vector_y, 0.0f + position.vector_z,
		0.0f + color.vector_x, 0.0f + color.vector_y, 0.0f + color.vector_z, 0.0f + color.vector_w, 0.0f,1.0f, life,0.0f,0.0f
	};
	return DefaultParticleRect;
}

namespace GraphicsEngineParticle {
	// color channels filter.
	Vector3T<Vector2T<float>> __COLOR_SYSTEM_TYPE(
		Vector2T<float> r, Vector2T<float> g, Vector2T<float> b,
		ColorChannelMode mode, bool* gray_switch
	) {
		switch (mode) {
		case(ChannelsRG):  { b = Vector2T<float>(); break; }
		case(ChannelsRB):  { g = Vector2T<float>(); break; }
		case(ChannelsGB):  { r = Vector2T<float>(); break; }
		case(Grayscale):   { *gray_switch = true;   break; }
		case(ChannelsRGB): { break; }
		}
		return Vector3T<Vector2T<float>>(r, g, b);
	}

	void GeneratorPointsDiffu::CreateAddParticleDataset(vector<ParticleAttributes>& data) {
		for (size_t i = 0; i < ParticlesNumber; ++i) {
			ParticleAttributes ParticleTemp = {};

			ParticleDispPosition(ParticleTemp, RandomPosition, OffsetPosition);
			ParticleDispVector  (ParticleTemp, RandomSpeed);
			ParticleColorsys    (ParticleTemp, RandomColorSystem, EnableGrayscale);

			ParticleTemp.ParticleLife      = RandomTimeSeedFP32(RandomLimitLife.vector_x, RandomLimitLife.vector_y);
			ParticleTemp.ParticleScaleSize = RandomTimeSeedFP32(RandomScaleSize.vector_x, RandomScaleSize.vector_y);
			GeneratorCache.push_back(ParticleTemp);
		}
		// particles cache data => out(insert).
		data.insert(data.end(), GeneratorCache.begin(), GeneratorCache.end());
	}

	void GeneratorShape::CreateAddParticleDataset(vector<ParticleAttributes>& data) {
		float STEP = 360.0f / ParticlesNumber;
		switch (ParticlesMode) {
		case(1):
			for (size_t i = 0; i < ParticlesNumber; ++i) {
				ParticleAttributes ParticleTemp = {};

				ParticleDispPosition(ParticleTemp, RandomPosition, OffsetPosition);
				ParticleColorsys(ParticleTemp, RandomColorSystem, EnableGrayscale);

				ParticleTemp.ParticleVector = Vector3T<float>(
					PTCMS_SIN(i * STEP) * RandomSpeed.vector_x,
					PTCMS_COS(i * STEP) * RandomSpeed.vector_y,
					0.0f
				);
				ParticleTemp.ParticleLife      = RandomTimeSeedFP32(RandomLimitLife.vector_x, RandomLimitLife.vector_y);
				ParticleTemp.ParticleScaleSize = RandomTimeSeedFP32(RandomScaleSize.vector_x, RandomScaleSize.vector_y);
				GeneratorCache.push_back(ParticleTemp);
			}
			break;
		case(2):
			const float CenterX = 0.0f, CenterY = 0.0f;
			vector<Vector3T<float>> PointsSample = {};

			float SideLength = RandomSpeed.vector_x;
			float SampleStep = SideLength / (float)ParticlesNumber * 4.0f;

			for (int i = 0; i < ParticlesNumber / 4; ++i) {
				PointsSample.push_back(Vector3T<float>(-(SideLength / 2.0f) + (float)i * SampleStep, -(SideLength / 2.0f), 0.0f));
				PointsSample.push_back(Vector3T<float>(SideLength / 2.0f, - (SideLength / 2.0f) + (float)i * SampleStep, 0.0f));
				PointsSample.push_back(Vector3T<float>(-(SideLength / 2.0f) + (float)i * SampleStep, SideLength / 2.0f, 0.0f));
				PointsSample.push_back(Vector3T<float>(-(SideLength / 2.0f), - (SideLength / 2.0f) + (float)i * SampleStep, 0.0f));
			}

			for (const auto& Item : PointsSample) {
				ParticleAttributes ParticleTemp = {};

				ParticleDispPosition(ParticleTemp, RandomPosition, OffsetPosition);
				ParticleColorsys    (ParticleTemp, RandomColorSystem, EnableGrayscale);

				ParticleTemp.ParticleVector = Item;

				ParticleTemp.ParticleLife      = RandomTimeSeedFP32(RandomLimitLife.vector_x, RandomLimitLife.vector_y);
				ParticleTemp.ParticleScaleSize = RandomTimeSeedFP32(RandomScaleSize.vector_x, RandomScaleSize.vector_y);
				GeneratorCache.push_back(ParticleTemp);
			}
			break;
		}
		// particles cache data => out(insert).
		data.insert(data.end(), GeneratorCache.begin(), GeneratorCache.end());
	}

	void GeneratorDriftDown::CreateAddParticleDataset(vector<ParticleAttributes>& data) {
		for (size_t i = 0; i < ParticlesNumber; ++i) {
			ParticleAttributes ParticleTemp = {};

			ParticleColorsys(ParticleTemp, RandomColorSystem, EnableGrayscale);

			ParticleTemp.ParticlePosition.vector_y = RandomTimeSeedFP32(-700.0f, -300.0f) + OffsetPosition.vector_y;
			ParticleTemp.ParticlePosition.vector_x = RandomTimeSeedFP32(
				OffsetPosition.vector_x - ParticleWidth, OffsetPosition.vector_x + ParticleWidth
			);
			ParticleTemp.ParticlePosition.vector_z = 0.0f;

			Vector2T<float> RandomVector(1.0f * ParticleSpeed, 2.0f * ParticleSpeed);
			ParticleDispVector(ParticleTemp, RandomVector);

			ParticleTemp.ParticleLife      = RandomTimeSeedFP32(RandomLimitLife.vector_x, RandomLimitLife.vector_y);
			ParticleTemp.ParticleScaleSize = RandomTimeSeedFP32(RandomScaleSize.vector_x, RandomScaleSize.vector_y);
			GeneratorCache.push_back(ParticleTemp);
		}
		// particles cache data => out(insert).
		data.insert(data.end(), GeneratorCache.begin(), GeneratorCache.end());
	}

	// ************************************************ PARTICLE SYSTEM ************************************************
	// particles calc vec,pos,life,color. mode: main_thread, parallel_threads(pool).
#define DEFAULT_SPEED 1.2
	 
	void PsagGLEngineThread::ParticlesCalcuate(
		vector<ParticleAttributes>* pptc, double timestep
	) {
		const float FlySpeed = float(DEFAULT_SPEED * timestep);
		// calc particles position & life.
		for (auto it = pptc->begin(); it != pptc->end();) {
			// update position.
			it->ParticlePosition.vector_x += it->ParticleVector.vector_x * FlySpeed * 0.72f;
			it->ParticlePosition.vector_y += it->ParticleVector.vector_y * FlySpeed * 0.72f;
			it->ParticlePosition.vector_z += it->ParticleVector.vector_z * FlySpeed * 0.72f;
			// particle life calcuate: - std_speed * value * sw[ 0.0,1.0 ].
			it->ParticleLife -= FlySpeed * (float)it->ParticleLifeSwitch;
			// life > 0.0 : continue(check next).
			if (it->ParticleLife > 0.0f) { ++it; continue; }
			// delete particles entities.
			it = pptc->erase(it);
		}
	}

	void PsagGLEngineThread::ParticlesConvertData(
		const vector<ParticleAttributes>& src, vector<float>* dst,
		const Vector2T<float>& center
	) {
		// clear vertex src data_cache => convert.
		dst->clear();
		for (const auto& Item : src) {
			// format: vertex[vec3], color[vec4], uv[vec2], normal[vec3].
			auto DataTemp = ParticleBaseElement(
				Vector3T<float>(
					Item.ParticlePosition.vector_x + center.vector_x,
					Item.ParticlePosition.vector_y + center.vector_y,
					Item.ParticlePosition.vector_z
				),
				// normal: first vec(x) is particle life value.
				Item.ParticleColor, Item.ParticleScaleSize, Item.ParticleLife
			);
			// (add)push_back partiel.
			dst->insert(dst->end(), DataTemp.begin(), DataTemp.end());
		}
	}

	void PsagGLEngineThread::CalcThisContextFramerateStep(double base_fps) {
		int64_t TimerCountI64 = chrono::duration_cast<chrono::microseconds>(
			chrono::system_clock::now() - FramerateTimer
		).count();
		// calcuate framerate time step(scale).
		FramerateStep = (double)TimerCountI64 / 1000000.0 * base_fps;
		// frame context end => reset timer.
		FramerateTimer = chrono::system_clock::now();
	}

	void PsagGLEngineThread::PPTS_ParticleGeneratorTask(ParticleGeneratorBase* ptr) {
		vector<ParticleAttributes> ParticleEntities = {};
		ptr->CreateAddParticleDataset(ParticleEntities);
		// generator => gen particles => push queue => proc thread.
		lock_guard<mutex> Lock(GenParticleMutex);
		GenParticle.push(ParticleEntities);
	}

	void PsagGLEngineThread::PPTS_SetDataReadStatus() {
		// swap data buffer status => pawn proc thread.
		VertexDataReadStatus = true;
	}

	const vector<float>* PsagGLEngineThread::PPTS_GetVertexData() const {
		// double buffer, pawn: index, read: !(index).
		return &VertexDataDBuffer[(size_t)(!VertexDataDBufferIndex)];
	}

	array<size_t, 4> PsagGLEngineThread::PPTS_GetBackRunParams() {
		const size_t VertDataBytes = 
			VertexDataDBuffer[(size_t)(!VertexDataDBufferIndex)].size() * sizeof(float);
		size_t GenTempBytes = 0;
		{
			lock_guard<mutex> Lock(GenParticleMutex);
			GenTempBytes = GenParticle.size() * sizeof(ParticleAttributes);
		}
		// frame step => calc 1000x framerate.
		size_t KxFrame = size_t(60.0 / FramerateStep * 1000.0);
		KxFrame = KxFrame > (size_t)99999999 ? (size_t)100000000 : KxFrame;
		// buffer bytes * double + entities bytes + entities temp bytes.
		size_t MemoryUsage = VertDataBytes * 2 + 
			ParticlesData.size() * sizeof(ParticleAttributes) + GenTempBytes;
		return { ParticlesDataSize, VertDataBytes, KxFrame, MemoryUsage };
	}

	vector<ParticleAttributes> PsagGLEngineThread::PPTS_GetCurrentEntities() {
		lock_guard<mutex> Lock(ParticlesDataMutex);
		return ParticlesData;
	}

#define FRAME_LOAD_GROUPS_NUM 4
	PsagGLEngineThread::PsagGLEngineThread() {
		ParticleTaskProcFlag = true;
		// particle calc => convert => buffer.
		auto TASK_PROC_FUNC = [&]() {
			while (ParticleTaskProcFlag) {
				std::vector<ParticleAttributes> DataTemp = {};
				{
					lock_guard<mutex> Lock(GenParticleMutex);
					size_t MaxSizeCount = 0;
					while (!GenParticle.empty()) {
						if (MaxSizeCount > FRAME_LOAD_GROUPS_NUM) 
							break;
						DataTemp.insert(DataTemp.end(), 
							GenParticle.front().begin(), GenParticle.front().end());
						GenParticle.pop();
						++MaxSizeCount;
					}
				}
				lock_guard<mutex> Lock(ParticlesDataMutex);
				// insert gen particle entities. group(vector) / frame. [均衡负载]
				ParticlesData.insert(ParticlesData.end(), DataTemp.begin(), DataTemp.end());
				// particles attribute(entities) calcuate.
				ParticlesCalcuate(&ParticlesData, FramerateStep);
				ParticlesDataSize = ParticlesData.size();

				// PARTCILE SYSTEM BASE_FPS: 60.0 FPS. 2025_02_22 RCSZ.
				CalcThisContextFramerateStep(60.0);

				// 前台未读取完成, 后台不进行 convert & swap 操作.
				if (VertexDataReadStatus == false) continue;
				// particls attribute => vertex data.
				ParticlesConvertData(
					ParticlesData,
					&VertexDataDBuffer[(size_t)VertexDataDBufferIndex],
					Vector2T<float>(CoordCenterX, CoordCenterY)
				);
				// swap double data_buffer. pawn: proc thread.
				VertexDataDBufferIndex = !VertexDataDBufferIndex;
				VertexDataReadStatus = false;
			}
		};
		ParticleTaskProc = new thread(TASK_PROC_FUNC);
		PushLogger(LogInfo, PSAGM_GLENGINE_PARTICLE_LABEL, "particle system proc thread init.");
	}

	PsagGLEngineThread::~PsagGLEngineThread() {
		ParticleTaskProcFlag = false;
		ParticleTaskProc->join();
		delete ParticleTaskProc;
	}

	void PsagGLEngineParticle::ParticleSystemSample(int64_t sample_time) {
		int64_t TimerCountI64 = chrono::duration_cast<chrono::milliseconds>(
			chrono::system_clock::now() - BackInfoSampleTimer
		).count();
		if (TimerCountI64 > sample_time) {
			BackInfoSampleTimer = chrono::system_clock::now();
			auto SrcStateParams = PPTS_GetBackRunParams();
			
			SystemStateTemp.DarwParticlesNumber = SrcStateParams[0];
			SystemStateTemp.DarwDatasetBytes    = SrcStateParams[1];

			SystemStateTemp.BPT_RunFramerate   = (float)SrcStateParams[2] / 1000.0f;
			SystemStateTemp.BPT_BackMemoryUsed = (float)SrcStateParams[3] / 1048576.0f;
		}
	}

	size_t PsagGLEngineParticle::ParticleSystemCount = 0;
	PsagGLEngineParticle::PsagGLEngineParticle(const Vector2T<uint32_t>& render_size, const ImageRawData& image) {
		PSAG_SYS_GENERATE_KEY GenResourceID;

		// 粒子系统(实体对象)数量限制.
		if (ParticleSystemCount == PSAG_PARTICLE_SYSTEM_LIMIT) {
			PushLogger(LogError, PSAGM_GLENGINE_PARTICLE_LABEL, 
				"particle system invalid, number > %u.", PSAG_PARTICLE_SYSTEM_LIMIT);
			return;
		}
		ParticleSystemValid = true;
		PsagLow::PsagSupGraphicsOper::PsagGraphicsShader ShaderProcess;
		ShaderProcess.ShaderLoaderPushVS(GLOBALRES.Get().PublicShaders.ShaderVertTemplate, StringScript);

		ShaderProcess.ShaderLoaderPushFS(GLOBALRES.Get().PublicShaders.ShaderFragHeader,    StringScript);
		ShaderProcess.ShaderLoaderPushFS(GLOBALRES.Get().PublicShaders.ShaderFragTools,     StringScript);
		ShaderProcess.ShaderLoaderPushFS(GLOBALRES.Get().PrivateShaders.ShaderFragParticle, StringScript);

		// create & storage particles_shader.
		if (ShaderProcess.CreateCompileShader()) {
			ShaderProcessFinal = GenResourceID.PsagGenUniqueKey();
			GraphicShaders->ResourceStorage(ShaderProcessFinal, &ShaderProcess);
		}
		// => mag"GraphicsEngineDataset::GLEngineStcVertexData".
		DyVertexSysItem = GenResourceID.PsagGenUniqueKey();
		VerDyDataItemAlloc(DyVertexSysItem);

		VirTextureItem = GenResourceID.PsagGenUniqueKey();
		// virtual texture item alloc.
		if (image.ImagePixels.empty())
			VirTextureItemAllocEmpty(VirTextureItem, Vector2T<uint32_t>(256, 256));
		else
			VirTextureItemAlloc(VirTextureItem, image);
		// set uniform params name.
		VirTextureUniform.TexParamSampler  = "ParticleVirTex";
		VirTextureUniform.TexParamLayer    = "ParticleVirTexLayer";
		VirTextureUniform.TexParamCropping = "ParticleVirTexCropping";
		VirTextureUniform.TexParamSize     = "ParticleVirTexSize";

		PushLogger(LogInfo, PSAGM_GLENGINE_PARTICLE_LABEL, "particle system init.");
		PushLogger(LogInfo, PSAGM_GLENGINE_PARTICLE_LABEL, "particle system render_size: %u x %u", 
			render_size.vector_x, render_size.vector_y);
		++ParticleSystemCount;
	}

	PsagGLEngineParticle::~PsagGLEngineParticle() {
		// free graphics particle resource.
		VirTextureItemFree(VirTextureItem);
		VerDyDataItemFree(DyVertexSysItem);
		GraphicShaders->ResourceDelete(ShaderProcessFinal);
		// particle system valid => sub counter.
		if (ParticleSystemValid) --ParticleSystemCount;
	}

#define PARTICLE_SYSTEM_STATE_SMP 250
	void PsagGLEngineParticle::UpdateParticleData() {
		// particle vertex_data => graph data cache. 
		VerDyOperFramePushData(DyVertexSysItem, *PPTS_GetVertexData());
		// vertex_data reading completed.
		PPTS_SetDataReadStatus();
		ParticleSystemSample(PARTICLE_SYSTEM_STATE_SMP);
	}

	void PsagGLEngineParticle::RenderParticleFX() {
		auto ShaderTemp = GraphicShaders->ResourceFind(ShaderProcessFinal);
		OGLAPI_OPER.RenderBindShader(ShaderTemp);

		// system parset uniform.
		ShaderUniform.UniformMatrix4x4(ShaderTemp, "MvpMatrix",  MatrixDataWindow);

		ShaderUniform.UniformFloat(ShaderTemp, "RenderTime",  RenderTimer);
		ShaderUniform.UniformVec2 (ShaderTemp, "RenderMove",  RenderMove);
		ShaderUniform.UniformVec2 (ShaderTemp, "RenderScale", RenderScale);
		ShaderUniform.UniformFloat(ShaderTemp, "RenderAngle", RenderAngle);
		ShaderUniform.UniformFloat(ShaderTemp, "RenderTwist", RenderTwist);

		// draw virtual texture.
		VirTextureItemDraw(VirTextureItem, ShaderTemp, VirTextureUniform);
		// draw particles vertex.
		VerDyOperFrameDraw(DyVertexSysItem);

		OGLAPI_OPER.RenderUnbindShader();
		RenderTimer += PSAGM_VIR_TICKSTEP_GL * GraphicsEngineTimeStep;
	}

	ParticleSystemState PsagGLEngineParticle::GetParticleState() const {
		// return particle system state sample temp. 
		return SystemStateTemp;
	}

	void PsagGLEngineParticle::ParticleCreate(ParticleGeneratorBase* generator) {
		// generator call particle system.
		PPTS_ParticleGeneratorTask(generator);
	}

	vector<ParticleAttributes> PsagGLEngineParticle::GetParticleDataCopy() {
		// copy background data. thread safe.
		return PPTS_GetCurrentEntities();
	}
}