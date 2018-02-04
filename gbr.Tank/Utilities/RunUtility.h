#pragma once

#include <GWCA/GWCA.h>

#include "../Awaitable.h"

namespace gbr::Tank::Utilities {

	class RunUtility {
	private:
	public:
		static concurrency::task<void> FollowWaypoints(std::vector<std::vector<GW::GamePos>>& waypoints);
		static concurrency::task<void> FollowWaypoints(std::vector<GW::GamePos>& waypoints);
	};
}