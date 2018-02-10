#pragma once

#include <GWCA/GWCA.h>

#include "../Awaitable.h"

namespace gbr::Tank::Utilities {

	class RunUtility {
	private:
	public:
		static awaitable<void> FollowWaypoints(std::vector<std::vector<GW::GamePos>>& waypoints, std::function<void()> afterSleepCheck = nullptr);
		static awaitable<void> FollowWaypoints(std::vector<GW::GamePos>& waypoints, std::function<void()> afterSleepCheck = nullptr);
		static awaitable<void> FollowWaypointsWithoutStuck(std::vector<GW::GamePos>& waypoints, const std::function<awaitable<void>()> afterSleepCheck, int stuckTimeout);
	};
}