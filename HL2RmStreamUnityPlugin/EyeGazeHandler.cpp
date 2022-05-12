#pragma once
#include "pch.h"
#include "winrt/base.h"
//#include <winrt/Windows.Foundation.Collections.0.h>
//#include <winrt/Windows.Foundation.h>
#include "winrt/Windows.UI.Input.Spatial.h"
#include "Cannon/MixedReality.h"
#include <mutex>
#include "EyeGazeHandler.h"
#include <iostream>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.Web.Syndication.h>

using namespace winrt::Windows::Foundation;
using namespace winrt::Windows::Media::Capture;
using namespace winrt::Windows::Media::Capture::Frames;
using namespace winrt::Windows::Graphics::Imaging;
using namespace winrt::Windows::Perception::Spatial;
using namespace winrt::Windows::Networking::Sockets;
using namespace winrt::Windows::Storage::Streams;

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


void DumpEyeGazeIfPresentElseZero(bool present, const XMVECTOR& origin, const XMVECTOR& direction, float distance, std::ostream& out)
{
	XMFLOAT4 zeros{ 0, 0, 0, 0 };
	XMVECTOR zero4 = XMLoadFloat4(&zeros);
	/*out << present << ",";
	if (present)
	{
		out << origin << "," << direction;
	}
	else
	{
		out << zero4 << "," << zero4;
	}
	out << "," << distance; */
}

void EyeGazeStreamer::OnConnectionReceived(
	StreamSocketListener /* sender */,
	StreamSocketListenerConnectionReceivedEventArgs args)
{
	try
	{
		m_streamSocket = args.Socket();
		m_writer = (winrt::Windows::Storage::Streams::DataWriter)args.Socket().OutputStream();
		m_writer.UnicodeEncoding(UnicodeEncoding::Utf8);
		m_writer.ByteOrder(ByteOrder::LittleEndian);

		m_writeInProgress = false;
		OutputDebugStringW(L"EyeGazeStreamer::OnConnectionReceived: Received connection! \n");
	}
	catch (winrt::hresult_error const& ex)
	{
		SocketErrorStatus webErrorStatus{ SocketError::GetStatus(ex.to_abi()) };
		winrt::hstring message = webErrorStatus != SocketErrorStatus::Unknown ?
			winrt::to_hstring((int32_t)webErrorStatus) : winrt::to_hstring(ex.to_abi());
		OutputDebugStringW(L"VideoCameraStreamer::StartServer: Failed to open listener with ");
		OutputDebugStringW(message.c_str());
		OutputDebugStringW(L"\n");
	}
}
IAsyncAction EyeGazeStreamer::StartEyeStreamServer()
{
	try
	{
		// The ConnectionReceived event is raised when connections are received.
		m_streamSocketListener.ConnectionReceived({ this, &EyeGazeStreamer::OnConnectionReceived });

		// Start listening for incoming TCP connections on the specified port. You can specify any port that's not currently in use.
		// Every protocol typically has a standard port number. For example, HTTP is typically 80, FTP is 20 and 21, etc.
		// For this example, we'll choose an arbitrary port number.
		co_await m_streamSocketListener.BindServiceNameAsync(m_portName);
		//m_streamSocketListener.Control().KeepAlive(true);

		wchar_t msgBuffer[200];
		swprintf_s(msgBuffer, L"VideoCameraStreamer::StartServer: Server is listening at %ls \n",
			m_portName.c_str());
		OutputDebugStringW(msgBuffer);
	}
	catch (winrt::hresult_error const& ex)
	{
 
		SocketErrorStatus webErrorStatus{ SocketError::GetStatus(ex.to_abi()) };
		winrt::hstring message = webErrorStatus != SocketErrorStatus::Unknown ?
			winrt::to_hstring((int32_t)webErrorStatus) : winrt::to_hstring(ex.to_abi());
		OutputDebugStringW(L"VideoCameraStreamer::StartServer: Failed to open listener with ");
		OutputDebugStringW(message.c_str());
		OutputDebugStringW(L"\n");

	}
}

IAsyncAction EyeGazeStreamer::InitializeAsync(
	const long long minDelta,
	const SpatialCoordinateSystem& coordSystem,
	std::wstring portName)
{
	OutputDebugStringW(L"EyeStreamer::InitializeAsync: Creating Eye Streamer\n");
	m_worldCoordSystem = coordSystem;
	m_portName = portName;
	m_minDelta = minDelta;

	m_streamingEnabled = true;

	StartEyeStreamServer();
	m_pStreamThread = new std::thread(EyeStreamThread, this);
	//m_OnFrameArrivedRegistration = mediaFrameReader.FrameArrived({ this, &VideoCameraStreamer::OnFrameArrived });
}
 void EyeGazeStreamer::EyeStreamThread(EyeGazeStreamer* pStreamer) {
	 OutputDebugString(L"EyeStreamer::CameraStreamThread: Starting eye streaming thread.\n");
	 while (!pStreamer->m_fExit)
	 {
		 std::lock_guard<std::shared_mutex> reader_guard(pStreamer->m_frameMutex);
		 if (pStreamer->m_latestFrame)
		 {
			 auto frame = pStreamer->m_latestFrame;
			 long long timestamp = pStreamer->m_converter.RelativeTicksToAbsoluteTicks(
				 HundredsOfNanoseconds(frame.SystemRelativeTime().Value().count())).count();

			 if (timestamp != pStreamer->m_latestTimestamp)
			 {
				 long long delta = timestamp - pStreamer->m_latestTimestamp;
				 if (delta > pStreamer->m_minDelta)
				 {
					 pStreamer->m_latestTimestamp = timestamp;
					 pStreamer->SendFrame(frame, timestamp);
					 pStreamer->m_writeInProgress = false;
				 }
			 }
		 }
	 }
}
 void EyeGazeStreamer::SendFrame(
	 MediaFrameReference pFrame, long long pTimestamp)
 {
	 OutputDebugStringW(L"EyeStreamer::SendFrame: Received eye for sending!\n");
	 if (!m_streamSocket || !m_writer)
	 {
		 OutputDebugStringW(
			 L"VideoCameraStreamer::SendFrame: No connection.\n");
	 }
	 if (!m_streamingEnabled)
	 {
		 OutputDebugStringW(L"Streamer::SendFrame: Streaming disabled.\n");
		 return;
	 }
 }

void getEyegaze() {

	HeTHaTEyeFrame frame;

	MixedReality m_mixedReality;
	m_mixedReality.EnableMixedReality();
	m_mixedReality.EnableEyeTracking();
	m_mixedReality.Update();
	const XMVECTOR headPosition = m_mixedReality.GetHeadPosition();
	const XMVECTOR headForward = m_mixedReality.GetHeadForwardDirection();
	const XMVECTOR headUp = m_mixedReality.GetHeadUpDirection();
	frame.timestamp = m_mixedReality.GetPredictedDisplayTime();

	if (m_mixedReality.IsEyeTrackingEnabled() && m_mixedReality.IsEyeTrackingActive())
	{
		frame.eyeGazePresent = true;
		frame.eyeGazeOrigin = m_mixedReality.GetEyeGazeOrigin();
		frame.eyeGazeDirection = m_mixedReality.GetEyeGazeDirection();
		frame.eyeGazeDistance = 0.0f;
		// Use surface mapping to compute the distance the user is looking at
		if (m_mixedReality.IsSurfaceMappingActive())
		{
			float distance;
			XMVECTOR normal;
			if (m_mixedReality.GetSurfaceMappingInterface()->TestRayIntersection(frame.eyeGazeOrigin, frame.eyeGazeDirection, distance, normal))
			{
				frame.eyeGazeDistance = distance;
			}
		}
	}

	//DumpEyeGazeIfPresentElseZero(frame.eyeGazePresent, frame.eyeGazeOrigin, frame.eyeGazeDirection, frame.eyeGazeDistance, file);
}

