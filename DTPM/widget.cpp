#include <map>

#include "configuration.hpp"
#include "brightness.hpp"

#include "widget.hxx"
#include "planet.hpp"
#include "about.hpp"

#include "widget/settings.hpp"
#include "widget/timestream.hpp"

#include "graphlet/ui/textlet.hpp"
#include "graphlet/ui/buttonlet.hpp"
#include "graphlet/ui/togglet.hpp"

#include "peer/slang.hpp"

#include "datum/time.hpp"
#include "datum/path.hpp"

#include "preference.hxx"
#include "module.hpp"
#include "system.hpp"

using namespace WarGrey::SCADA;
using namespace WarGrey::DTPM;
using namespace WarGrey::GYDM;

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
private enum class SS : unsigned int { Brightnessd, Brightness, _ };
private enum class Icon : unsigned int { Settings, TimeMachine, PrintScreen, FullScreen, About, _ };

static ICanvasBrush^ about_bgcolor = Colours::WhiteSmoke;
static ICanvasBrush^ about_fgcolor = Colours::Black;

static const float widget_label_font_size = large_font_size;
static const float widget_icon_size = 32.0F;
static const float widget_line_gap = tiny_font_size;

static CanvasTextFormat^ widget_label_font = nullptr;
static CanvasTextFormat^ widget_icon_font = nullptr;

static void widget_initialize() {
	if (widget_label_font == nullptr) {
		widget_label_font = make_text_format("Microsoft YaHei", widget_label_font_size);
		widget_icon_font = make_text_format("Consolas", widget_icon_size);
	}
}

/*************************************************************************************************/
float WarGrey::DTPM::widget_evaluate_height() {
	TextExtent icon_te = get_text_extent(_speak(Icon::About), widget_icon_font);
	TextExtent label_te = get_text_extent(_speak(SS::Brightness), widget_label_font);

	return icon_te.height + (label_te.height + widget_line_gap) * float(_N(SS)) + widget_line_gap * 2.0F;
}

/*************************************************************************************************/
private class Widget : public Planet, public SlangLocalPeer<uint8> {
public:
	Widget(SplitView^ frame, UniverseDisplay^ master)
		: Planet(__MODULE__), frame(frame), master(master) {
		Platform::String^ localhost = system_ipv4_address();
		
		this->inset = widget_line_gap * 0.5F;
		this->btn_xgapsize = 2.0F;

		this->brightnessd = new SlangDaemon<uint8>(make_system_logger(default_slang_logging_level, "Slang"), brightness_slang_port(BrightnessPort::DTPM), this);
		this->brightnessd->join_multicast_group(slang_multicast_group);
	}

public:
	void load(CanvasCreateResourcesReason reason, float width, float height) override {
		float button_height, label_width;
		ButtonStyle button_style;

		button_style.font = make_bold_text_format(tiny_font_size);
		button_style.corner_radius = 2.0F;
		button_style.thickness = 1.0F;

		for (SS id = _E0(SS); id < SS::_; id++) {
			this->labels[id] = this->insert_one(new Labellet(_speak(id), widget_label_font, Colours::GhostWhite));
			this->labels[id]->fill_extent(0.0F, 0.0F, &label_width, &button_height);
			this->label_max = fmaxf(label_width, this->label_max);
		}

		for (Icon id = _E0(Icon); id < Icon::_; id++) {
			this->icons[id] = this->insert_one(new Credit<Labellet, Icon>(_speak(id), widget_icon_font, Colours::GhostWhite), id);
		}

		this->load_buttons(this->brightnesses, button_style, button_height);
		this->load_toggles(this->toggles);
	}

	void reflow(float width, float height) override {
		float fx = 1.0F / float(_N(Icon) + 1);
		float button_y;

		this->move_to(this->labels[SS::Brightnessd], this->inset, height - tiny_font_size, GraphletAnchor::LB);
		this->move_to(this->labels[SS::Brightness], this->labels[SS::Brightnessd], GraphletAnchor::LT, GraphletAnchor::LB, 0.0F, -tiny_font_size);

		this->reflow_buttons(this->toggles, this->labels[SS::Brightnessd]);
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
			|| (dynamic_cast<Togglet*>(g) != nullptr)
			|| button_enabled(g));
	}

	void on_tap_selected(IGraphlet* g, float local_x, float local_y) override {
		auto b_btn = dynamic_cast<Credit<Buttonlet, Brightness>*>(g);
		auto p_btn = dynamic_cast<Credit<Buttonlet, TCPMode>*>(g);
		auto icon = dynamic_cast<Credit<Labellet, Icon>*>(g);
		auto t_btn = dynamic_cast<Credit<Togglet, BrightnessPort>*>(g);

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
				for (BrightnessPort bp = _E0(BrightnessPort); bp < BrightnessPort::_; bp++) {
					if (this->toggles[bp]->checked()) {
						this->brightnessd->multicast(brightness_slang_port(bp), asn_real_to_octets(alpha));
						this->get_logger()->log_message(Log::Notice, L"%s GROUP EVENT: change screen brightness", bp.ToString()->Data());
					} else if (bp == BrightnessPort::DTPM) {
						this->set_brightness(alpha);
					}
				}
			}
		} else if (t_btn != nullptr) {
			t_btn->toggle();
			put_preference(brightness_preference_key(t_btn->id), t_btn->checked());
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

public:
	void on_message(long long timepoint_ms, Platform::String^ remote_peer, uint16 remote_port, uint8 type, const uint8* message, Syslog* logger) override {
		// NOTE: the brightnessd is a standalone daemon, all messages therefore concern the brightness setting

		for (BrightnessPort bp = _E0(BrightnessPort); bp < BrightnessPort::_; bp++) {
			if (this->toggles[bp]->checked() && (brightness_slang_port(bp) == remote_port)) {
				double alpha = asn_octets_to_real(message);

				if ((alpha >= 0.0) && (alpha <= 1.0)) {
					this->set_brightness(alpha);
					this->get_logger()->log_message(Log::Info, L"brightness has been changed by %s[%s]",
						remote_peer->Data(), bp.ToString()->Data());
				}

				break;
			}
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

	template<typename BP>
	void load_toggles(std::map<BP, Credit<Togglet, BP>*>& ts, float reserved = 0.0F) {
		float toggle_width = (this->min_width() - this->inset * 3.0F - this->label_max - reserved) / float(_N(BP)) - this->btn_xgapsize;

		for (BP bp = _E0(BP); bp < BP::_; bp++) {
			Platform::String^ name = bp.ToString();
			Platform::String^ on_label = name + "_On";
			Platform::String^ off_label = name + "_Off";
			bool toggle_state = get_preference(brightness_preference_key(bp), true);

			ts[bp] = this->insert_one(new Credit<Togglet, BP>(toggle_state, on_label, off_label, toggle_width, nullptr, nullptr, __MODULE__), bp);
		}
	}

private:
	template<typename E, class Let>
	void reflow_buttons(std::map<E, Credit<Let, E>*>& bs, IGraphlet* target) {
		float target_width;
		float xoff = this->inset;

		target->fill_extent(0.0F, 0.0F, &target_width, nullptr);

		xoff += (this->label_max - target_width);

		for (E e = _E0(E); e < E::_; e++) {
			this->move_to(bs[e], target, GraphletAnchor::RC, GraphletAnchor::LC, xoff);
			target = bs[e];
			xoff = this->btn_xgapsize;
		}
	}

private:
	void set_brightness(double alpha) {
		this->master->global_mask_alpha = alpha;
	}

private:
	float inset;
	float btn_xgapsize;
	float label_max;

private:
	SplitView^ frame;
	UniverseDisplay^ master;
	ISatellite* about;

private:
	SlangDaemon<uint8>* brightnessd;

private: // never delete these graphlets manually.
	std::map<Brightness, Credit<Buttonlet, Brightness>*> brightnesses;
	std::map<BrightnessPort, Credit<Togglet, BrightnessPort>*> toggles;
	std::map<Icon, Credit<Labellet, Icon>*> icons;
	std::map<SS, Labellet*> labels;
};

/*************************************************************************************************/
UniverseWidget::UniverseWidget(SplitView^ frame, UniverseDisplay^ master, MRMaster* plc)
	: UniverseDisplay(master->get_logger()), frame(frame), master(master), plc(plc) {
	this->use_global_mask_setting(false);
	this->disable_predefined_shortcuts(true);
	widget_initialize();
}

void UniverseWidget::construct(CanvasCreateResourcesReason reason) {
	initialize_the_timemachine(this->plc, timemachine_speed, timemachine_frame_per_second);
	this->push_planet(new Widget(this->frame, this->master));
}
