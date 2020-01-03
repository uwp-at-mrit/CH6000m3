#pragma once

#include "planet.hpp"

#include "system.hpp"
#include "network/tcp.hpp"

namespace WarGrey::DTPM {
	private class StatusFrame : public WarGrey::SCADA::Planet {
	public:
		virtual ~StatusFrame() noexcept;
		StatusFrame(WarGrey::SCADA::ITCPConnection* plc = nullptr);

	public:
		void load(Microsoft::Graphics::Canvas::UI::CanvasCreateResourcesReason reason, float width, float height) override;
		void reflow(float width, float height) override;
		void update(long long count, long long interval, long long uptime) override;

	public:
		bool can_select(WarGrey::SCADA::IGraphlet* g) override;

	private:
		WarGrey::SCADA::ISystemStatusListener* statusbar;
	};
}
