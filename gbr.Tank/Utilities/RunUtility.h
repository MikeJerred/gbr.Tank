#pragma once

#include <GWCA/GWCA.h>

#include "../Awaitable.h"

namespace gbr::Tank::Utilities {

	class RunUtility {
	private:
	public:
		static concurrency::task<bool> FollowWaypoints(std::vector<std::vector<GW::GamePos>>& waypoints, std::function<concurrency::task<bool>()> afterSleepCheck = nullptr);
		static concurrency::task<bool> FollowWaypoints(std::vector<GW::GamePos>& waypoints, std::function<concurrency::task<bool>()> afterSleepCheck = nullptr);
		static concurrency::task<bool> FollowWaypointsWithoutStuck(std::vector<GW::GamePos>& waypoints, std::function<concurrency::task<bool>()> afterSleepCheck, int stuckTimeout);
	};
}