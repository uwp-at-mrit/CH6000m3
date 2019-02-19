#include <map>

#include "widget/settings.hpp"
#include "configuration.hpp"

#include "graphlet/textlet.hpp"
#include "graphlet/shapelet.hpp"
#include "graphlet/buttonlet.hpp"

#include "iotables/ao_settings.hpp"

#include "system.hpp"
#include "module.hpp"
#include "brushes.hxx"

using namespace WarGrey::SCADA;

using namespace Windows::Foundation;
using namespace Windows::System;

using namespace Microsoft::Graphics::Canvas;
using namespace Microsoft::Graphics::Canvas::UI;
using namespace Microsoft::Graphics::Canvas::Text;
using namespace Microsoft::Graphics::Canvas::Brushes;

static CanvasSolidColorBrush^ region_background = Colours::make(diagnostics_region_background);
static CanvasSolidColorBrush^ settings_background = Colours::make(diagnostics_alarm_background);
static CanvasSolidColorBrush^ settings_foreground = Colours::Silver;
static CanvasSolidColorBrush^ caption_foreground = Colours::make(diagnostics_caption_foreground);
static CanvasSolidColorBrush^ caption_background = Colours::make(diagnostics_caption_background);

static const float settings_corner_radius = 8.0F;

static const wchar_t* hints[] = { L"Pressure", L"Degrees", L"Length", L"Density" };
static Platform::String^ units[] = { "bar", "degrees", "meter", "tpm3" };

// WARNING: order matters
private enum class Mode {
	psTrunnion, psIntermediate, psDragHead, sbTrunnion, sbIntermediate, sbDragHead,
	psDragPipes, sbDragPipes, psDoors, sbDoors, ShoreDischarge,
	Others,
	_
};

static Platform::String^ editor_unit(const wchar_t* metrics) {
	Platform::String^ unit = "%";

	for (unsigned int idx = 0; idx < sizeof(hints) / sizeof(const wchar_t*); idx++) {
		if (wcsstr(metrics, hints[idx]) != nullptr) {
			unit = units[idx];
			break;
		}
	}

	return unit;
}

/*************************************************************************************************/
template<typename S>
private struct Metrics {
	std::map<S, Credit<Labellet, S>*> labels;
	std::map<S, Credit<Dimensionlet, S>*> editors;
	std::map<S, Credit<RoundedRectanglet, S>*> boxes;
	float max_width;
};

private class Settings : public ISatellite, public PLCConfirmation {
public:
	Settings(PLCMaster* device) : ISatellite(default_logging_level, __MODULE__), gapsize(normal_font_size), device(device) {
		this->caption_font = make_bold_text_format("Consolas", large_font_size);
		this->label_font = make_bold_text_format("Consolas", normal_font_size);
		this->setting_font = make_bold_text_format("Microsoft YeHei", normal_font_size);
		this->settings_style = make_highlight_dimension_style(large_font_size, 8U, 1);

		this->button_style.font = make_bold_text_format("Microsoft YeHei", normal_font_size);
		this->button_style.corner_radius = 3.0F;
		this->button_style.thickness = 2.0F;

		this->device->push_confirmation_receiver(this);
	}

	void Settings::fill_extent(float* width, float* height) {
		SET_VALUES(width, 700.0F, height, 700.0F);
	}

public:
	void pre_read_data(Syslog* logger) override {
		this->enter_critical_section();
		this->begin_update_sequence();
	}

	void on_forat(long long timepoint_ms, const uint8* DB20, size_t count, Syslog* logger) override {
		this->AI_settings(DB20, this->pst_metrics, AO_gantry_winch_trunnion_settings, true);
		this->AI_settings(DB20, this->psi_metrics, AO_gantry_winch_intermediate_settings, true);
		this->AI_settings(DB20, this->psh_metrics, AO_gantry_winch_draghead_settings, true);

		this->AI_settings(DB20, this->sbt_metrics, AO_gantry_winch_trunnion_settings, false);
		this->AI_settings(DB20, this->sbi_metrics, AO_gantry_winch_intermediate_settings, false);
		this->AI_settings(DB20, this->sbh_metrics, AO_gantry_winch_draghead_settings, false);

		this->AI_settings(DB20, this->psdp_metrics, AO_drag_pipes_settings, true);
		this->AI_settings(DB20, this->sbdp_metrics, AO_drag_pipes_settings, false);

		this->AI_settings(DB20, this->psds_metrics, AO_doors_settings, true);
		this->AI_settings(DB20, this->sbds_metrics, AO_doors_settings, false);

		this->AI_settings(DB20, this->shore_discharge_metrics, AO_shore_discharge_settings, true);

		this->AI_settings(DB20, this->other_metrics, AO_other_settings, true);
	}

	void post_read_data(Syslog* logger) override {
		this->end_update_sequence();
		this->leave_critical_section();
	}

public:
	void load(CanvasCreateResourcesReason reason, float width, float height) override {
		float caption_width = width * 0.618F;
		float caption_height = this->caption_font->FontSize * 1.618F;
		float settings_height = this->setting_font->FontSize * 2.4F;
		float button_width = width - caption_width - this->gapsize * 4.0F;
		float button_height = this->button_style.font->FontSize * 2.4F;

		this->change_mode(0U);
		this->load_buttons(this->tabs, button_width, button_height);
		
		this->load_settings(Mode::psTrunnion, this->pst_metrics, caption_width, caption_height, default_ps_color);
		this->load_settings(Mode::psIntermediate, this->psi_metrics, caption_width, caption_height, default_ps_color);
		this->load_settings(Mode::psDragHead, this->psh_metrics, caption_width, caption_height, default_ps_color);
		this->load_settings(Mode::sbTrunnion, this->sbt_metrics, caption_width, caption_height, default_sb_color);
		this->load_settings(Mode::sbIntermediate, this->sbi_metrics, caption_width, caption_height, default_sb_color);
		this->load_settings(Mode::sbDragHead, this->sbh_metrics, caption_width, caption_height, default_sb_color);
		this->load_settings(Mode::psDragPipes, this->psdp_metrics, caption_width, caption_height, default_ps_color);
		this->load_settings(Mode::sbDragPipes, this->sbdp_metrics, caption_width, caption_height, default_sb_color);
		this->load_settings(Mode::psDoors, this->psds_metrics, caption_width, caption_height, default_ps_color);
		this->load_settings(Mode::sbDoors, this->sbds_metrics, caption_width, caption_height, default_sb_color);
		this->load_settings(Mode::ShoreDischarge, this->shore_discharge_metrics, caption_width, caption_height);
		this->load_settings(Mode::Others, this->other_metrics, caption_width, caption_height);
	}

	void reflow(float width, float height) override {
		float button_x = this->gapsize;
		float button_y = this->gapsize;
		float caption_rx = width - this->gapsize;
		float caption_y = this->gapsize;
		float button_width, button_height;

		this->change_mode(0U);
		this->tabs[Mode::psDragHead]->fill_extent(0.0F, 0.0F, &button_width, &button_height);
		for (Mode m = _E(Mode, 0); m < Mode::_; m++) {
			this->move_to(this->tabs[m], button_x, button_height * _F(m) + button_y);
		}

		this->reflow_settings(Mode::psTrunnion, this->pst_metrics, caption_rx, caption_y);
		this->reflow_settings(Mode::psIntermediate, this->psi_metrics, caption_rx, caption_y);
		this->reflow_settings(Mode::psDragHead, this->psh_metrics, caption_rx, caption_y);
		this->reflow_settings(Mode::sbTrunnion, this->sbt_metrics, caption_rx, caption_y);
		this->reflow_settings(Mode::sbIntermediate, this->sbi_metrics, caption_rx, caption_y);
		this->reflow_settings(Mode::sbDragHead, this->sbh_metrics, caption_rx, caption_y);
		this->reflow_settings(Mode::psDragPipes, this->psdp_metrics, caption_rx, caption_y);
		this->reflow_settings(Mode::sbDragPipes, this->sbdp_metrics, caption_rx, caption_y);
		this->reflow_settings(Mode::psDoors, this->psds_metrics, caption_rx, caption_y);
		this->reflow_settings(Mode::sbDoors, this->sbds_metrics, caption_rx, caption_y);
		this->reflow_settings(Mode::ShoreDischarge, this->shore_discharge_metrics, caption_rx, caption_y);
		this->reflow_settings(Mode::Others, this->other_metrics, caption_rx, caption_y);

		this->change_mode(1U);
	}

public:
	bool available() override {
		return (this->surface_ready() && this->shown());
	}

	void update(long long count, long long interval, long long uptime) override {
		unsigned int mode = this->current_mode();

		for (auto it = this->tabs.begin(); it != this->tabs.end(); it++) {
			it->second->set_state((mode >> _I(it->second->id)) == 1U, ButtonState::Executing, ButtonState::Ready);
		}
	}

public:
	bool Settings::can_select(IGraphlet* g) override {
		return button_enabled(g);
	}

	void Settings::on_tap_selected(IGraphlet* g, float local_x, float local_y) override {
		auto tab = dynamic_cast<Credit<Buttonlet, Mode>*>(g);

		if (tab != nullptr) {
			this->change_mode(1U << _I(tab->id));
		}
	}

	void Settings::on_focus(IGraphlet* g, bool yes) override {
		if (yes) {
			auto editor = dynamic_cast<IEditorlet*>(g);

			if (editor != nullptr) {
				this->show_virtual_keyboard(ScreenKeyboard::Numpad);
			}
		}
	}


	bool on_key(VirtualKey key, bool wargrey_keyboard) override {
		bool handled = Planet::on_key(key, wargrey_keyboard);

		if ((!handled) && (this->device->get_mode() != PLCMasterMode::User)) {
			auto editor = dynamic_cast<Dimensionlet*>(this->get_focus_graphlet());
			
			if (editor != nullptr) {
				float v = float(editor->get_input_number());
				unsigned int addr = 0U;
				Mode m = Mode::_;

				for (auto it = this->tabs.begin(); it != this->tabs.end(); it++) {
					if (it->second->get_state() == ButtonState::Executing) {
						m = it->second->id;
						break;
					}
				}

				switch (m) {
				case Mode::psTrunnion: addr = AO_gantry_winch_trunnion_settings(this->id<GantryWinchTrunnionSettings>(editor), true); break;
				case Mode::psIntermediate: addr = AO_gantry_winch_intermediate_settings(this->id<GantryWinchIntermediateSettings>(editor), true); break;
				case Mode::psDragHead: addr = AO_gantry_winch_draghead_settings(this->id<GantryWinchDragHeadSettings>(editor), true); break;

				case Mode::sbTrunnion: addr = AO_gantry_winch_trunnion_settings(this->id<GantryWinchTrunnionSettings>(editor), false); break;
				case Mode::sbIntermediate: addr = AO_gantry_winch_intermediate_settings(this->id<GantryWinchIntermediateSettings>(editor), false); break;
				case Mode::sbDragHead: addr = AO_gantry_winch_draghead_settings(this->id<GantryWinchDragHeadSettings>(editor), false); break;

				case Mode::psDragPipes: addr = AO_drag_pipes_settings(this->id<DragPipesSettings>(editor), true); break;
				case Mode::sbDragPipes: addr = AO_drag_pipes_settings(this->id<DragPipesSettings>(editor), false); break;

				case Mode::psDoors: addr = AO_doors_settings(this->id<DoorsSettings>(editor), true); break;
				case Mode::sbDoors: addr = AO_doors_settings(this->id<DoorsSettings>(editor), false); break;

				case Mode::ShoreDischarge: addr = AO_shore_discharge_settings(this->id<ShoreDischargeSettings>(editor), true); break;

				case Mode::Others: addr = AO_other_settings(this->id<OtherSettings>(editor), true); break;
				}

				this->device->send_setting(addr, v);

				this->hide_virtual_keyboard();
				this->set_caret_owner(nullptr);

				handled = true;
			}
		}

		return handled;
	}

private:
	template<typename CMD>
	void load_buttons(std::map<CMD, Credit<Buttonlet, CMD>*>& bs, float width, float height) {
		for (CMD cmd = _E0(CMD); cmd < CMD::_; cmd++) {
			bs[cmd] = this->insert_one(new Credit<Buttonlet, CMD>(cmd.ToString(), width, height, __MODULE__), cmd);
			bs[cmd]->set_style(this->button_style);
		}
	}

	template<typename S>
	void load_settings(Mode m, Metrics<S>& ms, float width, float height, unsigned int subcolor = 0xF8F8FFU) {
		unsigned int idx = (1U << _I(m));
		float editor_width;

		this->change_mode(idx);

		this->caption_boxes[m] = this->insert_one(new Rectanglet(width, height, caption_background));
		this->captions[m] = this->insert_one(new Labellet(_speak(m), this->caption_font, caption_foreground));

		ms.max_width = 0.0F;

		for (S id = _E(S, 0); id < S::_; id++) {
			const wchar_t* lbl_cpt = id.ToString()->Data();

			if (lbl_cpt[0] == L'_') {
				ms.labels[id] = this->insert_one(new Credit<Labellet, S>(_speak(id), this->label_font, subcolor), id);
			} else {
				ms.boxes[id] = this->insert_one(new Credit<RoundedRectanglet, S>(width, height, settings_corner_radius, settings_background), id);
				ms.labels[id] = this->insert_one(new Credit<Labellet, S>(_speak(id), this->setting_font), id);
				ms.editors[id] = this->insert_one(new Credit<Dimensionlet, S>(DimensionState::Input, this->settings_style, editor_unit(lbl_cpt)), id);

				ms.editors[id]->fill_extent(0.0F, 0.0F, &editor_width);
				ms.max_width = std::fmaxf(ms.max_width, editor_width);
			}
		}
	}

private:
	template<typename S>
	void reflow_settings(Mode m, Metrics<S>& ms, float rx, float y) {
		float caption_width, caption_height, setting_height;
		float inset = this->gapsize * 0.42F;
		unsigned int idx = (1U << _I(m));
		
		this->change_mode(idx);

		this->caption_boxes[m]->fill_extent(0.0F, 0.0F, &caption_width, &caption_height);
		this->move_to(this->caption_boxes[m], rx, y, GraphletAnchor::RT);
		this->move_to(this->captions[m], rx - caption_width * 0.5F, y + caption_height * 0.5F, GraphletAnchor::CC);

		y += (caption_height + inset);
		for (S id = _E(S, 0); id < S::_; id++) {
			if (ms.editors.find(id) == ms.editors.end()) {
				ms.labels[id]->fill_extent(0.0F, 0.0F, nullptr, &setting_height);
				this->move_to(ms.labels[id], rx - caption_width * 0.5F, y, GraphletAnchor::CT);
			} else {
				ms.boxes[id]->fill_extent(0.0F, 0.0F, nullptr, &setting_height);
				this->move_to(ms.boxes[id], rx, y, GraphletAnchor::RT);
				this->move_to(ms.labels[id], ms.boxes[id], GraphletAnchor::LC, GraphletAnchor::LC, inset);
				this->move_to(ms.editors[id], ms.boxes[id], GraphletAnchor::RC, GraphletAnchor::LC, -(inset + ms.max_width));
			}

			y += (setting_height + inset);
		}
	}

private:
	template<typename S, typename F, typename A>
	void AI_settings(const uint8* db20, Metrics<S>& ms, F AI, A arg) {
		for (S id = _E(S, 0); id < S::_; id++) {
			unsigned int idx = AI(id, arg);

			if (idx > 0) {
				ms.editors[id]->set_value(DBD(db20, idx));
			}
		}
	}

	template<typename S>
	S id(Dimensionlet* editor) {
		return dynamic_cast<Credit<Dimensionlet, S>*>(editor)->id;
	}

private: // never delete these graphlets manually.
	std::map<Mode, Labellet*> captions;
	std::map<Mode, Rectanglet*> caption_boxes;
	Metrics<GantryWinchTrunnionSettings> pst_metrics;
	Metrics<GantryWinchIntermediateSettings> psi_metrics;
	Metrics<GantryWinchDragHeadSettings> psh_metrics;
	Metrics<GantryWinchTrunnionSettings> sbt_metrics;
	Metrics<GantryWinchIntermediateSettings> sbi_metrics;
	Metrics<GantryWinchDragHeadSettings> sbh_metrics;
	Metrics<DragPipesSettings> psdp_metrics;
	Metrics<DragPipesSettings> sbdp_metrics;
	Metrics<DoorsSettings> psds_metrics;
	Metrics<DoorsSettings> sbds_metrics;
	Metrics<ShoreDischargeSettings> shore_discharge_metrics;
	Metrics<OtherSettings> other_metrics;

private:
	std::map<Mode, Credit<Buttonlet, Mode>*> tabs;

private:
	CanvasTextFormat^ caption_font;
	CanvasTextFormat^ label_font;
	CanvasTextFormat^ setting_font;
	DimensionStyle settings_style;
	ButtonStyle button_style;

private:
	float gapsize;

private:
	PLCMaster* device;
};

/*************************************************************************************************/
ISatellite* WarGrey::SCADA::make_settings(PLCMaster* plc) {
	return new Settings(plc);
}
