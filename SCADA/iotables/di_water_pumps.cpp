#include "plc.hpp"
#include "iotables/di_water_pumps.hpp"

using namespace WarGrey::SCADA;

void WarGrey::SCADA::DI_water_pump(WaterPumplet* target, const uint8* db4, size_t idx4_p1, const uint8* db205, size_t idx205_p1) {
	target->set_remote_control(DI_water_pump_remote_control(db4, idx4_p1));

	//if (DI_water_pump_limited(db4, idx4_p1)) {
	//	target->set_state(WaterPumpState::Limited);
	//} else
		
	if (DI_water_pump_running(db4, idx4_p1)) {
		target->set_state(WaterPumpState::Running);
	} else if (DI_water_pump_alert(db4, idx4_p1)) {
		target->set_state(WaterPumpState::Alert);
	} else if (DI_water_pump_broken(db4, idx4_p1)) {
		target->set_state(WaterPumpState::Broken);
	} else if (DI_water_pump_repair(db4, idx4_p1)) {
		target->set_state(WaterPumpState::Maintenance);
	} else {
		if (DBX(db205, idx205_p1 - 1U)) {
			target->set_state(WaterPumpState::Starting);
		} else if (DBX(db205, idx205_p1 + 0U)) {
			target->set_state(WaterPumpState::Stopping);
		} else if (DI_water_pump_ready(db4, idx4_p1)) {
			target->set_state(WaterPumpState::Ready);
		} else if (DBX(db205, idx205_p1 + 1U)) {
			target->set_state(WaterPumpState::Unstartable);
		} else if (DBX(db205, idx205_p1 + 2U)) {
			target->set_state(WaterPumpState::Unstoppable);
		} else {
			target->set_state(WaterPumpState::Stopped);
		}
	}

	// the rest are unused or implied
	//target->set_state(DBX(db205, idx205 + 4U), HopperPumpState::StartReady);
	//target->set_state(DBX(db205, idx205 + 5U), HopperPumpState::StopReady);
}

void WarGrey::SCADA::DI_shift_button(Buttonlet* target, const uint8* db205, unsigned int idx_p1) {
	if (DBX(db205, idx_p1 - 1U)) {
		target->set_state(ButtonState::Executing);
	} else if (DBX(db205, idx_p1 + 1U)) {
		target->set_state(ButtonState::Ready);
	} else if (DBX(db205, idx_p1 + 3U)) {
		target->set_state(ButtonState::Failed);
	} else {
		target->set_state(ButtonState::Disabled);
	}
}

/*************************************************************************************************/
bool WarGrey::SCADA::DI_water_pump_remote_control(const uint8* db4, size_t idx4_p1) {
	return DBX(db4, idx4_p1 - 1U);
}

bool WarGrey::SCADA::DI_water_pump_ready(const uint8* db4, size_t idx4_p1) {
	return DBX(db4, idx4_p1 + 0U);
}

bool WarGrey::SCADA::DI_water_pump_running(const uint8* db4, size_t idx4_p1) {
	return DBX(db4, idx4_p1 + 1U);
}

bool WarGrey::SCADA::DI_water_pump_alert(const uint8* db4, size_t idx4_p1) {
	return DBX(db4, idx4_p1 + 2U);
}

bool WarGrey::SCADA::DI_water_pump_broken(const uint8* db4, size_t idx4_p1) {
	return DBX(db4, idx4_p1 + 3U);
}

bool WarGrey::SCADA::DI_water_pump_limited(const uint8* db4, size_t idx4_p1) {
	return DBX(db4, idx4_p1 + 4U);
}

bool WarGrey::SCADA::DI_water_pump_repair(const uint8* db4, size_t idx4_p1) {
	return DBX(db4, idx4_p1 + 5U);
}

bool WarGrey::SCADA::DI_water_pump_emergence(const uint8* db4, size_t idx4_p1) {
	return DBX(db4, idx4_p1 + 6U);
}
