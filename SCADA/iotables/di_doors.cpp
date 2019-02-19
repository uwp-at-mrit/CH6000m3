#include "plc.hpp"
#include "iotables/di_doors.hpp"

using namespace WarGrey::SCADA;

/************************************************************************************************/
void WarGrey::SCADA::DI_hopper_door(IHopperDoorlet* target, const uint8* db205, size_t idx205_p1) {
	if (DBX(db205, idx205_p1 + 6U)) {
		target->set_state(DoorState::Disabled);
	} else if (DBX(db205, idx205_p1 - 1U)) {
		target->set_state(DoorState::Opening);
	} else if (DBX(db205, idx205_p1 + 0U)) {
		target->set_state(DoorState::Closing);
	} else {
		target->set_state(DoorState::Default);
	}
}

void WarGrey::SCADA::DI_hopper_door(IHopperDoorlet* target, const uint8* db4, size_t idx4_p1, const uint8* db205, size_t idx205_p1) {
	if (DBX(db4, idx4_p1 - 1U)) {
		target->set_state(DoorState::Closed);
	} else {
		DI_hopper_door(target, db205, idx205_p1);
	}
}

void WarGrey::SCADA::DI_hopper_doors_auto_lock(Alarmlet* alarmer, const uint8* db205) {
	alarmer->set_state(DBX(db205, 1087U - 1U), AlarmState::Notice, AlarmState::None);
}

void WarGrey::SCADA::DI_hopper_doors_locked(Alarmlet* alarmer, const uint8* db205) {
	alarmer->set_state(DBX(db205, 1086U - 1U), AlarmState::Notice, AlarmState::None);
}

void WarGrey::SCADA::DI_hopper_doors_checks_button(Buttonlet* button, BottomDoorCommand cmd, const uint8* db205) {
	unsigned int idx_p1 = 0U;

	switch (cmd) {
	case BottomDoorCommand::OpenDoorCheck:  idx_p1 = 1897U; break;
	case BottomDoorCommand::CloseDoorCheck: idx_p1 = 1899U; break;
	}

	if (idx_p1 > 0) {
		if (DBX(db205, idx_p1 + 0U)) {
			button->set_state(ButtonState::Executing);
		} else if (DBX(db205, idx_p1 - 1U)) {
			button->set_state(ButtonState::Ready);
		} else {
			button->set_state(ButtonState::Disabled);
		}
	}
}
