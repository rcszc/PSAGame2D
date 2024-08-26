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
	mt19937 MtGen((uint32_t)chrono::duration_cast<chrono::microseconds>(chrono::system_clock::now().time_since_epoch()).count());
	uniform_real_distribution<float> Distr(value_min, value_max);
	return Distr(MtGen);
}

inline void ParticleDispPosition(GraphicsEngineParticle::ParticleAttributes& ptc, const Vector2T<float>& pos, const Vector3T<float>& off) {
	// position dispersion.
	ptc.ParticlePosition.vector_x = RandomTimeSeedFP32(pos.vector_x, pos.vector_y) + off.vector_x;
	ptc.ParticlePosition.vector_y = RandomTimeSeedFP32(pos.vector_x, pos.vector_y) + off.vector_y;
	ptc.ParticlePosition.vector_z = STD_PARTICLES_Z + off.vector_z;
}

inline void ParticleDispVector(GraphicsEngineParticle::ParticleAttributes& ptc, Vector2T<float>& vec) {
	// vector dispersion.
	ptc.ParticleVector.vector_x = RandomTimeSeedFP32(vec.vector_x, vec.vector_y);
	ptc.ParticleVector.vector_y = RandomTimeSeedFP32(vec.vector_x, vec.vector_y);
	// ptc.ParticleVector.vector_z = RandomTimeSeedFP32(vec.vector_x, vec.vector_y); // z-non-speed [20240819]
	ptc.ParticleVector.vector_z = 0.0f;
}

inline void ParticleColorsys(
	GraphicsEngineParticle::ParticleAttributes& ptc, const Vector3T<Vector2T<float>>& color_system,
	bool gray_enable
) {
	float ColorAhpla = float(rand() % (1500 - 500 + 1) + 500) / 1000.0f;
	if (gray_enable) {
		float ColorGray = RandomTimeSeedFP32(color_system.vector_x.vector_x, color_system.vector_x.vector_y);
		ptc.ParticleColor = Vector4T<float>(ColorGray, ColorGray, ColorGray, ColorAhpla);
		return;
	}
	ptc.ParticleColor.vector_x = RandomTimeSeedFP32(color_system.vector_x.vector_x, color_system.vector_x.vector_y);
	ptc.ParticleColor.vector_y = RandomTimeSeedFP32(color_system.vector_y.vector_x, color_system.vector_y.vector_y);
	ptc.ParticleColor.vector_z = RandomTimeSeedFP32(color_system.vector_z.vector_x, color_system.vector_z.vector_y);
	ptc.ParticleColor.vector_w = ColorAhpla;
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

	bool ParticleGenerator::ConfigCreateNumber(float number) {
		// generator particles < 8.
		if (number < 8.0f)
			return false;
		ParticlesNumber = (size_t)number;
		return true;
	}

	void ParticleGenerator::ConfigCreateMode(ParticlesGenMode::EmittersMode mode) {
		ParticlesModeType = mode;
	}

	void ParticleGenerator::ConfigLifeDispersion(Vector2T<float> rand_limit_life) {
		// particle rand limit: [0.0,n].
		rand_limit_life.vector_x = rand_limit_life.vector_x <= 0.0f ? 0.0f : rand_limit_life.vector_x;
		rand_limit_life.vector_y = rand_limit_life.vector_y <= 0.0f ? 0.0f : rand_limit_life.vector_y;
		RandomLimitLife = Vector2T<uint32_t>((uint32_t)rand_limit_life.vector_x, (uint32_t)rand_limit_life.vector_y);
	}

	void ParticleGenerator::ConfigSizeDispersion(Vector2T<float> rand_limit_size) {
		// particle rand size: [0.0,n].
		rand_limit_size.vector_x = rand_limit_size.vector_x <= 0.0f ? 0.0f : rand_limit_size.vector_x;
		rand_limit_size.vector_y = rand_limit_size.vector_y <= 0.0f ? 0.0f : rand_limit_size.vector_y;
		RandomScaleSize = rand_limit_size;
	}

	void ParticleGenerator::ConfigRandomColorSystem(
		Vector2T<float> r, Vector2T<float> g, Vector2T<float> b, 
		ParticlesGenMode::ColorChannelMode mode
	) {
		switch (mode) {
		case(ParticlesGenMode::Grayscale):   { EnableGrayscale = true; break; }
		case(ParticlesGenMode::ChannelsRG):  { b = Vector2T<float>();  break; }
		case(ParticlesGenMode::ChannelsRB):  { g = Vector2T<float>();  break; }
		case(ParticlesGenMode::ChannelsGB):  { r = Vector2T<float>();  break; }
		case(ParticlesGenMode::ChannelsRGB): { break; }
		}
		RandomColorSystem = Vector3T<Vector2T<float>>(r, g, b);
	}

	void ParticleGenerator::ConfigRandomDispersion(
		Vector2T<float> rand_limit_vector, Vector2T<float> rand_limit_position, Vector3T<float> offset_position
	) {
		RandomLimitVector   = rand_limit_vector;
		RandomLimitPosition = rand_limit_position;
		PositionOffset      = offset_position;
	}

	void ParticleGenerator::CreateAddParticleDataset(vector<ParticleAttributes>& data) {
		switch (ParticlesModeType) {
		// rand_limit vec2 x,y [speed]
		case(ParticlesGenMode::PrtcPoints): {
			for (size_t i = 0; i < ParticlesNumber; ++i) {
				ParticleAttributes ParticleTemp = {};

				ParticleDispPosition(ParticleTemp, RandomLimitPosition, PositionOffset);
				ParticleDispVector(ParticleTemp, RandomLimitVector);
				ParticleColorsys(ParticleTemp, RandomColorSystem, EnableGrayscale);

				ParticleTemp.ParticleLife      = RandomTimeSeedFP32((float)RandomLimitLife.vector_x, (float)RandomLimitLife.vector_y);
				ParticleTemp.ParticleScaleSize = RandomTimeSeedFP32(RandomScaleSize.vector_x, RandomScaleSize.vector_y);
				ParticlesGenCache.push_back(ParticleTemp);
			}
			break;
		}
		// rand_limit vec2 x:cricle_x_scale, y:cricle_y_scale [speed]
		case(ParticlesGenMode::PrtcCircle): {
			for (size_t i = 0; i < ParticlesNumber; ++i) {
				ParticleAttributes ParticleTemp = {};

				ParticleDispPosition(ParticleTemp, RandomLimitPosition, PositionOffset);
				ParticleColorsys(ParticleTemp, RandomColorSystem, EnableGrayscale);

				ParticleTemp.ParticleVector =
					Vector3T<float>(PTCMS_SIN(i * 3.8f) * RandomLimitVector.vector_x, PTCMS_COS(i * 3.8f) * RandomLimitVector.vector_y, 0.0f);

				ParticleTemp.ParticleLife      = RandomTimeSeedFP32((float)RandomLimitLife.vector_x, (float)RandomLimitLife.vector_y);
				ParticleTemp.ParticleScaleSize = RandomTimeSeedFP32(RandomScaleSize.vector_x, RandomScaleSize.vector_y);
				ParticlesGenCache.push_back(ParticleTemp);
			}
			break;
		}
		// rand_limit vec2 x:square_side, y:null [speed] 
		case(ParticlesGenMode::PrtcSquare): {
			const float CenterX = 0.0f, CenterY = 0.0f;
			vector<Vector3T<float>> PointsSample = {};

			float SideLength = RandomLimitVector.vector_x;
			float SampleStep = SideLength / (float)ParticlesNumber * 4.0f;

			for (int i = 0; i < ParticlesNumber / 4; ++i) {
				PointsSample.push_back(Vector3T<float>(-(SideLength / 2.0f) + (float)i * SampleStep, -(SideLength / 2.0f), 0.0f));
				PointsSample.push_back(Vector3T<float>(SideLength / 2.0f, -(SideLength / 2.0f) + (float)i * SampleStep, 0.0f));
				PointsSample.push_back(Vector3T<float>(-(SideLength / 2.0f) + (float)i * SampleStep, SideLength / 2.0f, 0.0f));
				PointsSample.push_back(Vector3T<float>(-(SideLength / 2.0f), -(SideLength / 2.0f) + (float)i * SampleStep, 0.0f));
			}

			for (const auto& Item : PointsSample) {
				ParticleAttributes ParticleTemp = {};

				ParticleDispPosition(ParticleTemp, RandomLimitPosition, PositionOffset);
				ParticleColorsys(ParticleTemp, RandomColorSystem, EnableGrayscale);

				ParticleTemp.ParticleVector = Item;

				ParticleTemp.ParticleLife      = RandomTimeSeedFP32((float)RandomLimitLife.vector_x, (float)RandomLimitLife.vector_y);
				ParticleTemp.ParticleScaleSize = RandomTimeSeedFP32(RandomScaleSize.vector_x, RandomScaleSize.vector_y);
				ParticlesGenCache.push_back(ParticleTemp);
			}
			break;
		}
		// rand_limit vec2 x,y [speed]
		// vector vec2 length * vec.
		case(ParticlesGenMode::PrtcPoly): {
			for (size_t i = 0; i < ParticlesNumber; ++i) {
				ParticleAttributes ParticleTemp = {};

				ParticleDispPosition(ParticleTemp, RandomLimitPosition, PositionOffset);
				ParticleColorsys(ParticleTemp, RandomColorSystem, EnableGrayscale);
				
				float Speed = RandomTimeSeedFP32(RandomLimitVector.vector_x, RandomLimitVector.vector_y);

				ParticleTemp.ParticleVector.vector_x 
					= (PositionOffset.vector_x - ParticleTemp.ParticlePosition.vector_x) * 0.02f * Speed;
				ParticleTemp.ParticleVector.vector_y 
					= (PositionOffset.vector_y - ParticleTemp.ParticlePosition.vector_y) * 0.02f * Speed;

				ParticleTemp.ParticleLife      = RandomTimeSeedFP32((float)RandomLimitLife.vector_x, (float)RandomLimitLife.vector_y);
				ParticleTemp.ParticleScaleSize = RandomTimeSeedFP32(RandomScaleSize.vector_x, RandomScaleSize.vector_y);
				ParticlesGenCache.push_back(ParticleTemp);
			}
			break;
		}
		// rand_limit vec2 x:abs(y_scale), y [speed]
		// offset pos vec2 x,y: high(offset)
		case(ParticlesGenMode::PrtcDrift): {
			for (size_t i = 0; i < ParticlesNumber; ++i) {
				ParticleAttributes ParticleTemp = {};

				ParticleColorsys(ParticleTemp, RandomColorSystem, EnableGrayscale);

				ParticleTemp.ParticlePosition.vector_y = RandomTimeSeedFP32(-700.0f, -300.0f) + PositionOffset.vector_y;
				ParticleTemp.ParticlePosition.vector_x = RandomTimeSeedFP32(RandomLimitPosition.vector_x, RandomLimitPosition.vector_y) + PositionOffset.vector_x;
				ParticleTemp.ParticlePosition.vector_z = RandomTimeSeedFP32(RandomLimitPosition.vector_x, RandomLimitPosition.vector_y) + PositionOffset.vector_z;

				ParticleDispVector(ParticleTemp, RandomLimitVector);
				ParticleTemp.ParticleVector.vector_y = abs(RandomLimitVector.vector_x);

				ParticleTemp.ParticleLife      = RandomTimeSeedFP32((float)RandomLimitLife.vector_x, (float)RandomLimitLife.vector_y);
				ParticleTemp.ParticleScaleSize = RandomTimeSeedFP32(RandomScaleSize.vector_x, RandomScaleSize.vector_y);
				ParticlesGenCache.push_back(ParticleTemp);
			}
			break;
		}}
		// create particles dataset.
		data.insert(data.end(), ParticlesGenCache.begin(), ParticlesGenCache.end());
	}

	// ************************************************ PARTICLE SYSTEM ************************************************
	// particles calc vec,pos,life,color. mode: main_thread, parallel_threads(pool).

#define DEFAULT_SPEED 1.0f
	class ParticleSystemDataCalc {
	protected:
		vector<ParticleAttributes> ParticlesCalcCache = {};
	public:
		ParticleSystemDataCalc(vector<ParticleAttributes> block_particles, float speed, float lifesub) {
			// calc particles position.
			for (size_t i = 0; i < block_particles.size(); ++i) {
				// update position.
				block_particles[i].ParticlePosition.vector_x += block_particles[i].ParticleVector.vector_x * DEFAULT_SPEED * speed;
				block_particles[i].ParticlePosition.vector_y += block_particles[i].ParticleVector.vector_y * DEFAULT_SPEED * speed;
				block_particles[i].ParticlePosition.vector_z += block_particles[i].ParticleVector.vector_z * DEFAULT_SPEED * speed;

				block_particles[i].ParticleLife -= DEFAULT_SPEED * lifesub * (float)block_particles[i].ParticleLifeSwitch;
				// delete particle.
				if (block_particles[i].ParticleLife <= 0.0f)
					block_particles.erase(block_particles.begin() + i);
			}
			// copy result particles_data.
			ParticlesCalcCache = block_particles;
		}
		vector<ParticleAttributes> GetDataset() {
			return move(ParticlesCalcCache);
		}
	};

	void PsagGLEngineParticle::CalcUpdateParticlesPARA(vector<ParticleAttributes>& particles, float speed, float lifesub) {
		size_t TasksNumber = particles.size() / DataBlockSize + 1;
		// tasks object future.
		vector<future<shared_ptr<ParticleSystemDataCalc>>> ResultObject(TasksNumber);
		
		// craete task => parallel.
		for (size_t i = 0; i < TasksNumber; ++i) {
			size_t IndexStart = i * DataBlockSize;
			size_t IndexEnd = __psag_bit_min(IndexStart + DataBlockSize, particles.size());

			vector<ParticleAttributes> BlockCreateTemp(IndexEnd - IndexStart);
			// particles dataset memory_block copy.
			copy(particles.begin() + IndexStart, particles.begin() + IndexEnd, BlockCreateTemp.begin());

			ResultObject[i] = ThreadsParallel->PushTask<ParticleSystemDataCalc>(BlockCreateTemp, speed, lifesub);
		}
		
		// clear data => get task result.
		particles.clear();
		for (size_t i = 0; i < TasksNumber; ++i) {
			// read parallel threads data.
			auto DataResultPtr = ResultObject[i].get()->GetDataset();
			particles.insert(particles.end(), DataResultPtr.begin(), DataResultPtr.end());
		}
	}

	void PsagGLEngineParticle::CalcUpdateParticles(vector<ParticleAttributes>& particles, float speed, float lifesub) {
		// calc particles position & life.
		for (size_t i = 0; i < particles.size(); ++i) {
			// update position.
			particles[i].ParticlePosition.vector_x += particles[i].ParticleVector.vector_x * DEFAULT_SPEED * speed;
			particles[i].ParticlePosition.vector_y += particles[i].ParticleVector.vector_y * DEFAULT_SPEED * speed;
			particles[i].ParticlePosition.vector_z += particles[i].ParticleVector.vector_z * DEFAULT_SPEED * speed;

			particles[i].ParticleLife -= DEFAULT_SPEED * lifesub * (float)particles[i].ParticleLifeSwitch;
			// delete particle.
			if (particles[i].ParticleLife <= 0.0f)
				particles.erase(particles.begin() + i);
		}
	}

	void PsagGLEngineParticle::VertexDataConvert(
		const vector<ParticleAttributes>& src, vector<float>& dst, const Vector2T<float>& center
	) {
		// clear vertex srcdata_cache => convert.
		dst.clear();
		for (const auto& Item : src) {
			// format: vertex[vec3], color[vec4], uv(coord)[vec2], normal[vec3].
			auto PTC_TEMP = ParticleBaseElement(
				Vector3T<float>(
					Item.ParticlePosition.vector_x + center.vector_x, 
					Item.ParticlePosition.vector_y + center.vector_y,
					Item.ParticlePosition.vector_z
				), 
				Item.ParticleColor, Item.ParticleScaleSize, Item.ParticleLife
			);
			// (add)push_back partiel.
			dst.insert(dst.end(), PTC_TEMP.begin(), PTC_TEMP.end());
		}
	}

	PsagGLEngineParticle::PsagGLEngineParticle(const Vector2T<uint32_t>& render_resolution, const ImageRawData& image) {
		PSAG_SYS_GENERATE_KEY GenResourceID;
		
		PsagLow::PsagSupGraphicsOper::PsagGraphicsShader ShaderProcess;
		ShaderProcess.ShaderLoaderPushVS(GLOBALRES.Get().PublicShaders.ShaderVertTemplate, StringScript);

		ShaderProcess.ShaderLoaderPushFS(GLOBALRES.Get().PublicShaders.ShaderFragHeader,    StringScript);
		ShaderProcess.ShaderLoaderPushFS(GLOBALRES.Get().PublicShaders.ShaderFragTools,     StringScript);
		ShaderProcess.ShaderLoaderPushFS(GLOBALRES.Get().PrivateShaders.ShaderFragParticle, StringScript);

		// create & storage particles_shader.
		if (ShaderProcess.CreateCompileShader()) {
			ShaderPostProgram = GenResourceID.PsagGenUniqueKey();
			LLRES_Shaders->ResourceStorage(ShaderPostProgram, &ShaderProcess);
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

		UPDATE_CALC_FUNC = [&](vector<ParticleAttributes>& p, float s, float f) { CalcUpdateParticles(p, s, f); };

		PushLogger(LogInfo, PSAGM_GLENGINE_PARTICLE_LABEL, "particle system init.");
		PushLogger(LogInfo, PSAGM_GLENGINE_PARTICLE_LABEL, "particle system render_resolution: %u x %u", render_resolution.vector_x, render_resolution.vector_y);
	}

	PsagGLEngineParticle::~PsagGLEngineParticle() {
		// free graphics particle resource.
		VirTextureItemFree(VirTextureItem);
		VerDyDataItemFree(DyVertexSysItem);
		LLRES_Shaders->ResourceDelete(ShaderPostProgram);

		if (ThreadsParallel != nullptr)
			delete ThreadsParallel;
	}

	void PsagGLEngineParticle::UpdateParticleData() {
		// calculate particles coord.
		UPDATE_CALC_FUNC(DataParticles, 0.72f * GraphicsEngineTimeStep, GraphicsEngineTimeStep);
		// "particle_attributes" => float
		VertexDataConvert(DataParticles, DataVertices, ParticlesCoordCenter);
		// particles_data => data cache. 
		VerDyOperFramePushData(DyVertexSysItem, DataVertices);
	}

	void PsagGLEngineParticle::RenderParticleFX() {
		auto ShaderTemp = LLRES_Shaders->ResourceFind(ShaderPostProgram);
		ShaderRender.RenderBindShader(ShaderTemp);

		// system parset uniform.
		ShaderUniform.UniformMatrix4x4(ShaderTemp, "MvpMatrix",  MatrixDataWindow);

		ShaderUniform.UniformFloat(ShaderTemp, "RenderTime",   RenderTimer);
		ShaderUniform.UniformVec2 (ShaderTemp, "RenderMove",   RenderMove);
		ShaderUniform.UniformVec2 (ShaderTemp, "RenderScale",  RenderScale);
		ShaderUniform.UniformFloat(ShaderTemp, "RenderRotate", RenderRotate);
		ShaderUniform.UniformFloat(ShaderTemp, "RenderTwist",  RenderTwist);

		// draw virtual texture.
		VirTextureItemDraw(VirTextureItem, ShaderTemp, VirTextureUniform);
		// draw particles vertex.
		VerDyOperFrameDraw(DyVertexSysItem);

		ShaderRender.RenderUnbindShader();
		RenderTimer += PSAGM_VIR_TICKSTEP_GL * GraphicsEngineTimeStep;
	}

	ParticleSystemState PsagGLEngineParticle::GetParticleState() {
		ParticleSystemState ResultState = {};

		ResultState.DarwParticlesNumber = DataParticles.size();
		ResultState.DarwDatasetSize     = DataVertices.size();

		return ResultState;
	}

	void PsagGLEngineParticle::ParticleCreate(ParticleGeneratorBase* generator) {
#if ENABLE_DEBUG_MODE
		FTDcapture::CaptureContext CapPoint;

		CapPoint.CaptureSettingFilter(FTD_TagLv4);
		CapPoint.CaptureSettingPointer(this);
		CapPoint.CaptureSettingTagging("particles create.");
		CapPoint.CaptureBegin();
#endif
		// generator => particle_system.
		generator->CreateAddParticleDataset(DataParticles);
	}

	void PsagGLEngineParticle::ParticleClacMode(ParticleCalcMode mode, size_t block_size, uint32_t threads) {
		// parallel mode ? => failed.
		if (ThreadsParallel != nullptr) return;
		// particles calc(update) func_load.
		switch (mode) {
		case(CALC_DEFAULT): { 
			UPDATE_CALC_FUNC = [&](vector<ParticleAttributes>& p, float s, float f)
				{ CalcUpdateParticles(p, s, f); };
			break; 
		}
		case(CALC_PARALLEL): { 
			UPDATE_CALC_FUNC = [&](vector<ParticleAttributes>& p, float s, float f)
				{ CalcUpdateParticlesPARA(p, s, f); };
			// block default(min) size = 1000.
			DataBlockSize = block_size > 1000 ? block_size : 1000;
			// create threads_pool.
			ThreadsParallel = new PSAG_THREAD_POOL::PsagThreadTasks(threads);
			break;
		}
		case(CALC_NO_CALC): { 
			UPDATE_CALC_FUNC = [&](vector<ParticleAttributes>& p, float s, float f)
				{ CalcUpdateParticlesNULL(p, s, f); }; 
			break; 
		}}
	}

	vector<ParticleAttributes>* PsagGLEngineParticle::GetParticleDataset() {
		// particle_system data pointer.
		return &DataParticles;
	}
}