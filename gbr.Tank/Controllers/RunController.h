#pragma once

#include <experimental/resumable>
#include "City/CityController.h"

namespace gbr::Tank::Controllers {
	enum class State {
		Sleeping,
		Begin,
		City,
		Veil,
		Gloom,
		Foundry,
		End
	};

	class RunController {
	private:
		DWORD hookId;
		State state;
		City::CityController* cityController;

		DWORD sleepUntilTick;

		void Tick();
	public:
		RunController();
		~RunController();
	};
}