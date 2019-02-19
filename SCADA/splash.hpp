#pragma once

#include "planet.hpp"

namespace WarGrey::SCADA {
	private class SplashScreen : public WarGrey::SCADA::Planet {
	public:
		virtual ~SplashScreen() noexcept;

	public:
		SplashScreen(float square_size);
		SplashScreen(float wide_width, float wide_height);

	public:
		void load(Microsoft::Graphics::Canvas::UI::CanvasCreateResourcesReason reason, float width, float height) override;
		void reflow(float width, float height) override;

	public:
		bool can_select(IGraphlet* g) override;

	private:
		float logo_width;
		float logo_height;
	};
}
