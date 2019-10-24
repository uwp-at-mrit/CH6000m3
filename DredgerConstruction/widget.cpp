#include <map>

#include "widget.hxx"
#include "planet.hpp"
#include "about.hpp"
#include "ch6000m3/configuration.hpp"

#include "widget/settings.hpp"
#include "widget/timestream.hpp"

#include "graphlet/ui/textlet.hpp"
#include "graphlet/ui/buttonlet.hpp"

#include "network/tcp.hpp"

#include "datum/time.hpp"
#include "datum/path.hpp"

#include "module.hpp"
#include "system.hpp"

using namespace WarGrey::SCADA;

using namespace Windows::Foundation;
using namespace Windows::System;
using namespace Windows::Storage;

using namespace Windows::UI::ViewManagement;
using namespace Windows::UI::Xaml::Controls;

using namespace Microsoft::Graphics::Canvas;
using namespace Microsoft::Graphics::Canvas::UI;
using namespace Microsoft::Graphics::Canvas::Text;
using namespace Microsoft::Graphics::Canvas::Brushes;

private enum class Brightness { Brightness100, Brightness80, Brightness60, Brightness40, Brightness30, Brightness20, _ };

// WARNING: order matters
private enum class SS : unsigned int { Brightness, _ };
private enum class Icon : unsigned int { Settings, TimeMachine, PrintScreen, FullScreen, About, _ };

static ICanvasBrush^ about_bgcolor = Colours::WhiteSmoke;
static ICanvasBrush^ about_fgcolor = Colours::Black;

/*************************************************************************************************/
private class Widget : public Planet {
public:
	Widget(SplitView^ frame, UniverseDisplay^ master)
		: Planet(__MODULE__), frame(frame), master(master) {
		Platform::String^ localhost = system_ipv4_address();
		
		this->inset = tiny_font_size * 0.5F;
		this->btn_xgapsize = 2.0F;
	}

public:
	void load(CanvasCreateResourcesReason reason, float width, float height) override {
		auto label_font = make_text_format("Microsoft YaHei", large_font_size);
		auto icon_font = make_text_format("Consolas", 32.0F);
		float button_height, label_width;
		ButtonStyle button_style;

		button_style.font = make_bold_text_format(tiny_font_size);
		button_style.corner_radius = 2.0F;
		button_style.thickness = 1.0F;

		for (SS id = _E0(SS); id < SS::_; id++) {
			this->labels[id] = this->insert_one(new Labellet(_speak(id), label_font, Colours::GhostWhite));
			this->labels[id]->fill_extent(0.0F, 0.0F, &label_width, &button_height);
			this->label_max = fmaxf(label_width, this->label_max);
		}

		for (Icon id = _E0(Icon); id < Icon::_; id++) {
			this->icons[id] = this->insert_one(new Credit<Labellet, Icon>(_speak(id), icon_font, Colours::GhostWhite), id);
		}

		this->load_buttons(this->brightnesses, button_style, button_height);
	}

	void reflow(float width, float height) override {
		float fx = 1.0F / float(_N(Icon) + 1);
		float button_y;

		this->move_to(this->labels[SS::Brightness], this->inset, height - tiny_font_size, GraphletAnchor::LB);

		this->reflow_buttons(this->brightnesses, this->labels[SS::Brightness]);

		this->fill_graphlet_location(this->brightnesses[Brightness::Brightness100], nullptr, &button_y);
		for (Icon id = _E0(Icon); id < Icon::_; id++) {
			this->move_to(this->icons[id], width * fx * float(_I(id) + 1), button_y, GraphletAnchor::CB, 0.0F, -tiny_font_size);
		}
	}

public:
	void update(long long count, long long interval, long long uptime) override {
		double alpha = this->master->global_mask_alpha;
		Buttonlet* target = nullptr;

		if (alpha > 0.75) {
			target = this->brightnesses[Brightness::Brightness20];
		} else if (alpha > 0.65) {
			target = this->brightnesses[Brightness::Brightness30];
		} else if (alpha > 0.50) {
			target = this->brightnesses[Brightness::Brightness40];
		} else if (alpha > 0.30) {
			target = this->brightnesses[Brightness::Brightness60];
		} else if (alpha > 0.10) {
			target = this->brightnesses[Brightness::Brightness80];
		} else {
			target = this->brightnesses[Brightness::Brightness100];
		}

		for (Brightness cmd = _E0(Brightness); cmd < Brightness::_; cmd++) {
			this->brightnesses[cmd]->set_state(this->brightnesses[cmd] == target, ButtonState::Executing, ButtonState::Ready);
		}
	}

public:
	bool can_select(IGraphlet* g) override {
		return ((dynamic_cast<Credit<Labellet, Icon>*>(g) != nullptr)
			|| button_enabled(g));
	}

	void on_tap_selected(IGraphlet* g, float local_x, float local_y) override {
		auto b_btn = dynamic_cast<Credit<Buttonlet, Brightness>*>(g);
		auto p_btn = dynamic_cast<Credit<Buttonlet, TCPMode>*>(g);
		auto icon = dynamic_cast<Credit<Labellet, Icon>*>(g);

		if (b_btn != nullptr) {
			double alpha = -1.0;

			switch (b_btn->id) {
			case Brightness::Brightness100: alpha = 0.0; break;
			case Brightness::Brightness80:  alpha = 0.2; break;
			case Brightness::Brightness60:  alpha = 0.4; break;
			case Brightness::Brightness40:  alpha = 0.6; break;
			case Brightness::Brightness30:  alpha = 0.7; break;
			case Brightness::Brightness20:  alpha = 0.8; break;
			}

			if (alpha >= 0.0) {
				this->master->global_mask_alpha = alpha;
			}
		} else if (icon != nullptr) {
			switch (icon->id) {
			case Icon::Settings: launch_the_settings(); break;
			case Icon::TimeMachine: launch_the_timemachine(); break;
			case Icon::About: {
				if (this->about == nullptr) {
					this->about = make_about("about.png", about_bgcolor, about_fgcolor);
				}

				this->about->show();
			}; break;
			case Icon::PrintScreen: {
				this->master->save(this->master->current_planet->name() + "-"
					+ file_basename_from_second(current_seconds()) + ".png");
			}; break;
			case Icon::FullScreen: {
				auto self = ApplicationView::GetForCurrentView();

				if (self->IsFullScreenMode) {
					self->ExitFullScreenMode();
				} else {
					self->TryEnterFullScreenMode();
				}

			}; break;
			}

			this->frame->IsPaneOpen = false;
		}
	}

private:
	template<typename CMD>
	void load_buttons(std::map<CMD, Credit<Buttonlet, CMD>*>& bs, ButtonStyle& style, float button_height, float reserved = 0.0F) {
		float button_width = (this->min_width() - this->inset * 3.0F - this->label_max - reserved) / float(_N(CMD)) - this->btn_xgapsize;
		
		for (CMD cmd = _E0(CMD); cmd < CMD::_; cmd++) {
			bs[cmd] = this->insert_one(new Credit<Buttonlet, CMD>(cmd.ToString(), button_width, button_height, __MODULE__), cmd);
			bs[cmd]->set_style(style);
		}
	}

private:
	template<typename CMD>
	void reflow_buttons(std::map<CMD, Credit<Buttonlet, CMD>*>& bs, IGraphlet* target) {
		float target_width;
		float xoff = this->inset;

		target->fill_extent(0.0F, 0.0F, &target_width, nullptr);

		xoff += (this->label_max - target_width);

		for (CMD cmd = _E0(CMD); cmd < CMD::_; cmd++) {
			this->move_to(bs[cmd], target, GraphletAnchor::RC, GraphletAnchor::LC, xoff);
			target = bs[cmd];
			xoff = this->btn_xgapsize;
		}
	}

private:
	float inset;
	float btn_xgapsize;
	float label_max;

private:
	SplitView^ frame;
	UniverseDisplay^ master;
	ISatellite* about;

private: // never delete these graphlets manually.
	std::map<Brightness, Credit<Buttonlet, Brightness>*> brightnesses;
	std::map<TCPMode, Credit<Buttonlet, TCPMode>*> permissions;
	std::map<Icon, Credit<Labellet, Icon>*> icons;
	std::map<SS, Labellet*> labels;
};

/*************************************************************************************************/
UniverseWidget::UniverseWidget(SplitView^ frame, UniverseDisplay^ master, MRMaster* plc)
	: UniverseDisplay(master->get_logger()), frame(frame), master(master), plc(plc) {
	this->use_global_mask_setting(false);
	this->disable_predefined_shortcuts(true);
}

void UniverseWidget::construct(CanvasCreateResourcesReason reason) {
	initialize_the_timemachine(this->plc, timemachine_speed, timemachine_frame_per_second);
	this->push_planet(new Widget(this->frame, this->master));
}
