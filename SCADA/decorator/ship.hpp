#pragma once

#include "configuration.hpp"

#include "decorator/decorator.hpp"

namespace WarGrey::SCADA {
	private class ShipDecorator : public IPlanetDecorator {
	public:
		ShipDecorator();

	public:
		void draw_before(Microsoft::Graphics::Canvas::CanvasDrawingSession^ ds, float Width, float Height) override;

	public:
		void fill_ship_extent(float* x, float* y, float* width, float* height, bool full = false);

	public:
		void fill_ship_anchor(float fx, float fy, float* x, float *y, bool full = false);
		void fill_ascent_anchor(float fx, float fy, float* x, float *y);
		void fill_descent_anchor(float fx, float fy, float* x, float *y);

	protected:
		Microsoft::Graphics::Canvas::Geometry::CanvasGeometry^ ship;

	protected:
		float x;
		float y;
		float ship_width;
		float ship_height;
	};

	private class BottomDoorDecorator : public WarGrey::SCADA::ShipDecorator {
	public:
		BottomDoorDecorator();

	public:
		void draw_before(Microsoft::Graphics::Canvas::CanvasDrawingSession^ ds, float Width, float Height) override;

	public:
		void fill_door_cell_extent(float* x, float* y, float* width, float* height, size_t idx, float side_hint);
		
	private:
		Microsoft::Graphics::Canvas::Text::CanvasTextLayout^ ps_seqs[hopper_count];
		Microsoft::Graphics::Canvas::Text::CanvasTextLayout^ sb_seqs[hopper_count];
		Microsoft::Graphics::Canvas::Brushes::ICanvasBrush^ seq_color;
	};
}
