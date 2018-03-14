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
			static const double pi = 3.14159265358979;
			if (angle > pi) {
				angle -= 2 * pi;
				return wrapAngle(angle);
			}

			if (angle < -pi) {
				angle += 2 * pi;
				return wrapAngle(angle);
			}

			return angle;
		}
	};
}