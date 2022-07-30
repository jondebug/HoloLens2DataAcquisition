#pragma once
#include <mutex>
#include <shared_mutex>
#include "Cannon/MixedReality.h"
#include "Cannon/TrackedHands.h"
#include "EyeGazeHandler.h"

class VideoCameraStreamer
{
public:
    VideoCameraStreamer()
    {
    }

    virtual ~VideoCameraStreamer()
    {
        m_fExit = true;
        m_pStreamThread->join();
        //m_pEyeCollenctionThread -> join();

    }

    winrt::Windows::Foundation::IAsyncAction InitializeAsync(
        const long long minDelta,
        const winrt::Windows::Perception::Spatial::SpatialCoordinateSystem& coordSystem,
        std::wstring portName/*, std::wstring eyeGaze_portName*/);

    //#######################################
    //eye gaze members
    //#######################################
    void StreamingToggle();
    MixedReality m_mixedReality;
    //const std::vector<HeTHaTEyeFrame>& Log() const;
    //size_t FrameCount() const;
    //void AddFrame(HeTHaTEyeFrame&& frame);
    //bool DumpToDisk(const winrt::Windows::Storage::StorageFolder& folder, const std::wstring& datetime_path) const;
    //bool DumpTransformToDisk(const DirectX::XMMATRIX& mtx, const winrt::Windows::Storage::StorageFolder& folder,
    //    const std::wstring& datetime_path, const std::wstring& suffix) const;
    //void SendEyeGazeFrame(HeTHaTEyeFrame pFrame, long long pTimestamp);
    //TrackedHands m_hands;
    //std::shared_mutex m_EyeGazeframeMutex;
    //long long m_latestEyeGazeTimestamp = 0;
    //HeTHaTEyeFrame m_latestEyeGazeFrame;
    //bool m_latestEyeGazeFrameExists = true;
    //std::thread* m_pEyeCollenctionThread = nullptr;
    //winrt::Windows::Networking::Sockets::StreamSocketListener m_EyeGazeStreamSocketListener;
    //winrt::Windows::Networking::Sockets::StreamSocket m_EyeGazeStreamSocket = nullptr;
    //std::vector<HeTHaTEyeFrame> m_hethateyeLog;
    //void OnEyeGazeConnectionReceived(
    //    winrt::Windows::Networking::Sockets::StreamSocketListener /* sender */,
    //    winrt::Windows::Networking::Sockets::StreamSocketListenerConnectionReceivedEventArgs args);


protected:
    void OnFrameArrived(const winrt::Windows::Media::Capture::Frames::MediaFrameReader& sender,
        const winrt::Windows::Media::Capture::Frames::MediaFrameArrivedEventArgs& args);

private:
    winrt::Windows::Foundation::IAsyncAction StartPvServer();
    //winrt::Windows::Foundation::IAsyncAction StartEyeStreamServer();
    void OnPvConnectionReceived(
        winrt::Windows::Networking::Sockets::StreamSocketListener /* sender */,
        winrt::Windows::Networking::Sockets::StreamSocketListenerConnectionReceivedEventArgs args);

    static void CameraStreamThread(VideoCameraStreamer* pProcessor);
    //static void GetAndSendThread(VideoCameraStreamer* pProcessor);
    //HeTHaTEyeFrame ReturnEyeGazeFrame();

    void SendPvFrame(
        winrt::Windows::Media::Capture::Frames::MediaFrameReference pFrame,
        long long pTimestamp);

    void WriteMatrix4x4(
        _In_ winrt::Windows::Foundation::Numerics::float4x4 matrix);

    std::shared_mutex m_PvframeMutex;
    long long m_latestPvTimestamp = 0;
    winrt::Windows::Media::Capture::Frames::MediaFrameReference m_latestPvFrame = nullptr;

    winrt::Windows::Media::Capture::Frames::MediaFrameReader m_mediaFrameReader = nullptr;
    winrt::event_token m_OnFrameArrivedRegistration;

    // streaming thread
    std::thread* m_pStreamThread = nullptr;
    bool m_fExit = false;

    bool m_streamingEnabled = true;

    TimeConverter m_converter;
    winrt::Windows::Perception::Spatial::SpatialCoordinateSystem m_worldCoordSystem = nullptr;

    winrt::Windows::Networking::Sockets::StreamSocketListener m_PvStreamSocketListener;
    winrt::Windows::Networking::Sockets::StreamSocket m_PvStreamSocket = nullptr;
    winrt::Windows::Storage::Streams::DataWriter m_writer;
    bool m_writeInProgress = false;

    std::wstring m_pv_portName;
    //std::wstring m_eyeGaze_portName;
    // minDelta allows to enforce a certain time delay between frames
    // should be set in hundreds of nanoseconds (ms * 1e-4)
    long long m_minDelta;

    static const int kImageWidth;
    static const wchar_t kSensorName[3];
    
};

