#include "schema/datalet/track_ds.hpp"
#include "schema/track.hpp"
#include "dbmisc.hpp"

using namespace WarGrey::SCADA;
using namespace WarGrey::DTPM;

using namespace Concurrency;

using namespace Windows::Storage;

/*************************************************************************************************/
namespace {
	private class TrackCursor : public ITrackCursor {
	public:
		TrackCursor(ITrackDataReceiver* receiver, long long open_s, long long close_s) : receiver(receiver), open_s(open_s) {
			if (open_s < close_s) {
				this->open_timepoint = open_s * 1000LL;
				this->close_timepoint = close_s * 1000LL;
			} else {
				this->open_timepoint = close_s * 1000LL;
				this->close_timepoint = open_s * 1000LL;
			}
		}

	public:
		bool step(Track& track, bool asc, int code) override {
			long long ts = track.timestamp;

			if ((ts >= this->open_timepoint) && (ts <= this->close_timepoint)) {
				this->dot.x = track.x;
				this->dot.y = track.y;
				this->dot.z = track.z;

				this->receiver->on_datum_values(this->open_s, ts, track.group, this->dot);

				this->count++;
			}

			return (asc ? (ts <= this->close_timepoint) : (ts >= this->open_timepoint));
		}

	public:
		unsigned int count;

	private:
		double3 dot;
		ITrackDataReceiver* receiver;
		long long open_timepoint;
		long long close_timepoint;
		long long open_s;
	};
}

static int track_busy_handler(void* args, int count) {
	// keep trying until it works
	return 1;
}

/*************************************************************************************************/
TrackDataSource::TrackDataSource(Syslog* logger, RotationPeriod period, unsigned int period_count)
	: RotativeSQLite3("track", logger, period, period_count), open_timepoint(0LL) {}

TrackDataSource::~TrackDataSource() {
	this->cancel();

	if (this->dbc != nullptr) {
		delete this->dbc;
	}
}

bool TrackDataSource::ready() {
	return IRotativeDirectory::root_ready() && RotativeSQLite3::ready();
}

bool TrackDataSource::loading() {
	return (this->open_timepoint > 0LL);
}

void TrackDataSource::cancel() {
	if (this->loading()) {
		this->watcher.cancel();
	}
}

void TrackDataSource::on_database_rotated(WarGrey::SCADA::SQLite3* prev_dbc, WarGrey::SCADA::SQLite3* dbc, long long timepoint) {
	// TODO: move the temporary data from in-memory SQLite3 into the current SQLite3

	create_track(dbc, true);
	this->get_logger()->log_message(Log::Debug, L"current file: %S", dbc->filename().c_str());
}

void TrackDataSource::load(ITrackDataReceiver* receiver, long long open_s, long long close_s) {
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

void TrackDataSource::save(long long timepoint, long long group, double3& dot) {
	Track track;

	track.uuid = pk64_timestamp();
	track.group = group;
	track.x = dot.x;
	track.y = dot.y;
	track.z = dot.z;
	track.timestamp = timepoint;

	insert_track(this, track);
}

void TrackDataSource::do_loading_async(ITrackDataReceiver* receiver
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
				TrackCursor tcursor(receiver, this->open_timepoint, this->close_timepoint);
				double ms = current_inexact_milliseconds();
				
				this->dbc = new SQLite3(db->Path->Data(), this->get_logger());
				this->dbc->set_busy_handler(track_busy_handler);

				receiver->begin_maniplation_sequence();
				foreach_track(this->dbc, &tcursor, 0, 0, track::timestamp, asc);
				receiver->end_maniplation_sequence();

				delete this->dbc;
				this->dbc = nullptr;

				ms = current_inexact_milliseconds() - ms;
				this->get_logger()->log_message(Log::Debug, L"loaded %d record(s) from[%s] within %lfms",
					tcursor.count, dbsource->Data(), ms);

				this->do_loading_async(receiver, next_timepoint, end, interval,
					file_count + 1LL, total + tcursor.count, span_ms + ms);
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
