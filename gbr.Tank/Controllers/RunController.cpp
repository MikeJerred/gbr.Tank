#include <thread>
#include <GWCA/GWCA.h>
#include <GWCA/Managers/GameThreadMgr.h>
#include <GWCA/Managers/MapMgr.h>
#include <GWCA/Managers/PartyMgr.h>

#include "../Utilities/LogUtility.h"
#include "../Utilities/SleepUtility.h"
#include "RunController.h"

namespace gbr::Tank::Controllers {
	using LogUtility = Utilities::LogUtility;
	using SleepUtility = Utilities::SleepUtility;

	RunController::RunController() {
		state = State::Begin;
		hookId = GW::GameThread::AddPermanentCall([this]() {
			Tick();
		});
	}

	RunController::~RunController() {
		GW::GameThread::RemovePermanentCall(hookId);
	}

	void RunController::Tick() {
		for (auto& awaitable : SleepUtility::SleepAwaitables) {
			if (awaitable.hasFinished())
				awaitable.complete();
		}

		/*if (SleepUtility::IsSleeping())
			return;*/


		if (GW::Map::GetInstanceType() == GW::Constants::InstanceType::Loading)
			return;

		if (GW::Map::GetInstanceType() == GW::Constants::InstanceType::Explorable && GW::PartyMgr::GetIsPartyDefeated()) {
			LogUtility::Log(L"Returning to outpost");
			// return to outpost
			state = State::Begin;
			return;
		}

		if (GW::Map::GetInstanceType() == GW::Constants::InstanceType::Outpost) {
			state = State::Begin;
		}

		switch (state) {
		case State::Begin:
			// assume that we are starting in DoA outpost, with all party members added in HM

			LogUtility::Log(L"Starting a run");

			// run to city entrance

			cityController = new City::CityController();
			state = State::City;
			break;
		case State::City:
			if (cityController->Tick() == City::State::End) {
				LogUtility::Log(L"Starting Veil");
				delete cityController;
				//veilController = new Veil::VeilController();
				state = State::Veil;
			};
			break;
		case State::End:
			// resign, return to outpost, collect gems, start again
			break;
		}
	}
}