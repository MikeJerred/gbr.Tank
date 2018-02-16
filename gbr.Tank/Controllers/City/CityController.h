#pragma once

#include <vector>

#include <GWCA/GWCA.h>
#include <GWCA/GWStructures.h>

#include "MargoAnalyzer.h"

namespace gbr::Tank::Controllers::City {
	class CityController {
	private:
		static std::vector<GW::GamePos> TeamWaypoints;
		MargoAnalyzer* _margoAnalyzer;
	public:
		CityController();
		~CityController();

		awaitable<void> DoRun();
		awaitable<void> WaitForBonds();
		awaitable<void> Maintenence();
		awaitable<void> MaintainEnchants();
		bool KeepBonderInRange();

		void CheckForFail();
	};
}