#include <algorithm>
#include <thread>
#include <GWCA/GWCA.h>
#include <GWCA/Managers/AgentMgr.h>
#include <GWCA/Managers/ChatMgr.h>
#include <GWCA/Managers/GameThreadMgr.h>
#include <GWCA/Managers/MapMgr.h>
#include <GWCA/Managers/PartyMgr.h>

#include "../Utilities/LogUtility.h"
#include "../Utilities/RunUtility.h"
#include "RunController.h"

namespace gbr::Tank::Controllers {
	using LogUtility = Utilities::LogUtility;
	using RunUtility = Utilities::RunUtility;

	RunController::RunController() {
		_hookId = GW::GameThread::AddPermanentCall([]() {
			executor::singleton().tick();

			static int lastStuckCheck = 0;

			if (GW::Map::GetInstanceType() == GW::Constants::InstanceType::Explorable && GetTickCount() > lastStuckCheck + 5000) {
				lastStuckCheck = GetTickCount();

				GW::Chat::SendChat('/', L"stuck");
			}
		});
		GW::GameThread::Enqueue([]() { RunController::Start(); });
	}

	RunController::~RunController() {
		GW::GameThread::RemovePermanentCall(_hookId);
	}



	awaitable<void> RunController::Start() {
		while (true) {
			co_await DoRun();

			if (GW::Map::GetInstanceType() == GW::Constants::InstanceType::Explorable) {
				while (!GW::PartyMgr::GetIsPartyDefeated()) {
					// resign
					co_await Sleep(2000);
				}

				// return party to outpost
			}

			while (GW::Map::GetInstanceType() != GW::Constants::InstanceType::Outpost)
				co_await Sleep(2000);

			// collect gems
		}
	}

	awaitable<void> RunController::DoRun() {

		while (GW::Map::GetInstanceType() != GW::Constants::InstanceType::Outpost) {
			co_await Sleep(3000);
		}

		LogUtility::Log(L"Starting a run");

		// run to city entrance
		try {
			co_await RunUtility::FollowWaypoints(
				std::vector<GW::GamePos> {
					GW::GamePos(5646.0f, -16987.0f),
					GW::GamePos(6854.0f, -15241.0f),
					GW::GamePos(7162.0f, -13127.0f),
					GW::GamePos(9370.0f, -10381.0f),
					GW::GamePos(11423.0f, -11418.0f),
				},
				[]() { return ZonedCheck(); });
		}
		catch (ZonedException& e) {
			// we have zoned
		}

		while (GW::Map::GetInstanceType() != GW::Constants::InstanceType::Explorable) {
			co_await Sleep(2000);
		}

		co_await Sleep(3000);
		auto cityController = new City::CityController();

		try {
			co_await cityController->DoRun();
		}
		catch (RunFailedException& e) {
			LogUtility::Log(L"Run failed!");
			delete cityController;
			return;
		}

		delete cityController;


	}

	void RunController::ZonedCheck() {
		if (!GW::Agents::GetPlayer() || GW::Map::GetInstanceType() != GW::Constants::InstanceType::Outpost) {
			throw new ZonedException();
		}
	}
}