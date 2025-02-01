// psag_core_drivers_config.
#include "psag_core_drivers.h"

using namespace std;
using namespace PSAG_LOGGER;

namespace PsagFrameworkCore {
	// json config params load.
	bool PSAGCoreDriversInit::DriversParamsLoader(rapidjson::Document* ConfigDoc) {
		bool CHECK_FLAG_GUI = false;
		RendererGLversion = "#version 460 core";

		CHECK_FLAG_GUI |= !(ConfigDoc->HasMember("GuiWindowRounding") && (*ConfigDoc)["GuiWindowRounding"].IsFloat());
		CHECK_FLAG_GUI |= !(ConfigDoc->HasMember("GuiFrameRounding")  && (*ConfigDoc)["GuiFrameRounding"].IsFloat());
		CHECK_FLAG_GUI |= !(ConfigDoc->HasMember("GuiFontFilepath")   && (*ConfigDoc)["GuiFontFilepath"].IsString());
		CHECK_FLAG_GUI |= !(ConfigDoc->HasMember("GuiFontSize")       && (*ConfigDoc)["GuiFontSize"].IsFloat());
		CHECK_FLAG_GUI |= !(ConfigDoc->HasMember("GuiFontColor")      && (*ConfigDoc)["GuiFontColor"].IsArray());

		if (CHECK_FLAG_GUI) {
			PSAG_LOGGER::PushLogger(LogError, PSAGM_DRIVE_CORE_LABEL, "config(gui) loader: lost keys.");
			return false;
		}
		InitConfigImGui.WindowRounding  = (*ConfigDoc)["GuiWindowRounding"].GetFloat();
		InitConfigImGui.FrameRounding   = (*ConfigDoc)["GuiFrameRounding"].GetFloat();
		InitConfigImGui.FontsFilepath   = (*ConfigDoc)["GuiFontFilepath"].GetString();
		InitConfigImGui.FontsGlobalSize = (*ConfigDoc)["GuiFontSize"].GetFloat();

		// type: float32, array: 0: color_r, 1: color_g, 2: color_b, 3: color_a.
		InitConfigImGui.FontsGlobalColor.vector_x = (*ConfigDoc)["GuiFontColor"][0].GetFloat();
		InitConfigImGui.FontsGlobalColor.vector_y = (*ConfigDoc)["GuiFontColor"][1].GetFloat();
		InitConfigImGui.FontsGlobalColor.vector_z = (*ConfigDoc)["GuiFontColor"][2].GetFloat();
		InitConfigImGui.FontsGlobalColor.vector_w = (*ConfigDoc)["GuiFontColor"][3].GetFloat();

		InitConfigImGui.ShaderVersionStr = RendererGLversion;

		// ******************************** framework window params_config ********************************
		bool CHECK_FLAG_WINDOW = false; // lost keys_flag.

		CHECK_FLAG_WINDOW |= !(ConfigDoc->HasMember("WindowName") && (*ConfigDoc)["WindowName"].IsString());
		CHECK_FLAG_WINDOW |= !(ConfigDoc->HasMember("WindowFull") && (*ConfigDoc)["WindowFull"].IsBool());
		CHECK_FLAG_WINDOW |= !(ConfigDoc->HasMember("WindowSize") && (*ConfigDoc)["WindowSize"].IsArray());

		if (CHECK_FLAG_GUI) {
			PSAG_LOGGER::PushLogger(LogError, PSAGM_DRIVE_CORE_LABEL, "config(window) loader: lost keys.");
			return false;
		}
		InitConfigWindow.WindowName     = (*ConfigDoc)["WindowName"].GetString();
		InitConfigWindow.WindowFullFlag = (*ConfigDoc)["WindowFull"].GetBool();

		// type: unsigned int32, array: 0:x width, 1:y height.
		InitConfigWindow.WindowSizeWidth  = (*ConfigDoc)["WindowSize"][0].GetUint();
		InitConfigWindow.WindowSizeHeight = (*ConfigDoc)["WindowSize"][1].GetUint();

		// ******************************** framework render params_config ********************************
		bool CHECK_FLAG_RENDER = false;

		CHECK_FLAG_RENDER |= !(ConfigDoc->HasMember("RenderMSAA")        && (*ConfigDoc)["RenderMSAA"].IsInt());
		CHECK_FLAG_RENDER |= !(ConfigDoc->HasMember("RenderWindowFixed") && (*ConfigDoc)["RenderWindowFixed"].IsBool());
		CHECK_FLAG_RENDER |= !(ConfigDoc->HasMember("TextureBasicSize")  && (*ConfigDoc)["TextureBasicSize"].IsArray());
		CHECK_FLAG_RENDER |= !(ConfigDoc->HasMember("TextureGen")        && (*ConfigDoc)["TextureGen"].IsArray());
		CHECK_FLAG_RENDER |= !(ConfigDoc->HasMember("TextureEdge")       && (*ConfigDoc)["TextureEdge"].IsBool());
		CHECK_FLAG_RENDER |= !(ConfigDoc->HasMember("TextureHigh")       && (*ConfigDoc)["TextureHigh"].IsBool());

		RendererMSAA        = (*ConfigDoc)["RenderMSAA"].GetInt();
		RendererWindowFixed = (*ConfigDoc)["RenderWindowFixed"].GetBool();

		// type: unsigned int32, array: 0:u coord.x, 1:v coord.y.
		RenderingVirTexBasicSize.vector_x = (*ConfigDoc)["TextureBasicSize"][0].GetUint();
		RenderingVirTexBasicSize.vector_y = (*ConfigDoc)["TextureBasicSize"][1].GetUint();

		VirTextureFlags.vector_x = (*ConfigDoc)["TextureHigh"].GetBool();
		VirTextureFlags.vector_y = (*ConfigDoc)["TextureEdge"].GetBool();
		
		// type: unsigned int64, array: 0: 1/8x, 1: 1/4x, 2: 1/2x, 3: 1/1x.
		VirTexturesMax.Tex1Xnum = (size_t)(*ConfigDoc)["TextureGen"][0].GetInt();
		VirTexturesMax.Tex2Xnum = (size_t)(*ConfigDoc)["TextureGen"][1].GetInt();
		VirTexturesMax.Tex4Xnum = (size_t)(*ConfigDoc)["TextureGen"][2].GetInt();
		VirTexturesMax.Tex8Xnum = (size_t)(*ConfigDoc)["TextureGen"][3].GetInt();
		// return mul-flag.
		return !CHECK_FLAG_RENDER;
	}
}