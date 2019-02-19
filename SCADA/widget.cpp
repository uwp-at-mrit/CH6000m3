#include <map>

#include "widget.hxx"
#include "planet.hpp"
#include "configuration.hpp"

#include "widget/alarms.hpp"
#include "widget/gallery.hpp"
#include "widget/settings.hpp"
#include "widget/timestream.hpp"

#include "graphlet/textlet.hpp"
#include "graphlet/buttonlet.hpp"

#include "module.hpp"
#include "system.hpp"
#include "time.hpp"
#include "path.hpp"

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
private enum class SS : unsigned int { Brightness, Permission, _ };
private enum class Icon : unsigned int { Gallery , Settings, TimeMachine, Alarm, PrintScreen, FullScreen, _ };

static Platform::String^ mode_setting_key = "PLC_Master_Mode";

/*************************************************************************************************/
private class Widget : public Planet {
public:
	Widget(SplitView^ frame, UniverseDisplay^ master, PLCMaster* plc)
		: Planet(__MODULE__), frame(frame), master(master), device(plc) {
		Platform::String^ localhost = system_ipv4_address();
		
		this->inset = tiny_font_size * 0.5F;
		this->btn_xgapsize = 2.0F;
		this->root = false;

		for (unsigned int idx = 0; idx < sizeof(root_machines) / sizeof(Platform::String^); idx++) {
			if (localhost->Equals(root_machines[idx])) {
				this->root = true;
				break;
			}
		}

		this->set_plc_master_mode(PLCMasterMode::_);
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
			this->label_max = std::fmaxf(label_width, this->label_max);
		}

		for (Icon id = _E0(Icon); id < Icon::_; id++) {
			this->icons[id] = this->insert_one(new Credit<Labellet, Icon>(_speak(id), icon_font, Colours::GhostWhite), id);
		}

		this->load_buttons(this->brightnesses, button_style, button_height);
		this->load_buttons(this->permissions, button_style, button_height);
	}

	void reflow(float width, float height) override {
		float fx = 1.0F / float(_N(Icon) + 1);
		float button_y;

		this->move_to(this->labels[SS::Brightness], this->inset, height - tiny_font_size, GraphletAnchor::LB);
		this->move_to(this->labels[SS::Permission], this->labels[SS::Brightness], GraphletAnchor::LT, GraphletAnchor::LB, 0.0F, -tiny_font_size);

		this->reflow_buttons(this->brightnesses, this->labels[SS::Brightness]);
		this->reflow_buttons(this->permissions, this->labels[SS::Permission]);

		this->fill_graphlet_location(this->permissions[PLCMasterMode::Root], nullptr, &button_y);
		for (Icon id = _E0(Icon); id < Icon::_; id++) {
			this->move_to(this->icons[id], width * fx * float(_I(id) + 1), button_y, GraphletAnchor::CB, 0.0F, -tiny_font_size);
		}
	}

public:
	void update(long long count, long long interval, long long uptime) override {
		double alpha = this->master->global_mask_alpha;
		Buttonlet* target = nullptr;

		update_the_shown_gallery(count, interval, uptime, false);
		update_the_shown_alarm(count, interval, uptime);

		if ((this->settings != nullptr) && (this->settings->shown())) {
			this->settings->on_elapse(count, interval, uptime);
		}

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

		switch (this->device->get_mode()) {
		case PLCMasterMode::Root:  target = this->permissions[PLCMasterMode::Root]; break;
		case PLCMasterMode::User:  target = this->permissions[PLCMasterMode::User]; break;
		case PLCMasterMode::Debug: target = this->permissions[PLCMasterMode::Debug]; break;
		}

		for (PLCMasterMode cmd = _E0(PLCMasterMode); cmd < PLCMasterMode::_; cmd++) {
			if ((cmd == PLCMasterMode::Root) && (!this->root)) {
				this->permissions[cmd]->set_state(ButtonState::Disabled);
			} else {
				this->permissions[cmd]->set_state(this->permissions[cmd] == target, ButtonState::Executing, ButtonState::Ready);
			}
		}
	}

public:
	bool can_select(IGraphlet* g) override {
		return ((dynamic_cast<Credit<Labellet, Icon>*>(g) != nullptr)
			|| button_enabled(g));
	}

	void on_tap_selected(IGraphlet* g, float local_x, float local_y) override {
		auto b_btn = dynamic_cast<Credit<Buttonlet, Brightness>*>(g);
		auto p_btn = dynamic_cast<Credit<Buttonlet, PLCMasterMode>*>(g);
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
		} else if (p_btn != nullptr) {
			this->set_plc_master_mode(p_btn->id);
		} else if (icon != nullptr) {
			switch (icon->id) {
			case Icon::Gallery: display_the_gallery(); break;
			case Icon::TimeMachine: launch_the_timemachine(); break;
			case Icon::Alarm: display_the_alarm(); break;
			case Icon::Settings: {
				if (this->settings == nullptr) {
					this->settings = make_settings(this->device);
				}

				this->settings->show();
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
	void set_plc_master_mode(PLCMasterMode mode) {
		ApplicationDataContainer^ settings = ApplicationData::Current->LocalSettings;

		if (mode == PLCMasterMode::_) {
			bool set = false;
			
			if (settings->Values->HasKey(mode_setting_key)) {
				Platform::String^ mode_str = settings->Values->Lookup(mode_setting_key)->ToString();

				for (PLCMasterMode m = _E0(PLCMasterMode); m < PLCMasterMode::_; m++) {
					if (m.ToString()->Equals(mode_str)) {
						if ((m != PLCMasterMode::Root) || this->root) {
							this->device->set_mode(m);
							set = true;
						}

						break;
					}
				}
			}

			if (!set) {
				this->set_plc_master_mode(PLCMasterMode::User);
			}
		} else {
			this->device->set_mode(mode);
			settings->Values->Insert(mode_setting_key, mode.ToString());
		}
	}

private:
	float inset;
	float btn_xgapsize;
	float label_max;

private:
	SplitView^ frame;
	UniverseDisplay^ master;
	ISatellite* settings;
	PLCMaster* device;
	bool root;

private: // never delete these graphlets manually.
	std::map<Brightness, Credit<Buttonlet, Brightness>*> brightnesses;
	std::map<PLCMasterMode, Credit<Buttonlet, PLCMasterMode>*> permissions;
	std::map<Icon, Credit<Labellet, Icon>*> icons;
	std::map<SS, Labellet*> labels;
};

/*************************************************************************************************/
UniverseWidget::UniverseWidget(SplitView^ frame, UniverseDisplay^ master, PLCMaster* plc)
	: UniverseDisplay(master->get_logger()), frame(frame), master(master), plc(plc) {
	this->use_global_mask_setting(false);
	this->disable_predefined_shortcuts(true);
}

void UniverseWidget::construct(CanvasCreateResourcesReason reason) {
	initialize_the_alarm(this->plc);
	initialize_the_timemachine(this->plc, timemachine_speed, timemachine_frame_per_second);
	this->push_planet(new Widget(this->frame, this->master, this->plc));
}
