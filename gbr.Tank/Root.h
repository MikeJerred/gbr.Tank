#pragma once

#include <Windows.h>
#include <thread>
#include <GWCA/GWCA.h>

#include "Controllers/ConsController.h"
#include "Controllers/RunController.h"
#include "Utilities/LogUtility.h"

namespace gbr::Tank {

	class Root {
	private:
		static Root* instance;
		static bool mustQuit;

		Controllers::ConsController* consController;
		Controllers::RunController* runController;

		Root(HMODULE hModule);
		~Root();
	public:
		static void ThreadStart(HMODULE hModule);
		static void Quit();
	};
}