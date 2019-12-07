#pragma once

#include "graphlet/symbol/pump/water_pumplet.hpp"
#include "graphlet/ui/buttonlet.hpp"

namespace WarGrey::SCADA {
	// DB4, starts from 1
	static unsigned int ps_water_pump_feedback = 17U;
	static unsigned int sb_water_pump_feedback = 41U;

	static unsigned int ps_water_pump_speed_knob_moved = 555U;
	static unsigned int ps_water_pump_speed_knob_moved0 = 556U;

	static unsigned int sb_water_pump_speed_knob_moved = 619U;
	static unsigned int sb_water_pump_speed_knob_moved0 = 620U;

	// DB205, starts from 1
	static unsigned int ps_water_pump_details = 1001U;
	static unsigned int sb_water_pump_details = 1017U;

	static unsigned int ps_ps_details = 577U;
	static unsigned int ps_sb_details = 585U;
	static unsigned int sb_ps_details = 593U;
	static unsigned int sb_sb_details = 601U;
	static unsigned int ps_2_details = 609U;
	static unsigned int sb_2_details = 617U;
	static unsigned int s2_ps_details = 625U;
	static unsigned int s2_sb_details = 633U;
	static unsigned int s2_2_details = 641U;
	static unsigned int p2_2_details = 649U;
	static unsigned int ps_h_details = 657U;
	static unsigned int sb_h_details = 665U;
	static unsigned int p2_h_details = 673U;
	static unsigned int s2_h_details = 681U;
	static unsigned int i2_2_details = 1201U;

	static unsigned int ps_water_pump_pipeline_ready = 687U;
	static unsigned int sb_water_pump_pipeline_ready = 688U;

	static unsigned int left_shifting_details = 1249U;
	static unsigned int right_shifting_details = 1250U;

	/************************************************************************************************/
	void DI_water_pump(WarGrey::SCADA::WaterPumplet* target, const uint8* db4, size_t idx4_p1, const uint8* db205, size_t idx205_p1);
	void DI_water_condition_menu(Windows::UI::Xaml::Controls::MenuFlyout^ menu, unsigned int idx, const uint8* db205, size_t idx205_p1);

	bool DI_water_pump_remote_control(const uint8* db4, size_t idx4_p1);
	bool DI_water_pump_ready(const uint8* db4, size_t idx4_p1);
	bool DI_water_pump_running(const uint8* db4, size_t idx4_p1);
	bool DI_water_pump_alert(const uint8* db4, size_t idx4_p1);
	bool DI_water_pump_broken(const uint8* db4, size_t idx4_p1);
	bool DI_water_pump_limited(const uint8* db4, size_t idx4_p1);
	bool DI_water_pump_repair(const uint8* db4, size_t idx4_p1);
	bool DI_water_pump_emergence(const uint8* db4, size_t idx4_p1);
	
	void DI_shift_button(WarGrey::SCADA::Buttonlet* target, const uint8* db205, unsigned int idx_p1);
}
