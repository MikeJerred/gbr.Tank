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

	std::vector<GW::GamePos> CityController::TeamWaypoints {
		GW::GamePos(-16700, -9805),
		GW::GamePos(-15605, -10391),
		GW::GamePos(-15232, -11460),
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

	CityController::CityController() {
		_margoAnalyzer = new MargoAnalyzer();
	}

	CityController::~CityController() {
		if (_margoAnalyzer)
			delete _margoAnalyzer;
	}

	awaitable<void> CityController::DoRun() {
		LogUtility::Log(L"Starting City");

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
		co_await WaitForBonds();

		co_await MaintainEnchants();

		// waypoints for 1st ball
		LogUtility::Log(L"Starting 1st ball");
		co_await RunUtility::FollowWaypointsWithoutStuck(
			std::vector<GW::GamePos> {
				GW::GamePos(-15565.0f, -9515.0f),
				GW::GamePos(-13999.0f, -10830.0f),
				GW::GamePos(-12017.0f, -11611.0f),
				GW::GamePos(-11232, -11420),
				GW::GamePos(-10881, -10500)

				//GW::GamePos(-10850, -10918)
			},
			[=]() { return Maintenence(); },
			5000);

		LogUtility::Log(L"Waiting for any tuks to come in");
		co_await Sleep(5000, [=]() { return Maintenence(); });

		co_await RunUtility::FollowWaypointsWithoutStuck(
			std::vector<GW::GamePos> {
				GW::GamePos(-10858, -11077),
				GW::GamePos(-11720, -11568),
				GW::GamePos(-12336, -10051)
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
		while (!SkillUtility::TryUseSkill(GW::Constants::SkillID::Deaths_Charge, ball.GetCentralTarget()->Id)) {
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


		// waypoints for 2nd ball
		LogUtility::Log(L"Starting 2nd ball");
		co_await RunUtility::FollowWaypointsWithoutStuck(
			std::vector<GW::GamePos> {
				GW::GamePos(-10232, -10986),
				GW::GamePos(-9253, -10304),
				GW::GamePos(-8265, -10799),
				GW::GamePos(-7277, -10088),
				GW::GamePos(-6031, -10607),
				GW::GamePos(-5014, -10890),
				GW::GamePos(-4976, -9722),
				GW::GamePos(-5533, -8860),
				GW::GamePos(-6870, -7917),
				GW::GamePos(-7037, -8188),
				GW::GamePos(-6540, -8314),
				GW::GamePos(-6126, -8276),
				GW::GamePos(-5646, -8857),
				GW::GamePos(-5992, -9503),
				GW::GamePos(-7040, -9634)
			},
			[=]() { return Maintenence(); },
			5000);

		// order eoe
		LogUtility::Log(L"Order EoE");
		eoeTask = TeamUtility::PlaceEoe(-7995, -10059, [=]() { return Maintenence(); });

		co_await Sleep(3000, [=]() { return Maintenence(); });

		ball = Utilities::Ball(-6223, -9121);

		LogUtility::Log(L"Jumping into ball");
		while (!SkillUtility::TryUseSkill(GW::Constants::SkillID::Deaths_Charge, ball.GetCentralTarget()->Id)) {
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


		// order kill city wall

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
			// stop moving
			GW::Agents::Move(GW::Agents::GetPlayer()->pos);

			while (!KeepBonderInRange()) {
				co_await Sleep(100);
				co_await MaintainEnchants();
			}
		}

		if (_margoAnalyzer->PlayerShouldWait()) {
			// stop moving
			GW::Agents::Move(GW::Agents::GetPlayer()->pos);

			while (_margoAnalyzer->PlayerShouldWait()) {
				co_await Sleep(100);
				co_await MaintainEnchants();
			}
		}
	}

	awaitable<void> CityController::MaintainEnchants() {
		while (GW::Skillbar::GetPlayerSkillbar().Casting > 0) {
			co_await Sleep(50);
			CheckForFail();
		}

		auto sfEffect = GW::Effects::GetPlayerEffectById(GW::Constants::SkillID::Shadow_Form);

		if (sfEffect.SkillId == 0 || sfEffect.GetTimeRemaining() < 10000) {
			if (SkillUtility::TryUseSkill(GW::Constants::SkillID::I_Am_Unstoppable, 0)) {

				co_await Sleep(100);
				CheckForFail();
			}
		}

		if (sfEffect.SkillId == 0 || sfEffect.GetTimeRemaining() < 2000) {
			SkillUtility::TryUseSkill(GW::Constants::SkillID::Deadly_Paradox, 0);
			SkillUtility::TryUseSkill(GW::Constants::SkillID::Shadow_Form, 0);
			co_await Sleep(200);
			CheckForFail();

			while (GW::Skillbar::GetPlayerSkillbar().Casting > 0) {
				co_await Sleep(50);
				CheckForFail();
			}
		}

		auto shroudEffect = GW::Effects::GetPlayerEffectById(GW::Constants::SkillID::Shroud_of_Distress);

		if (shroudEffect.SkillId == 0 || shroudEffect.GetTimeRemaining() < 2000) {
			SkillUtility::TryUseSkill(GW::Constants::SkillID::Shroud_of_Distress, 0);

			co_await Sleep(200);
			CheckForFail();

			while (GW::Skillbar::GetPlayerSkillbar().Casting > 0) {
				co_await Sleep(50);
				CheckForFail();
			}
		}

		if (GW::Effects::GetPlayerEffectById(GW::Constants::SkillID::Quickening_Zephyr).SkillId > 0) {
			if (SkillUtility::TryUseSkill(GW::Constants::SkillID::I_Am_Unstoppable, 0)) {

				co_await Sleep(100);
				CheckForFail();
			}
		}
	}

	// return false if we need to stop moving and wait
	bool CityController::KeepBonderInRange() {

		auto bonder = TeamUtility::GetBonder();
		auto distance = bonder->pos.DistanceTo(GW::Agents::GetPlayer()->pos);

		if (distance > 4000) {
			for (auto waypoint : TeamWaypoints) {
				if (waypoint.DistanceTo(GW::Agents::GetPlayer()->pos) < 3800) {
					if (AgentUtility::GetEnemiesInRange(waypoint.x, waypoint.y, GW::Constants::Range::Spellcast).size() <= 1) {
						TeamUtility::Move(waypoint.x, waypoint.y);
					}
					break;
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