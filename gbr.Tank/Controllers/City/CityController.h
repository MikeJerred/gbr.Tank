#pragma once

#include <vector>

#include <GWCA/GWCA.h>
#include <GWCA/GWStructures.h>

#include "CityWallAnalyzer.h"
#include "MargoAnalyzer.h"

namespace gbr::Tank::Controllers::City {
	class CityController {
	private:
		static const std::vector<GW::GamePos> TeamWaypoints;
		static const std::vector<GW::GamePos> CityWallPositions;
		CityWallAnalyzer* _cityWallAnalyzer;
		MargoAnalyzer* _margoAnalyzer;
	public:
		CityController();
		~CityController();

		awaitable<void> DoRun();
		awaitable<void> WaitForBonds();
		awaitable<void> Maintenence();
		awaitable<void> SpikeCityWall();
		awaitable<void> MaintainEnchants();
		bool KeepBonderInRange();

		void CheckForFail();
	};
}