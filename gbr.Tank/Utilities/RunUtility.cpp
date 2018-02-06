#include <algorithm>
#include <GWCA/GWCA.h>
#include <GWCA/Managers/AgentMgr.h>
#include <GWCA/Managers/SkillbarMgr.h>

#include "RunUtility.h"

namespace gbr::Tank::Utilities {
	concurrency::task<bool> RunUtility::FollowWaypoints(std::vector<std::vector<GW::GamePos>>& waypoints, std::function<concurrency::task<bool>()> afterSleepCheck) {
		auto playerPos = GW::Agents::GetPlayer()->pos;

		auto closest = *std::min_element(waypoints.cbegin(), waypoints.cend(), [=](std::vector<GW::GamePos> listA, std::vector<GW::GamePos> listB) {

			auto closestA = *std::min_element(listA.cbegin(), listA.cend(), [=](GW::GamePos a, GW::GamePos b) {
				return playerPos.SquaredDistanceTo(a) < playerPos.SquaredDistanceTo(b);
			});

			auto closestB = *std::min_element(listA.cbegin(), listA.cend(), [=](GW::GamePos a, GW::GamePos b) {
				return playerPos.SquaredDistanceTo(a) < playerPos.SquaredDistanceTo(b);
			});

			return playerPos.SquaredDistanceTo(closestA) < playerPos.SquaredDistanceTo(closestB);
		});

		return co_await FollowWaypoints(closest, afterSleepCheck);
	}

	concurrency::task<bool> RunUtility::FollowWaypoints(std::vector<GW::GamePos>& waypoints, std::function<concurrency::task<bool>()> afterSleepCheck) {
		auto playerPos = GW::Agents::GetPlayer()->pos;

		auto currentIterater = std::min_element(waypoints.cbegin(), waypoints.cend(), [=](GW::GamePos a, GW::GamePos b) {
			return playerPos.SquaredDistanceTo(a) < playerPos.SquaredDistanceTo(b);
		});

		for (; currentIterater < waypoints.cend(); currentIterater++) {
			auto currentWaypoint = *currentIterater;

			GW::Agents::Move(currentWaypoint);
			while (GW::Agents::GetPlayer()->pos.DistanceTo(currentWaypoint) > GW::Constants::Range::Adjacent) {
				co_await Sleep(50);

				if (afterSleepCheck && !co_await afterSleepCheck())
					return false;
			}
		}
	}

	concurrency::task<bool> RunUtility::FollowWaypointsWithoutStuck(std::vector<GW::GamePos>& waypoints, std::function<concurrency::task<bool>()> afterSleepCheck, int stuckTimeout) {
		auto playerPos = GW::Agents::GetPlayer()->pos;

		auto currentIterater = std::min_element(waypoints.cbegin(), waypoints.cend(), [=](GW::GamePos a, GW::GamePos b) {
			return playerPos.SquaredDistanceTo(a) < playerPos.SquaredDistanceTo(b);
		});

		for (; currentIterater < waypoints.cend(); currentIterater++) {
			auto currentWaypoint = *currentIterater;

			auto stuckTick = GetTickCount() + stuckTimeout;
			while (GW::Agents::GetPlayer()->pos.DistanceTo(currentWaypoint) > GW::Constants::Range::Adjacent) {
				GW::Agents::Move(currentWaypoint);
				co_await Sleep(50);
				if (afterSleepCheck && !co_await afterSleepCheck())
					return false;

				if (GetTickCount() > stuckTick) {
					// assume we are stuck, try to use hos to get unstuck
					GW::SkillbarMgr::UseSkillByID((DWORD)GW::Constants::SkillID::Heart_of_Shadow);

					co_await Sleep(150);
					if (afterSleepCheck && !co_await afterSleepCheck())
						return false;

					stuckTick = GetTickCount() + 10000;
				}
			}
		}

		return true;
	}
}