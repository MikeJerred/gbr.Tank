#pragma once

namespace gbr::Tank::Controllers::City {
	enum State {
		Begin,
		End
	};

	class CityController {
	private:
		State state;
	public:
		CityController();

		State Tick();
	};
}