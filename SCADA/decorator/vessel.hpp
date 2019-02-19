#pragma once

#include "decorator/decorator.hpp"

namespace WarGrey::SCADA {
	template<class V, typename E>
	private class TVesselDecorator : public WarGrey::SCADA::IPlanetDecorator {
	public:
		TVesselDecorator(V* master) : master(master) {
			float height = 1.0F;
			float xradius = height * 0.10F;
			float yradius = height * 0.50F;

			this->ship_width = 1.0F - xradius;
			this->ship = geometry_union(rectangle(this->ship_width, height),
				segment(this->ship_width, yradius, -90.0, 90.0, xradius, yradius));

			this->ship_style = make_dash_stroke(CanvasDashStyle::Dash);
		}

	public:
		void draw_before(Microsoft::Graphics::Canvas::CanvasDrawingSession^ ds, float Width, float Height) override {
			this->master->draw_relationships(ds, Width, Height);
		}

		void draw_before_graphlet(WarGrey::SCADA::IGraphlet* g, Microsoft::Graphics::Canvas::CanvasDrawingSession^ ds
			, float x, float y, float width, float height, bool is_selected) override {
			auto station = dynamic_cast<WarGrey::SCADA::Tracklet<E>*>(g);

			if (station != nullptr) {
				float ps_y, sb_y, deck_lx, deck_ty, deck_rx, deck_by;

				station->fill_anchor_location(E::ps, nullptr, &ps_y, false);
				station->fill_anchor_location(E::sb, nullptr, &sb_y, false);

				station->fill_anchor_location(E::deck_lx, &deck_lx, nullptr, false);
				station->fill_anchor_location(E::deck_rx, &deck_rx, nullptr, false);
				station->fill_anchor_location(E::deck_ty, nullptr, &deck_ty, false);
				station->fill_anchor_location(E::deck_by, nullptr, &deck_by, false);

				{ // draw ship
					float ship_width = this->actual_width();
					float ship_height = std::fabsf(sb_y - ps_y);
					auto real_ship = geometry_scale(this->ship, ship_width, ship_height);
					Rect ship_box = real_ship->ComputeBounds();
					float sx = 0.0F;
					float sy = y + std::fminf(sb_y, ps_y);

					ds->DrawGeometry(real_ship, sx, sy, Colours::SeaGreen, 1.0F, this->ship_style);
				}

				{ // draw deck region
					float dx = x + std::fminf(deck_lx, deck_rx);
					float dy = y + std::fminf(deck_ty, deck_by);
					float dw = std::fabsf((deck_rx - deck_lx));
					float dh = std::fabsf((deck_by - deck_ty));

					ds->DrawGeometry(rectangle(dx, dy, dw, dh), Colours::SeaGreen, 1.0F, this->ship_style);
				}

				this->draw_non_important_lines(station, ds, x, y, this->ship_style);
			}
		}

	protected:
		virtual void draw_non_important_lines(WarGrey::SCADA::Tracklet<E>* station,
			Microsoft::Graphics::Canvas::CanvasDrawingSession^ ds, float x, float y,
			Microsoft::Graphics::Canvas::Geometry::CanvasStrokeStyle^ style) {}

	private:
		Microsoft::Graphics::Canvas::Geometry::CanvasGeometry^ ship;
		Microsoft::Graphics::Canvas::Geometry::CanvasStrokeStyle^ ship_style;

	private:
		float ship_width;

	private:
		V* master;
	};
}
