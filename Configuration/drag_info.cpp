#include "drag_info.hpp"
#include "configuration.hpp"

using namespace WarGrey::SCADA;

/*************************************************************************************************/
void WarGrey::SCADA::fill_ps_drag_info(DragInfo* info) {
	info->offset_gapsize = ps_drag_offset_gapsize;
	info->offset_length = ps_drag_offset_length;
	info->pipe_lengths[0] = ps_drag_pipe1_length;
	info->pipe_lengths[1] = ps_drag_pipe2_length;
	info->pipe_radius = ps_drag_radius;
	info->head_width = ps_drag_head_width;
	info->head_height = ps_drag_head_length;
	info->visor_degrees_min = drag_visor_degrees_min;
	info->visor_degrees_max = drag_visor_degrees_max;
	info->arm_degrees_min = drag_arm_degrees_min;
	info->arm_degrees_max = drag_arm_degrees_max;
}

void WarGrey::SCADA::fill_sb_drag_info(DragInfo* info) {
	info->offset_gapsize = sb_drag_offset_gapsize;
	info->offset_length = sb_drag_offset_length;
	info->pipe_lengths[0] = sb_drag_pipe1_length;
	info->pipe_lengths[1] = sb_drag_pipe2_length;
	info->pipe_radius = sb_drag_radius;
	info->head_width = sb_drag_head_width;
	info->head_height = sb_drag_head_length;
	info->visor_degrees_min = drag_visor_degrees_min;
	info->visor_degrees_max = drag_visor_degrees_max;
	info->arm_degrees_min = drag_arm_degrees_min;
	info->arm_degrees_max = drag_arm_degrees_max;
}
