#include "CityController.h"


namespace gbr::Tank::Controllers::City {
	CityController::CityController() : state(State::Begin) {
	}

	State CityController::Tick() {
		switch (state) {
		case State::Begin:

			break;
		}

		return state;
	}
}