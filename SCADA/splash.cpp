#include "splash.hpp"
#include "configuration.hpp"

#include "paint.hpp"
#include "shape.hpp"
#include "brushes.hxx"

#include "graphlet/shapelet.hpp"
#include "graphlet/symbol/door/hopper_doorlet.hpp"

using namespace WarGrey::SCADA;

using namespace Windows::Foundation;

using namespace Microsoft::Graphics::Canvas;
using namespace Microsoft::Graphics::Canvas::UI;
using namespace Microsoft::Graphics::Canvas::Geometry;

static float icon_thickness = 24.0F;

private class IconCanvas final {
public:
	~IconCanvas() {
		if (this->hdoors != nullptr) {
			for (unsigned int idx = 0; idx < this->hcount * 2; idx++) {
				delete this->hdoors[idx];
			}

			delete this->hdoors;
		}
	}

	IconCanvas(SplashScreen* master) : master(master) {}

public:
	void load(float width, float height, float logo_width, float logo_height) {
		bool wide_icon = (logo_width != logo_height);
		float w = logo_width - icon_thickness;
		float h = logo_height - icon_thickness;
		float ship_width = logo_width * 0.5F;
		float ship_height = logo_height * 0.382F;
		float bow_radius = ship_height * 0.5F;
		float door_esize = (ship_height - icon_thickness * 7.0F) * 0.5F;
		float dragarm_length = ship_width * 0.618F;
		float suctin_line_length = door_esize * 1.618F;
		float thinness = icon_thickness * 0.382F;
		auto suction_line = vline(0.0F, 0.0F, door_esize * 1.618F, thinness);
		auto dragarm = hline(thinness, 0.0F, dragarm_length, icon_thickness);
		auto ps_dragbody = geometry_union(dragarm, suction_line, dragarm_length, 0.0F);
		auto sb_dragbody = geometry_union(dragarm, suction_line, dragarm_length, -suctin_line_length);
		
		this->hcount = (wide_icon ? hopper_count : (hopper_count / 2));
		this->hdoors = new Rectanglet*[this->hcount * 2U];

		this->frame = this->master->insert_one(new Rectanglet(w, h, Colours::Background, Colours::LightSeaGreen, icon_thickness));
		this->ship = this->master->insert_one(new Shiplet(ship_width, bow_radius, nullptr, Colours::Yellow, icon_thickness));
		this->ps_dragarm = this->master->insert_one(new Shapelet(ps_dragbody, Colours::make(default_ps_color)));
		this->sb_dragarm = this->master->insert_one(new Shapelet(sb_dragbody, Colours::make(default_sb_color)));
		this->ps_draghead = this->master->insert_one(new Segmentlet(-90.0, 90.0, door_esize * 1.618F, door_esize, default_ps_color, thinness));
		this->sb_draghead = this->master->insert_one(new Segmentlet(-90.0, 90.0, door_esize * 1.618F, door_esize, default_sb_color, thinness));
		
		for (unsigned int idx = 0; idx < this->hcount * 2U; idx++) {
			this->hdoors[idx] = this->master->insert_one(new Rectanglet(door_esize, Colours::Yellow, Colours::ForestGreen, door_esize * 0.5F));
		}
	}

	void reflow(float width, float height, float logo_width, float logo_height) {
		float cx = width * 0.5F;
		float cy = height * 0.5F;
		float cxoff_unit = float(this->hcount / 2) + ((logo_width == logo_height) ? 0.5F : 1.0F);
		float desize;

		this->hdoors[0]->fill_extent(0.0F, 0.0F, &desize);

		this->master->move_to(this->frame, cx, cy, GraphletAnchor::CC);
		this->master->move_to(this->ship, cx, cy, GraphletAnchor::CC);
		this->master->move_to(this->ps_dragarm, this->ship, GraphletAnchor::CT, GraphletAnchor::CB);
		this->master->move_to(this->sb_dragarm, this->ship, GraphletAnchor::CB, GraphletAnchor::CT);
		this->master->move_to(this->ps_draghead, this->ps_dragarm, GraphletAnchor::LT, GraphletAnchor::RC, 0.0F, icon_thickness * 0.5F);
		this->master->move_to(this->sb_draghead, this->sb_dragarm, GraphletAnchor::LB, GraphletAnchor::RC, 0.0F, -icon_thickness * 0.5F);

		for (unsigned int idx = 0; idx < this->hcount; idx++) {
			this->master->move_to(this->hdoors[idx], this->ship, GraphletAnchor::CT, GraphletAnchor::CT,
				(desize + icon_thickness) * (float(idx) - cxoff_unit), icon_thickness * 2.0F);
		}

		for (unsigned int idx = 0; idx < this->hcount; idx++) {
			this->master->move_to(this->hdoors[idx + this->hcount], this->ship, GraphletAnchor::CB, GraphletAnchor::CB,
				(desize + icon_thickness) * (float(idx) - cxoff_unit), -icon_thickness * 2.0F);
		}
	}

	// never deletes these graphlets mannually	
private:
	Rectanglet* frame;
	Rectanglet** hdoors;
	Shiplet* ship;
	Segmentlet* ps_draghead;
	Segmentlet* sb_draghead;
	Shapelet* ps_dragarm;
	Shapelet* sb_dragarm;
	
private:
	SplashScreen* master;
	unsigned int hcount;
};

/*************************************************************************************************/
static std::unordered_map<SplashScreen*, IconCanvas*> screens;

SplashScreen::SplashScreen(float square_size) : SplashScreen(square_size, square_size) {}

SplashScreen::SplashScreen(float wide_width, float wide_height)
	: Planet("icon"), logo_width(wide_width), logo_height(wide_height) {

	if (this->logo_height <= 0.0F) {
		this->logo_height = this->logo_width * 600.0F / 1240.0F;
	}
}

SplashScreen::~SplashScreen() {
	auto maybe_stage = screens.find(this);

	if (maybe_stage != screens.end()) {
		delete maybe_stage->second;

		screens.erase(maybe_stage);
	}
}

bool SplashScreen::can_select(IGraphlet* g) {
	return false;
}

void SplashScreen::load(CanvasCreateResourcesReason reason, float width, float height) {
	if (screens.find(this) == screens.end()) {
		IconCanvas* canvas = new IconCanvas(this);
		
		screens.insert(std::pair<SplashScreen*, IconCanvas*>(this, canvas));

		canvas->load(width, height, this->logo_width, this->logo_height);
	}
}

void SplashScreen::reflow(float width, float height) {
	auto maybe_canvas = screens.find(this);
	
	if (maybe_canvas != screens.end()) {
		maybe_canvas->second->reflow(width, height, this->logo_width, this->logo_height);
	}
}
