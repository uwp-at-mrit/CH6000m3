#pragma once

#include <ppltasks.h>
#include <map>

#include "schema/alarm.hpp"

#include "graphlet/tablet.hpp"

#include "sqlite3/rotation.hpp"

namespace WarGrey::SCADA {
	private enum class AMS { Code, Event, Type, AlarmTime, FixedTime, _ };

	float alarm_column_width_configure(unsigned int idx, unsigned int total);
	void alarm_cell_style_configure(unsigned int idx, long long salt, WarGrey::SCADA::TableCellStyle* style);

	long long alarm_salt(WarGrey::SCADA::Alarm& alarm, bool alert);

	void alarm_extract(WarGrey::SCADA::Alarm& alarm, Platform::String^ fields[]);

	void alarm_index_translate(unsigned int index, unsigned int* db, unsigned int* dbx);
	unsigned int alarm_index_to_code(unsigned int index);

	private class AlarmDataSource
		: public WarGrey::SCADA::ITableDataSource
		, public WarGrey::SCADA::RotativeSQLite3 {
	public:
		AlarmDataSource(WarGrey::SCADA::IAlarmCursor* alerts_cursor, WarGrey::SCADA::Syslog* logger = nullptr,
			WarGrey::SCADA::RotationPeriod period = RotationPeriod::Daily,
			unsigned int period_count = 1U, unsigned int search_file_count_max = 31);

	public:
		void save(long long timepoint_ms, unsigned int index, WarGrey::SCADA::Alarm& alarm);
		void save(long long timepoint_ms, WarGrey::SCADA::Alarm& alerting_alarm, WarGrey::SCADA::Alarm& alarm);

	public:
		bool ready() override;
		bool loading() override;
		void cancel() override;

	public:
		void load(WarGrey::SCADA::ITableDataReceiver* receiver, long long request_count) override;

	protected:
		void on_folder_ready(Windows::Storage::StorageFolder^ root, bool newly_created) override;
		void on_database_rotated(WarGrey::SCADA::SQLite3* prev_dbc, WarGrey::SCADA::SQLite3* current_dbc, long long timepoint) override;

	protected:
		~AlarmDataSource() noexcept;

	private:
		void do_loading_async(WarGrey::SCADA::ITableDataReceiver* receiver, long long start, long long interval,
			unsigned int actual_file_count, unsigned int search_file_count, long long total, double span_ms);

	private:
		Concurrency::cancellation_token_source watcher;
		WarGrey::SCADA::ISQLite3* dbc;
		unsigned int search_file_count_max;
		long long request_count;
		double time0;

	private:
		WarGrey::SCADA::ISQLite3* alerts_dbc;
		WarGrey::SCADA::IAlarmCursor* alerts_cursor; // managed by itself
	};
}
