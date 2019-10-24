#pragma once

#include "syslog.hpp"

#ifdef _DEBUG
static WarGrey::SCADA::Log default_logging_level = WarGrey::SCADA::Log::Debug;
#else
static WarGrey::SCADA::Log default_logging_level = WarGrey::SCADA::Log::Info;
#endif

static WarGrey::SCADA::Log default_plc_master_logging_level = WarGrey::SCADA::Log::Info;
static WarGrey::SCADA::Log default_gps_logging_level = WarGrey::SCADA::Log::Info;

static Platform::String^ remote_test_server = "255.255.255.255";
static Platform::String^ system_subnet_prefix = "192.168";
// static Platform::String^ moxa_gateway = "192.168.0.253";

static const unsigned short gps1_port = 4006;
static const unsigned short gps2_port = 4002;
static const unsigned short gyro_port = 4001;

static const unsigned short scada_plc_master_port = 2008;
static const unsigned short construction_plc_master_port = 2008;

// UWP can connect to, but cannot be connected by, local non-UWP application.
static Platform::String^ moxa_gateway = "127.0.0.1";
static Platform::String^ plc_hostname = "127.0.0.1";

/*************************************************************************************************/
static const unsigned int frame_per_second = 5U;
static const unsigned int timemachine_frame_per_second = 4U;
static const long long timemachine_speed = 2; // seconds per step
static const long long plc_master_suicide_timeout = 4000;
static const long long plc_settings_pinfree_seconds = 600;
static const long long gps_suicide_timeout = 4000;

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
static const double payload_range = 13000.0;
static const double displacement_range = 22000.0;
static const double compensator_range = 3.0;
static const double timeseries_range = displacement_range + 2000.0;

static const double ps_drag_offset_gapsize = 1.845;
static const double ps_drag_offset_length = 4.135;
static const double ps_drag_pipe1_length = 22.978;
static const double ps_drag_pipe2_length = 21.78;
static const double ps_drag_radius = 0.5;
static const double ps_drag_head_width = 4.03;
static const double ps_drag_head_length = 2.54;
static const double ps_drag_head_compensation = 0.5;

static const double sb_drag_offset_gapsize = 1.845;
static const double sb_drag_offset_length = 4.135;
static const double sb_drag_pipe1_length = 22.978;
static const double sb_drag_pipe2_length = 22.642;
static const double sb_drag_pipe2_enlength = 12.0;
static const double sb_drag_radius = 0.5;
static const double sb_drag_head_width = 4.03;
static const double sb_drag_head_length = 2.54;
static const double sb_drag_head_compensation = 0.5;

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
