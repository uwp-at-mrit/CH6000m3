#pragma once

#include "planet.hpp"

#include "gps.hpp"

namespace WarGrey::SCADA {
	private class StatusFrame : public WarGrey::SCADA::Planet {
	public:
		virtual ~StatusFrame() noexcept;
		StatusFrame(WarGrey::SCADA::ITCPStatedConnection* plc = nullptr,
			WarGrey::SCADA::GPS* gps1 = nullptr, WarGrey::SCADA::GPS* gps2 = nullptr,
			WarGrey::SCADA::GPS* gyro = nullptr);

	public:
		void load(Microsoft::Graphics::Canvas::UI::CanvasCreateResourcesReason reason, float width, float height) override;
		void reflow(float width, float height) override;
		void update(long long count, long long interval, long long uptime) override;

	public:
		bool can_select(IGraphlet* g) override;

	private:
		WarGrey::SCADA::IGPSReceiver* statusbar;
	};
}
