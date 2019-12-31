#include <map>

#include "frame/metrics.hpp"

#include "datum/credit.hpp"
#include "datum/string.hpp"

#include "module.hpp"
#include "brushes.hxx"

#include "graphlet/shapelet.hpp"
#include "graphlet/ui/textlet.hpp"

#include "configuration.hpp"
#include "plc.hpp"

#include "iotables/ai_metrics.hpp"
#include "iotables/di_hopper_pumps.hpp"

using namespace WarGrey::SCADA;
using namespace WarGrey::DTPM;

using namespace Windows::Storage;

using namespace Microsoft::Graphics::Canvas;
using namespace Microsoft::Graphics::Canvas::UI;
using namespace Microsoft::Graphics::Canvas::Text;
using namespace Microsoft::Graphics::Canvas::Brushes;

static CanvasSolidColorBrush^ region_background = Colours::make(diagnostics_region_background);
static CanvasSolidColorBrush^ metrics_background = Colours::make(diagnostics_alarm_background);

static unsigned int default_slot_count = 20U;

#define SET_METRICS(ms, id, v, p) \
do { \
    auto t = ms.find(id); \
    if (t != ms.end()) { \
        t->second->set_text(flstring(v, p), GraphletAnchor::RC); \
    } \
} while(0)

#define SET_VALID_METRICS(ms, id, b, v, p) \
do { \
    auto t = ms.find(id); \
    if (t != ms.end()) { \
        t->second->set_text((b ? flstring(v, p) : "-"), GraphletAnchor::RC); \
    } \
} while(0)

// WARNING: order matters
namespace {
	private enum class M : unsigned int {
		// Key Labels
		BowDirection, TrackDirection, FlowPressureAngle, LineDirection, Speed, TurnRate,
		BowDraught, SternDraught, AverageDraught,
		PSFlow, SBFlow, PSConcentration, SBConcentration, AverageDensity,
		Capacity, Payload, Displacement, EarthWork,
		PSProduct, SBProduct,
		MudDensity,
		_
	};

	static ICanvasBrush^ matrics_color_map(M id) {
		ICanvasBrush^ color = nullptr;

		switch (id) {
		case M::BowDirection: case M::TrackDirection: case M::FlowPressureAngle: case M::LineDirection: color = Colours::Yellow; break;
		case M::Speed: case M::TurnRate: color = Colours::Green; break;
		case M::BowDraught: case M::SternDraught: case M::AverageDraught: color = Colours::Orange; break;
		case M::PSFlow: case M::SBFlow: case M::PSConcentration: case M::SBConcentration: case M::AverageDensity: color = Colours::DodgerBlue; break;
		case M::EarthWork: case M::Capacity: case M::Displacement: case M::MudDensity: case M::Payload: color = Colours::Salmon; break;
		case M::PSProduct: case M::SBProduct: color = Colours::Cyan; break;
		default: color = Colours::Transparent;
		}

		return color;
	}

	private class Metrics final : public PLCConfirmation, public GPSReceiver {
	public:
		Metrics(MetricsFrame* master, float width, unsigned int slots, GPS* gps1, GPS* gps2, GPS* gyro)
			: master(master), width(width), slot_count(slots > 0 ? slots : default_slot_count)
			, bow_direction(0.0), gps1(gps1), gps2(gps2), gyro(gyro) {
			this->label_font = make_bold_text_format("Microsoft YaHei", large_font_size);
			this->metrics_font = make_bold_text_format("Cambria Math", small_metrics_font_size);

			this->slot_height = this->metrics_font->FontSize * 1.2F;
			this->inset = this->metrics_font->FontSize * 0.618F;
			this->hgapsize = this->inset * 0.618F;

			if (this->gps1 != nullptr) {
				this->gps1->push_confirmation_receiver(this);
			}

			if (this->gps2 != nullptr) {
				this->gps2->push_confirmation_receiver(this);
			}

			if (this->gyro != nullptr) {
				this->gyro->push_confirmation_receiver(this);
			}
		}

	public:
		void pre_read_data(Syslog* logger) override {
			this->master->enter_critical_section();
			this->master->begin_update_sequence();
		}

		void on_analog_input(long long timepoint_ms, const uint8* DB2, size_t count2, const uint8* DB203, size_t count203, Syslog* logger) override {
			SET_METRICS(this->metrics, M::BowDraught, DBD(DB2, fixed_bow_draught), 2);
			SET_METRICS(this->metrics, M::SternDraught, DBD(DB2, fixed_stern_draught), 2);
			SET_METRICS(this->metrics, M::AverageDraught, DBD(DB2, average_draught), 2);

			SET_METRICS(this->metrics, M::PSFlow, DBD(DB2, ps_flow), 0);
			SET_METRICS(this->metrics, M::SBFlow, DBD(DB2, sb_flow), 0);
			SET_METRICS(this->metrics, M::PSConcentration, DBD(DB2, ps_concentration), 2);
			SET_METRICS(this->metrics, M::SBConcentration, DBD(DB2, sb_concentration), 2);
			SET_METRICS(this->metrics, M::PSProduct, DBD(DB2, ps_accumulated_product), 0);
			SET_METRICS(this->metrics, M::SBProduct, DBD(DB2, sb_accumulated_product), 0);
			SET_METRICS(this->metrics, M::MudDensity, DBD(DB2, mud_density), 2);
			SET_METRICS(this->metrics, M::AverageDensity, DBD(DB2, average_density), 2);

			SET_METRICS(this->metrics, M::EarthWork, DBD(DB2, earthwork_value), 0);
			SET_METRICS(this->metrics, M::Capacity, DBD(DB2, vessel_value), 0);
			SET_METRICS(this->metrics, M::Displacement, DBD(DB2, displacement_value), 0);
			SET_METRICS(this->metrics, M::Payload, DBD(DB2, payload_value), 0);
		}

		void post_read_data(Syslog* logger) override {
			this->master->end_update_sequence();
			this->master->leave_critical_section();
		}

	public:
		void pre_scan_data(int id, Syslog* logger) override {
			this->pre_read_data(logger);
		}

		void on_VTG(int id, long long timepoint_ms, VTG* vtg, Syslog* logger) override {
			double angle = this->bow_direction - vtg->track_deg;

			SET_METRICS(this->metrics, M::Speed, vtg->s_kn, 1);
			SET_METRICS(this->metrics, M::TrackDirection, vtg->track_deg, 1);
			SET_METRICS(this->metrics, M::FlowPressureAngle, angle, 1);
		}

		void on_HDT(int id, long long timepoint_ms, HDT* hdt, Syslog* logger) override {
			bool valid = false;

			if ((this->gyro != nullptr) && this->gyro->connected()) {
				if (id == this->gyro->device_identity()) {
					valid = true;
				}
			} else {
				valid = true;
			}

			if (valid) {
				SET_METRICS(this->metrics, M::BowDirection, hdt->heading_deg, 1);
				this->bow_direction = hdt->heading_deg;
			}
		}

		void on_ROT(int id, long long timepoint_ms, ROT* rot, Syslog* logger) override {
			bool valid = false;

			if (this->gyro->connected()) {
				if (id == this->gyro->device_identity()) {
					valid = true;
				}
			} else {
				valid = true;
			}

			if (valid) {
				SET_VALID_METRICS(this->metrics, M::TurnRate, rot->validity, rot->degpmin, 1);
			}
		}

		void post_scan_data(int id, Syslog* logger) override {
			this->post_read_data(logger);
		}

		bool available(int id) override {
			return ((id == this->gyro->device_identity())
				|| (id == this->gps1->device_identity())
				|| (!this->gps1->connected()));
		}

	public:
		void load(float width, float height) {
			float corner_radius = 8.0F;
			float slot_width = this->width - this->inset * 2.0F;
			float bgheight = (this->slot_height + this->hgapsize) * float(this->slot_count) + this->inset * 2.0F;

			this->background = this->master->insert_one(new Rectanglet(this->width, bgheight, region_background));

			for (unsigned int idx = 0; idx < this->slot_count; idx++) {
				M id = _E(M, idx);
				ICanvasBrush^ color = matrics_color_map(id);

				this->slots[id] = this->master->insert_one(new Credit<RoundedRectanglet, M>(
					slot_width, this->slot_height, corner_radius, metrics_background), id);

				this->labels[id] = this->master->insert_one(new Credit<Labellet, M>(_speak(id), this->label_font, color), id);
				this->metrics[id] = this->master->insert_one(new Credit<Labellet, M>("0.0", this->metrics_font, color));
			}
		}

		void reflow(float width, float height) {
			for (unsigned int idx = 0; idx < this->slot_count; idx++) {
				M id = _E(M, idx);

				this->master->move_to(this->slots[id], this->inset, this->inset + (this->slot_height + this->hgapsize) * idx);
				
				this->master->move_to(this->labels[id], this->slots[id], GraphletAnchor::LC, GraphletAnchor::LC, this->hgapsize);
				this->master->move_to(this->metrics[id], this->slots[id], GraphletAnchor::RC, GraphletAnchor::RC, -this->hgapsize);
			}
		}

	private: // never deletes these graphlets manually
		std::map<M, Credit<RoundedRectanglet, M>*> slots;
		std::map<M, Credit<Labellet, M>*> labels;
		std::map<M, Credit<Labellet, M>*> metrics;
		Rectanglet* background;

	private:
		CanvasTextFormat^ label_font;
		CanvasTextFormat^ metrics_font;

	private:
		unsigned int slot_count;
		float width;
		float slot_height;
		float hgapsize;
		float inset;

	private:
		double bow_direction;

	private: // never deletes these objects manually
		MetricsFrame* master;
		GPS* gps1;
		GPS* gps2;
		GPS* gyro;
	};
}

/*************************************************************************************************/
MetricsFrame::MetricsFrame(float width, unsigned int slot_count, MRMaster* plc, GPS* gps1, GPS* gps2, GPS* gyro) : Planet(__MODULE__), plc(plc) {
	Metrics* dashboard = new Metrics(this, width, slot_count, gps1, gps2, gyro);
	
	this->dashboard = dashboard;

	if (this->plc != nullptr) {
		this->plc->push_confirmation_receiver(dashboard);
	}
}

MetricsFrame::~MetricsFrame() {
	if (this->dashboard != nullptr) {
		delete this->dashboard;
	}
}

void MetricsFrame::load(CanvasCreateResourcesReason reason, float width, float height) {
	auto dashboard = dynamic_cast<Metrics*>(this->dashboard);
	
	if (dashboard != nullptr) {
		dashboard->load(width, height);
	}
}

void MetricsFrame::reflow(float width, float height) {
	auto dashboard = dynamic_cast<Metrics*>(this->dashboard);

	if (dashboard != nullptr) {
		dashboard->reflow(width, height);
	}
}

bool MetricsFrame::can_select(IGraphlet* g) {
	return false;
}
