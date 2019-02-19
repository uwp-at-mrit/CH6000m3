#include "schema/datalet/alarm_tbl.hpp"
#include "stone/tongue/alarm.hpp"
#include "dbmisc.hpp"

using namespace WarGrey::SCADA;

using namespace Concurrency;

using namespace Windows::Foundation;
using namespace Windows::System;
using namespace Windows::Storage;

using namespace Microsoft::Graphics::Canvas::Brushes;

#define IS_Alert(salt) (((salt >> 62U) & 0b1) == 0b1)

static int alarm_busy_handler(void* args, int count) {
	// keep trying until it works
	return 1;
}

/*************************************************************************************************/
float WarGrey::SCADA::alarm_column_width_configure(unsigned int idx, unsigned int total) {
	float percentage = 1.0F / float(total);
	
	switch (_E(AMS, idx)) {
	case AMS::Event: percentage = 0.4F; break;
	case AMS::AlarmTime: case AMS::FixedTime: percentage = 0.2F; break;
	case AMS::Code: case AMS::Type: percentage = 0.1F; break;
	}

	return percentage;
}

long long WarGrey::SCADA::alarm_salt(Alarm& alarm, bool alert) {
	long long salt = alarm.uuid; // see pk64_timestamp();

	if (alert) {
		salt = (0b1ULL << 62U) | salt;
	}

	return salt;
}

void WarGrey::SCADA::alarm_cell_style_configure(unsigned int idx, long long salt, TableCellStyle* style) {
	bool alert = IS_Alert(salt);

	switch (_E(AMS, idx)) {
	case AMS::Event: style->align_fx = 0.0F; break;
	default: style->align_fx = 0.5F; break;
	}

	style->align_fy = 0.5F;
	style->background_color = alert ? Colours::Red : Colours::RoyalBlue;
}

void WarGrey::SCADA::alarm_extract(Alarm& alarm, Platform::String^ fields[]) {
	unsigned int index = (unsigned int)alarm.index;
	long long alarmtime = alarm.alarmtime / 1000LL;
	long long fixedtime = (alarm.fixedtime.has_value() ? (alarm.fixedtime.value() / 1000LL) : 0LL);

	fields[_I(AMS::Code)] = alarm_index_to_code(index).ToString();
	fields[_I(AMS::Event)] = Alarms::fromIndex(index)->ToLocalString();
	fields[_I(AMS::Type)] = alarm.type.value_or(1000LL).ToString();
	fields[_I(AMS::AlarmTime)] = make_timestamp_utc(alarmtime, true);

	if (fixedtime > alarmtime) {
		fields[_I(AMS::FixedTime)] = make_timestamp_utc(fixedtime, true);
	} else if (fixedtime == 0) {
		fields[_I(AMS::FixedTime)] = "";
	} else {
		fields[_I(AMS::FixedTime)] = "-";
	}
}

void WarGrey::SCADA::alarm_index_translate(unsigned int index, unsigned int* db, unsigned int* dbx) {
	SET_BOX(db, index >> 16);
	SET_BOX(dbx, index & 0xFFFF);
}

unsigned int WarGrey::SCADA::alarm_index_to_code(unsigned int index) {
	unsigned int db, dbx;

	alarm_index_translate(index, &db, &dbx);

	if (db == 4) {
		dbx += (4U * 8U);
	} else {
		dbx += (204U * 8U);
	}

	return dbx;
}

/*************************************************************************************************/
private class AlarmCursor : public IAlarmCursor {
public:
	AlarmCursor(ITableDataReceiver* receiver, long long request_count, long long loaded_count)
		: receiver(receiver), request_count(request_count), loaded_count(loaded_count) {}

public:
	bool step(Alarm& alarm, bool asc, int code) override {
		bool go_on = (this->loaded_count < this->request_count);
		
		if (go_on) {
			long long salt = alarm_salt(alarm, alarm.fixedtime != alarm.alarmtime);
		
			alarm_extract(alarm, this->tempdata);
			this->receiver->on_row_datum(this->request_count, ++this->loaded_count, salt, this->tempdata, _N(AMS));
		}

		return go_on;
	}

public:
	long long loaded_count;

private:
	Platform::String^ tempdata[_N(AMS)];
	ITableDataReceiver* receiver;
	long long request_count;
};

/*************************************************************************************************/
AlarmDataSource::AlarmDataSource(IAlarmCursor* cursor, Syslog* logger, RotationPeriod period, unsigned int period_count, unsigned int search_max)
	: RotativeSQLite3("alarm", logger, period, period_count), alerts_cursor(cursor), search_file_count_max(search_max), request_count(0LL) {}

AlarmDataSource::~AlarmDataSource() {
	this->cancel();

	if (this->dbc != nullptr) {
		delete this->dbc;
	}

	if (this->alerts_dbc != nullptr) {
		delete this->alerts_dbc;
	}
}

bool AlarmDataSource::ready() {
	return IRotativeDirectory::root_ready() && RotativeSQLite3::ready();
}

bool AlarmDataSource::loading() {
	return (this->request_count > 0LL);
}

void AlarmDataSource::cancel() {
	if (this->loading()) {
		this->watcher.cancel();
	}
}

void AlarmDataSource::on_folder_ready(StorageFolder^ root, bool newly_created) {
	this->alerts_dbc = new SQLite3((root->Path + "/alerts.db")->Data(), this->get_logger());
	this->alerts_dbc->set_busy_handler(alarm_busy_handler);

	create_alarm(this->alerts_dbc, true);
	this->get_logger()->log_message(Log::Info, L"unfixed alarms file: %S", this->alerts_dbc->filename().c_str());

	foreach_alarm(this->alerts_dbc, this->alerts_cursor);
}

void AlarmDataSource::on_database_rotated(SQLite3* prev_dbc, SQLite3* dbc, long long timepoint) {
	// TODO: move the temporary data from in-memory SQLite3 into the current SQLite3 if necessary

	create_alarm(dbc, true);
	this->get_logger()->log_message(Log::Debug, L"current file: %S", dbc->filename().c_str());
}

void AlarmDataSource::save(long long timepoint_ms, unsigned int index, Alarm& alarm) { // alerting
	default_alarm(alarm);

	alarm.index = index;
	alarm.alarmtime = timepoint_ms;
	alarm.fixedtime = 0LL;

	insert_alarm(this->alerts_dbc, alarm);
	insert_alarm(this, alarm);
}

void AlarmDataSource::save(long long timepoint_ms, Alarm& alerting_alarm, Alarm& alarm) { // response
	default_alarm(alarm);

	alarm.index = alerting_alarm.index;
	alarm.alarmtime = timepoint_ms;
	alarm.fixedtime = timepoint_ms;

	insert_alarm(this, alarm);

	{ // update alarm state
		ISQLite3* target = new SQLite3(this->resolve_pathname(alerting_alarm.alarmtime / 1000LL)->Data(), this->get_logger());
		Alarm_pk pk = alarm_identity(alerting_alarm); // TODO: why cannot do `delete_alarm(dbc, alarm_identity(a))` directly?

		alerting_alarm.fixedtime = timepoint_ms;

		update_alarm(target, alerting_alarm);
		delete_alarm(this->alerts_dbc, pk);

		delete target;
	}
}

void AlarmDataSource::load(ITableDataReceiver* receiver, long long request_count) {
	if (!this->loading()) {
		long long start = this->resolve_timepoint(current_seconds());
		long long interval = -this->span_seconds();
		
		this->get_logger()->log_message(Log::Debug, "start loading from now");

		this->request_count = request_count;
		this->time0 = current_inexact_milliseconds();
		this->do_loading_async(receiver, start, interval, 0LL, 0LL, 0LL, 0.0);
	}
}

void AlarmDataSource::do_loading_async(ITableDataReceiver* receiver, long long start, long long interval
	, unsigned int actual_file_count, unsigned int search_file_count, long long total, double span_ms) {
	bool asc = (interval > 0);

	if ((total >= this->request_count) || (search_file_count > this->search_file_count_max)) {
		double span_total = current_inexact_milliseconds() - this->time0;

		this->get_logger()->log_message(Log::Debug, L"loaded %d records from %d file(s) within %lfms(wasted: %lfms)",
			total, actual_file_count, span_total, span_total - span_ms);

		receiver->on_maniplation_complete(this->request_count);
		this->request_count = 0LL;
	} else {
		Platform::String^ dbsource = this->resolve_filename(start);
		cancellation_token token = this->watcher.get_token();

		create_task(this->rootdir()->TryGetItemAsync(dbsource), token).then([=](task<IStorageItem^> getting) {
			IStorageItem^ db = getting.get();
			long long next_timepoint = start + interval;

			if ((db != nullptr) && (db->IsOfType(StorageItemTypes::File))) {
				AlarmCursor acursor(receiver, this->request_count, total);
				double ms = current_inexact_milliseconds();
				
				this->dbc = new SQLite3(db->Path->Data(), this->get_logger());
				this->dbc->set_busy_handler(alarm_busy_handler);

				receiver->begin_maniplation_sequence();
				foreach_alarm(this->dbc, &acursor, this->request_count - total, 0, alarm::alarmtime, asc);
				receiver->end_maniplation_sequence();

				delete this->dbc;
				this->dbc = nullptr;

				ms = current_inexact_milliseconds() - ms;
				this->get_logger()->log_message(Log::Debug, L"loaded %d record(s) from[%s] within %lfms",
					acursor.loaded_count - total, dbsource->Data(), ms);

				this->do_loading_async(receiver, next_timepoint, interval,
					actual_file_count + 1U, search_file_count + 1U, acursor.loaded_count, span_ms + ms);
			} else {
				this->get_logger()->log_message(Log::Debug, L"skip non-existent source[%s]", dbsource->Data());
				this->do_loading_async(receiver, next_timepoint, interval,
					actual_file_count, search_file_count + 1U, total, span_ms);
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
