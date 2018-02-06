#include <algorithm>
#include <thread>
#include <GWCA/GWCA.h>
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
			auto awaitableIterator = std::find_if(Awaitable::Awaitables.begin(), Awaitable::Awaitables.end(), [](Awaitable* element) {
				return element && element->hasFinished();
			});

			if (awaitableIterator != Awaitable::Awaitables.end()) {
				auto awaitable = *awaitableIterator;
				Awaitable::Awaitables.erase(awaitableIterator);
				awaitable->complete();
			}
		});

		GW::GameThread::Enqueue([]() { RunController::DoRun(); });
	}

	RunController::~RunController() {
		GW::GameThread::RemovePermanentCall(_hookId);
	}

	concurrency::task<void> RunController::DoRun() {

		while (GW::Map::GetInstanceType() != GW::Constants::InstanceType::Outpost) {
			co_await Sleep(3000);
		}

		LogUtility::Log(L"Starting a run");

		// run to city entrance
		co_await RunUtility::FollowWaypoints(
			std::vector<GW::GamePos>{
				GW::GamePos(5646.0f, -16987.0f),
				GW::GamePos(6854.0f, -15241.0f),
				GW::GamePos(7162.0f, -13127.0f),
				GW::GamePos(9370.0f, -10381.0f),
				GW::GamePos(11423.0f, -11418.0f),
			});

		while (GW::Map::GetInstanceType() != GW::Constants::InstanceType::Explorable) {
			co_await Sleep(1000);
		}

		auto cityController = new City::CityController();

		auto success = co_await cityController->DoRun();

		delete cityController;

	}
}