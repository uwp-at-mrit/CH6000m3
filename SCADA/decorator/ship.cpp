#include "decorator/ship.hpp"

#include "box.hpp"
#include "tongue.hpp"

#include "shape.hpp"
#include "brushes.hxx"
#include "geometry.hpp"
#include "transformation.hpp"

using namespace WarGrey::SCADA;

using namespace Windows::Foundation;
using namespace Windows::System;

using namespace Microsoft::Graphics::Canvas;
using namespace Microsoft::Graphics::Canvas::UI;
using namespace Microsoft::Graphics::Canvas::Text;
using namespace Microsoft::Graphics::Canvas::Brushes;
using namespace Microsoft::Graphics::Canvas::Geometry;

/*************************************************************************************************/
ShipDecorator::ShipDecorator() {
	float height = 0.618F * 0.72F;
	float radius = height * 0.5F;

	this->ship_width = 0.618F;
	this->ship_height = height;
	this->x = (1.0F - this->ship_width - radius) * 0.70F;
	this->y = 0.5F;
	this->ship = geometry_union(rectangle(this->ship_width, height),
		segment(this->ship_width, radius, -90.0, 90.0, radius, radius));
}

void ShipDecorator::draw_before(CanvasDrawingSession^ ds, float Width, float Height) {
	auto real_ship = geometry_scale(this->ship, Width, Height);
	Rect ship_box = real_ship->ComputeBounds();
	float thickness = 2.0F;
	float sx = this->x * Width;
	float sy = this->y * Height;

	ds->DrawGeometry(real_ship, sx, sy, Colours::Silver, thickness);
}

void ShipDecorator::fill_ship_extent(float* x, float* y, float* width, float* height, bool full) {
	float awidth = this->actual_width();
	float aheight = this->actual_height();
	auto abox = this->ship->ComputeBounds(make_scale_matrix(awidth, aheight));

	SET_VALUES(x, this->x * awidth, y, this->y * aheight);
	SET_BOX(width, (full ? abox.Width : this->ship_width * awidth));
	SET_BOX(height, abox.Height);
}

void ShipDecorator::fill_ship_anchor(float fx, float fy, float* x, float *y, bool full) {
	float awidth = this->actual_width();
	float aheight = this->actual_height();
	auto abox = this->ship->ComputeBounds(make_scale_matrix(awidth, aheight));
	float width = (full ? abox.Width : this->ship_width * awidth);

	SET_BOX(x, this->x * awidth + width * fx);
	SET_BOX(y, this->y * aheight + abox.Height * fy);
}


void ShipDecorator::fill_ascent_anchor(float fx, float fy, float* x, float *y) {
	float awidth = this->actual_width();
	float aheight = this->actual_height();
	auto abox = this->ship->ComputeBounds(make_scale_matrix(awidth, aheight));

	SET_BOX(x, this->x * awidth + this->ship_width * awidth * fx);
	SET_BOX(y, this->y * aheight * fy);
}

void ShipDecorator::fill_descent_anchor(float fx, float fy, float* x, float *y) {
	float awidth = this->actual_width();
	float aheight = this->actual_height();
	auto abox = this->ship->ComputeBounds(make_scale_matrix(awidth, aheight));

	SET_BOX(x, this->x * awidth + this->ship_width * awidth * fx);
	SET_BOX(y, aheight * fy + (this->y * aheight + abox.Height) * (1.0F - fy));
}

/*************************************************************************************************/
BottomDoorDecorator::BottomDoorDecorator() {
	this->y = (0.618F - this->ship_height) * 0.5F;
	
	{ // initializing sequence labels
		CanvasTextFormat^ cpt_font = make_bold_text_format("Microsoft YaHei", large_font_size);

		this->seq_color = Colours::Tomato;

		for (unsigned int idx = 0; idx < hopper_count; idx++) {
			Platform::String^ id = (hopper_count - idx).ToString();

			this->ps_seqs[idx] = make_text_layout(speak("PS" + id), cpt_font);
			this->sb_seqs[idx] = make_text_layout(speak("SB" + id), cpt_font);
		}
	}
}

void BottomDoorDecorator::draw_before(CanvasDrawingSession^ ds, float Width, float Height) {
	auto real_ship = geometry_scale(this->ship, Width, Height);
	Rect ship_box = real_ship->ComputeBounds();
	float thickness = 2.0F;
	float sx = this->x * Width;
	float sy = this->y * Height;
	float cell_width = this->ship_width * Width / float(hopper_count);
	float ps_y = sy + (ship_box.Height - this->ps_seqs[0]->LayoutBounds.Height) * 0.4F;
	float sb_y = sy + (ship_box.Height - this->sb_seqs[0]->LayoutBounds.Height) * 0.6F;

	ds->DrawGeometry(real_ship, sx, sy, Colours::Silver, thickness);

	for (size_t idx = 0; idx < hopper_count; idx++) {
		float cell_x = sx + cell_width * float(idx);
		float seq_width = this->ps_seqs[idx]->LayoutBounds.Width;
		float seq_x = cell_x + (cell_width - seq_width) * 0.5F;

		ds->DrawTextLayout(this->ps_seqs[idx], seq_x, ps_y, this->seq_color);
		ds->DrawTextLayout(this->sb_seqs[idx], seq_x, sb_y, this->seq_color);
	}
}

void BottomDoorDecorator::fill_door_cell_extent(float* x, float* y, float* width, float* height, size_t idx, float side_hint) {
	float awidth = this->actual_width();
	float aheight = this->actual_height();
	auto abox = this->ship->ComputeBounds(make_scale_matrix(awidth, aheight));
	float cell_width = this->ship_width * awidth / float(hopper_count);
	float cell_height = abox.Height * 0.25F;

	SET_VALUES(width, cell_width, height, cell_height);
	SET_BOX(x, this->x * awidth + cell_width * float(hopper_count - idx));
	SET_BOX(y, this->y * aheight + cell_height * side_hint);
}
