#pragma once

#include <algorithm>
#include <iterator>
#include <map>
#include <sstream>

#include <GWCA/GWCA.h>
#include <GWCA/GWStructures.h>

namespace gbr::Tank::Controllers::City {
	template<typename E>
	struct enable_bitmask_operators {
		static constexpr bool enable = false;
	};

	template<typename E>
	typename std::enable_if<enable_bitmask_operators<E>::enable, E>::type
	operator|(E lhs, E rhs) {
		typedef typename std::underlying_type<E>::type underlying;
		return static_cast<E>(static_cast<underlying>(lhs) | static_cast<underlying>(rhs));
	}

	template<typename E>
	bool operator&(E lhs, E rhs) {
		typedef typename std::underlying_type<E>::type underlying;
		return static_cast<underlying>(lhs) & static_cast<underlying>(rhs);
	}

	class MargoAnalyzer {
	private:
		class MargoGroup {
		private:
			enum class StateType {
				None = 0,
				Wandering = 1 << 0,
				Collapsing = 1 << 1,
				Collapsed = 1 << 2,
				Running = 1 << 3,
				WaitingToWander = 1 << 4
			};

			struct State {
				StateType state;
				DWORD tick;

				State(StateType state, DWORD tick) : state(state), tick(tick) {}
			};

			static const DWORD WanderingTime = 9000;
			static const DWORD WaitingTime = 2000;
			static const DWORD RunningTime = 5000;

			std::vector<State> _states;
			std::vector<DWORD> _agentIds;
			bool _isAggroed;
		public:
			const static std::vector<std::vector<int>> PossibleGroups;

			MargoGroup(const std::vector<GW::Agent*>& agents);

			bool StillExists();
			bool IsAggroed();
			void Update();
			bool ContainsAgent(DWORD agentId);
			static float GetSpeed(DWORD agentId);
			DWORD GetTimeBeforeBall();
			float GetDistanceFromPlayer();
			float GetSqrDistanceFrom(float x, float y);

			std::wstring GetLastStateName() {
				std::vector<std::wstring> stateNames;

				if (_states.size() == 0)
					return L"None";

				auto state = _states.back().state;

				if (state & StateType::Wandering)
					stateNames.push_back(L"Wandering");
				if (state & StateType::Collapsing)
					stateNames.push_back(L"Collapsing");
				if (state & StateType::Collapsed)
					stateNames.push_back(L"Collapsed");
				if (state & StateType::Running)
					stateNames.push_back(L"Running");
				if (state & StateType::WaitingToWander)
					stateNames.push_back(L"WaitingToWander");

				std::wostringstream stream;
				for (auto val : stateNames) {
					stream << val << L", ";
				}
				return stream.str();
			}
		};

		std::vector<MargoGroup*> _margoGroups;
		DWORD _hookId;
	public:
		MargoAnalyzer();
		~MargoAnalyzer();

		void Tick();
		bool MatchesGroup(const std::vector<GW::Agent*>& potentialBall, const std::vector<int>& validModelIds);
		bool AgentIsMargonite(GW::Agent* agent);

		bool PlayerShouldWait();
		bool AllGroupsInRangeAggroed(float x, float y, float sqrRange);
	};
}