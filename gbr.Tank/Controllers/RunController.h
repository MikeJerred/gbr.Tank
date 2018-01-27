#pragma once

#include "City/CityController.h"

namespace gbr::Tank::Controllers {
	enum State {
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

		void Tick();
	public:
		RunController();
		~RunController();
	};
}