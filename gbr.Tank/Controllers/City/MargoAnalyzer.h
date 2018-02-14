#pragma once

#include <map>

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
	bool operator&&(E lhs, E rhs) {
		typedef typename std::underlying_type<E>::type underlying;
		return static_cast<underlying>(lhs) && static_cast<underlying>(rhs);
	}

	class MargoAnalyzer {
	private:
		class MargoGroup {
		private:
			enum class StateType {
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
			static const DWORD RunningTime = 5000;

			std::vector<State> _states;
			std::vector<DWORD> _agentIds;
		public:
			const static std::vector<std::vector<DWORD>> PossibleGroups;

			MargoGroup(const std::vector<GW::Agent*>& agents);

			bool StillExists();
			void Update();
			bool ContainsAgent(DWORD agentId);
			static float GetSpeed(DWORD agentId);
			DWORD GetTimeBeforeBall();
			float GetDistanceFromPlayer();
		};

		std::vector<MargoGroup*> _margoGroups;
		DWORD _hookId;
	public:
		MargoAnalyzer();
		~MargoAnalyzer();

		void Tick();
		bool MatchesGroup(std::vector<GW::Agent*> potentialBall, std::vector<DWORD> validModelIds);
		bool AgentIsMargonite(GW::Agent* agent);
	};
}