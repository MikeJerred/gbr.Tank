#include <thread>
#include <GWCA/GWCA.h>
#include <GWCA/Managers/GameThreadMgr.h>

#include "../Utilities/LogUtility.h"
#include "../Utilities/SkillUtility.h"
#include "RunController.h"

namespace gbr::Tank::Controllers {
	using LogUtility = Utilities::LogUtility;
	using SkillUtility = Utilities::SkillUtility;

	RunController::RunController() {
		state = State::Begin;
		hookId = GW::GameThread::AddPermanentCall([this]() {
			Tick();
		});
	}

	RunController::~RunController() {
		GW::GameThread::RemovePermanentCall(hookId);
	}

	void RunController::Tick() {
		switch (state) {
		case State::Begin:
			// assume that we are starting in DoA outpost, with all party members added in HM

			LogUtility::Log(L"Starting a run");


			cityController = new City::CityController();
			state = State::City;
			break;
		case State::City:
			if (cityController->Tick() == City::State::End) {
				LogUtility::Log(L"Starting Veil");
				delete cityController;
				//veilController = new Veil::VeilController();
				state = State::Veil;
			};
			break;
		case State::End:
			// resign, return to outpost, collect gems, start again
			break;
		}
	}
}