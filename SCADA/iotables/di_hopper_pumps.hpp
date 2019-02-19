#pragma once

#include "graphlet/symbol/pump/hopper_pumplet.hpp"
#include "graphlet/symbol/pump/hydraulic_pumplet.hpp"
#include "graphlet/dashboard/alarmlet.hpp"

namespace WarGrey::SCADA {
#define DI_hopper_type(db4, idx_p1) DBX(db4, idx_p1 + 0U)
#define DI_underwater_type(db4, idx_p1) DBX(db4, idx_p1 + 1U)

	// DB4, starts from 1
	static unsigned int ps_hopper_pump_feedback = 1U;
	static unsigned int sb_hopper_pump_feedback = 25U;

	static unsigned int ps_hopper_master_gland_pump_feedback = 9U;
	static unsigned int ps_hopper_spare_gland_pump_feedback = 13U;
	static unsigned int sb_hopper_master_gland_pump_feedback = 33U;
	static unsigned int sb_hopper_spare_gland_pump_feedback = 37U;

	static unsigned int ps_underwater_master_gland_pump_feedback = 513U;
	static unsigned int ps_underwater_spare_gland_pump_feedback = 517U;
	static unsigned int sb_underwater_master_gland_pump_feedback = 529U;
	static unsigned int sb_underwater_spare_gland_pump_feedback = 533U;

	static unsigned int ps_hopper_pump_speed_knob_moved = 554U;
	static unsigned int ps_hopper_pump_emergence_feedback = 559U;

	static unsigned int ps_hopper_lubricating_unit_feedback = 481U;
	static unsigned int ps_hopper_lubricating_unit_alarms = 486U;

	static unsigned int ps_hopper_gearbox_master_feedback = 521U;
	static unsigned int ps_hopper_gearbox_spare_feedback = 524U;
	static unsigned int ps_hopper_gearbox_alarms = 527U;

	static unsigned int sb_hopper_pump_speed_knob_moved = 618U;
	static unsigned int sb_hopper_pump_emergence_feedback = 623U;

	static unsigned int sb_hopper_lubricating_unit_feedback = 497U;
	static unsigned int sb_hopper_lubricating_unit_alarms = 502U;
	
	static unsigned int sb_hopper_gearbox_master_feedback = 537U;
	static unsigned int sb_hopper_gearbox_spare_feedback = 540U;
	static unsigned int sb_hopper_gearbox_alarms = 543U;

	static unsigned int ps_gate_flushing_pump_feedback = 105U;
	static unsigned int sb_gate_flushing_pump_feedback = 109U;
	
	// DB205, starts from 1
	static unsigned int ps_hopper_pump_details = 857U;
	static unsigned int ps_underwater_pump_details = 825U;
	static unsigned int ps_hopper_gearbox_master_details = 1209U;
	static unsigned int ps_hopper_gearbox_spare_details = 1217U;
	
	static unsigned int sb_hopper_pump_details = 873U;
	static unsigned int sb_underwater_pump_details = 841U;
	static unsigned int sb_hopper_gearbox_master_details = 1225U;
	static unsigned int sb_hopper_gearbox_spare_details = 1233U;

	static unsigned int ps_hopper_master_gland_pump_status = 1713U;
	static unsigned int ps_hopper_spare_gland_pump_status = 1721U;
	static unsigned int sb_hopper_master_gland_pump_status = 1729U;
	static unsigned int sb_hopper_spare_gland_pump_status = 1737U;

	static unsigned int ps_underwater_master_gland_pump_status = 1697U;
	static unsigned int ps_underwater_spare_gland_pump_status = 1705U;
	static unsigned int sb_underwater_master_gland_pump_status = 1745U;
	static unsigned int sb_underwater_spare_gland_pump_status = 1753U;

	static unsigned int ps_hopper_pipeline_ready = 819U;
	static unsigned int sb_hopper_pipeline_ready = 820U;
	static unsigned int ps_underwater_pipeline_ready = 817U;
	static unsigned int sb_underwater_pipeline_ready = 818U;

	static unsigned int ps_gate_flushing_pump_status = 1673U;
	static unsigned int sb_gate_flushing_pump_status = 1681U;

	/************************************************************************************************/
	void DI_hopper_pumps(WarGrey::SCADA::HopperPumplet* t1, WarGrey::SCADA::HopperPumplet* t2, const uint8* db4, size_t idx4_p1, const uint8* db205, size_t idx205_1_p1, size_t idx205_2_p1);
	void DI_hopper_pump(WarGrey::SCADA::HopperPumplet* target, const uint8* db4, size_t idx_p1, const uint8* db205, size_t idx205_p1);

	bool DI_hopper_pump_remote_control(const uint8* db4, size_t idx4_p1, bool on);
	bool DI_hopper_pump_ready(const uint8* db4, size_t idx4_p1, bool on);
	bool DI_hopper_pump_running(const uint8* db4, size_t idx4_p1, bool on);
	bool DI_hopper_pump_alert(const uint8* db4, size_t idx4_p1, bool on);
	bool DI_hopper_pump_broken(const uint8* db4, size_t idx4_p1, bool on);
	bool DI_hopper_pump_repair(const uint8* db4, size_t idx4_p1, bool on);

	/************************************************************************************************/
	void DI_gate_flushing_pump(WarGrey::SCADA::HydraulicPumplet* target, const uint8* db4, size_t idx4_p1, const uint8* db205, size_t idx205_p1);

	bool DI_gate_flushing_pump_remote_control(const uint8* db4, size_t idx4_p1);
	bool DI_gate_flushing_pump_running(const uint8* db4, size_t idx4_p1);
	bool DI_gate_flushing_pump_broken(const uint8* db4, size_t idx4_p1);
	bool DI_gate_flushing_pump_ready(const uint8* db205, size_t idx205_p1);

	/************************************************************************************************/
	void DI_gland_pump(WarGrey::SCADA::HydraulicPumplet* target, bool hopper,
		const uint8* db4, size_t idx_p1, const uint8* db205, size_t idx205_p1);
	
	bool DI_gland_pump_remote_control(const uint8* db4, size_t idx4_p1, bool hopper);
	bool DI_gland_pump_ready(const uint8* db4, size_t idx4_p1, bool hopper);
	bool DI_gland_pump_running(const uint8* db4, size_t idx4_p1, bool hopper);
	bool DI_gland_pump_broken(const uint8* db4, size_t idx4_p1, bool hopper);

	/************************************************************************************************/
	void DI_hopper_pump_lubricating_unit(WarGrey::SCADA::HydraulicPumplet* target, const uint8* db4, size_t idx4_p1, const uint8* db205, size_t idx205_p1);
	void DI_hopper_pump_gearbox(WarGrey::SCADA::HydraulicPumplet* target, const uint8* db4, size_t idx4_p1, const uint8* db205, size_t idx205_p1);
	void DI_hopper_lubricating_unit_alarm(WarGrey::SCADA::Alarmlet* target, const uint8* db4, unsigned int idx_p1);

	bool DI_hopper_pump_lubricating_unit_running(const uint8* db4, size_t idx4_p1);
	bool DI_hopper_pump_gearbox_running(const uint8* db4, size_t idx4_p1);
}
