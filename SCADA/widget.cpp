#include <map>
#include <ppltasks.h>

#include "widget.hxx"
#include "planet.hpp"
#include "about.hpp"
#include "configuration.hpp"

#include "widget/alarms.hpp"
#include "widget/gallery.hpp"
#include "widget/settings.hpp"
#include "widget/timestream.hpp"

#include "graphlet/ui/textlet.hpp"
#include "graphlet/ui/buttonlet.hpp"
#include "graphlet/ui/togglet.hpp"

#include "datum/time.hpp"
#include "datum/path.hpp"
#include "datum/flonum.hpp"

#include "peer/slang.hpp"

#include "preference.hxx"
#include "system.hpp"
#include "module.hpp"

using namespace WarGrey::GYDM;
using namespace WarGrey::SCADA;

using namespace Concurrency;

using namespace Windows::Foundation;
using namespace Windows::System;

using namespace Windows::Security::Credentials;
using namespace Windows::Security::Credentials::UI;

using namespace Windows::UI::ViewManagement;
using namespace Windows::UI::Xaml::Controls;

using namespace Microsoft::Graphics::Canvas;
using namespace Microsoft::Graphics::Canvas::UI;
using namespace Microsoft::Graphics::Canvas::Text;
using namespace Microsoft::Graphics::Canvas::Brushes;

private enum class Brightness { Brightness100, Brightness80, Brightness60, Brightness40, Brightness30, Brightness20, _ };

// WARNING: order matters
private enum class SS : unsigned int { Brightnessd, Brightness, Permission, _ };
private enum class Icon : unsigned int { Gallery , Settings, TimeMachine, Alarm, PrintScreen, FullScreen, About, _ };

static ICanvasBrush^ about_bgcolor = Colours::WhiteSmoke;
static ICanvasBrush^ about_fgcolor = Colours::Black;

static Platform::String^ brightness_switch_key = "Brightness_Switch_Key";

/*************************************************************************************************/
namespace {
	private class Widget : public Planet, public SlangLocalPeer<uint8> {
	public:
		Widget(SplitView^ frame, UniverseDisplay^ master, PLCMaster* plc)
			: Planet(__MODULE__), frame(frame), master(master), device(plc) {
			Platform::String^ localhost = system_ipv4_address();
			
			this->inset = tiny_font_size * 0.5F;
			this->btn_xgapsize = 2.0F;
			this->root = false;
			this->set_plc_master_mode(TCPMode::User);

			this->brightnessd = new SlangDaemon<uint8>(make_system_logger(default_slang_logging_level, "Slang"), slang_brightness_port, this);
			this->brightnessd->join_multicast_group(slang_multicast_group);
			
			create_task(KeyCredentialManager::IsSupportedAsync()).then([this](task<bool> available) {
				try {
					this->root = available.get();
				} catch (Platform::Exception^ e) {
					this->master->get_logger()->log_message(Log::Warning, e->Message);
				}
			});
		}

	public:
		void load(CanvasCreateResourcesReason reason, float width, float height) override {
			auto label_font = make_text_format("Microsoft YaHei", large_font_size);
			auto icon_font = make_text_format("Consolas", 30.0F);
			float button_height, label_width;
			ButtonStyle button_style;

			button_style.font = make_bold_text_format(tiny_font_size);
			button_style.corner_radius = 2.0F;
			button_style.thickness = 1.0F;

			for (SS id = _E0(SS); id < SS::_; id++) {
				this->labels[id] = this->insert_one(new Labellet(_speak(id), label_font, Colours::GhostWhite));
				this->labels[id]->fill_extent(0.0F, 0.0F, &label_width, &button_height);
				this->label_max = flmax(label_width, this->label_max);
			}

			for (Icon id = _E0(Icon); id < Icon::_; id++) {
				this->icons[id] = this->insert_one(new Credit<Labellet, Icon>(_speak(id), icon_font, Colours::GhostWhite), id);
			}

			this->load_buttons(this->brightnesses, button_style, button_height);
			this->load_buttons(this->permissions, button_style, button_height);

			{ // load toggle
				float toggle_width = this->min_width() - this->inset * 3.0F - this->label_max;
				bool toggle_state = get_preference(brightness_switch_key, true);

				this->global_brightness = this->insert_one(new Togglet(toggle_state, "On", "Off", toggle_width));
			}
		}

		void reflow(float width, float height) override {
			float fx = 1.0F / float(_N(Icon) + 1);
			float button_y;

			this->move_to(this->labels[SS::Brightnessd], this->inset, height - tiny_font_size, GraphletAnchor::LB);
			this->move_to(this->labels[SS::Brightness], this->labels[SS::Brightnessd], GraphletAnchor::LT, GraphletAnchor::LB, 0.0F, -tiny_font_size);
			this->move_to(this->labels[SS::Permission], this->labels[SS::Brightness], GraphletAnchor::LT, GraphletAnchor::LB, 0.0F, -tiny_font_size);

			this->move_to(this->global_brightness, this->labels[SS::Brightnessd], GraphletAnchor::RC, GraphletAnchor::LC, this->inset);
			this->reflow_buttons(this->brightnesses, this->labels[SS::Brightness]);
			this->reflow_buttons(this->permissions, this->labels[SS::Permission]);

			this->fill_graphlet_location(this->permissions[TCPMode::Root], nullptr, &button_y);
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
			case TCPMode::Root:  target = this->permissions[TCPMode::Root]; break;
			case TCPMode::User:  target = this->permissions[TCPMode::User]; break;
			case TCPMode::Debug: target = this->permissions[TCPMode::Debug]; break;
			}

			for (TCPMode cmd = _E0(TCPMode); cmd < TCPMode::_; cmd++) {
				if ((cmd == TCPMode::Root) && (!this->root)) {
					this->permissions[cmd]->set_state(ButtonState::Disabled);
				} else {
					this->permissions[cmd]->set_state(this->permissions[cmd] == target, ButtonState::Executing, ButtonState::Ready);
				}
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
			auto t_btn = dynamic_cast<Togglet*>(g);

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
					if (this->global_brightness->checked()) {
						this->brightnessd->multicast(slang_brightness_port, asn_real_to_octets(alpha));
						this->get_logger()->log_message(Log::Notice, "GROUP EVENT: change screen brightness");
					} else {
						this->set_brightness(alpha);
					}
				}
			} else if (p_btn != nullptr) {
				if (this->device->get_mode() != p_btn->id) {
					this->set_plc_master_mode(p_btn->id);
				}
			} else if (t_btn != nullptr) {
				t_btn->toggle();
				put_preference(brightness_switch_key, t_btn->checked());
			} else if (icon != nullptr) {
				switch (icon->id) {
				case Icon::Gallery: display_the_gallery(); break;
				case Icon::TimeMachine: launch_the_timemachine(); break;
				case Icon::Alarm: display_the_alarm(); break;
				case Icon::About: {
					if (this->about == nullptr) {
						this->about = make_about("about.png", about_bgcolor, about_fgcolor);
					}

					this->about->show();
				}; break;
				case Icon::Settings: {
					if (this->settings == nullptr) {
						// NOTE: this will reset the last setting timestamp
						this->settings = make_settings(this->device);
					}
					
					if ((current_seconds() - last_setting_seconds()) > plc_settings_pinfree_seconds) {
						auto verify = create_task(KeyCredentialManager::RequestCreateAsync("root", KeyCredentialCreationOption::ReplaceExisting));

						/** NOTE
					 	 * The touch keyboard will show automatically if:
						 *   there is not hardware keyboard connected, and
						 *     the device is in tablet mode, or
						 *     "Show the touch keyboard when not in tablet mode" is On
						 *       [settings -> device -> typing (this option may not exists)]
						 */

						verify.then([this](task<KeyCredentialRetrievalResult^> result) {
							try {
								KeyCredentialStatus state = result.get()->Status;

								switch (state) {
									case KeyCredentialStatus::Success : this->settings->show(); break;
									case KeyCredentialStatus::NotFound: this->get_logger()->log_message(Log::Warning, _speak(state)); break;
								}
							} catch (Platform::Exception ^ e) {
								this->master->get_logger()->log_message(Log::Warning, e->Message);
							}
						});
					} else {
						this->settings->show();
					}
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
		void on_message(long long timepoint_ms, Platform::String^ remote_peer, uint16 port, uint8 type, const uint8* message, Syslog* logger) override {
			// NOTE: the brightnessd is a standalone daemon, all messages therefore concern the brightness setting
			
			if (this->global_brightness->checked()) {
				double alpha = asn_octets_to_real(message);

				if ((alpha >= 0.0) && (alpha <= 1.0)) {
					this->set_brightness(alpha);
					this->get_logger()->log_message(Log::Info, L"brightness has been changed by %s", remote_peer->Data());
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
		void set_plc_master_mode(TCPMode mode) {
			this->device->set_mode(mode);
		}

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
		ISatellite* settings;
		ISatellite* about;
		PLCMaster* device;
		SlangDaemon<uint8>* brightnessd;
		Togglet* global_brightness;
		bool root;

	private: // never delete these graphlets manually.
		std::map<Brightness, Credit<Buttonlet, Brightness>*> brightnesses;
		std::map<TCPMode, Credit<Buttonlet, TCPMode>*> permissions;
		std::map<Icon, Credit<Labellet, Icon>*> icons;
		std::map<SS, Labellet*> labels;
	};
}

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
