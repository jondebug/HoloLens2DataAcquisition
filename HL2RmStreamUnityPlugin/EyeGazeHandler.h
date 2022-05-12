#pragma once
#include "pch.h"
#include "winrt/base.h"
//#include <winrt/Windows.Foundation.Collections.0.h>
//#include <winrt/Windows.Foundation.h>
#include "winrt/Windows.UI.Input.Spatial.h"
#include "Cannon/MixedReality.h"
#include <mutex>
#include <iostream>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.Web.Syndication.h>
#include "EyeGazeHandler.h"
struct HeTHaTEyeFrame
{
	DirectX::XMMATRIX headTransform;
	std::array<DirectX::XMMATRIX, (size_t)HandJointIndex::Count> leftHandTransform;
	std::array<DirectX::XMMATRIX, (size_t)HandJointIndex::Count> rightHandTransform;
	DirectX::XMVECTOR eyeGazeOrigin;
	DirectX::XMVECTOR eyeGazeDirection;
	float eyeGazeDistance;
	bool leftHandPresent;
	bool rightHandPresent;
	bool eyeGazePresent;
	long long timestamp;
};

class EyeGazeStreamer
{

public:
	EyeGazeStreamer()
	{
	}

	virtual ~EyeGazeStreamer()
	{
		m_fExit = true;
		m_pStreamThread->join();
	}

	winrt::Windows::Foundation::IAsyncAction InitializeAsync(
		const long long minDelta,
		const winrt::Windows::Perception::Spatial::SpatialCoordinateSystem& coordSystem,
		std::wstring portName);

protected:
	//void OnFrameArrived(const winrt::Windows::Media::Capture::Frames::MediaFrameReader& sender,
	//	const winrt::Windows::Media::Capture::Frames::MediaFrameArrivedEventArgs& args);

private:

	winrt::Windows::Foundation::IAsyncAction StartEyeStreamServer();

	void OnConnectionReceived(
		winrt::Windows::Networking::Sockets::StreamSocketListener /* sender */,
		winrt::Windows::Networking::Sockets::StreamSocketListenerConnectionReceivedEventArgs args);

	static void EyeStreamThread(EyeGazeStreamer* pProcessor);

	void SendFrame(HeTHaTEyeFrame pFrame,	long long pTimestamp);

	//void WriteMatrix4x4(
	//	_In_ winrt::Windows::Foundation::Numerics::float4x4 matrix);

	std::shared_mutex m_frameMutex;
	long long m_latestTimestamp = 0;
	HeTHaTEyeFrame m_latestFrame;
	bool m_latestFrameExists = true;
	winrt::Windows::Media::Capture::Frames::MediaFrameReader m_mediaFrameReader = nullptr;
	winrt::event_token m_OnFrameArrivedRegistration;

	// streaming thread
	std::thread* m_pStreamThread = nullptr;
	bool m_fExit = false;

	bool m_streamingEnabled = true;

	TimeConverter m_converter;
	winrt::Windows::Perception::Spatial::SpatialCoordinateSystem m_worldCoordSystem = nullptr;

	winrt::Windows::Networking::Sockets::StreamSocketListener m_streamSocketListener;
	winrt::Windows::Networking::Sockets::StreamSocket m_streamSocket = nullptr;
	winrt::Windows::Storage::Streams::DataWriter m_writer;
	bool m_writeInProgress = false;

	std::wstring m_portName;
	// minDelta allows to enforce a certain time delay between frames
	// should be set in hundreds of nanoseconds (ms * 1e-4)
	long long m_minDelta;

	static const int kImageWidth;
	static const wchar_t kSensorName[3];


};