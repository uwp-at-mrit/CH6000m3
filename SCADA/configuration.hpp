#pragma once

#include "syslog.hpp"

#ifdef _DEBUG
static WarGrey::SCADA::Log default_logging_level = WarGrey::SCADA::Log::Debug;
#else
static WarGrey::SCADA::Log default_logging_level = WarGrey::SCADA::Log::Info;
#endif

static WarGrey::SCADA::Log default_plc_master_logging_level = WarGrey::SCADA::Log::Info;

//static Platform::String^ remote_test_server = "172.20.10.2";
static Platform::String^ remote_test_server = "192.168.0.152";

static Platform::String^ root_machines[] = {
	"192.168.0.6",
	"192.168.0.9",
	"192.168.0.7",
	"192.168.0.11",
	"192.168.0.12",

	"192.168.0.10"
};

/*************************************************************************************************/
static const unsigned int frame_per_second = 5U;
static const unsigned int timemachine_frame_per_second = 4U;
static const long long timemachine_speed = 2; // second per step
static const long long plc_master_suicide_timeout = 4000;

static const unsigned int diagnostics_caption_background = 0x8FBC8F;
static const unsigned int diagnostics_caption_foreground = 0xF8F8FF;
static const unsigned int diagnostics_region_background = 0x414141U;
static const unsigned int diagnostics_alarm_background = 0x141414U;

static const float large_metrics_font_size = 24.0F;
static const float normal_metrics_font_size = 22.0F;
static const float small_metrics_font_size = 20.0F;

static const float large_font_size = 18.0F;
static const float normal_font_size = 16.0F;
static const float small_font_size = 14.0F;
static const float tiny_font_size = 12.0F;

/*************************************************************************************************/
static const size_t hopper_count = 7;

static const unsigned int gland_pump_rpm_range = 1500U;

static const double hopper_height_range = 13.85;
static const double earthwork_range = 10000.0;
static const double vessel_range = 10000.0;
static const double loading_range = 13000.0;
static const double displacement_range = 22000.0;
static const double compensator_range = 3.0;
static const double timeseries_range = displacement_range + 2000.0;

static const float ps_drag_trunnion_gapsize = 1.845F;
static const float ps_drag_trunnion_length = 4.135F;
static const float ps_drag_pipe1_length = 22.978F;
static const float ps_drag_pipe2_length = 21.78F;
static const float ps_drag_radius = 0.5F;
static const float ps_drag_head_width = 4.03F;
static const float ps_drag_head_length = 2.54F;
static const float ps_drag_head_compensation = 0.5F;

static const float sb_drag_trunnion_gapsize = 1.845F;
static const float sb_drag_trunnion_length = 4.135F;
static const float sb_drag_pipe1_length = 22.978F;
static const float sb_drag_pipe2_length = 22.642F;
static const float sb_drag_pipe2_enlength = 12.0F;
static const float sb_drag_radius = 0.5F;
static const float sb_drag_head_width = 4.03F;
static const float sb_drag_head_length = 2.54F;
static const float sb_drag_head_compensation = 0.5F;

static const double drag_visor_tank_range = 80.0;
static const double drag_visor_degrees_min = 0.0;
static const double drag_visor_degrees_max = 50.0;
static const double drag_arm_degrees_min = 0.0;
static const double drag_arm_degrees_max = 60.0;
static const double drag_depth_degrees_max = 42.0;

static const unsigned int default_ps_color = 0xFF0000;
static const unsigned int default_sb_color = 0x008000;
static const unsigned int default_pipe_color = 0xC0C0C0;

static const float default_pipe_thickness = 2.0F;

/*************************************************************************************************/
#define resolve_gridsize(gwidth, gheight) ((gwidth < gheight) ? gheight : gwidth)
