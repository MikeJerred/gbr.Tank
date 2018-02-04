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
		_maintainEnchantsHookId = GW::GameThread::AddPermanentCall([=]() {
			auto currentTick = GetTickCount();
			static auto nextTick = 0;

			if (currentTick > nextTick) {
				CityController::MaintainEnchants();
				nextTick = currentTick + 500;
			}
		});
	}

	CityController::~CityController() {
		if (_maintainEnchantsHookId > 0)
			GW::GameThread::RemovePermanentCall(_maintainEnchantsHookId);
	}

	concurrency::task<void> CityController::DoRun() {
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
		}

		GW::Agents::Dialog(0x82EF01);

		co_await Sleep(500);

		// wait for bonds

		// put up shroud and SF

		// waypoints for 1st ball

		// order eoe
		// order spike

		// waypoints for 2nd ball

		// order eoe
		// order spike

		// order kill city wall
	}

	void CityController::MaintainEnchants() {
		if (GW::Map::GetInstanceType() != GW::Constants::InstanceType::Explorable
			|| GW::Skillbar::GetPlayerSkillbar().Casting > 0)
			return;

		auto sfEffect = GW::Effects::GetPlayerEffectById(GW::Constants::SkillID::Shadow_Form);

		if (sfEffect.SkillId == 0 || sfEffect.GetTimeRemaining() < 2000) {
			GW::SkillbarMgr::UseSkillByID((DWORD)GW::Constants::SkillID::Shadow_Form);
			return;
		}

		auto shroudEffect = GW::Effects::GetPlayerEffectById(GW::Constants::SkillID::Shroud_of_Distress);

		if (shroudEffect.SkillId == 0 || shroudEffect.GetTimeRemaining() < 2000) {
			GW::SkillbarMgr::UseSkillByID((DWORD)GW::Constants::SkillID::Shroud_of_Distress);
			return;
		}
	}
}