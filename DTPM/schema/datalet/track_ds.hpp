#pragma once

#include <ppltasks.h>

#include "graphlet/filesystem/project/dredgetracklet.hpp"

#include "sqlite3/rotation.hpp"

namespace WarGrey::SCADA {
	private class TrackDataSource
		: public WarGrey::DTPM::ITrackDataSource
		, public WarGrey::SCADA::RotativeSQLite3 {
	public:
		TrackDataSource(WarGrey::SCADA::Syslog* logger = nullptr,
			WarGrey::SCADA::RotationPeriod period = RotationPeriod::Daily,
			unsigned int period_count = 1U);

	public:
		bool ready() override;
		bool loading() override;
		void cancel() override;

	public:
		void load(WarGrey::DTPM::ITrackDataReceiver* receiver, long long open_s, long long close_s) override;
		void save(long long timepoint, long long type, WarGrey::SCADA::double3& dot) override;

	protected:
		void on_database_rotated(WarGrey::SCADA::SQLite3* prev_dbc, WarGrey::SCADA::SQLite3* current_dbc, long long timepoint) override;

	protected:
		~TrackDataSource() noexcept;

	private:
		void do_loading_async(WarGrey::DTPM::ITrackDataReceiver* receiver,
			long long start, long long end, long long interval,
			unsigned int file_count, unsigned int total, double span_ms);

	private:
		Concurrency::cancellation_token_source watcher;
		WarGrey::SCADA::ISQLite3* dbc;
		long long open_timepoint;
		long long close_timepoint;
		double time0;
	};
}
