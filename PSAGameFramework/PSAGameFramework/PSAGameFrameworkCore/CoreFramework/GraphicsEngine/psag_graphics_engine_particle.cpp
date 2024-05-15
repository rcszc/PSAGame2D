// psag_graphics_engine_particle.
#include "psag_graphics_engine.h"

using namespace std;
using namespace PSAG_LOGGER;

#define DEG_TO_RAD(degrees) ((degrees) * 3.14159265 / 180.0)

#define PTCMS_SIN(deg) (float)sin(DEG_TO_RAD(deg))
#define PTCMS_COS(deg) (float)cos(DEG_TO_RAD(deg))

constexpr size_t ParticleVertAtt = 12;
constexpr size_t ParticleVertLen = 72;

constexpr float FxSpaceSize = 256.0f;

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
	ptc.ParticlePosition.vector_z = RandomTimeSeedFP32(pos.vector_x, pos.vector_y) + off.vector_z;
}

inline void ParticleDispVector(GraphicsEngineParticle::ParticleAttributes& ptc, Vector2T<float>& vec) {
	// vector dispersion.
	ptc.ParticleVector.vector_x = RandomTimeSeedFP32(vec.vector_x, vec.vector_y);
	ptc.ParticleVector.vector_y = RandomTimeSeedFP32(vec.vector_x, vec.vector_y);
	ptc.ParticleVector.vector_z = RandomTimeSeedFP32(vec.vector_x, vec.vector_y);
}

inline void ParticleColorsys(
	GraphicsEngineParticle::ParticleAttributes& ptc, const Vector3T<Vector2T<float>>& colorsys,
	bool enable_gray
) {
	float ColorAhpla = float(rand() % (1500 - 500 + 1) + 500) / 1000.0f;
	if (enable_gray) {
		float ColorGray = RandomTimeSeedFP32(colorsys.vector_x.vector_x, colorsys.vector_x.vector_y);
		ptc.ParticleColor = Vector4T<float>(ColorGray, ColorGray, ColorGray, ColorAhpla);
		return;
	}
	ptc.ParticleColor.vector_x = RandomTimeSeedFP32(colorsys.vector_x.vector_x, colorsys.vector_x.vector_y);
	ptc.ParticleColor.vector_y = RandomTimeSeedFP32(colorsys.vector_y.vector_x, colorsys.vector_y.vector_y);
	ptc.ParticleColor.vector_z = RandomTimeSeedFP32(colorsys.vector_z.vector_x, colorsys.vector_z.vector_y);
	ptc.ParticleColor.vector_w = ColorAhpla;
}

namespace GraphicsEngineParticle {

	bool ParticleGenerator::ConfigCreateNumber(float number) {
		// generator particles < 8.
		if (number < 8.0f)
			return false;
		ParticlesNumber = (size_t)number;
		return true;
	}

	void ParticleGenerator::ConfigCreateMode(EmittersMode mode) {
		ParticlesModeType = mode;
	}

	void ParticleGenerator::ConfigLifeDispersion(Vector2T<float> rand_limit_life) {
		// particle rand limit: [0.0,n].
		rand_limit_life.vector_x = rand_limit_life.vector_x <= 0.0f ? 0.0f : rand_limit_life.vector_x;
		rand_limit_life.vector_y = rand_limit_life.vector_y <= 0.0f ? 0.0f : rand_limit_life.vector_y;
		RandomLimitLife = Vector2T<uint32_t>((uint32_t)rand_limit_life.vector_x, (uint32_t)rand_limit_life.vector_y);
	}

	void ParticleGenerator::ConfigRandomColorSystem(Vector2T<float> r, Vector2T<float> g, Vector2T<float> b, ColorChannelMode mode) {
		switch (mode) {
		case(Grayscale):  { EnableGrayscale = true; break; }
		case(ChannelsRG): { b = Vector2T<float>();  break; }
		case(ChannelsRB): { g = Vector2T<float>();  break; }
		case(ChannelsGB): { r = Vector2T<float>();  break; }
		case(ChannelsRGB): { break; }
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
		case(PrtcPoints): {
			for (size_t i = 0; i < ParticlesNumber; ++i) {
				ParticleAttributes ParticleTemp = {};

				ParticleDispPosition(ParticleTemp, RandomLimitPosition, PositionOffset);
				ParticleDispVector(ParticleTemp, RandomLimitVector);
				ParticleColorsys(ParticleTemp, RandomColorSystem, EnableGrayscale);

				ParticleTemp.ParticleLife = RandomTimeSeedFP32((float)RandomLimitLife.vector_x, (float)RandomLimitLife.vector_y);
				ParticlesGenCache.push_back(ParticleTemp);
			}
			break;
		}
		// rand_limit vec2 x:cricle_x_scale, y:cricle_y_scale [speed]
		case(PrtcCircle): {
			for (size_t i = 0; i < ParticlesNumber; ++i) {
				ParticleAttributes ParticleTemp = {};

				ParticleDispPosition(ParticleTemp, RandomLimitPosition, PositionOffset);
				ParticleColorsys(ParticleTemp, RandomColorSystem, EnableGrayscale);

				ParticleTemp.ParticleVector =
					Vector3T<float>(PTCMS_SIN(i * 3.8f) * RandomLimitVector.vector_x, PTCMS_COS(i * 3.8f) * RandomLimitVector.vector_y, 0.0f);

				ParticleTemp.ParticleLife = RandomTimeSeedFP32((float)RandomLimitLife.vector_x, (float)RandomLimitLife.vector_y);
				ParticlesGenCache.push_back(ParticleTemp);
			}
			break;
		}
		// rand_limit vec2 x:square_side, y:null [speed] 
		case(PrtcSquare): {
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

				ParticleTemp.ParticleLife = RandomTimeSeedFP32((float)RandomLimitLife.vector_x, (float)RandomLimitLife.vector_y);
				ParticlesGenCache.push_back(ParticleTemp);
			}
			break;
		}
		// rand_limit vec2 x:abs(y_scale), y [speed]
		// offset pos vec2 x,y: high(offset)
		case(PrtcDrift): {
			for (size_t i = 0; i < ParticlesNumber; ++i) {
				ParticleAttributes ParticleTemp = {};

				ParticleColorsys(ParticleTemp, RandomColorSystem, EnableGrayscale);

				ParticleTemp.ParticlePosition.vector_y = RandomTimeSeedFP32(-700.0f, -300.0f) + PositionOffset.vector_y;
				ParticleTemp.ParticlePosition.vector_x = RandomTimeSeedFP32(RandomLimitPosition.vector_x, RandomLimitPosition.vector_y) + PositionOffset.vector_x;
				ParticleTemp.ParticlePosition.vector_z = RandomTimeSeedFP32(RandomLimitPosition.vector_x, RandomLimitPosition.vector_y) + PositionOffset.vector_z;

				ParticleDispVector(ParticleTemp, RandomLimitVector);
				ParticleTemp.ParticleVector.vector_y = abs(RandomLimitVector.vector_x);

				ParticleTemp.ParticleLife = RandomTimeSeedFP32((float)RandomLimitLife.vector_x, (float)RandomLimitLife.vector_y);
				ParticlesGenCache.push_back(ParticleTemp);
			}
			break;
		}
		}
		// create particles dataset.
		data.insert(data.end(), ParticlesGenCache.begin(), ParticlesGenCache.end());
	}

#define DEFAULT_SPEED 1.0f
	void PsagGLEngineParticle::CalcUpdateParticles(vector<ParticleAttributes>& particles, float speed, float lifesub) {
		// calc particles.
		for (size_t i = 0; i < particles.size(); ++i) {
			// update position.
			particles[i].ParticlePosition.vector_x += particles[i].ParticleVector.vector_x * DEFAULT_SPEED * speed;
			particles[i].ParticlePosition.vector_y += particles[i].ParticleVector.vector_y * DEFAULT_SPEED * speed;
			particles[i].ParticlePosition.vector_z += particles[i].ParticleVector.vector_z * DEFAULT_SPEED * speed;

			particles[i].ParticleLife -= DEFAULT_SPEED * lifesub;
			// delete particle.
			if (particles[i].ParticleLife <= 0.0f)
				particles.erase(particles.begin() + i);
		}
	}

	void PsagGLEngineParticle::VertexDataConvert(const vector<ParticleAttributes>& src, vector<float>& cvt) {
		Vector3T<float> PositionTemp = {};
		Vector4T<float> ColorTemp    = {};

		// clear vertex srcdata_cache => convert.
		cvt.clear();
		for (const auto& Item : src) {

			// create particle: position,color.
			PositionTemp = Item.ParticlePosition;
			ColorTemp = Item.ParticleColor;

			// format: vertex[vec3], color[vec4], uv(coord)[vec2], normal[vec3].
			float DefaultParticle[72] = {
				-5.0f + PositionTemp.vector_x, -5.0f + PositionTemp.vector_y, 0.0f + PositionTemp.vector_z,
				 0.0f + ColorTemp.vector_x, 0.0f + ColorTemp.vector_y, 0.0f + ColorTemp.vector_z, 0.0f + ColorTemp.vector_w, 0.0f,0.0f, 0.0f,0.0f,0.0f,
				 5.0f + PositionTemp.vector_x, -5.0f + PositionTemp.vector_y, 0.0f + PositionTemp.vector_z,
				 0.0f + ColorTemp.vector_x, 0.0f + ColorTemp.vector_y, 0.0f + ColorTemp.vector_z, 0.0f + ColorTemp.vector_w, 1.0f,0.0f, 0.0f,0.0f,0.0f,
				 5.0f + PositionTemp.vector_x,  5.0f + PositionTemp.vector_y, 0.0f + PositionTemp.vector_z,
				 0.0f + ColorTemp.vector_x, 0.0f + ColorTemp.vector_y, 0.0f + ColorTemp.vector_z, 0.0f + ColorTemp.vector_w, 1.0f,1.0f, 0.0f,0.0f,0.0f,

				-5.0f + PositionTemp.vector_x, -5.0f + PositionTemp.vector_y, 0.0f + PositionTemp.vector_z,
				 0.0f + ColorTemp.vector_x, 0.0f + ColorTemp.vector_y, 0.0f + ColorTemp.vector_z, 0.0f + ColorTemp.vector_w, 0.0f,0.0f, 0.0f,0.0f,0.0f,
				 5.0f + PositionTemp.vector_x,  5.0f + PositionTemp.vector_y, 0.0f + PositionTemp.vector_z,
				 0.0f + ColorTemp.vector_x, 0.0f + ColorTemp.vector_y, 0.0f + ColorTemp.vector_z, 0.0f + ColorTemp.vector_w, 1.0f,1.0f, 0.0f,0.0f,0.0f,
				-5.0f + PositionTemp.vector_x,  5.0f + PositionTemp.vector_y, 0.0f + PositionTemp.vector_z,
				 0.0f + ColorTemp.vector_x, 0.0f + ColorTemp.vector_y, 0.0f + ColorTemp.vector_z, 0.0f + ColorTemp.vector_w, 0.0f,1.0f, 0.0f,0.0f,0.0f
			};
			// src => particle vert => cvt
			cvt.insert(cvt.end(), DefaultParticle, DefaultParticle + ParticleVertLen);
		}
	}

	PsagGLEngineParticle::PsagGLEngineParticle(const Vector2T<uint32_t>& render_resolution, const ImageRawData& image) {
		PSAG_SYSGEN_TIME_KEY GenResourceID;

		PsagLow::PsagSupGraphicsTool::PsagGraphicsShader ShaderProcess;
		ShaderProgramItem = to_string(GenResourceID.PsagGenTimeKey());

		ShaderProcess.ShaderLoaderVS(GraphicsShaderScript::PsagShaderScriptPublicVS, StringScript);
		ShaderProcess.ShaderLoaderFS(GraphicsShaderScript::PsagShaderScriptParticleFS, StringScript);

		if (ShaderProcess.CreateCompileShader())
			LLRES_Shaders->ResourceStorage(ShaderProgramItem, &ShaderProcess);

		// => mag"GraphicsEngineDataset::GLEngineStcVertexData".
		DyVertexSysItem = to_string(GenResourceID.PsagGenTimeKey());
		VerDataItemAlloc(DyVertexSysItem);

		float RenderScale = (float)render_resolution.vector_x / (float)render_resolution.vector_y;
		// porj matrix + scale.
		glm::mat4 ProjectionMatrix = glm::ortho(
			-FxSpaceSize * RenderScale, FxSpaceSize * RenderScale, -FxSpaceSize, FxSpaceSize, -FxSpaceSize, FxSpaceSize
		);
		// convert: glm matrix => imfx matrix.
		const float* glmmatptr = glm::value_ptr(ProjectionMatrix);
		memcpy_s(RenderMatrix.matrix, 16 * sizeof(float), glmmatptr, 16 * sizeof(float));

		// virtual texture item alloc.
		if (image.ImagePixels.empty())
			VirTextureItemAlloc(VirTextureItem, Vector2T<uint32_t>(512, 512));
		else
			VirTextureItemAlloc(VirTextureItem, image);
		// set uniform params name.
		VirTextureUniform.TexParamSampler  = "ParticleVirTex";
		VirTextureUniform.TexParamLayer    = "ParticleVirTexLayer";
		VirTextureUniform.TexParamCropping = "ParticleVirTexCropping";
		VirTextureUniform.TexParamSize     = "ParticleVirTexSize";

		PushLogger(LogInfo, PSAGM_GLENGINE_PARTICLE_LABEL, "particle system init.");
		PushLogger(LogInfo, PSAGM_GLENGINE_PARTICLE_LABEL, "particle system render_resolution: %u x %u", render_resolution.vector_x, render_resolution.vector_y);
	}

	PsagGLEngineParticle::~PsagGLEngineParticle() {
		// delete particles res.
		VerDataItemFree(DyVertexSysItem);
		LLRES_Shaders->ResourceDelete(ShaderProgramItem);
	}

	void PsagGLEngineParticle::UpdateParticleData() {
		CalcUpdateParticles(DataParticles, 0.5f, 1.0f);
		// "particle_attributes" => float
		VertexDataConvert(DataParticles, DataVertices);
		// particles_data => data cache. 
		VerOperFramePushData(DyVertexSysItem, DataVertices);
	}

	void PsagGLEngineParticle::RenderParticleFX() {
		auto ShaderTemp = LLRES_Shaders->ResourceFind(ShaderProgramItem);
		PsagLow::PsagSupGraphicsTool::PsagGraphicsFuncShaderContextBind(ShaderTemp);
		// draw particles vertex.
		VerOperFrameDraw(DyVertexSysItem);

		// system parset uniform.
		ShaderUniform.UniformMatrix4x4(ShaderTemp, "MvpMatrix",  RenderMatrix);
		ShaderUniform.UniformFloat    (ShaderTemp, "RenderTime", RenderTimer);

		ShaderUniform.UniformVec2(ShaderTemp, "RenderMove",  RenderMove);
		ShaderUniform.UniformVec2(ShaderTemp, "RenderScale", RenderScale);

		// draw virtual texture.
		VirTextureItemDraw(VirTextureItem, ShaderTemp, VirTextureUniform);

		PsagLow::PsagSupGraphicsTool::PsagGraphicsFuncShaderContextUnbind();
		RenderTimer += PSAGM_VIR_TICKSTEP;
	}

	ParticleSystemState PsagGLEngineParticle::GetParticleState() {
		ParticleSystemState ResultState = {};

		ResultState.DarwParticlesNumber = DataParticles.size();
		ResultState.DarwDatasetSize     = DataVertices.size();

		return ResultState;
	}

	void PsagGLEngineParticle::ParticleCreate(ParticleGeneratorBase* generator) {
		// generator => particle_system.
		generator->CreateAddParticleDataset(DataParticles);
	}

	vector<ParticleAttributes>* PsagGLEngineParticle::GetParticleDataset() {
		// particle_system ptc_data pointer.
		return &DataParticles;
	}
}