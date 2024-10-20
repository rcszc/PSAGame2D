// psagame_dev_test.
#include "psagame_dev_test.h"

using namespace std;
using namespace PSAG_LOGGER;

ThreadSafeResource<RunSettingLimitParams> GLOBAL_SAFE_RESOURCE::RunThreadParams = {};
ThreadSafeResource<std::string>           GLOBAL_SAFE_RESOURCE::RunThreadStrLog = {};

atomic<bool> GLOBAL_SAFE_RESOURCE::ThreadFlagStart = false;
atomic<bool> GLOBAL_SAFE_RESOURCE::ThreadFlagExit  = true;

atomic<size_t> GLOBAL_SAFE_RESOURCE::RunThreadTickCount = NULL;
atomic<size_t> GLOBAL_SAFE_RESOURCE::RunThreadDataCount = NULL;

PsagManager::GuiTools::PlotSmpDataSRC TestDataPoints = {};

bool PsaGameDevTest::LogicInitialization(const Vector2T<uint32_t>& WinSize) {

	MainControlPanel = new PSA_PANEL::MainControlPanel();

	TestImGuiPlot = new PsagManager::GuiTools::ImMegaPlotDataView();

	for (size_t set = 0; set < 12; ++set) {
		vector<float> TempData = {};
		for (size_t i = 0; i < 768000; ++i)
			TempData.push_back(sin((float)i * 0.000012f + (float)set * 12.0f) * 1050.0f + 1500.0f);
		TestDataPoints.push_back(TempData);
	}

	*TestImGuiPlot->GetPlotDatasetPtr() = TestDataPoints;

	return true;
}

void PsaGameDevTest::LogicCloseFree() {

	delete TestImGuiPlot;
	delete MainControlPanel;
}

bool PsaGameDevTest::LogicEventLoopGame(GameLogic::FrameworkParams& RunningState) {

	MainControlPanel->RenderPanel();

	TestImGuiPlot->DrawImGuiDataPlot("TEST_PLOT");

	return true;
}