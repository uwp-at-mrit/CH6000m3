#pragma once

namespace WarGrey::SCADA {
	// DB203, real data
	static unsigned int visor_tank_temperature = 18U;
	static unsigned int master_tank_temperature = 19U;
	static unsigned int visor_tank_level = 20U;
	static unsigned int master_back_oil_pressure = 21U;

	static unsigned int overflow_pipe_progress = 55U;

	// DB2, DBD
	static unsigned int tilde_mark = 0U;
	static unsigned int gps_speed = 584U;

	static unsigned int ps_suction_draught = 20U; // same as the ps suction depth 
	static unsigned int sb_suction_draught = 96U; // same as the sb suction depth

	static unsigned int ps_fixed_bow_draught = 156U;
	static unsigned int sb_fixed_bow_draught = 160U;
	static unsigned int fixed_bow_draught = 164U;
	static unsigned int ps_fixed_center_draught = 168U;
	static unsigned int sb_fixed_center_draught = 172U;
	static unsigned int fixed_certer_draught = 176U;
	static unsigned int ps_fixed_stern_draught = 180U;
	static unsigned int sb_fixed_stern_draught = 184U;
	static unsigned int fixed_stern_draught = 188U;
	static unsigned int average_draught = 192U;

	static unsigned int ps_bow_hopper_height = 208U;
	static unsigned int sb_bow_hopper_height = 212U;
	static unsigned int ps_stern_hopper_height = 216U;
	static unsigned int sb_stern_hopper_height = 220U;
	
	static unsigned int average_hopper_height = 224U;
	static unsigned int displacement_value = 228U;
	static unsigned int loading_value = 232U;
	static unsigned int earthwork_value = 236U;
	static unsigned int vessel_value = 320U;
	static unsigned int vessal_netweight = 324U;

	static unsigned int trim_degrees = 200U;
	static unsigned int heel_degrees = 204U;
}
