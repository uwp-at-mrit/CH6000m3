#include "iotables/ai_dredges.hpp"

using namespace WarGrey::SCADA;

static DredgeAddress* ps_schema;
static DredgeAddress* sb_schema;

/*************************************************************************************************/
DredgeAddress* WarGrey::SCADA::make_ps_dredging_system_schema() {
	if (ps_schema == nullptr) {
		DredgeAddress* schema = new DredgeAddress();

		schema->drag_position = 20U;
		schema->visor_angle = 272U;

		schema->compensator = 82U;
		schema->density_speed = 172U;
		schema->pulling_force = 110U;

		schema->suction_inflator_pressure = 33U;
		schema->differential_pressure = ps_draghead_differential_pressure;
		schema->vacuum_pressure = 164U;
		schema->discharge_pressure = 165U;

		schema->winch_speed = 280U;
		schema->winch_length = 544U;

		schema->backarm_vertical_angle = 104U;
		schema->backarm_horizontal_angle = 105U;
		schema->forearm_vertical_angle = 106U;
		schema->forearm_horizontal_angle = 107U;
		schema->visor_progress = 108U;
		
		ps_schema = schema;
	}

	return ps_schema;
}

DredgeAddress* WarGrey::SCADA::make_sb_dredging_system_schema() {
	if (sb_schema == nullptr) {
		DredgeAddress* schema = new DredgeAddress();

		schema->drag_position = 96U;
		schema->visor_angle = 276U;

		schema->compensator = 98U;
		schema->density_speed = 174U;
		schema->pulling_force = 126U;

		schema->suction_inflator_pressure = 37U;
		schema->differential_pressure = sb_draghead_differential_pressure;
		schema->vacuum_pressure = 166U;
		schema->discharge_pressure = 167U;

		schema->winch_speed = 292U;
		schema->winch_length = 556U;

		schema->backarm_vertical_angle = 120U;
		schema->backarm_horizontal_angle = 121U;
		schema->forearm_vertical_angle = 122U;
		schema->forearm_horizontal_angle = 123U;
		schema->visor_progress = 124U;
		
		sb_schema = schema;
	}

	return sb_schema;
}
