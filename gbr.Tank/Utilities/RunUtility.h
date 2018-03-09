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
		static DWORD FindBestHoSTarget(GW::GamePos& posToGoTowards);

		static inline double wrapAngle(double angle) {
			static const double twoPi = 2.0 * 3.141592865358979;
			return angle - twoPi * floor(angle / twoPi);
		}
	};
}