#include "widget/timestream.hpp"
#include "configuration.hpp"

#include "datum/box.hpp"

#include "system.hpp"
#include "module.hpp"

using namespace WarGrey::SCADA;

using namespace Windows::Foundation;
using namespace Windows::Storage;

using namespace Microsoft::Graphics::Canvas::UI;

/*************************************************************************************************/
namespace {
	private class TimeStream : public TimeMachine, public MRConfirmation {
	public:
		TimeStream(long long time_speed, int frame_rate)
			: TimeMachine(L"timemachine", time_speed * 1000LL, frame_rate, make_system_logger(default_logging_level, __MODULE__))
			, last_timepoint(current_milliseconds()) {}

		void fill_extent(float* width, float* height) override {
			float margin = normal_font_size * 2.0F;
			Size size = system_screen_size();

			SET_BOX(width, size.Width - margin);
			SET_BOX(height, size.Height - margin);
		}

	public:
		void on_all_signals(long long timepoint_ms, size_t addr0, size_t addrn, uint8* data, size_t size, Syslog* logger) override {
			if ((timepoint_ms - last_timepoint) >= this->get_time_speed()) {
				this->save_snapshot(timepoint_ms, addr0, addrn, data, size);
				this->last_timepoint = timepoint_ms;
			}
		}

	public:
		void construct(CanvasCreateResourcesReason reason) override {
		}

	private:
		long long last_timepoint;
	};
}

/*************************************************************************************************/
static TimeStream* the_timemachine = nullptr;

void WarGrey::SCADA::initialize_the_timemachine(MRMaster* plc, long long speed, int frame_rate) {
	if (the_timemachine == nullptr) {
		the_timemachine = new TimeStream(speed, frame_rate);

		plc->push_confirmation_receiver(the_timemachine);
	}
}

void WarGrey::SCADA::launch_the_timemachine() {
	if (the_timemachine != nullptr) {
		the_timemachine->show();
	}
}
