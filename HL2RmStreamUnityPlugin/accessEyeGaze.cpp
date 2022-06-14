#include "pch.h"
#include <winrt/Windows.Perception.Spatial.h>
using namespace winrt::Windows::UI::Input::Spatial;
using namespace winrt::Windows::Foundation::Numerics;
using namespace winrt::Windows::Perception::Spatial;
using namespace winrt::Windows::Graphics::Holographic;






void accessEyeGaze() {

	OutputDebugStringW(L"starting attempt to access eye gaze.\n");
	winrt::Windows::Perception::Spatial::SpatialLocator m_locator{ nullptr };
	winrt::Windows::Perception::Spatial::SpatialLocatability m_locatability{ winrt::Windows::Perception::Spatial::SpatialLocatability::Unavailable };
	winrt::Windows::Perception::Spatial::SpatialCoordinateSystem m_worldCoordinateSystemOverride{ nullptr };
	
	
	auto m_holoSpace = winrt::Windows::Graphics::Holographic::HolographicSpace::CreateForCoreWindow(winrt::Windows::UI::Core::CoreWindow::GetForCurrentThread());
	auto m_holoFrame = m_holoSpace.CreateNextFrame();
	auto prediction = m_holoFrame.CurrentPrediction();
	auto timestamp_pred = prediction.Timestamp();


	auto m_referenceFrame = m_locator.CreateStationaryFrameOfReferenceAtCurrentLocation();
	auto coordinateSystem = m_referenceFrame.CoordinateSystem();

	SpatialPointerPose pointerPose = SpatialPointerPose::TryGetAtTimestamp(coordinateSystem, timestamp_pred);
	if (pointerPose)
	{
		float3 headPosition = pointerPose.Head().Position();
		float3 headForwardDirection = pointerPose.Head().ForwardDirection();

		// Do something with the head-gaze
	}
}