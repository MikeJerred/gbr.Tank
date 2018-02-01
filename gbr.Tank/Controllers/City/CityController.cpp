#include <experimental/resumable>
#include <GWCA/GWCA.h>
#include <GWCA/Managers/AgentMgr.h>
#include "../../Utilities/SleepUtility.h"

#include "CityController.h"

namespace gbr::Tank::Controllers::City {
	using SleepUtility = Utilities::SleepUtility;

	CityController::CityController() : state(State::Begin) {
	}

	State CityController::Tick() {
		switch (state) {
		case State::Begin:
			Begin();
			break;
		case State::End:
			break;
		}

		return state;
	}

	Utilities::Task Begin() {
		// take quest
		auto questSnake = GW::Agents::GetAgentIdByLoginNumber(GW::Constants::ModelID::DoA::Snake);
		GW::Agents::GoNPC(questSnake);


		co_await SleepUtility::Sleep(100);

		/*SleepUtility::Sleep(100)->Then([] {
			
		});*/

		// wait for bonds

		// put up shroud and SF
	}
}