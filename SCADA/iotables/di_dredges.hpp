#pragma once

#include "graphlet/device/gantrylet.hpp"
#include "graphlet/buttonlet.hpp"

namespace WarGrey::SCADA {
	// DB4, starts from 1
	static unsigned int gantry_ps_trunnion_limited = 325U;
	static unsigned int gantry_ps_intermediate_limited = 357U;
	static unsigned int gantry_ps_draghead_limited = 365U;

	static unsigned int gantry_sb_trunnion_limited = 341U;
	static unsigned int gantry_sb_intermediate_limited = 389U;
	static unsigned int gantry_sb_long_draghead_limited = 409U;
	static unsigned int gantry_sb_short_draghead_limited = 397U;

	static unsigned int console_ps_intermediate_winch_lift_button = 585U;
	static unsigned int console_ps_draghead_winch_lift_button = 586U;
	static unsigned int console_ps_winch_gantry_stop_button = 587U;

	static unsigned int console_sb_intermediate_winch_lift_button = 649U;
	static unsigned int console_sb_draghead_winch_lift_button = 650U;
	static unsigned int console_sb_winch_gantry_stop_button = 651U;

	static unsigned int ps_trunnion_details = 169U;
	static unsigned int ps_intermediate_details = 177U;
	static unsigned int ps_draghead_details = 185U;

	static unsigned int sb_trunnion_details = 193U;
	static unsigned int sb_intermediate_details = 201U;
	static unsigned int sb_draghead_details = 209U;

	// DB205, starts from 1
	static unsigned int gantry_ps_trunnion_details = 1305U;
	static unsigned int gantry_ps_intermediate_details = 1313U;
	static unsigned int gantry_ps_draghead_details = 1321U;

	static unsigned int gantry_ps_trunnion_virtual_up_limited = 2041U;
	static unsigned int gantry_ps_trunnion_virtual_out_limited = 2042U;
	static unsigned int gantry_ps_intermediate_virtual_up_limited = 2043U;
	static unsigned int gantry_ps_intermediate_virtual_out_limited = 2044U;
	static unsigned int gantry_ps_draghead_virtual_up_limited = 2045U;
	static unsigned int gantry_ps_draghead_virtual_out_limited = 2046U;

	static unsigned int gantry_sb_trunnion_details = 1329U;
	static unsigned int gantry_sb_intermediate_details = 1337U;
	static unsigned int gantry_sb_draghead_details = 1345U;

	static unsigned int gantry_sb_trunnion_virtual_up_limited = 2049U;
	static unsigned int gantry_sb_trunnion_virtual_out_limited = 2050U;
	static unsigned int gantry_sb_intermediate_virtual_up_limited = 2051U;
	static unsigned int gantry_sb_intermediate_virtual_out_limited = 2052U;
	static unsigned int gantry_sb_draghead_virtual_up_limited = 2053U;
	static unsigned int gantry_sb_draghead_virtual_out_limited = 2054U;

	static unsigned int ps_winch_gantry_allowed = 1312U;
	static unsigned int sb_winch_gantry_allowed = 1336U;

	static unsigned int all_ps_gantries_out = 1805U;
	static unsigned int all_ps_gantries_in = 1806U;
	static unsigned int all_sb_gantries_out = 1807U;
	static unsigned int all_sb_gantries_in = 1808U;

	static unsigned int suction_ps_buttons = 1913U;
	static unsigned int suction_sb_buttons = 1921U;

	static unsigned int ctension_ps_button = 1569U;
	static unsigned int ctension_sb_button = 1570U;

	static unsigned int ps_almo_auto = 1872U;
	static unsigned int sb_almo_auto = 1880U;

	/************************************************************************************************/
	void DI_gantry(WarGrey::SCADA::Gantrylet* target, const uint8* db4, unsigned int idx4_p1, const uint8* db205, unsigned int idx205_p1);
	void DI_gantry(WarGrey::SCADA::GantrySymbollet* target, const uint8* db4, unsigned int idx4_p1, const uint8* db205, unsigned int idx205_p1);

	bool DI_gantry_pushed_out(const uint8* db4, unsigned int idx4_p1);
	bool DI_gantry_pulled_in(const uint8* db4, unsigned int idx4_p1);

	bool DI_dredges_scene_control(const uint8* db4, unsigned int idx4_p1);
	bool DI_dredges_remote_control(const uint8* db4, unsigned int idx4_p1);
	bool DI_dredges_winch_allowed(const uint8* db4, unsigned int idx4_p1);
	bool DI_dredges_gantry_allowed(const uint8* db4, unsigned int idx4_p1);
	bool DI_dredges_emergence_stop(const uint8* db4, unsigned int idx4_p1);

	void DI_suction_buttons(WarGrey::SCADA::Buttonlet* intarget, WarGrey::SCADA::Buttonlet* detarget, const uint8* db205, unsigned int idx_p1);
	void DI_boolean_button(WarGrey::SCADA::Buttonlet* target, const uint8* db205, unsigned idx_p1);

	bool DI_suction_inflating(const uint8* db205, unsigned int idx_p1);
	bool DI_suction_deflating(const uint8* db205, unsigned int idx_p1);

	bool DI_long_sb_drag(const uint8* db205);
}
