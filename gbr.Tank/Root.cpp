#include <GWCA/GWCA.h>
#include <GWCA/Managers/AgentMgr.h>
#include <GWCA/Managers/GameThreadMgr.h>

#include "Root.h"

namespace gbr::Tank {
	Root* Root::instance = nullptr;
	bool Root::mustQuit = false;

	Root::Root(HMODULE hModule) {
		auto playerName = GW::Agents::GetPlayerNameByLoginNumber(GW::Agents::GetPlayer()->LoginNumber);

		Utilities::LogUtility::Init(playerName);

		//consController = new Controllers::ConsController();
		runController = new Controllers::RunController();

		mustQuit = false;
	}

	Root::~Root() {
		if (consController)
			delete consController;

		if (runController)
			delete runController;

		Utilities::LogUtility::Close();
	}

	void Root::ThreadStart(HMODULE hModule) {
		if (instance) {
			MessageBoxA(0, "Already loaded.", "gbr Error", 0);
			FreeLibraryAndExitThread(hModule, EXIT_SUCCESS);
		}

		if (!GW::Initialize()) {
			MessageBoxA(0, "Failed to load GWCA.", "gbr Error", 0);
			FreeLibraryAndExitThread(hModule, EXIT_SUCCESS);
		}

		instance = new Root(hModule);

		//GW::GameThread::ToggleRenderHook();

		while (!mustQuit) {
			Sleep(250);

			if (GetAsyncKeyState(VK_END) & 1) {
				//mustQuit = true;
			}

			if (GetAsyncKeyState(VK_HOME) & 1) {
				GW::GameThread::ToggleRenderHook();
				Sleep(2000);
			}
		}

		if (instance)
			delete instance;

		std::this_thread::sleep_for(std::chrono::seconds(1));

		GW::Terminate();
		FreeLibraryAndExitThread(hModule, EXIT_SUCCESS);
	}

	void Root::Quit() {
		mustQuit = true;
	}
}