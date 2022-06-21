#pragma once
#include "pch.h"
#include "winrt/base.h"
#include <Windows.h>
//#include <winrt/Windows.Foundation.Collections.0.h>
//#include <winrt/Windows.Foundation.h>
#include "winrt/Windows.UI.Input.Spatial.h"
#include "Cannon/MixedReality.h"
#include <mutex>
#include <iostream>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.Web.Syndication.h>
#include "EyeGazeHandler.h"
#include <winrt/Windows.Storage.h>
#include <fstream>
#include <iomanip>


using namespace winrt::Windows::Foundation;
using namespace winrt::Windows::Media::Capture;
using namespace winrt::Windows::Media::Capture::Frames;
using namespace winrt::Windows::Graphics::Imaging;
using namespace winrt::Windows::Perception::Spatial;
using namespace winrt::Windows::Networking::Sockets;
using namespace winrt::Windows::Storage::Streams;
using namespace DirectX;
using namespace winrt::Windows::Storage;
using namespace winrt::Windows::ApplicationModel;



EyeGazeStreamer::EyeGazeStreamer()
{
	// reserve for 10 seconds at 60fps
	m_hethateyeLog.reserve(10 * 60);
}

void EyeGazeStreamer::AddFrame(HeTHaTEyeFrame&& frame)
{
	m_hethateyeLog.push_back(std::move(frame));
}

void EyeGazeStreamer::Clear()
{
	m_hethateyeLog.clear();
}

size_t EyeGazeStreamer::FrameCount() const
{
	return m_hethateyeLog.size();
}

const std::vector<HeTHaTEyeFrame>& EyeGazeStreamer::Log() const
{
	return m_hethateyeLog;
}

std::ostream& operator<<(std::ostream& out, const XMMATRIX& m)
{
	XMFLOAT4X4 mView;
	XMStoreFloat4x4(&mView, m);
	for (int j = 0; j < 4; ++j)
	{
		for (int i = 0; i < 4; ++i)
		{
			out << std::setprecision(8) << mView.m[i][j];
			if (i < 3 || j < 3)
			{
				out << ",";
			}
		}
	}
	return out;
}

std::ostream& operator<<(std::ostream& out, const XMVECTOR& v)
{
	XMFLOAT4 mView;
	XMStoreFloat4(&mView, v);

	out << std::setprecision(8) << mView.x << "," << mView.y << "," << mView.z << "," << mView.w;

	return out;
}

void DumpHandIfPresentElseZero(bool present, const XMMATRIX& handTransform, std::ostream& out)
{
	static const float zeros[16] = { 0.0 };
	static const XMMATRIX zero4x4(zeros);
	if (present)
	{
		out << handTransform;
	}
	else
	{
		out << zero4x4;
	}
}

void DumpEyeGazeIfPresentElseZero(bool present, const XMVECTOR& origin, const XMVECTOR& direction, float distance, std::ostream& out)
{
	XMFLOAT4 zeros{ 0, 0, 0, 0 };
	XMVECTOR zero4 = XMLoadFloat4(&zeros);
	out << present << ",";
	if (present)
	{
		out << origin << "," << direction;
	}
	else
	{
		out << zero4 << "," << zero4;
	}
	out << "," << distance;
}


bool EyeGazeStreamer::DumpTransformToDisk(const XMMATRIX& mtx, const StorageFolder& folder, const std::wstring& datetime_path, const std::wstring& suffix) const
{
	auto path = folder.Path().data();
	std::wstring fullName(path);
	fullName += +L"\\" + datetime_path + suffix;
	std::ofstream file(fullName);
	if (!file)
	{
		return false;
	}
	file << mtx;
	file.close();
	return true;
}
bool EyeGazeStreamer::DumpToDisk(const StorageFolder& folder, const std::wstring& datetime_path) const
{
	auto path = folder.Path().data();
	std::wstring fullName(path);
	fullName += +L"\\" + datetime_path + L"_head_hand_eye.csv";
	std::ofstream file(fullName);
	if (!file)
	{
		return false;
	}

	for (const HeTHaTEyeFrame& frame : m_hethateyeLog)
	{
		file << frame.timestamp << ",";
		file << frame.headTransform;
		file << ",";
		file << frame.leftHandPresent;
		for (int j = 0; j < (int)HandJointIndex::Count; ++j)
		{
			file << ",";
			DumpHandIfPresentElseZero(frame.leftHandPresent, frame.leftHandTransform[j], file);
		}
		file << ",";
		file << frame.rightHandPresent;
		for (int j = 0; j < (int)HandJointIndex::Count; ++j)
		{
			file << ",";
			DumpHandIfPresentElseZero(frame.rightHandPresent, frame.rightHandTransform[j], file);
		}
		file << ",";
		DumpEyeGazeIfPresentElseZero(frame.eyeGazePresent, frame.eyeGazeOrigin, frame.eyeGazeDirection, frame.eyeGazeDistance, file);
		file << std::endl;
	}
	file.close();
	return true;
}
/*
void DumpEyeGazeIfPresentElseZero(bool present, const XMVECTOR& origin, const XMVECTOR& direction, float distance, std::ostream& out)
{
	XMFLOAT4 zeros{ 0, 0, 0, 0 };
	//XMVECTOR zero4 = XMLoadFloat4(&zeros);
	out << present << ",";
	if (present)
	{
		
		out << origin.n128_f32[0] << "," << origin.n128_f32[1] << "," << origin.n128_f32[2] << "," << origin.n128_f32[3]
			<< "," << direction.n128_f32[0] << "," << direction.n128_f32[1] << "," << direction.n128_f32[2] << "," << direction.n128_f32[3];
	}
	else
	{
		out << 0 << "," << 0 << "," << 0 << "," << 0 << "," << 0 << "," << 0 << "," << 0 << "," << 0;
	}
	out << "," << distance; 
}
*/
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
	//########### remove this ##############
	//m_mixedReality.EnableMixedReality();
	//OutputDebugStringW(L"finished enabling mixed reality");
	//m_mixedReality.EnableEyeTracking();
	//OutputDebugStringW(L"finished enable eye tracker");
	//#####################################
	StartEyeStreamServer();
	m_pStreamThread = new std::thread(EyeGazeStreamer::GetAndSendThread, this);
	//m_pStreamThread = new std::thread(EyeStreamThread, this);
	//m_pEyeCollenctionThread  = new std::thread(GetEyeGazeThread, this);
	//m_OnFrameArrivedRegistration = mediaFrameReader.FrameArrived({ this, &VideoCameraStreamer::OnFrameArrived });
}

std::wstring SetDateTimePath()
{
	std::wstring  datetime;
	wchar_t m_datetime_c[200];
	const std::time_t now = std::time(nullptr);
	std::tm tm;
	if (localtime_s(&tm, &now))
	{
		return false;
	}
	if (!std::wcsftime(m_datetime_c, sizeof(m_datetime_c), L"%F-%H%M%S", &tm))
	{
		return false;
	}
	datetime.assign(m_datetime_c);
	return datetime;
}

IAsyncAction EyeGazeStreamer::DumpEyesToFile() {

	StorageFolder localFolder = ApplicationData::Current().LocalFolder();
	auto archiveSourceFolder = co_await localFolder.CreateFolderAsync(
		L"archiveSource",
		CreationCollisionOption::ReplaceExisting);
	if (archiveSourceFolder)
	{
		auto m_archiveFolder = archiveSourceFolder;
		std::wstring datetime = SetDateTimePath();
		DumpToDisk(m_archiveFolder, datetime);
	}
}

void EyeGazeStreamer::GetAndSendThread(EyeGazeStreamer* pProcessor) {
	OutputDebugString(L"EyeStreamer::CameraStreamThread: Starting eye streaming thread.\n");

	//Windows::ApplicationModel::Core::CoreApplication::MainView->CoreWindow->Dispatcher->RunAsync(CoreDispatcherPriority::Normal,
	//ref new Windows::UI::Core::DispatchedHandler([this]()
	//	{
	//		//pProcessor->m_mixedReality.EnableMixedReality();
	//		//pProcessor->m_mixedReality.EnableEyeTracking();

	//	}));





	while (!pProcessor->m_fExit)
	{
		//std::lock_guard<std::shared_mutex> reader_guard(m_frameMutex);

		HeTHaTEyeFrame frame = pProcessor->ReturnEyeGazeFrame();
		long long timestamp = frame.timestamp;
		//long long timestamp = pProcessor->m_latestFrame.timestamp;
		//long long timestamp = pStreamer->m_converter.RelativeTicksToAbsoluteTicks(
		//	 HundredsOfNanoseconds(frame.SystemRelativeTime().Value().count())).count();

		if (timestamp != pProcessor->m_latestTimestamp)
		{
			long long delta = timestamp - pProcessor->m_latestTimestamp;
			if (delta > pProcessor->m_minDelta)
			{
				pProcessor->AddFrame(std::move(frame));
				pProcessor->m_latestTimestamp = timestamp;
				pProcessor->SendFrame(frame, timestamp);
				pProcessor->m_writeInProgress = false;
			}
		}
	}	


}

/*
 void EyeGazeStreamer::EyeStreamThread(EyeGazeStreamer* pStreamer) {
	 OutputDebugString(L"EyeStreamer::CameraStreamThread: Starting eye streaming thread.\n");
	 long EyeFrameCounter = 0;
	 while (!pStreamer->m_fExit)
	 {
		 std::lock_guard<std::shared_mutex> reader_guard(pStreamer->m_frameMutex);
		 if (pStreamer->m_latestFrameExists)
		 {
			 HeTHaTEyeFrame frame = pStreamer->m_latestFrame;
			 long long timestamp = pStreamer->m_latestFrame.timestamp;
			 //long long timestamp = pStreamer->m_converter.RelativeTicksToAbsoluteTicks(
			 //	 HundredsOfNanoseconds(frame.SystemRelativeTime().Value().count())).count();

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
*/
 void EyeGazeStreamer::SendFrame(
	 HeTHaTEyeFrame pFrame, long long pTimestamp)
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
	 auto prevTimestamp = pFrame.timestamp;
	 auto absoluteTimestamp = m_converter.RelativeTicksToAbsoluteTicks(HundredsOfNanoseconds((long long)prevTimestamp)).count();


	 //m_writer.WriteBytes();
	 //std::vector<BYTE> EyeByteData;
	 auto ptr = reinterpret_cast<BYTE*>(&pFrame);
	 auto EyeByteData = std::vector<BYTE>(ptr, ptr + sizeof pFrame);
	 m_writeInProgress = false;

	 try
	 {	 
		 m_writer.WriteBytes(EyeByteData);
		 OutputDebugStringW(L"Streamer::SendFrame: Trying to store writer...\n");
		 m_writer.StoreAsync();
	 }
	 catch (winrt::hresult_error const& ex)
	 {
		 SocketErrorStatus webErrorStatus{ SocketError::GetStatus(ex.to_abi()) };
		 if (webErrorStatus == SocketErrorStatus::ConnectionResetByPeer)
		 {
			 // the client disconnected!
			 m_writer == nullptr;
			 m_streamSocket == nullptr;
			 m_writeInProgress = false;
		 }
		 winrt::hstring message = ex.message();
		 OutputDebugStringW(L"Streamer::SendFrame: Sending failed with ");
		 OutputDebugStringW(message.c_str());
		 OutputDebugStringW(L"\n");
	 }
	 OutputDebugStringW(L"Streamer::SendFrame: Frame sent!\n");
	 //this is how we rebuild the data on the client side: auto p_obj = reinterpret_cast<obj_t*>(&buffer[0]);
 }
 /*
 void GetEyeGazeThread(EyeGazeStreamer Streamer) {

	HeTHaTEyeFrame frame;

	Streamer.m_mixedReality.EnableMixedReality();
	Streamer.m_mixedReality.EnableEyeTracking();
	while (true) {
		Streamer.m_mixedReality.Update();
		const XMVECTOR headPosition = Streamer.m_mixedReality.GetHeadPosition();
		const XMVECTOR headForward = Streamer.m_mixedReality.GetHeadForwardDirection();
		const XMVECTOR headUp = Streamer.m_mixedReality.GetHeadUpDirection();
		frame.timestamp = Streamer.m_mixedReality.GetPredictedDisplayTime();

		if (Streamer.m_mixedReality.IsEyeTrackingEnabled() && Streamer.m_mixedReality.IsEyeTrackingActive())
		{
			frame.eyeGazePresent = true;
			frame.eyeGazeOrigin = Streamer.m_mixedReality.GetEyeGazeOrigin();
			frame.eyeGazeDirection = Streamer.m_mixedReality.GetEyeGazeDirection();
			frame.eyeGazeDistance = 0.0f;
			// Use surface mapping to compute the distance the user is looking at
			if (Streamer.m_mixedReality.IsSurfaceMappingActive())
			{
				float distance;
				XMVECTOR normal;
				if (Streamer.m_mixedReality.GetSurfaceMappingInterface()->TestRayIntersection(frame.eyeGazeOrigin, frame.eyeGazeDirection, distance, normal))
				{
					frame.eyeGazeDistance = distance;
				}
			}
		}
	}
	//DumpEyeGazeIfPresentElseZero(frame.eyeGazePresent, frame.eyeGazeOrigin, frame.eyeGazeDirection, frame.eyeGazeDistance, file);
	//return frame;
}

*/
 HeTHaTEyeFrame EyeGazeStreamer::ReturnEyeGazeFrame() {
	 
	 //float frameDelta = m_frameDeltaTimer.GetTime();
	 //m_frameDeltaTimer.Reset();

	 m_mixedReality.Update();
	 m_hands.UpdateFromMixedReality(m_mixedReality);

	 HeTHaTEyeFrame frame;
	 // Get head transform
	 frame.headTransform = m_hands.GetHeadTransform();
	 // Get hand joints transforms
	 for (int j = 0; j < (int)HandJointIndex::Count; ++j)
	 {
		 frame.leftHandTransform[j] = m_hands.GetOrientedJoint(0, HandJointIndex(j));
	 }

	 // Get timestamp
	 frame.timestamp = m_mixedReality.GetPredictedDisplayTime();

	 // Get eye gaze tracking data
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
	 return frame;
 }

