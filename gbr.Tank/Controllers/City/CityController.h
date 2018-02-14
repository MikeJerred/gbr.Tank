#pragma once

#include <vector>

#include <GWCA/GWCA.h>
#include <GWCA/GWStructures.h>

namespace gbr::Tank::Controllers::City {
	class CityController {
	private:
		static std::vector<GW::GamePos> TeamWaypoints;
		MargoAnalyzer* _margoAnalyzer;
	public:
		CityController();
		~CityController();

		static awaitable<void> DoRun();
		static awaitable<void> WaitForBonds();
		static awaitable<void> Maintenence();
		static awaitable<void> MaintainEnchants();
		static bool KeepBonderInRange();

		static void CheckForFail();
	};
}