#include "schema/datalet/earthwork_ts.hpp"
#include "schema/earthwork.hpp"
#include "dbmisc.hpp"

using namespace WarGrey::SCADA;

using namespace Concurrency;

using namespace Windows::Foundation;
using namespace Windows::System;
using namespace Windows::Storage;

using namespace Microsoft::Graphics::Canvas::Brushes;

/*************************************************************************************************/
CanvasSolidColorBrush^ WarGrey::SCADA::earthwork_line_color_dictionary(unsigned int index) {
	CanvasSolidColorBrush^ color = nullptr;

	switch (_E(EWTS, index % _N(EWTS))) {
	case EWTS::EarthWork: color = Colours::Khaki; break;
	case EWTS::Capacity: color = Colours::Cyan; break;
	case EWTS::HopperHeight: color = Colours::Crimson; break;
	case EWTS::Loading: color = Colours::Orange; break;
	case EWTS::Displacement: color = Colours::MediumSeaGreen; break;
	}

	return color;
}

private class EarthWorkCursor : public IEarthWorkCursor {
public:
	EarthWorkCursor(ITimeSeriesDataReceiver* receiver, long long open_s, long long close_s) : receiver(receiver), open_s(open_s) {
		if (open_s < close_s) {
			this->open_timepoint = open_s * 1000LL;
			this->close_timepoint = close_s * 1000LL;
		} else {
			this->open_timepoint = close_s * 1000LL;
			this->close_timepoint = open_s * 1000LL;
		}
	}

public:
	bool step(EarthWork& ework, bool asc, int code) override {
		long long ts = ework.timestamp;
		
		if ((ts >= this->open_timepoint) && (ts <= this->close_timepoint)) {
			this->tempdata[_I(EWTS::EarthWork)] = ework.product;
			this->tempdata[_I(EWTS::Capacity)] = ework.vessel;
			this->tempdata[_I(EWTS::HopperHeight)] = ework.hopper_height;
			this->tempdata[_I(EWTS::Loading)] = ework.loading;
			this->tempdata[_I(EWTS::Displacement)] = ework.displacement;

			this->receiver->on_datum_values(this->open_s, ts, this->tempdata, _N(EWTS));

			this->count++;
		}

		return (asc ? (ts <= this->close_timepoint) : (ts >= this->open_timepoint));
	}

public:
	unsigned int count;

private:
	double tempdata[_N(EWTS)];
	ITimeSeriesDataReceiver* receiver;
	long long open_timepoint;
	long long close_timepoint;
	long long open_s;
};

static int earthwork_busy_handler(void* args, int count) {
	// keep trying until it works
	return 1;
}

/*************************************************************************************************/
EarthWorkDataSource::EarthWorkDataSource(Syslog* logger, RotationPeriod period, unsigned int period_count)
	: RotativeSQLite3("earthwork", logger, period, period_count), open_timepoint(0LL) {}

EarthWorkDataSource::~EarthWorkDataSource() {
	this->cancel();

	if (this->dbc != nullptr) {
		delete this->dbc;
	}
}

bool EarthWorkDataSource::ready() {
	return IRotativeDirectory::root_ready() && RotativeSQLite3::ready();
}

bool EarthWorkDataSource::loading() {
	return (this->open_timepoint > 0LL);
}

void EarthWorkDataSource::cancel() {
	if (this->loading()) {
		this->watcher.cancel();
	}
}

void EarthWorkDataSource::on_database_rotated(WarGrey::SCADA::SQLite3* prev_dbc, WarGrey::SCADA::SQLite3* dbc, long long timepoint) {
	// TODO: move the temporary data from in-memory SQLite3 into the current SQLite3

	create_earthwork(dbc, true);
	this->get_logger()->log_message(Log::Debug, L"current file: %S", dbc->filename().c_str());
}

void EarthWorkDataSource::load(ITimeSeriesDataReceiver* receiver, long long open_s, long long close_s) {
	if (!this->loading()) {
		long long start = this->resolve_timepoint(open_s);
		long long end = this->resolve_timepoint(close_s);
		long long interval = this->span_seconds() * ((open_s < close_s) ? 1LL : -1LL);
		
		this->get_logger()->log_message(Log::Debug, L"start loading from %s to %s",
			make_timestamp_utc(open_s, true)->Data(), make_timestamp_utc(close_s, true)->Data());

		this->open_timepoint = open_s;
		this->close_timepoint = close_s;
		this->time0 = current_inexact_milliseconds();
		this->do_loading_async(receiver, start, end, interval, 0LL, 0LL, 0.0);
	}
}

void EarthWorkDataSource::save(long long timepoint, double* values, unsigned int n) {
	EarthWork ework;

	ework.uuid = pk64_timestamp();
	ework.timestamp = timepoint;

	for (unsigned int i = 0; i < n; i++) {
		switch (_E(EWTS, i)) {
		case EWTS::EarthWork: ework.product = values[i]; break;
		case EWTS::Capacity : ework.vessel = values[i]; break;
		case EWTS::HopperHeight: ework.hopper_height = values[i]; break;
		case EWTS::Loading: ework.loading = values[i]; break;
		case EWTS::Displacement: ework.displacement = values[i]; break;
		}
	}

	insert_earthwork(this, ework);
}

void EarthWorkDataSource::do_loading_async(ITimeSeriesDataReceiver* receiver
	, long long start, long long end, long long interval
	, unsigned int file_count, unsigned int total, double span_ms) {
	bool asc = (interval > 0);

	if ((asc ? (start > end) : (start < end))) {
		double span_total = current_inexact_milliseconds() - this->time0;

		this->get_logger()->log_message(Log::Debug, L"loaded %d records from %d file(s) within %lfms(wasted: %lfms)",
			total, file_count, span_total, span_total - span_ms);

		receiver->on_maniplation_complete(this->open_timepoint, this->close_timepoint);
		this->open_timepoint = 0LL;
	} else {
		Platform::String^ dbsource = this->resolve_filename(start);
		cancellation_token token = this->watcher.get_token();

		create_task(this->rootdir()->TryGetItemAsync(dbsource), token).then([=](task<IStorageItem^> getting) {
			IStorageItem^ db = getting.get();
			long long next_timepoint = start + interval;

			if ((db != nullptr) && (db->IsOfType(StorageItemTypes::File))) {
				EarthWorkCursor ecursor(receiver, this->open_timepoint, this->close_timepoint);
				double ms = current_inexact_milliseconds();
				
				this->dbc = new SQLite3(db->Path->Data(), this->get_logger());
				this->dbc->set_busy_handler(earthwork_busy_handler);

				receiver->begin_maniplation_sequence();
				foreach_earthwork(this->dbc, &ecursor, 0, 0, earthwork::timestamp, asc);
				receiver->end_maniplation_sequence();

				delete this->dbc;
				this->dbc = nullptr;

				ms = current_inexact_milliseconds() - ms;
				this->get_logger()->log_message(Log::Debug, L"loaded %d record(s) from[%s] within %lfms",
					ecursor.count, dbsource->Data(), ms);

				this->do_loading_async(receiver, next_timepoint, end, interval,
					file_count + 1LL, total + ecursor.count, span_ms + ms);
			} else {
				this->get_logger()->log_message(Log::Debug, L"skip non-existent source[%s]", dbsource->Data());
				this->do_loading_async(receiver, next_timepoint, end, interval, file_count, total, span_ms);
			}
		}).then([=](task<void> check_exn) {
			try {
				check_exn.get();
			} catch (Platform::Exception^ e) {
				this->on_exception(e);
			} catch (task_canceled&) {}
		});
	}
}
