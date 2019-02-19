#include "plc.hpp"

#include "iotables/di_hopper_pumps.hpp"
#include "iotables/di_pumps.hpp"

using namespace WarGrey::SCADA;

static void _DI_hopper_pump(HopperPumplet* target, const uint8* db4, size_t idx4_p1, const uint8* db205, size_t idx205_p1, bool on) {
	target->set_remote_control(DI_hopper_pump_remote_control(db4, idx4_p1, on));

	if (on) {
		if (DI_hopper_pump_running(db4, idx4_p1, true)) {
			target->set_state(HopperPumpState::Running);
		} else if (DI_hopper_pump_alert(db4, idx4_p1, true)) {
			target->set_state(HopperPumpState::Alert);
		} else if (DI_hopper_pump_broken(db4, idx4_p1, true)) {
			target->set_state(HopperPumpState::Broken);
		} else if (DI_hopper_pump_repair(db4, idx4_p1, true)) {
			target->set_state(HopperPumpState::Maintenance);
		} else if (DBX(db205, idx205_p1 - 1U)) {
			target->set_state(HopperPumpState::Starting);
		} else if (DBX(db205, idx205_p1 + 0U)) {
			target->set_state(HopperPumpState::Stopping);
		} else if (DI_hopper_pump_ready(db4, idx4_p1, true)) {
			target->set_state(HopperPumpState::Ready);
		} else if (DBX(db205, idx205_p1 + 1U)) {
			target->set_state(HopperPumpState::Unstartable);
		} else if (DBX(db205, idx205_p1 + 2U)) {
			target->set_state(HopperPumpState::Unstoppable);
		} else {
			target->set_state(HopperPumpState::Stopped);
		}

		// the rest are unused
		//target->set_state(DBX(db205, idx205 + 4U), HopperPumpState::StartReady);
		//target->set_state(DBX(db205, idx205 + 5U), HopperPumpState::StopReady);
		//target->set_state(DBX(db205, idx205 + 6U), HopperPumpState::Ready);
	} else {
		target->set_state(HopperPumpState::Stopped);
	}
}

/*************************************************************************************************/
void WarGrey::SCADA::DI_hopper_pumps(HopperPumplet* t1, HopperPumplet* t2
	, const uint8* db4, size_t idx4_p1 , const uint8* db205, size_t idx205_1_p1, size_t idx205_2_p1) {
	_DI_hopper_pump(t1, db4, idx4_p1, db205, idx205_1_p1, DI_hopper_type(db4, idx4_p1));
	_DI_hopper_pump(t2, db4, idx4_p1, db205, idx205_2_p1, DI_underwater_type(db4, idx4_p1));
}

void WarGrey::SCADA::DI_hopper_pump(HopperPumplet* target, const uint8* db4, size_t idx_p1, const uint8* db205, size_t idx205_p1) {
	_DI_hopper_pump(target, db4, idx_p1, db205, idx205_p1, DI_hopper_type(db4, idx_p1));
}

bool WarGrey::SCADA::DI_hopper_pump_remote_control(const uint8* db4, size_t idx4_p1, bool on) {
	return on && DBX(db4, idx4_p1 + 2U);
}

bool WarGrey::SCADA::DI_hopper_pump_ready(const uint8* db4, size_t idx4_p1, bool on) {
	return on && DBX(db4, idx4_p1 - 1U);
}

bool WarGrey::SCADA::DI_hopper_pump_running(const uint8* db4, size_t idx4_p1, bool on) {
	return on && DBX(db4, idx4_p1 + 5U);
}

bool WarGrey::SCADA::DI_hopper_pump_alert(const uint8* db4, size_t idx4_p1, bool on) {
	return on && DBX(db4, idx4_p1 + 3U);
}

bool WarGrey::SCADA::DI_hopper_pump_broken(const uint8* db4, size_t idx4_p1, bool on) {
	return on && DBX(db4, idx4_p1 + 4U);
}

bool WarGrey::SCADA::DI_hopper_pump_repair(const uint8* db4, size_t idx4_p1, bool on) {
	return on && DBX(db4, idx4_p1 + 6U);
}

/************************************************************************************************/
void WarGrey::SCADA::DI_gate_flushing_pump(HydraulicPumplet* target, const uint8* db4, size_t idx4_p1, const uint8* db205, size_t idx205_p1) {
	DI_hydraulic_pump(target, db4, idx4_p1, db205, idx205_p1);
}

bool WarGrey::SCADA::DI_gate_flushing_pump_remote_control(const uint8* db4, size_t idx4_p1) {
	return DI_hydraulic_pump_remote_control(db4, idx4_p1);
}

bool WarGrey::SCADA::DI_gate_flushing_pump_running(const uint8* db4, size_t idx4_p1) {
	return DI_hydraulic_pump_running(db4, idx4_p1);
}

bool WarGrey::SCADA::DI_gate_flushing_pump_broken(const uint8* db4, size_t idx4_p1) {
	return DI_hydraulic_pump_broken(db4, idx4_p1);
}

bool WarGrey::SCADA::DI_gate_flushing_pump_ready(const uint8* db205, size_t idx205_p1) {
	return DI_hydraulic_pump_ready(db205, idx205_p1);
}

/************************************************************************************************/
void WarGrey::SCADA::DI_gland_pump(HydraulicPumplet* target, bool hopper
	, const uint8* db4, size_t idx_p1, const uint8* db205, size_t idx205_p1) {
	target->set_remote_control(DI_gland_pump_remote_control(db4, idx_p1, hopper));

	if (DI_gland_pump_running(db4, idx_p1, hopper)) {
		target->set_state(HydraulicPumpState::Running);
	} else if (DI_gland_pump_broken(db4, idx_p1, hopper)) {
		target->set_state(HydraulicPumpState::Broken);
	} else if (DI_gland_pump_ready(db4, idx_p1, hopper)) {
		target->set_state(HydraulicPumpState::Ready);
	} else {
		target->set_state(DBX(db205, idx205_p1 - 1U), HydraulicPumpState::Starting);
		target->set_state(DBX(db205, idx205_p1 + 0U), HydraulicPumpState::Stopping);
		//target->set_state(DBX(db205, idx205_p1 + 1), HydraulicPumpState::Reset);
		target->set_state(DBX(db205, idx205_p1 + 2U), HydraulicPumpState::Unstartable);
		target->set_state(DBX(db205, idx205_p1 + 3U), HydraulicPumpState::Unstoppable);

		// the rest 3 are implied or not used
		//target->set_state(DBX(db205, idx205_p1 + 4), HydraulicPumpState::StartReady);
		//target->set_state(DBX(db205, idx205_p1 + 5), HydraulicPumpState::StopReady);
		//target->set_state(DBX(db205, idx205_p1 + 6), HydraulicPumpState::Stopped);
	}
}

bool WarGrey::SCADA::DI_gland_pump_remote_control(const uint8* db4, size_t idx4_p1, bool hopper) {
	size_t idx = (hopper ? idx4_p1 : (idx4_p1 - 1U));

	return DBX(db4, idx);
}

bool WarGrey::SCADA::DI_gland_pump_ready(const uint8* db4, size_t idx4_p1, bool hopper) {
	size_t idx = (hopper ? (idx4_p1 - 1U) : idx4_p1);
	
	return DBX(db4, idx);
}

bool WarGrey::SCADA::DI_gland_pump_broken(const uint8* db4, size_t idx4_p1, bool hopper) {
	return DBX(db4, idx4_p1 + 2U);
}

bool WarGrey::SCADA::DI_gland_pump_running(const uint8* db4, size_t idx4_p1, bool hopper) {
	return DBX(db4, idx4_p1 + 1U);
}

/************************************************************************************************/
void WarGrey::SCADA::DI_hopper_pump_lubricating_unit(HydraulicPumplet* target
	, const uint8* db4, size_t idx4_p1, const uint8* db205, size_t idx205_p1) {
	target->set_remote_control(DBX(db4, idx4_p1 - 1U));

	if (DI_hopper_pump_lubricating_unit_running(db4, idx4_p1)) {
		target->set_state(HydraulicPumpState::Running);
	} else if (DBX(db4, idx4_p1 + 1U)) {
		target->set_state(HydraulicPumpState::Broken);
	} else {
		target->set_state(HydraulicPumpState::Stopped);
	}
}

void WarGrey::SCADA::DI_hopper_pump_gearbox(HydraulicPumplet* target, const uint8* db4, size_t idx4_p1, const uint8* db205, size_t idx205_p1) {
	target->set_remote_control(DBX(db4, idx4_p1 - 1U));

	if (DI_hopper_pump_gearbox_running(db4, idx4_p1)) {
		target->set_state(HydraulicPumpState::Running);
	} else if (DBX(db4, idx4_p1 + 1U)) {
		target->set_state(HydraulicPumpState::Broken);
	} else if (DBX(db205, idx205_p1 - 1U)) {
		target->set_state(HydraulicPumpState::Starting);
	} else if (DBX(db205, idx205_p1 + 0U)) {
		target->set_state(HydraulicPumpState::Stopping);
	} else if (DBX(db205, idx205_p1 + 1U)) {
		target->set_state(HydraulicPumpState::Unstartable);
	} else if (DBX(db205, idx205_p1 + 2U)) {
		target->set_state(HydraulicPumpState::Unstoppable);
	} else if (DBX(db205, idx205_p1 + 3U)) {
		target->set_state(HydraulicPumpState::Ready);
	}

	// the rest are unused
	//target->set_state(DBX(db205, idx205 + 4U), HopperPumpState::StartReady);
	//target->set_state(DBX(db205, idx205 + 5U), HopperPumpState::StopReady);
}

bool WarGrey::SCADA::DI_hopper_pump_lubricating_unit_running(const uint8* db4, size_t idx4_p1) {
	return DBX(db4, idx4_p1 + 0U);
}

bool WarGrey::SCADA::DI_hopper_pump_gearbox_running(const uint8* db4, size_t idx4_p1) {
	return DBX(db4, idx4_p1 + 0U);
}

void WarGrey::SCADA::DI_hopper_lubricating_unit_alarm(Alarmlet* target, const uint8* db4, unsigned int idx_p1) {
	target->set_state(DBX(db4, idx_p1 - 1U), AlarmState::Alert, AlarmState::None);
}
