#include <GWCA/GWCA.h>
#include <GWCA/Managers/AgentMgr.h>
#include <GWCA/Managers/EffectMgr.h>
#include <GWCA/Managers/GameThreadMgr.h>
#include <GWCA/Managers/MapMgr.h>
#include <GWCA/Managers/PartyMgr.h>
#include <GWCA/Managers/SkillbarMgr.h>

#include "../../Awaitable.h"
#include "../../Exceptions.h"
#include "../../Utilities/AgentUtility.h"
#include "../../Utilities/LogUtility.h"
#include "../../Utilities/RunUtility.h"
#include "../../Utilities/SkillUtility.h"
#include "../../Utilities/TeamUtility.h"
#include "MargoAnalyzer.h"
#include "CityController.h"

namespace gbr::Tank::Controllers::City {
	using AgentUtility = Utilities::AgentUtility;
	using LogUtility = Utilities::LogUtility;
	using RunUtility = Utilities::RunUtility;
	using SkillUtility = Utilities::SkillUtility;
	using TeamUtility = Utilities::TeamUtility;

	const std::vector<GW::GamePos> CityController::TeamWaypoints {
		GW::GamePos(-16700, -9805),
		GW::GamePos(-16214, -9805),
		GW::GamePos(-15825, -10046),
		GW::GamePos(-15736, -10369),
		GW::GamePos(-15577, -10853),
		GW::GamePos(-15320, -11417),
		GW::GamePos(-15150, -11806),
		GW::GamePos(-14983, -12094),
		GW::GamePos(-14763, -12264),
		GW::GamePos(-14038, -12690),
		GW::GamePos(-12981, -12835),
		// spike 1st ball
		GW::GamePos(-11718, -12537),
		GW::GamePos(-10377, -12037),
		GW::GamePos(-9031, -11681),
		GW::GamePos(-7846, -11536),
		GW::GamePos(-6805, -11514),
		GW::GamePos(-5865, -11495)
	};

	const std::vector<GW::GamePos> CityController::CityWallPositions {
		GW::GamePos(-9826, -12092),
		GW::GamePos(-8884, -11889),
		GW::GamePos(-8173, -11730),
		GW::GamePos(-7449, -11720),
		GW::GamePos(-7131, -11688),
		GW::GamePos(-5733, -11719),
		GW::GamePos(-5402, -11672),
		GW::GamePos(-4721, -11643),
		GW::GamePos(-3957, -11686),
		GW::GamePos(-3034, -11695)
	};

	CityController::CityController() {
		_cityWallAnalyzer = new CityWallAnalyzer();
		_margoAnalyzer = new MargoAnalyzer();
	}

	CityController::~CityController() {
		if (_cityWallAnalyzer)
			delete _cityWallAnalyzer;

		if (_margoAnalyzer)
			delete _margoAnalyzer;
	}

	awaitable<void> CityController::DoRun() {
		LogUtility::Log(L"Starting City");

		co_await Sleep(3000);

		// take quest
		auto questSnake = AgentUtility::FindAgent(-17710, -8811, 2000, 4998);
		if (!questSnake) {
			LogUtility::Log(L"Quest snake not found!");
			throw RunFailedException();
		}

		GW::Agents::GoNPC(questSnake);
		auto questSnakeId = questSnake->Id;

		while (GW::Agents::GetPlayer()->pos.SquaredDistanceTo(GW::Agents::GetAgentByID(questSnakeId)->pos) > GW::Constants::SqrRange::Adjacent) {
			co_await Sleep(500);
			CheckForFail();
		}

		co_await Sleep(2000);
		CheckForFail();

		LogUtility::Log(L"Taking quest");
		GW::Agents::Dialog(0x82EF01);

		co_await Sleep(500);
		CheckForFail();

		LogUtility::Log(L"Waiting for bonds");
		//co_await WaitForBonds();

		co_await MaintainEnchants();

		// waypoints for 1st ball
		LogUtility::Log(L"Starting 1st ball");
		co_await RunUtility::FollowWaypointsWithoutStuck(
			std::vector<GW::GamePos> {
				GW::GamePos(-16633, -9583),
				GW::GamePos(-15973, -9520),
				GW::GamePos(-15287, -9644),
				GW::GamePos(-14644, -9903),
				GW::GamePos(-14265, -10549),
				GW::GamePos(-13723, -10843),
				GW::GamePos(-13038, -10688),
				GW::GamePos(-12479, -10946),
				GW::GamePos(-11928, -11134),
				GW::GamePos(-11439, -11083),
				GW::GamePos(-11059, -10646),
				GW::GamePos(-10881, -10500),
				GW::GamePos(-10772, -10517)
			},
			[=]() { return Maintenence(); },
			5000);

		LogUtility::Log(L"Waiting for any tuks to come in");
		co_await Sleep(5000, [=]() { return Maintenence(); });

		co_await RunUtility::FollowWaypointsWithoutStuck(
			std::vector<GW::GamePos> {
				GW::GamePos(-10762, -10760),
				GW::GamePos(-10872, -11217),
				GW::GamePos(-11425, -11497),
				GW::GamePos(-12030, -11219),
				GW::GamePos(-12371, -10889),
				GW::GamePos(-12347, -10428),
				GW::GamePos(-12336, -10051),
				GW::GamePos(-12436, -9874)
			},
			[=]() { return Maintenence(); },
			5000);

		// order eoe
		LogUtility::Log(L"Order EoE");
		auto eoeTask = TeamUtility::PlaceEoe(-12520, -12575, [=]() { return Maintenence(); });

		co_await Sleep(3000, [=]() { return Maintenence(); });

		// use dcharge
		auto ball = Utilities::Ball(-11760, -10964);

		LogUtility::Log(L"Jump into ball");
		while (GW::Agents::GetPlayer()->pos.SquaredDistanceTo(ball.GetCentralTarget()->pos) > GW::Constants::SqrRange::Nearby) {
			SkillUtility::TryUseSkill(GW::Constants::SkillID::Deaths_Charge, ball.GetCentralTarget()->Id);
			co_await Sleep(100);
			co_await Maintenence();
		}

		// wait for eoe
		LogUtility::Log(L"Wait for EoE");
		co_await eoeTask;

		// bring the bonder a bit closer so he can heal the spikers
		LogUtility::Log(L"Move team for spike");
		TeamUtility::Move(-12650, -12347);
		co_await Sleep(3000, [=]() { return Maintenence(); });

		LogUtility::Log(L"Waiting for enemies to settle");
		co_await AgentUtility::WaitForSettle([=]() { return Maintenence(); });

		// order spike
		LogUtility::Log(L"Spiking");
		co_await TeamUtility::Spike(ball.GetCentralTarget()->Id, [=]() { return Maintenence(); });

		LogUtility::Log(L"Cleaning up");
		co_await TeamUtility::CleanupEnemies(GW::Agents::GetPlayer()->X, GW::Agents::GetPlayer()->Y, [=]() { return Maintenence(); });


		// collect any drops
		LogUtility::Log(L"Collecting drops");
		TeamUtility::Move(-11760, -10964);
		co_await Sleep(4000);
		LogUtility::Log(L"Waiting for bonds");
		co_await WaitForBonds();

		// waypoints for 2nd ball
		// todo: ensure that each group is aggroed: often the 1st group is missed at the moment
		// when balling, another pass needs to be made to pull the runds in
		LogUtility::Log(L"Starting 2nd ball");
		co_await RunUtility::FollowWaypointsWithoutStuck(
			std::vector<GW::GamePos> {
				GW::GamePos(-10232, -10986),
				GW::GamePos(-9253, -10304),
				GW::GamePos(-8842, -9791),
				GW::GamePos(-8581, -9424)
			},
			[=]() { return Maintenence(); },
			5000);

		while (!_margoAnalyzer->AllGroupsInRangeAggroed(-9728, -9462, GW::Constants::SqrRange::Spirit)) {
			co_await Sleep(100);
			co_await Maintenence();
		}

		co_await RunUtility::FollowWaypointsWithoutStuck(
			std::vector<GW::GamePos> {
				GW::GamePos(-9029, -10340),
				GW::GamePos(-8726, -10780),
				GW::GamePos(-8028, -10960),
				GW::GamePos(-7114, -10910),
				GW::GamePos(-6166, -10904),
				GW::GamePos(-5208, -10982),
				GW::GamePos(-4499, -11001)
			},
			[=]() { return Maintenence(); },
			5000);

		while (!_margoAnalyzer->AllGroupsInRangeAggroed(-3791, -11335, GW::Constants::SqrRange::Spirit)) {
			co_await Sleep(100);
			co_await Maintenence();
		}

		co_await RunUtility::FollowWaypointsWithoutStuck(
			std::vector<GW::GamePos> {
				GW::GamePos(-4829, -10306),
				GW::GamePos(-5477, -9982),
				GW::GamePos(-5873, -9549),
				GW::GamePos(-6018, -8802),
				GW::GamePos(-6290, -8481),
				GW::GamePos(-6867, -8033),
				GW::GamePos(-7234, -7367),
				GW::GamePos(-7439, -7647),
				GW::GamePos(-7290, -8028),
				GW::GamePos(-6888, -8028),
				GW::GamePos(-6462, -8093),
				GW::GamePos(-6083, -8371),
				GW::GamePos(-5726, -8768)
			},
			[=]() { return Maintenence(); },
			5000);

		co_await Sleep(4000, [=]() { return Maintenence(); });

		co_await RunUtility::FollowWaypointsWithoutStuck(
			std::vector<GW::GamePos> {
				GW::GamePos(-5990, -9269),
				GW::GamePos(-6315, -9678),
				GW::GamePos(-6683, -9966),
				GW::GamePos(-7030, -9767),
				GW::GamePos(-7459, -9736)
			},
			[=]() { return Maintenence(); },
			5000);

		co_await Sleep(2000, [=]() { return Maintenence(); });

		co_await RunUtility::FollowWaypointsWithoutStuck(
			std::vector<GW::GamePos> {
				GW::GamePos(-7229, -10029),
				GW::GamePos(-6552, -9912),
				GW::GamePos(-6003, -9584),
				GW::GamePos(-5999, -8910),
				GW::GamePos(-6226, -8542),
				GW::GamePos(-5784, -8315)
			},
			[=]() { return Maintenence(); },
			5000);

		// order eoe
		LogUtility::Log(L"Order EoE");
		eoeTask = TeamUtility::PlaceEoe(-7995, -10059, [=]() { return Maintenence(); });

		co_await Sleep(3000, [=]() { return Maintenence(); });

		ball = Utilities::Ball(-6223, -9121);

		LogUtility::Log(L"Jumping into ball");
		while (GW::Agents::GetPlayer()->pos.SquaredDistanceTo(ball.GetCentralTarget()->pos) > GW::Constants::SqrRange::Nearby) {
			SkillUtility::TryUseSkill(GW::Constants::SkillID::Deaths_Charge, ball.GetCentralTarget()->Id);
			co_await Sleep(100);
			co_await Maintenence();
		}

		// wait for eoe
		LogUtility::Log(L"Wait for EoE");
		co_await eoeTask;

		// bring the bonder a bit closer so he can heal the spikers
		LogUtility::Log(L"Move team for spike");
		TeamUtility::Move(-7211, -9890);
		co_await Sleep(3000, [=]() { return Maintenence(); });

		LogUtility::Log(L"Waiting for enemies to settle");
		co_await AgentUtility::WaitForSettle([=]() { return Maintenence(); });

		// order spike
		LogUtility::Log(L"Spiking");
		co_await TeamUtility::Spike(ball.GetCentralTarget()->Id, [=]() { return Maintenence(); });

		LogUtility::Log(L"Cleaning up");
		co_await TeamUtility::CleanupEnemies(GW::Agents::GetPlayer()->X, GW::Agents::GetPlayer()->Y, [=]() { return Maintenence(); });

		// collect any drops
		LogUtility::Log(L"Collecting drops");
		TeamUtility::Move(-6223, -9121);
		co_await Sleep(4000);
		LogUtility::Log(L"Waiting for bonds");
		co_await WaitForBonds();


		// run to gate
		co_await RunUtility::FollowWaypointsWithoutStuck(
			std::vector<GW::GamePos> { GW::GamePos(-6423, -11595) },
			[=]() { return Maintenence(); },
			5000);

		// order kill city wall
		while (!_cityWallAnalyzer->IsWallDead()) {
			auto enemy = _cityWallAnalyzer->GetNextWallEnemy();

			if (enemy) {
				auto agent = AgentUtility::FindAgent(enemy->position.x, enemy->position.y, GW::Constants::Range::Adjacent);
				if (agent) {
					auto mesmer = TeamUtility::GetMesmer();
					if (mesmer && mesmer->pos.SquaredDistanceTo(agent->pos) < GW::Constants::SqrRange::Spellcast) {
						co_await TeamUtility::Spike(agent->Id, [=]() { return Maintenence(); });
						continue;
					}
				}

				TeamUtility::Move(enemy->killPosition.x, enemy->killPosition.y);
			}
		}

		// do inside city

		// kill jadoth

		// order everyone to the chest

		// wait for all gems to be picked up
		LogUtility::Log(L"City done!");
	}

	awaitable<void> CityController::WaitForBonds() {
		while (GW::Effects::GetPlayerEffectById(GW::Constants::SkillID::Protective_Bond).SkillId == 0
			|| GW::Effects::GetPlayerEffectById(GW::Constants::SkillID::Life_Bond).SkillId == 0
			|| GW::Effects::GetPlayerEffectById(GW::Constants::SkillID::Balthazars_Spirit).SkillId == 0) {

			co_await Sleep(500);

			CheckForFail();

			// add checks to make sure bonder is in range
			// cancel recall if needed, or order team to move
		}
	}

	awaitable<void> CityController::Maintenence() {
		co_await MaintainEnchants();

		if (!KeepBonderInRange()) {
			LogUtility::Log(L"Waiting for bonder to stay in range");
			AgentUtility::StopMoving();

			while (!KeepBonderInRange()) {
				co_await Sleep(100);
				co_await MaintainEnchants();
			}
		}

		if (_margoAnalyzer->PlayerShouldWait()) {
			LogUtility::Log(L"Waiting for balled group");
			AgentUtility::StopMoving();

			while (_margoAnalyzer->PlayerShouldWait()) {
				co_await Sleep(100);
				co_await MaintainEnchants();
				co_await SpikeCityWall();

				if ((GW::Agents::GetPlayer()->HP * GW::Agents::GetPlayer()->MaxHP) < 120) {
					LogUtility::Log(L"Requesting seed");
					TeamUtility::Seed();
					co_await Sleep(100);
				}
			}

			LogUtility::Log(L"Finished waiting for balled group");
		}

		co_await SpikeCityWall();

		if ((GW::Agents::GetPlayer()->HP * GW::Agents::GetPlayer()->MaxHP) < 120) {
			LogUtility::Log(L"Requesting seed");
			TeamUtility::Seed();
			co_await Sleep(100);
		}
	}

	awaitable<void> CityController::SpikeCityWall() {
		auto mesmer = TeamUtility::GetMesmer();
		if (!mesmer)
			co_return;

		auto teamPos = mesmer->pos;

		for (auto cityWallPos : CityWallPositions) {
			if (cityWallPos.SquaredDistanceTo(teamPos) < GW::Constants::SqrRange::Spellcast) {
				auto enemies = AgentUtility::GetEnemiesInRange(cityWallPos.x, cityWallPos.y, GW::Constants::Range::Adjacent);

				if (enemies.size() == 0)
					continue;

				TeamUtility::Spike(enemies[0]->Id, nullptr);
				co_return;
			}
		}
	}

	awaitable<void> CityController::MaintainEnchants() {
		while (GW::Agents::GetPlayer()->Skill > 0) {
			co_await Sleep(100);
			CheckForFail();
		}

		auto sfEffect = GW::Effects::GetPlayerEffectById(GW::Constants::SkillID::Shadow_Form);

		if (sfEffect.SkillId == 0 || sfEffect.GetTimeRemaining() < 10000) {
			SkillUtility::TryUseSkill(GW::Constants::SkillID::I_Am_Unstoppable, 0);
		}

		if (sfEffect.SkillId == 0 || sfEffect.GetTimeRemaining() < 3000) {
			auto qz = GW::Effects::GetPlayerEffectById(GW::Constants::SkillID::Quickening_Zephyr);
			auto bu = GW::Effects::GetPlayerEffectById(GW::Constants::SkillID::Essence_of_Celerity_item_effect);
			if (bu.SkillId == 0 && (qz.SkillId == 0 || qz.GetTimeRemaining() < 3000)) {
				SkillUtility::TryUseSkill(GW::Constants::SkillID::Deadly_Paradox, 0);
			}

			SkillUtility::TryUseSkill(GW::Constants::SkillID::Shadow_Form, 0);
			co_await Sleep(500);
			CheckForFail();

			while (GW::Agents::GetPlayer()->Skill > 0) {
				co_await Sleep(100);
				CheckForFail();
			}

			co_await Sleep(200);
			CheckForFail();
		}

		auto shroudEffect = GW::Effects::GetPlayerEffectById(GW::Constants::SkillID::Shroud_of_Distress);

		if (shroudEffect.SkillId == 0 || shroudEffect.GetTimeRemaining() < 3000) {
			SkillUtility::TryUseSkill(GW::Constants::SkillID::Shroud_of_Distress, 0);

			co_await Sleep(500);
			CheckForFail();

			while (GW::Agents::GetPlayer()->Skill > 0) {
				co_await Sleep(100);
				CheckForFail();
			}

			co_await Sleep(200);
			CheckForFail();
		}

		if (GW::Effects::GetPlayerEffectById(GW::Constants::SkillID::Quickening_Zephyr).SkillId > 0) {
			SkillUtility::TryUseSkill(GW::Constants::SkillID::I_Am_Unstoppable, 0);
		}
	}

	// return false if we need to stop moving and wait
	bool CityController::KeepBonderInRange() {

		auto bonder = TeamUtility::GetBonder();
		if (!bonder)
			return true;

		auto distance = bonder->pos.DistanceTo(GW::Agents::GetPlayer()->pos);

		if (distance > 4000) {
			auto wallEnemies = _cityWallAnalyzer->GetAllLivingEnemies();

			for (auto waypoint : TeamWaypoints) {
				if (waypoint.DistanceTo(GW::Agents::GetPlayer()->pos) < 3800) {
					auto enemiesInRange = AgentUtility::GetEnemiesInRange(waypoint.x, waypoint.y, GW::Constants::Range::Spellcast);

					if (std::all_of(enemiesInRange.begin(), enemiesInRange.end(), [&](GW::Agent* agent) {
						return std::any_of(wallEnemies.begin(), wallEnemies.end(), [=](DWORD id) { return agent->Id == id; });
					})) {
						TeamUtility::Move(waypoint.x, waypoint.y);

						break;
					}
				}
			}

			if (distance > 4200) {
				return false;
			}
		}

		return true;
	}

	void CityController::CheckForFail() {
		if (!GW::Agents::GetPlayer() || GW::PartyMgr::GetIsPartyDefeated())
			throw RunFailedException();

		//if (GW::Agents::GetPlayer()->GetIsDead())
		//	throw PlayerDeadException();
	}
}