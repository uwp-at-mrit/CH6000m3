#include <map>

#include "widget/alarms.hpp"
#include "stone/tongue/alarm.hpp"
#include "configuration.hpp"

#include "graphlet/tablet.hpp"
#include "schema/datalet/alarm_tbl.hpp"

#include "satellite.hpp"
#include "system.hpp"
#include "module.hpp"

using namespace WarGrey::SCADA;

using namespace Windows::Foundation;

using namespace Microsoft::Graphics::Canvas;
using namespace Microsoft::Graphics::Canvas::UI;
using namespace Microsoft::Graphics::Canvas::Text;

/*************************************************************************************************/
private class AlarmMS : public ISatellite, public PLCConfirmation, public IAlarmCursor, public ITableFilter {
public:
	virtual ~AlarmMS() noexcept {
		this->datasource->destroy();
	}

	AlarmMS() : ISatellite(default_logging_level, __MODULE__), margin(0.0F) {
		Syslog* logger = make_system_logger(default_logging_level, "AlarmHistory");

		this->style.resolve_column_width_percentage = alarm_column_width_configure;
		this->style.prepare_cell_style = alarm_cell_style_configure;

		this->datasource = new AlarmDataSource(this, logger, RotationPeriod::Daily);
		this->datasource->reference();
	}

	void fill_extent(float* width, float* height) {
		float margin = large_font_size * 4.0F;
		Size size = system_screen_size();

		SET_BOX(width, size.Width - margin * 2.0F);
		SET_BOX(height, size.Height - margin);
	}

public:
	void pre_read_data(Syslog* logger) override {
		this->enter_critical_section();
		this->begin_update_sequence();
	}

	void on_digital_input(long long timepoint_ms, const uint8* DB4, size_t count4, const uint8* DB205, size_t count205, Syslog* logger) override {
		Platform::String^ fields[_N(AMS)];
		Alarms* alarm = Alarms::first();
		unsigned int db, dbx;
		Alarm event;

		while (alarm != nullptr) {
			unsigned int alarm_index = alarm->ToIndex();
			alarm_index_translate(alarm_index, &db, &dbx);
			bool alerting = DBX(((db == 4) ? DB4 : DB205), dbx);
			auto maybe_alert = this->alerts.find(alarm_index);
			
			if (alerting) {
				if (maybe_alert == this->alerts.end()) {
					this->datasource->save(timepoint_ms, alarm->ToIndex(), event);
					this->alerts.insert(std::pair<unsigned int, Alarm>(alarm_index, event));
					this->alert_salts.insert(std::pair<long long, bool>(alarm_salt(event, true), true));

					alarm_extract(event, fields);

					logger->log_message(Log::Error, L"[%s] %s - %s",
						fields[_I(AMS::AlarmTime)]->Data(),
						fields[_I(AMS::Event)]->Data(),
						_speak("Alert")->Data());

					if (this->table != nullptr) {
						this->table->push_row(alarm_salt(event, true), fields);
					}
				}
			} else {
				if (maybe_alert != this->alerts.end()) {
					long long salt = alarm_salt(maybe_alert->second, true);
					auto alert_slot = this->alert_salts.find(salt);

					this->datasource->save(timepoint_ms, maybe_alert->second, event);
					
					alarm_extract(event, fields);

					logger->log_message(Log::Notice, L"[%s] %s - %s",
						fields[_I(AMS::AlarmTime)]->Data(),
						fields[_I(AMS::Event)]->Data(),
						_speak("Fixed")->Data());

					if (this->table != nullptr) {
						this->table->push_row(alarm_salt(event, false), fields);
						
						alarm_extract(maybe_alert->second, fields);
						this->table->update_row(salt, fields);
					}

					this->alerts.erase(maybe_alert);

					if (alert_slot != this->alert_salts.end()) {
						this->alert_salts.erase(alert_slot);
					}
				}
			}

			alarm = alarm->foreward();
		}
	}

	bool step(Alarm& alarm, bool asc, int code) override {
		unsigned int key = (unsigned int)(alarm.index);
		auto maybe_alert = this->alerts.find(key);

		if (maybe_alert == this->alerts.end()) {
			this->alerts.insert(std::pair<unsigned int, Alarm>(key, alarm));
			this->alert_salts.insert(std::pair<long long, bool>(alarm_salt(alarm, true), true));

			this->get_logger()->log_message(Log::Debug, L"Alerting alarm: %s",
				Alarms::fromIndex(key)->ToLocalString()->Data());
		} else { // this should not happen
			this->get_logger()->log_message(Log::Warning, L"Unexpected alerting alarm: %s, ignored",
				Alarms::fromIndex(key)->ToLocalString()->Data());
		}

		return true;
	}

	void post_read_data(Syslog* logger) override {
		this->end_update_sequence();
		this->leave_critical_section();
	}

public:
	void load(CanvasCreateResourcesReason reason, float width, float height) override {
		float inset = this->margin * 2.0F;

		this->table = this->insert_one(new Tablet<AMS>(__MODULE__, this->datasource, width - inset, height - inset));
		this->table->set_filter(this, _speak("AlertsOnly"), _speak("AllAlarms"));
		this->table->enable_filter(false);
		this->table->set_style(this->style);
	}

	void reflow(float width, float height) override {
		this->move_to(this->table, this->margin, this->margin);
	}

public:
	bool can_select(IGraphlet* g) override {
		return false;
	}

public:
	bool filter(long long salt) override {
		return (this->alert_salts.find(salt) != this->alert_salts.end());
	}

private: // never delete these graphlets manually.
	Tablet<AMS>* table;

private:
	AlarmDataSource* datasource;
	TableStyle style;
	float margin;

private:
	std::map<unsigned int, Alarm> alerts;
	std::map<long long, bool> alert_salts;
};

/*************************************************************************************************/
static AlarmMS* the_alarm = nullptr;

void WarGrey::SCADA::initialize_the_alarm(PLCMaster* plc) {
	if (the_alarm == nullptr) {
		the_alarm = new AlarmMS();

		plc->push_confirmation_receiver(the_alarm);
	}
}

void WarGrey::SCADA::display_the_alarm() {
	if (the_alarm != nullptr) {
		the_alarm->show();
	}
}

void WarGrey::SCADA::update_the_shown_alarm(long long count, long long interval, long long uptime) {
	if (the_alarm != nullptr) {
		if (the_alarm->shown()) {
			the_alarm->on_elapse(count, interval, uptime);
		}
	}
}
