#pragma once

#include "mrit.hpp"

#include "datum/flonum.hpp"

#include "syslog.hpp"

namespace WarGrey::SCADA {
	bool DBX(const uint8* src, size_t idx);
	bool DBX(const uint8* src, size_t idx, size_t bidx);
	float DBD(const uint8* src, size_t idx);
	float RealData(const uint8* src, size_t idx);

	void read_drag_figures(const uint8* DB2,
		WarGrey::SCADA::double3* offset, WarGrey::SCADA::double3 ujoints[], WarGrey::SCADA::double3* draghead,
		unsigned int drag_idx);

	void read_drag_figures(const uint8* DB2, const uint8* DB203,
		WarGrey::SCADA::double3* offset, WarGrey::SCADA::double3 ujoints[], WarGrey::SCADA::double3* draghead,
		double* visor_angle, unsigned int drag_idx, unsigned int visor_idx, double visor_angle_min, double visor_angle_max);

	void read_drag_figures(const uint8* DB2, const uint8* DB203,
		WarGrey::SCADA::double3* offset, WarGrey::SCADA::double3 ujoints[], WarGrey::SCADA::double3* draghead,
		double* suction_depth, double* visor_angle, unsigned int drag_idx, unsigned int visor_idx, double visor_angle_min, double visor_angle_max);

	private class PLCConfirmation : public WarGrey::SCADA::MRConfirmation {
	public:
		void on_all_signals(long long timepoint_ms, size_t addr0, size_t addrn, uint8* data, size_t size, WarGrey::SCADA::Syslog* logger) override;

	public:
		virtual void on_digital_input(long long timepoint_ms, const uint8* db4, size_t count4, const uint8* db205, size_t count205, WarGrey::SCADA::Syslog* logger) {}
		virtual void on_analog_input(long long timepoint_ms, const uint8* db2, size_t count2, const uint8* db203, size_t count203, WarGrey::SCADA::Syslog* logger) {}
		virtual void on_forat(long long timepoint_ms, const uint8* db20, size_t count, WarGrey::SCADA::Syslog* logger) {}
		virtual void on_analog_io(long long timepoint_ms, const uint8* db204, size_t count204, WarGrey::SCADA::Syslog* logger) {}
	};

	private class PLCMaster : public WarGrey::SCADA::MRMaster {
	public:
		PLCMaster(Syslog* logger, Platform::String^ server, unsigned short port, long long timeout = 0LL);

	public:
		void send_scheduled_request(long long count, long long interval, long long uptime);
		void send_setting(int16 address, float datum);
		void send_command(uint8 idx, uint8 bidx);
		void send_command(uint16 index_p1);

	private:
		long long last_sent_time;
	};
}
