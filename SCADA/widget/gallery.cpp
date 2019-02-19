#include <map>

#include "widget/gallery.hpp"
#include "configuration.hpp"

#include "graphlet/textlet.hpp"
#include "graphlet/shapelet.hpp"

#include "graphlet/device/winchlet.hpp"
#include "graphlet/symbol/heaterlet.hpp"
#include "graphlet/symbol/pump/hydraulic_pumplet.hpp"
#include "graphlet/symbol/pump/hopper_pumplet.hpp"
#include "graphlet/symbol/pump/water_pumplet.hpp"
#include "graphlet/symbol/valve/gate_valvelet.hpp"
#include "graphlet/symbol/valve/tagged_valvelet.hpp"
#include "graphlet/symbol/door/hopper_doorlet.hpp"

#include "decorator/margin.hpp"

#include "satellite.hpp"
#include "system.hpp"
#include "module.hpp"

using namespace WarGrey::SCADA;

using namespace Windows::Foundation;

using namespace Microsoft::Graphics::Canvas;
using namespace Microsoft::Graphics::Canvas::UI;
using namespace Microsoft::Graphics::Canvas::Text;

private enum class GS {
	winch_1, winch_2,
	hydraulic_pump, hopper_pump, water_pump,
	gate_valve, motor_valve,
	hopperdoor, upperdoor,
	heater,
	_
};

/*************************************************************************************************/
private class Gallery : public ISatellite {
public:
	Gallery() : ISatellite(default_logging_level, __MODULE__) {
		this->push_decorator(new MarginDecorator(true, true));
	}

	void fill_extent(float* width, float* height) {
		float margin = normal_font_size * 4.0F;
		Size size = system_screen_size();

		SET_BOX(width, size.Width - margin * 2.0F);
		SET_BOX(height, size.Height - margin);
	}

public:
	void load(CanvasCreateResourcesReason reason, float width, float height) override {
		float vinset = normal_font_size;
		float unitsize = (height - vinset - vinset) / (float(_N(GS)) * 2.0F * 1.618F);

		this->font = make_bold_text_format(std::fminf(unitsize, 18.0F));
		this->status_font = make_bold_text_format(14.0F);

		this->background = this->insert_one(new Rectanglet(width, height, Colours::Background));

		this->remote_label = make_label(_speak("Remote"), this->font);
		for (GS id = _E(GS, 0); id < GS::_; id++) {
			this->captions[_I(id)] = make_label(_speak(id), this->font);
		}

		this->load_primitives(this->ps_winches, this->wlabels, unitsize * 2.0F);
		this->load_primitives(this->pumps, this->plabels, unitsize);
		this->load_primitives(this->hpumps, this->hplabels, unitsize * 0.5F);
		this->load_primitives(this->wpumps, this->wplabels, unitsize);
		this->load_primitives(this->gvalves, this->gvlabels, unitsize);
		this->load_primitives(this->evalves, this->evlabels, unitsize);
		this->load_primitives(this->hdoors, this->hdlabels, unitsize);
		this->load_primitives(this->udoors, this->udlabels, unitsize);
		this->load_primitives(this->heaters, this->hlabels, unitsize);

		this->load_remote_primitive(&this->remote_winch, WinchState::Default, unitsize * 2.0F);
		this->load_remote_primitive(&this->remote_pump, HydraulicPumpState::Stopped, unitsize);
		this->load_remote_primitive(&this->remote_hopper_pump, HopperPumpState::Stopped, unitsize * 0.5F);
		this->load_remote_primitive(&this->remote_water_pump, WaterPumpState::Stopped, unitsize);
		this->load_remote_primitive(&this->remote_heater, HeaterState::Stopped, unitsize);
	}

	void reflow(float width, float height) override {
		float vinset = normal_font_size;
		float unitsize, halfunit, cellsize, label_width;
		float label_max_width = 0.0F;
		float x = vinset * 0.5F;
		float y = 0.0F;

		for (size_t i = 0; i < _N(GS); i++) {
			if (this->captions[i] != nullptr) {
				this->captions[i]->fill_extent(0.0F, 0.0F, &label_width);
				label_max_width = std::fmaxf(label_max_width, label_width);
			}
		}

		this->pumps[_E0(HydraulicPumpState)]->fill_extent(0.0F, 0.0F, &unitsize);

		halfunit = unitsize * 0.5F;
		cellsize = unitsize * 1.618F;
		y = (height - cellsize * float(_N(GS))) * 0.5F;

		for (size_t i = 0; i < _N(GS); i++) {
			if (this->captions[i] != nullptr) {
				float yi = y + halfunit + float(i) * cellsize;

				this->move_to(this->captions[i], x + label_max_width, yi, GraphletAnchor::RC);
			}
		}

		x += (label_max_width + x + x + halfunit);

		{ // split winch statuses
			WinchState at = _E(WinchState, _N(WinchState) / 2);

			this->reflow_primitives(this->ps_winches, this->wlabels, _E0(WinchState), at, x, &y, halfunit, cellsize);
			this->reflow_primitives(this->ps_winches, this->wlabels, at, WinchState::_, x, &y, halfunit, cellsize);
		}

		this->reflow_primitives(this->pumps, this->plabels, x, &y, halfunit, cellsize);
		this->reflow_primitives(this->hpumps, this->hplabels, x, &y, halfunit, cellsize);
		this->reflow_primitives(this->wpumps, this->wplabels, x, &y, halfunit, cellsize);
		this->reflow_primitives(this->gvalves, this->gvlabels, x, &y, halfunit, cellsize);
		this->reflow_primitives(this->evalves, this->evlabels, x, &y, halfunit, cellsize);
		this->reflow_primitives(this->hdoors, this->hdlabels, x, &y, halfunit, cellsize);
		this->reflow_primitives(this->udoors, this->udlabels, x, &y, halfunit, cellsize);
		this->reflow_primitives(this->heaters, this->hlabels, x, &y, halfunit, cellsize);

		{ // reflow remote controlled graphlets
			x = width - x + label_max_width;
			y = vinset + cellsize * 0.5F;

			this->move_to(this->remote_label, x, y + cellsize * 0.0F, GraphletAnchor::CC);
			this->move_to(this->remote_winch, x, y + cellsize * 1.0F, GraphletAnchor::CC);
			this->move_to(this->remote_pump, x, y + cellsize * 2.0F, GraphletAnchor::CC);
			this->move_to(this->remote_hopper_pump, x, y + cellsize * 3.0F, GraphletAnchor::CC);
			this->move_to(this->remote_water_pump, x, y + cellsize * 4.0F, GraphletAnchor::CC);
			this->move_to(this->remote_heater, x, y + cellsize * 5.0F, GraphletAnchor::CC);
		}
	}

public:
	bool Gallery::can_select(IGraphlet* g) override {
		return true;
	}

	void update(long long count, long long interval, long long uptime) override {
		float progress = float(count % 11) / 10.0F;

		hdoors[DoorState::Opening]->set_value(progress);
		udoors[DoorState::Opening]->set_value(progress);
		hdoors[DoorState::Closing]->set_value(1.0 - progress);
		udoors[DoorState::Closing]->set_value(1.0 - progress);
	}

private:
	Labellet* make_label(Platform::String^ text, CanvasTextFormat^ font = nullptr) {
		return this->insert_one(new Labellet(text, font));
	}

	template<typename T, typename S>
	void load_remote_primitive(T** g, S s, float unitsize) {
		(*g) = this->insert_one(new T(s, unitsize));
		(*g)->set_remote_control(true);
	}

	template<typename T, typename S>
	void load_primitives(std::map<S, T*>& gs, std::map<S, Labellet*>& ls, float unitsize) {
		for (S s = _E0(S); s < S::_; s++) {
			gs[s] = this->insert_one(new T(s, unitsize));
			ls[s] = make_label(_speak(s), this->status_font);
		}
	}

	template<typename T, typename S>
	void reflow_primitives(std::map<S, T*>& gs, std::map<S, Labellet*>& ls
		, float x0, float* y, float halfunit, float cellsize) {
		this->reflow_primitives(gs, ls, _E0(S), S::_, x0, y, halfunit, cellsize);
	}

	template<typename T, typename S>
	void reflow_primitives(std::map<S, T*>& gs, std::map<S, Labellet*>& ls
		, S id0, S idN, float x0, float* y, float halfunit, float cellsize) {
		unsigned int i0 = _I(id0);

		for (S i = id0; i < idN; i++) {
			float x = x0 + float(_I(i) - i0) * cellsize;

			this->move_to(gs[i], x, (*y) + halfunit * 1.0F, GraphletAnchor::CC);
			this->move_to(ls[i], x, (*y) + halfunit * 2.0F, GraphletAnchor::CT);
		}

		(*y) = (*y) + cellsize;
	}

private: // never delete these graphlets manually.
	Labellet* captions[_N(GS)];
	std::map<WinchState, Winchlet*> ps_winches;
	std::map<WinchState, Labellet*> wlabels;
	std::map<HydraulicPumpState, HydraulicPumplet*> pumps;
	std::map<HydraulicPumpState, Labellet*> plabels;
	std::map<HopperPumpState, HopperPumplet*> hpumps;
	std::map<HopperPumpState, Labellet*> hplabels;
	std::map<WaterPumpState, WaterPumplet*> wpumps;
	std::map<WaterPumpState, Labellet*> wplabels;
	std::map<GateValveState, GateValvelet*> gvalves;
	std::map<GateValveState, Labellet*> gvlabels;
	std::map<TValveState, MotorValvelet*> evalves;
	std::map<TValveState, Labellet*> evlabels;
	std::map<DoorState, HopperDoorlet*> hdoors;
	std::map<DoorState, Labellet*> hdlabels;
	std::map<DoorState, UpperHopperDoorlet*> udoors;
	std::map<DoorState, Labellet*> udlabels;
	std::map<HeaterState, Heaterlet*> heaters;
	std::map<HeaterState, Labellet*> hlabels;

	Labellet* remote_label;
	Winchlet* remote_winch;
	HydraulicPumplet* remote_pump;
	HopperPumplet* remote_hopper_pump;
	WaterPumplet* remote_water_pump;
	Heaterlet* remote_heater;

	Rectanglet* background;

private:
	CanvasTextFormat^ font;
	CanvasTextFormat^ status_font;
};

/*************************************************************************************************/
static Gallery* the_gallery = nullptr;

void WarGrey::SCADA::display_the_gallery() {
	if (the_gallery == nullptr) {
		the_gallery = new Gallery();
	}

	the_gallery->show();
}

void WarGrey::SCADA::update_the_shown_gallery(long long count, long long interval, long long uptime, bool create) {
	if (create && (the_gallery == nullptr)) {
		the_gallery = new Gallery();
	}

	if (the_gallery != nullptr) {
		if (the_gallery->shown()) {
			the_gallery->on_elapse(count, interval, uptime);
		}
	}
}
