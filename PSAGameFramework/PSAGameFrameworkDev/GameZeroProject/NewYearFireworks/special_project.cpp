// special_project.
#include "special_project.h"

using namespace std;
using namespace PSAG_LOGGER;

bool FireworksSystem::LogicInitialization(const Vector2T<uint32_t>& WinSize) {
	PsagActorType::ActorTypeAllotter.ActorTypeCreate("ActorFIRE");

	FireCyclesTimer.CreatePointer();

	ActorShaders.CreatePointer();
	ActorEntities.CreatePointer();

	PsagManager::SyncLoader::SyncEncDecImage DecodeRawImage;
	PsagActor::PresetScript ShaderScript = {};

	auto ActorImage = LOADFILE::EasyFileReadRawData(SPE_SYSPATH_REF("spe_fireworks.png"));
	// create fireworks actor shader.
	PsagActor::ActorShader* FireworksShader = new PsagActor::ActorShader(ShaderScript.TmpScriptDrawImage(), WinSize);
	FireworksShader->ShaderImageLoad(DecodeRawImage.DecodeImageRawData(ActorImage));

	ActorShaders.Get()->CreateActorShader("BASE", FireworksShader);

	auto Music = LOADFILE::EasyFileReadRawData(SPE_SYSPATH_REF("Robin Hustin - Light It Up.ogg"));
	MainMusicPlayer.CreatePointer(AudioConvert(Music));

	InitParticleSystem(WinSize);
	return true;
}

void FireworksSystem::LogicCloseFree() {
	// free resource.
	MainMusicPlayer.DeletePointer();
	FireCyclesTimer.DeletePointer();

	ActorShaders.DeletePointer();
	ActorEntities.DeletePointer();
}

chrono::system_clock::time_point CreateTimePoint(
	int year, int month, int day, int hour, int minute, int second, int millisecond
) {
	tm TVALUE = {};
	TVALUE.tm_year = year - 1900;
	TVALUE.tm_mon  = month - 1;
	TVALUE.tm_mday = day;
	TVALUE.tm_hour = hour;
	TVALUE.tm_min  = minute;
	TVALUE.tm_sec  = second;

	time_t Time = mktime(&TVALUE);
	if (Time == -1) {
		throw invalid_argument("Invalid date");
	}
	auto TP = chrono::system_clock::from_time_t(Time);
	return TP + chrono::milliseconds(millisecond);
}

string FormatRemainingTime(chrono::milliseconds remaining_time) {
	auto Hours   = chrono::duration_cast<chrono::hours>(remaining_time);   remaining_time -= Hours;
	auto Minutes = chrono::duration_cast<chrono::minutes>(remaining_time); remaining_time -= Minutes;
	auto Seconds = chrono::duration_cast<chrono::seconds>(remaining_time); remaining_time -= Seconds;
	auto PillisecondsPart = remaining_time.count();

	stringstream ss;
	ss << setfill('0') << setw(2) << Hours.count() << ":" << 
		setfill('0') << setw(2) << Minutes.count() << ":" << 
		setfill('0') << setw(2) << Seconds.count() << "." << 
		setfill('0') << setw(3) << PillisecondsPart;
	return ss.str();
}

string TimeStatus = "-";
bool FireworksSystem::LogicEventLoopGame(GameLogic::FrameworkParams& RunningState) {
	// update & render actors.
	ActorEntities.Get()->UpdateManagerData();
	ActorEntities.Get()->RunAllGameActor();

	ParticleSystem.Get()->FxParticleRendering();

	ImGuiWindowFlags Flags = ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar;

	ImGui::SetNextWindowSize(ImVec2(640.0f, 120.0f));
	ImGui::SetNextWindowPos(ImVec2(25.0f, ImGui::GetIO().DisplaySize.y - 120.0f));
	ImGui::Begin("##SPECIAL", (bool*)NULL, Flags);
	ImGui::SetWindowFontScale(1.58f);
	{
		ImGui::TextColored(
			ImVec4(0.0f, 1.0f, 1.0f, 0.72f),
			"T: %s", FMT_TIME_STAMP(chrono::system_clock::now()).c_str()
		);
		// 计算时间差: 精度(毫秒).
		auto RemainingTime = chrono::duration_cast<chrono::milliseconds>(
			CreateTimePoint(2025, 1, 29, 0, 0, 0, 0) - chrono::system_clock::now()
		);
		// T - 50.5s player music.
		if (RemainingTime.count() < int64_t(50.5f * 1000.0f) && !MUSIC_FLAG) {
			MainMusicPlayer.Get()->AudioPlayerOperate()->SoundPlayer();
			MUSIC_FLAG = true;
		}
		if (RemainingTime.count() < 0) {
			FIRE_FLAG = true; TimeStatus = "+";
			RemainingTime = chrono::milliseconds(abs(RemainingTime.count()));
		}
		ImGui::TextColored(
			ImVec4(0.0f, 1.0f, 0.78f, 0.72f), 
			"T %s %s ms", TimeStatus.c_str(), FormatRemainingTime(RemainingTime).c_str()
		);
		ImGui::BeginChild("##PARAMS");
		ImGui::SetWindowFontScale(0.72f);
		ImGui::TextColored(
			ImVec4(0.7f, 0.7f, 0.7f, 0.58f),
			"Actors: %u, Particles: %u", 
			ActorEntities.Get()->GetSourceData()->size(),
			ParticleSystem.Get()->GetFxParticlesNumber()
		);
		ImGui::EndChild();
	}
	ImGui::End();

	if (FireCyclesTimer.Get()->CycleTimerGetFlag() && FIRE_FLAG) {
		CreateBaseFireworks(
			Vector2T<float>(PsagManager::Tools::Random::Func::GenerateRandomFuncFP32(180.0f, -180.0f), 500.0f), 
			PsagManager::Tools::Random::Func::GenerateRandomFuncFP32(3.0f, 2.25f)
		);
		FireCyclesTimer.Get()->CycleTimerClearReset(2500.0f);
	}
	return true;
}