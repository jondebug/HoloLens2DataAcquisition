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
__declspec(align(16))
struct HeTHaTEyeFrame
{
	DirectX::XMMATRIX headTransform;
	DirectX::XMVECTOR eyeGazeOrigin;
	DirectX::XMVECTOR eyeGazeDirection;
	float eyeGazeDistance;
	bool leftHandPresent;
	bool rightHandPresent;
	bool eyeGazePresent;
	long long timestamp;
	std::array<DirectX::XMMATRIX, (size_t)HandJointIndex::Count> leftHandTransform;
	std::array<DirectX::XMMATRIX, (size_t)HandJointIndex::Count> rightHandTransform;
};

class EyeGazeStreamer
{

public:
	EyeGazeStreamer();

	virtual ~EyeGazeStreamer()
	{
		m_fExit = true;
		DumpEyesToFile();
		m_pStreamThread->join();
		m_pEyeCollenctionThread->join();
	}

	const std::vector<HeTHaTEyeFrame>& Log() const;
	size_t FrameCount() const;
	void AddFrame(HeTHaTEyeFrame&& frame);
	void Clear();
	bool DumpToDisk(const winrt::Windows::Storage::StorageFolder& folder, const std::wstring& datetime_path) const;
	bool DumpTransformToDisk(const DirectX::XMMATRIX& mtx, const winrt::Windows::Storage::StorageFolder& folder,
		const std::wstring& datetime_path, const std::wstring& suffix) const;


	winrt::Windows::Foundation::IAsyncAction InitializeAsync(
		const long long minDelta,
		const winrt::Windows::Perception::Spatial::SpatialCoordinateSystem& coordSystem,
		std::wstring portName);

	MixedReality m_mixedReality;
	bool m_fExit = false;

protected:
	//void OnFrameArrived(const winrt::Windows::Media::Capture::Frames::MediaFrameReader& sender,
	//	const winrt::Windows::Media::Capture::Frames::MediaFrameArrivedEventArgs& args);

private:
	HeTHaTEyeFrame ReturnEyeGazeFrame();
	winrt::Windows::Foundation::IAsyncAction StartEyeStreamServer();

	void OnConnectionReceived(
		winrt::Windows::Networking::Sockets::StreamSocketListener /* sender */,
		winrt::Windows::Networking::Sockets::StreamSocketListenerConnectionReceivedEventArgs args);

	winrt::Windows::Foundation::IAsyncAction DumpEyesToFile();
	void SendFrame(HeTHaTEyeFrame pFrame,	long long pTimestamp);
	static void GetAndSendThread(EyeGazeStreamer* pProcessor);

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
	std::thread* m_pEyeCollenctionThread = nullptr;

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
	std::vector<HeTHaTEyeFrame> m_hethateyeLog;
};