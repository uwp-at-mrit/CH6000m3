#include "iotables/ai_doors.hpp"
#include "graphlet/symbol/door/hopper_doorlet.hpp"

using namespace WarGrey::SCADA;

void WarGrey::SCADA::AI_hopper_door(IHopperDoorlet* target, float progress, float open_threshold, float closed_threshold) {
	DoorState s = target->get_state();

	if (s == DoorState::Opening) {
		if (progress >= open_threshold) {
			target->set_state(DoorState::Open);
		}
	}

	if (s == DoorState::Closing) {
		if (progress <= closed_threshold) {
			target->set_state(DoorState::Closed);
		}
	}
}
