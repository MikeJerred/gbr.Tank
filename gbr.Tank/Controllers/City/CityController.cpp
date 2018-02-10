#include <experimental/resumable>
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
#include "CityController.h"

namespace gbr::Tank::Controllers::City {
	using AgentUtility = Utilities::AgentUtility;
	using LogUtility = Utilities::LogUtility;
	using RunUtility = Utilities::RunUtility;

	CityController::CityController() {
	}

	CityController::~CityController() {
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

		GW::Agents::Dialog(0x82EF01);

		co_await Sleep(500);
		CheckForFail();

		//co_await WaitForBonds();

		co_await MaintainEnchants();


		// waypoints for 1st ball
		co_await RunUtility::FollowWaypointsWithoutStuck(
			std::vector<GW::GamePos> {
				GW::GamePos(-15565.0f, -9515.0f),
				GW::GamePos(-13999.0f, -10830.0f),
				GW::GamePos(-12017.0f, -11611.0f),
				GW::GamePos(-10630, -10588),
				GW::GamePos(-10500, -11528),
				GW::GamePos(-11720, -11568),
				GW::GamePos(-12306, -10222)
			},
			[]() { return MaintainEnchants(); },
			15000);

		// use dcharge
		auto ball = AgentUtility::GetBall(-11760, -10964);

		// find target near (-11760, -10964)

		// order eoe
		// order spike

		// waypoints for 2nd ball
		co_await RunUtility::FollowWaypointsWithoutStuck(
			std::vector<GW::GamePos> {
				GW::GamePos(0.0f, 0.0f),
			},
			[]() { return MaintainEnchants(); },
			15000);

		// order eoe
		// order spike

		// order kill city wall

		// do inside city

		// kill jadoth

		// order everyone to the chest

		// wait for all gems to be picked up
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

	awaitable<void> CityController::MaintainEnchants() {
		while (GW::Skillbar::GetPlayerSkillbar().Casting > 0) {
			co_await Sleep(50);

			CheckForFail();
		}

		auto sfEffect = GW::Effects::GetPlayerEffectById(GW::Constants::SkillID::Shadow_Form);

		if (sfEffect.SkillId == 0 || sfEffect.GetTimeRemaining() < 10000) {
			GW::SkillbarMgr::UseSkillByID((DWORD)GW::Constants::SkillID::I_Am_Unstoppable);
			co_await Sleep(100);

			CheckForFail();
		}

		if (sfEffect.SkillId == 0 || sfEffect.GetTimeRemaining() < 2000) {
			GW::SkillbarMgr::UseSkillByID((DWORD)GW::Constants::SkillID::Shadow_Form);
			co_await Sleep(500);

			CheckForFail();

			while (GW::Skillbar::GetPlayerSkillbar().Casting > 0) {
				co_await Sleep(50);

				CheckForFail();
			}
		}

		auto shroudEffect = GW::Effects::GetPlayerEffectById(GW::Constants::SkillID::Shroud_of_Distress);

		if (shroudEffect.SkillId == 0 || shroudEffect.GetTimeRemaining() < 2000) {
			GW::SkillbarMgr::UseSkillByID((DWORD)GW::Constants::SkillID::Shroud_of_Distress);
			co_await Sleep(500);

			CheckForFail();

			while (GW::Skillbar::GetPlayerSkillbar().Casting > 0) {
				co_await Sleep(50);

				CheckForFail();
			}
		}
	}

	void CityController::CheckForFail() {
		if (!GW::Agents::GetPlayer() || GW::PartyMgr::GetIsPartyDefeated())
			throw RunFailedException();

		if (GW::Agents::GetPlayer()->GetIsDead())
			throw PlayerDeadException();
	}
}