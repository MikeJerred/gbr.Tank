#include <GWCA/GWCA.h>
#include <GWCA/Managers/AgentMgr.h>
#include <GWCA/Managers/EffectMgr.h>
#include <GWCA/Managers/GameThreadMgr.h>
#include <GWCA/Managers/ItemMgr.h>
#include <GWCA/Managers/MapMgr.h>
#include <GWCA/Managers/PartyMgr.h>

#include "ConsController.h"

namespace gbr::Tank::Controllers {
	ConsController::ConsController() {
		hookId = GW::GameThread::AddPermanentCall([this]() {
			Tick();
		});
	}

	ConsController::~ConsController() {
		GW::GameThread::RemovePermanentCall(hookId);
	}

	void ConsController::Tick() {
		auto currentTick = GetTickCount();
		if (sleepUntilTick - currentTick > 0)
			return;

		if (GW::Map::GetInstanceType() != GW::Constants::InstanceType::Explorable
			|| !GW::PartyMgr::GetIsPartyLoaded()
			|| !IsPartyAlive())
			return;

		auto player = GW::Agents::GetPlayer();

		if (!player || player->GetIsDead())
			return;

		bool shouldSleep = false;

		if (ShouldUseConsumable((DWORD)GW::Constants::SkillID::Essence_of_Celerity_item_effect)) {
			shouldSleep = shouldSleep || UseConsumable(GW::Constants::ItemID::ConsEssence);
		}
		if (ShouldUseConsumable((DWORD)GW::Constants::SkillID::Grail_of_Might_item_effect)) {
			shouldSleep = shouldSleep || UseConsumable(GW::Constants::ItemID::ConsGrail);
		}
		if (ShouldUseConsumable((DWORD)GW::Constants::SkillID::Armor_of_Salvation_item_effect)) {
			shouldSleep = shouldSleep || UseConsumable(GW::Constants::ItemID::ConsArmor);
		}

		if (shouldSleep) {
			sleepUntilTick = GetTickCount() + 5000;
		}
	}

	bool ConsController::IsPartyAlive() {
		auto mapAgents = GW::Agents::GetMapAgentArray();
		if (!mapAgents.valid())
			return false;

		auto playerCount = GW::Agents::GetAmountOfPlayersInInstance();
		for (DWORD i = 1; i <= playerCount; ++i) {
			auto agentId = GW::Agents::GetAgentIdByLoginNumber(i);
			if (agentId <= 0 || agentId >= mapAgents.size() || mapAgents[agentId].GetIsDead())
				return false;
		}

		return true;
	}

	bool ConsController::ShouldUseConsumable(DWORD skillId) {
		auto agentEffects = GW::Effects::GetPartyEffectArray();
		if (!agentEffects.valid())
			return false;

		auto playerEffects = agentEffects[0].Effects;
		if (!playerEffects.valid())
			return false;

		for (auto effect : playerEffects) {
			if (effect.SkillId == skillId && effect.GetTimeRemaining() > 1000) {
				return false;
			}
		}

		return true;
	}

	bool ConsController::UseConsumable(DWORD itemId) {
		auto bags = GW::Items::GetBagArray();

		if (!bags)
			return false;

		for (int bagIndex = 1; bagIndex <= 4; ++bagIndex) {
			auto bag = bags[bagIndex];

			if (bag) {
				auto items = bag->Items;

				if (items.valid()) {
					for (size_t i = 0; i < items.size(); i++) {
						auto item = items[i];

						if (item && item->ModelId == itemId) {
							GW::Items::UseItem(item);
							return true;
						}
					}
				}
			}
		}

		return false;
	}
}