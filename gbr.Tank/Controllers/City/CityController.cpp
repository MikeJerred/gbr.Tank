#include <experimental/resumable>
#include <GWCA/GWCA.h>
#include <GWCA/Managers/AgentMgr.h>
#include <GWCA/Managers/EffectMgr.h>
#include <GWCA/Managers/GameThreadMgr.h>
#include <GWCA/Managers/MapMgr.h>
#include <GWCA/Managers/SkillbarMgr.h>

#include "../../Awaitable.h"
#include "../../Utilities/AgentUtility.h"
#include "../../Utilities/LogUtility.h"
#include "CityController.h"

namespace gbr::Tank::Controllers::City {
	using AgentUtility = Utilities::AgentUtility;
	using LogUtility = Utilities::LogUtility;

	CityController::CityController() {
	}

	CityController::~CityController() {
	}

	concurrency::task<bool> CityController::DoRun() {
		LogUtility::Log(L"Starting City");

		// take quest
		auto questSnake = AgentUtility::FindAgent(-17710, -8811, 2000, 4998);
		if (!questSnake) {
			LogUtility::Log(L"Quest snake not found!");
			return;
		}

		GW::Agents::GoNPC(questSnake);
		auto questSnakeId = questSnake->Id;

		while (GW::Agents::GetPlayer()->pos.SquaredDistanceTo(GW::Agents::GetAgentByID(questSnakeId)->pos) > GW::Constants::SqrRange::Adjacent) {
			co_await Sleep(500);

			if (DeadCheck())
				return false;
		}

		GW::Agents::Dialog(0x82EF01);

		co_await Sleep(500);

		if (DeadCheck())
			return false;

		if (!co_await WaitForBonds())
			return false;

		if (!co_await MaintainEnchants())
			return false;

		// waypoints for 1st ball

		// order eoe
		// order spike

		// waypoints for 2nd ball

		// order eoe
		// order spike

		// order kill city wall

		// do inside city

		// kill jadoth

		// order everyone to the chest

		return true;
	}

	concurrency::task<bool> CityController::WaitForBonds() {

		auto pbondEffect = GW::Effects::GetPlayerEffectById(GW::Constants::SkillID::Protective_Bond);
		auto lbondEffect = GW::Effects::GetPlayerEffectById(GW::Constants::SkillID::Life_Bond);
		auto balthEffect = GW::Effects::GetPlayerEffectById(GW::Constants::SkillID::Balthazars_Spirit);

		if (pbondEffect.SkillId == 0 || lbondEffect.SkillId == 0 || balthEffect.SkillId == 0) {
			co_await Sleep(500);

			if (DeadCheck())
				return false;

			// add checks to make sure bonder is in range
			// cancel recall if needed, or order team to move
		}
	}

	concurrency::task<bool> CityController::MaintainEnchants() {
		while (GW::Skillbar::GetPlayerSkillbar().Casting > 0) {
			co_await Sleep(50);

			if (DeadCheck())
				return false;
		}

		auto sfEffect = GW::Effects::GetPlayerEffectById(GW::Constants::SkillID::Shadow_Form);

		if (sfEffect.SkillId == 0 || sfEffect.GetTimeRemaining() < 10000) {
			GW::SkillbarMgr::UseSkillByID((DWORD)GW::Constants::SkillID::I_Am_Unstoppable);
			co_await Sleep(100);

			if (DeadCheck())
				return false;
		}

		if (sfEffect.SkillId == 0 || sfEffect.GetTimeRemaining() < 2000) {
			GW::SkillbarMgr::UseSkillByID((DWORD)GW::Constants::SkillID::Shadow_Form);
			co_await Sleep(500);

			if (DeadCheck())
				return false;

			while (GW::Skillbar::GetPlayerSkillbar().Casting > 0) {
				co_await Sleep(50);

				if (DeadCheck())
					return false;
			}
		}

		auto shroudEffect = GW::Effects::GetPlayerEffectById(GW::Constants::SkillID::Shroud_of_Distress);

		if (shroudEffect.SkillId == 0 || shroudEffect.GetTimeRemaining() < 2000) {
			GW::SkillbarMgr::UseSkillByID((DWORD)GW::Constants::SkillID::Shroud_of_Distress);
			co_await Sleep(500);

			if (DeadCheck())
				return false;

			while (GW::Skillbar::GetPlayerSkillbar().Casting > 0) {
				co_await Sleep(50);

				if (DeadCheck())
					return false;
			}
		}

		return true;
	}

	bool DeadCheck() {
		auto player = GW::Agents::GetPlayer();

		return !player || player->GetIsDead();
	}
}