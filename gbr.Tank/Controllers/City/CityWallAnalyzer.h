#pragma once

#include <map>

#include <GWCA/GWCA.h>
#include <GWCA/GWStructures.h>

namespace gbr::Tank::Controllers::City {
	class CityWallAnalyzer {
	private:
		static const std::vector<std::tuple<GW::GamePos, GW::GamePos>> Positions;

		struct CityWallEnemy {
			GW::GamePos position;
			GW::GamePos killPosition;
			bool isAlive;

			CityWallEnemy(GW::GamePos position, GW::GamePos killPosition, bool isAlive) : position(position), killPosition(killPosition), isAlive(isAlive) {
			}
		};

		DWORD _hookId;
		std::vector<CityWallEnemy> _enemies;
	public:
		CityWallAnalyzer();
		~CityWallAnalyzer();

		void Tick();
		bool IsWallDead();
		CityWallEnemy* GetNextWallEnemy();

		std::vector<DWORD> GetAllLivingEnemies();
	};
}