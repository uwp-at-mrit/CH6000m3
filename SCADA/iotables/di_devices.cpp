#include "plc.hpp"
#include "iotables/di_devices.hpp"

using namespace WarGrey::SCADA;

void WarGrey::SCADA::DI_alarm(Alarmlet* target, const uint8* db4, unsigned int idx_p1, AlarmState hlstate) {
	target->set_state(DBX(db4, idx_p1 - 1U), hlstate, AlarmState::None);
}

void WarGrey::SCADA::DI_backoil_pressure_override(Buttonlet* target, const uint8* db205, unsigned int idx_p1) {
	target->set_state(DBX(db205, idx_p1 - 1U), ButtonState::Executing, ButtonState::Default);
}

void WarGrey::SCADA::DI_visor_tank(Tanklet* target, const uint8* db4, unsigned int idx_p1) {
	if (DI_tank_level_low(db4, idx_p1)) {
		target->set_state(TankState::Low);
	} else if (DI_tank_level_too_low(db4, idx_p1)) {
		target->set_state(TankState::UltraLow);
	} else {
		target->set_state(TankState::Normal);
	}
}

void WarGrey::SCADA::DI_tank_heater(Heaterlet* target, const uint8* db4, unsigned int idx4_p1, const uint8* db205, unsigned int idx205_p1) {
	target->set_remote_control(DBX(db4, idx4_p1 - 1U));
	target->set_auto_mode(DBX(db205, idx205_p1 + 6U));

	if (DBX(db4, idx4_p1 + 1U)) {
		target->set_state(HeaterState::Broken);
	} else if (DBX(db4, idx4_p1 + 0U)) {
		target->set_state(HeaterState::Running);	
	} else if (DBX(db205, idx205_p1 - 1U)) {
		target->set_state(HeaterState::Starting);
	} else if (DBX(db205, idx205_p1 + 0U)) {
		target->set_state(HeaterState::Stopping);
	} else if (DBX(db205, idx205_p1 + 3U)) {
		target->set_state(HeaterState::Ready);
	} else if (DBX(db205, idx205_p1 + 1U)) {
		target->set_state(HeaterState::Unstartable);
	} else if (DBX(db205, idx205_p1 + 2U)) {
		target->set_state(HeaterState::Unstoppable);
	} else {
		target->set_state(HeaterState::Stopped);
	}
}

/*************************************************************************************************/
bool WarGrey::SCADA::DI_tank_level_low(const uint8* db4, unsigned int idx_p1) {
	return DBX(db4, idx_p1 - 1U);
}

bool WarGrey::SCADA::DI_tank_level_too_low(const uint8* db4, unsigned int idx_p1) {
	return DBX(db4, idx_p1 + 0U);
}
