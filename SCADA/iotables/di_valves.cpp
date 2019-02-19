#include "plc.hpp"
#include "iotables/di_valves.hpp"

using namespace WarGrey::SCADA;

void WarGrey::SCADA::DI_manual_valve(ManualValvelet* target, const uint8* db4, size_t idx_p1) {
	target->set_state(DI_manual_valve_open(db4, idx_p1), ManualValveState::Open, ManualValveState::OpenReady); // OpenReady or StopReady
}

void WarGrey::SCADA::DI_gate_valve(GateValvelet* target, const uint8* db4, size_t idx4_p1, const uint8* db205, size_t idx205_p1) {
	if (DBX(db205, idx205_p1 + 5U)) {
		target->set_state(GateValveState::VirtualOpen);
	} else if (DBX(db205, idx205_p1 + 6U)) {
		target->set_state(GateValveState::VirtualClose);
	} else if (DBX(db4, idx4_p1 - 1U)) {
		target->set_state(GateValveState::Open);
	} else if (DBX(db4, idx4_p1 + 0U)) {
		target->set_state(GateValveState::Closed);
	} else if (DBX(db205, idx205_p1 - 1U)) {
		target->set_state(GateValveState::Opening);
	} else if (DBX(db205, idx205_p1 + 0U)) {
		target->set_state(GateValveState::Closing);
	} else if (DBX(db205, idx205_p1 + 3U)) {
		target->set_state(GateValveState::OpenReady);
	} else if (DBX(db205, idx205_p1 + 4U)) {
		target->set_state(GateValveState::CloseReady);
	} else if (DBX(db205, idx205_p1 + 1U)) {
		target->set_state(GateValveState::Unopenable);
	} else if (DBX(db205, idx205_p1 + 2U)) {
		target->set_state(GateValveState::Unclosable);
	}
}

void WarGrey::SCADA::DI_motor_valve(MotorValvelet* target, const uint8* db4, size_t idx4_p1, const uint8* db205, size_t idx205_p1) {
	if (DBX(db205, idx205_p1 + 5U)) {
		target->set_state(TValveState::VirtualOpen);
	} else if (DBX(db205, idx205_p1 + 6U)) {
		target->set_state(TValveState::VirtualClose);
	} else if (DBX(db4, idx4_p1 - 1U)) {
		target->set_state(TValveState::Open);
	} else if (DBX(db4, idx4_p1 + 0U)) {
		target->set_state(TValveState::Closed);
	} else if (DBX(db205, idx205_p1 - 1U)) {
		target->set_state(TValveState::Opening);
	} else if (DBX(db205, idx205_p1 + 0U)) {
		target->set_state(TValveState::Closing);
	} else if (DBX(db205, idx205_p1 + 3U)) {
		target->set_state(TValveState::OpenReady);
	} else if (DBX(db205, idx205_p1 + 4U)) {
		target->set_state(TValveState::CloseReady);
	} else if (DBX(db205, idx205_p1 + 1U)) {
		target->set_state(TValveState::Unopenable);
	} else if (DBX(db205, idx205_p1 + 2U)) {
		target->set_state(TValveState::Unclosable);
	}
}

/*************************************************************************************************/
bool WarGrey::SCADA::DI_manual_valve_open(const uint8* db4, size_t idx_p1) {
	return DBX(db4, idx_p1 - 1U);
}
