#include "pch.h"

#define DBG_ENABLE_VERBOSE_LOGGING 0
#define DBG_ENABLE_INFO_LOGGING 1
#define DBG_ENABLE_ERROR_LOGGING 1

using namespace winrt::Windows::Foundation;
using namespace winrt::Windows::Media::Capture;
using namespace winrt::Windows::Media::Capture::Frames;
using namespace winrt::Windows::Graphics::Imaging;
using namespace winrt::Windows::Perception::Spatial;
using namespace winrt::Windows::Networking::Sockets;
using namespace winrt::Windows::Storage::Streams;

const int VideoCameraStreamer::kImageWidth = 960;
const wchar_t VideoCameraStreamer::kSensorName[3] = L"PV";


IAsyncAction VideoCameraStreamer::InitializeAsync(
    const long long minDelta,
    const SpatialCoordinateSystem& coordSystem,
    std::wstring pv_portName/*, std::wstring eyeGaze_portName*/)
{
#if DBG_ENABLE_INFO_LOGGING
    OutputDebugStringW(L"VideoCameraStreamer::InitializeAsync: Creating Streamer for Video Camera. \n");
#endif
    m_worldCoordSystem = coordSystem;
    m_pv_portName = pv_portName;
    //m_eyeGaze_portName = eyeGaze_portName;
    m_minDelta = minDelta;
    //################ remove this ##################
    //m_mixedReality.EnableMixedReality();
    //m_mixedReality.EnableEyeTracking();
    //m_mixedReality.Update();
    //TrackedHands m_hands;
    //m_hands.UpdateFromMixedReality(m_mixedReality);
    //################################################
    m_streamingEnabled = true;

    winrt::Windows::Foundation::Collections::IVectorView<MediaFrameSourceGroup>
        mediaFrameSourceGroups{ co_await MediaFrameSourceGroup::FindAllAsync() };

    MediaFrameSourceGroup selectedSourceGroup = nullptr;
    MediaCaptureVideoProfile profile = nullptr;
    MediaCaptureVideoProfileMediaDescription desc = nullptr;
    std::vector<MediaFrameSourceInfo> selectedSourceInfos;

    // Find MediaFrameSourceGroup
    for (const MediaFrameSourceGroup& mediaFrameSourceGroup : mediaFrameSourceGroups)
    {
        auto knownProfiles = MediaCapture::FindKnownVideoProfiles(
            mediaFrameSourceGroup.Id(),
            KnownVideoProfile::VideoConferencing);

        for (const auto& knownProfile : knownProfiles)
        {
            for (auto knownDesc : knownProfile.SupportedRecordMediaDescription())
            {
#if DBG_ENABLE_VERBOSE_LOGGING
                wchar_t msgBuffer[500];
                swprintf_s(msgBuffer, L"Profile: Frame width = %i, Frame height = %i, Frame rate = %f \n",
                    knownDesc.Width(), knownDesc.Height(), knownDesc.FrameRate());
                OutputDebugStringW(msgBuffer);
#endif
                if ((knownDesc.Width() == kImageWidth)) // && (std::round(knownDesc.FrameRate()) == 15))
                {
                    profile = knownProfile;
                    desc = knownDesc;
                    selectedSourceGroup = mediaFrameSourceGroup;
                    break;
                }
            }
        }
    }

    winrt::check_bool(selectedSourceGroup != nullptr);

    for (auto sourceInfo : selectedSourceGroup.SourceInfos())
    {
        // Workaround since multiple Color sources can be found,
        // and not all of them are necessarily compatible with the selected video profile
        if (sourceInfo.SourceKind() == MediaFrameSourceKind::Color)
        {
            selectedSourceInfos.push_back(sourceInfo);
        }
    }
    winrt::check_bool(!selectedSourceInfos.empty());

    // Initialize a MediaCapture object
    MediaCaptureInitializationSettings settings;
    settings.VideoProfile(profile);
    settings.RecordMediaDescription(desc);
    settings.VideoDeviceId(selectedSourceGroup.Id());
    settings.StreamingCaptureMode(StreamingCaptureMode::Video);
    settings.MemoryPreference(MediaCaptureMemoryPreference::Cpu);
    settings.SharingMode(MediaCaptureSharingMode::ExclusiveControl);
    settings.SourceGroup(selectedSourceGroup);

    MediaCapture mediaCapture = MediaCapture();
    co_await mediaCapture.InitializeAsync(settings);

    MediaFrameSource selectedSource = nullptr;
    MediaFrameFormat preferredFormat = nullptr;

    for (MediaFrameSourceInfo sourceInfo : selectedSourceInfos)
    {
        auto tmpSource = mediaCapture.FrameSources().Lookup(sourceInfo.Id());
        for (MediaFrameFormat format : tmpSource.SupportedFormats())
        {
            if (format.VideoFormat().Width() == kImageWidth)
            {
                selectedSource = tmpSource;
                preferredFormat = format;
                break;
            }
        }
    }

    winrt::check_bool(preferredFormat != nullptr);

    co_await selectedSource.SetFormatAsync(preferredFormat);
    MediaFrameReader mediaFrameReader = co_await mediaCapture.CreateFrameReaderAsync(selectedSource);
    MediaFrameReaderStartStatus status = co_await mediaFrameReader.StartAsync();

    winrt::check_bool(status == MediaFrameReaderStartStatus::Success);

    StartPvServer();
    m_pStreamThread = new std::thread(CameraStreamThread, this);

    //StartEyeStreamServer();
    //m_pEyeCollenctionThread = new std::thread(VideoCameraStreamer::GetAndSendThread, this);

    m_OnFrameArrivedRegistration = mediaFrameReader.FrameArrived({ this, &VideoCameraStreamer::OnFrameArrived });

#if DBG_ENABLE_INFO_LOGGING
    OutputDebugStringW(L"VideoCameraStreamer::InitializeAsync: Done. \n");
#endif
}



void VideoCameraStreamer::OnFrameArrived(
    const MediaFrameReader& sender,
    const MediaFrameArrivedEventArgs& args)
{
    if (MediaFrameReference frame = sender.TryAcquireLatestFrame())
    {
        std::lock_guard<std::shared_mutex> lock(m_PvframeMutex);
        m_latestPvFrame = frame;
#if DBG_ENABLE_VERBOSE_LOGGING
        //OutputDebugStringW(L"VideoCameraStreamer::CameraUpdateThread: Updated frame.\n");
#endif
    }
}

//
//IAsyncAction VideoCameraStreamer::StartEyeStreamServer()
//{
//    try
//    {
//        // The ConnectionReceived event is raised when connections are received.
//        m_EyeGazeStreamSocketListener.ConnectionReceived({ this, &VideoCameraStreamer::OnEyeGazeConnectionReceived });
//
//        // Start listening for incoming TCP connections on the specified port. You can specify any port that's not currently in use.
//        // Every protocol typically has a standard port number. For example, HTTP is typically 80, FTP is 20 and 21, etc.
//        // For this example, we'll choose an arbitrary port number.
//        co_await m_EyeGazeStreamSocketListener.BindServiceNameAsync(m_eyeGaze_portName);
//        //m_streamSocketListener.Control().KeepAlive(true);
//
//        wchar_t msgBuffer[200];
//        swprintf_s(msgBuffer, L"EyeGazeStreamer::StartServer: Server is listening at %ls \n",
//            m_eyeGaze_portName.c_str());
//        OutputDebugStringW(msgBuffer);
//    }
//    catch (winrt::hresult_error const& ex)
//    {
//
//        SocketErrorStatus webErrorStatus{ SocketError::GetStatus(ex.to_abi()) };
//        winrt::hstring message = webErrorStatus != SocketErrorStatus::Unknown ?
//            winrt::to_hstring((int32_t)webErrorStatus) : winrt::to_hstring(ex.to_abi());
//        OutputDebugStringW(L"VideoCameraStreamer::StartServer: Failed to open listener with ");
//        OutputDebugStringW(message.c_str());
//        OutputDebugStringW(L"\n");
//
//    }
//}
//
//



IAsyncAction VideoCameraStreamer::StartPvServer()
{
    try
    {
        // The ConnectionReceived event is raised when connections are received.
        m_PvStreamSocketListener.ConnectionReceived({ this, &VideoCameraStreamer::OnPvConnectionReceived });

        // Start listening for incoming TCP connections on the specified port. You can specify any port that's not currently in use.
        // Every protocol typically has a standard port number. For example, HTTP is typically 80, FTP is 20 and 21, etc.
        // For this example, we'll choose an arbitrary port number.
        co_await m_PvStreamSocketListener.BindServiceNameAsync(m_pv_portName);
        //m_streamSocketListener.Control().KeepAlive(true);

#if DBG_ENABLE_INFO_LOGGING       
        wchar_t msgBuffer[200];
        swprintf_s(msgBuffer, L"VideoCameraStreamer::StartServer: Server is listening at %ls \n",
            m_pv_portName.c_str());
        OutputDebugStringW(msgBuffer);
#endif
    }
    catch (winrt::hresult_error const& ex)
    {
#if DBG_ENABLE_ERROR_LOGGING
        SocketErrorStatus webErrorStatus{ SocketError::GetStatus(ex.to_abi()) };
        winrt::hstring message = webErrorStatus != SocketErrorStatus::Unknown ?
            winrt::to_hstring((int32_t)webErrorStatus) : winrt::to_hstring(ex.to_abi());
        OutputDebugStringW(L"VideoCameraStreamer::StartServer: Failed to open listener with ");
        OutputDebugStringW(message.c_str());
        OutputDebugStringW(L"\n");
#endif
    }
}

void VideoCameraStreamer::OnPvConnectionReceived(
    StreamSocketListener /* sender */,
    StreamSocketListenerConnectionReceivedEventArgs args)
{
    try
    {
        m_PvStreamSocket = args.Socket();
        m_writer = (winrt::Windows::Storage::Streams::DataWriter)args.Socket().OutputStream();
        m_writer.UnicodeEncoding(UnicodeEncoding::Utf8);
        m_writer.ByteOrder(ByteOrder::LittleEndian);

        m_writeInProgress = false;
#if DBG_ENABLE_INFO_LOGGING
        OutputDebugStringW(L"VideoCameraStreamer::OnConnectionReceived: Received connection! \n");
#endif
    }
    catch (winrt::hresult_error const& ex)
    {
#if DBG_ENABLE_ERROR_LOGGING
        SocketErrorStatus webErrorStatus{ SocketError::GetStatus(ex.to_abi()) };
        winrt::hstring message = webErrorStatus != SocketErrorStatus::Unknown ?
            winrt::to_hstring((int32_t)webErrorStatus) : winrt::to_hstring(ex.to_abi());
        OutputDebugStringW(L"VideoCameraStreamer::StartServer: Failed to open listener with ");
        OutputDebugStringW(message.c_str());
        OutputDebugStringW(L"\n");
#endif
    }
}
//
//void VideoCameraStreamer::OnEyeGazeConnectionReceived(
//    StreamSocketListener /* sender */,
//    StreamSocketListenerConnectionReceivedEventArgs args)
//{
//    try
//    {
//        m_EyeGazeStreamSocket = args.Socket();
//        m_writer = (winrt::Windows::Storage::Streams::DataWriter)args.Socket().OutputStream();
//        m_writer.UnicodeEncoding(UnicodeEncoding::Utf8);
//        m_writer.ByteOrder(ByteOrder::LittleEndian);
//
//        m_writeInProgress = false;
//#if DBG_ENABLE_INFO_LOGGING
//        OutputDebugStringW(L"VideoCameraStreamer::OnConnectionReceived: Received connection! \n");
//#endif
//    }
//    catch (winrt::hresult_error const& ex)
//    {
//#if DBG_ENABLE_ERROR_LOGGING
//        SocketErrorStatus webErrorStatus{ SocketError::GetStatus(ex.to_abi()) };
//        winrt::hstring message = webErrorStatus != SocketErrorStatus::Unknown ?
//            winrt::to_hstring((int32_t)webErrorStatus) : winrt::to_hstring(ex.to_abi());
//        OutputDebugStringW(L"VideoCameraStreamer::StartServer: Failed to open listener with ");
//        OutputDebugStringW(message.c_str());
//        OutputDebugStringW(L"\n");
//#endif
//    }
//}
//
//HeTHaTEyeFrame VideoCameraStreamer::ReturnEyeGazeFrame() {
//
//    //float frameDelta = m_frameDeltaTimer.GetTime();
//    //m_frameDeltaTimer.Reset();
//
//    m_mixedReality.Update();
//    m_hands.UpdateFromMixedReality(m_mixedReality);
//
//    HeTHaTEyeFrame frame;
//    // Get head transform
//    frame.headTransform = m_hands.GetHeadTransform();
//    // Get hand joints transforms
//    for (int j = 0; j < (int)HandJointIndex::Count; ++j)
//    {
//        frame.leftHandTransform[j] = m_hands.GetOrientedJoint(0, HandJointIndex(j));
//    }
//
//    // Get timestamp
//    frame.timestamp = m_mixedReality.GetPredictedDisplayTime();
//
//    // Get eye gaze tracking data
//    if (m_mixedReality.IsEyeTrackingEnabled() && m_mixedReality.IsEyeTrackingActive())
//    {
//        frame.eyeGazePresent = true;
//        frame.eyeGazeOrigin = m_mixedReality.GetEyeGazeOrigin();
//        frame.eyeGazeDirection = m_mixedReality.GetEyeGazeDirection();
//        frame.eyeGazeDistance = 0.0f;
//        // Use surface mapping to compute the distance the user is looking at
//        if (m_mixedReality.IsSurfaceMappingActive())
//        {
//            float distance;
//            XMVECTOR normal;
//            if (m_mixedReality.GetSurfaceMappingInterface()->TestRayIntersection(frame.eyeGazeOrigin, frame.eyeGazeDirection, distance, normal))
//            {
//                frame.eyeGazeDistance = distance;
//            }
//        }
//    }
//    return frame;
//}
//void VideoCameraStreamer::AddFrame(HeTHaTEyeFrame&& frame)
//{
//    m_hethateyeLog.push_back(std::move(frame));
//}
//void VideoCameraStreamer::GetAndSendThread(VideoCameraStreamer* pProcessor) {
//    OutputDebugString(L"EyeStreamer::EyeStreamThread: Starting eye streaming thread.\n");
//
//    pProcessor->m_mixedReality.EnableMixedReality();
//    OutputDebugString(L"EyeStreamer::EyeStreamThread: Enabled mixed reality.\n");
//
//    pProcessor->m_mixedReality.EnableEyeTracking();
//    while (!pProcessor->m_fExit)
//    {
//        //std::lock_guard<std::shared_mutex> reader_guard(m_frameMutex);
//
//        HeTHaTEyeFrame frame = pProcessor->ReturnEyeGazeFrame();
//        long long timestamp = frame.timestamp;
//        //long long timestamp = pProcessor->m_latestFrame.timestamp;
//        //long long timestamp = pStreamer->m_converter.RelativeTicksToAbsoluteTicks(
//        //	 HundredsOfNanoseconds(frame.SystemRelativeTime().Value().count())).count();
//
//        if (timestamp != pProcessor->m_latestEyeGazeTimestamp)
//        {
//            long long delta = timestamp - pProcessor->m_latestEyeGazeTimestamp;
//            if (delta > pProcessor->m_minDelta)
//            {
//                pProcessor->AddFrame(std::move(frame));
//                pProcessor->m_latestEyeGazeTimestamp = timestamp;
//                pProcessor->SendEyeGazeFrame(frame, timestamp);
//                pProcessor->m_writeInProgress = false;
//            }
//        }
//    }
//
//
//}

void VideoCameraStreamer::CameraStreamThread(VideoCameraStreamer* pStreamer)
{
#if DBG_ENABLE_INFO_LOGGING
    OutputDebugString(L"VideoCameraStreamer::CameraStreamThread: Starting streaming thread.\n");
#endif
    while (!pStreamer->m_fExit)
    {
        //std::lock_guard<std::shared_mutex> reader_guard(pStreamer->m_EyeGazeframeMutex);
        std::lock_guard<std::shared_mutex> reader_guard(pStreamer->m_PvframeMutex);
        if (pStreamer->m_latestPvFrame)
        {
            MediaFrameReference frame = pStreamer->m_latestPvFrame;
            long long timestamp = pStreamer->m_converter.RelativeTicksToAbsoluteTicks(
                HundredsOfNanoseconds(frame.SystemRelativeTime().Value().count())).count();
            if (timestamp != pStreamer->m_latestPvTimestamp)
            {
                long long delta = timestamp - pStreamer->m_latestPvTimestamp;
                if (delta > pStreamer->m_minDelta)
                {
                    pStreamer->m_latestPvTimestamp = timestamp;
                    pStreamer->SendPvFrame(frame, timestamp);
                    pStreamer->m_writeInProgress = false;
                }
            }
        }
    }
}

void VideoCameraStreamer::SendPvFrame(
    MediaFrameReference pFrame,
    long long pTimestamp)
{
#if DBG_ENABLE_INFO_LOGGING
    OutputDebugStringW(L"VideoCameraStreamer::SendFrame: Received frame for sending!\n");
#endif
    if (!m_PvStreamSocket || !m_writer)
    {
#if DBG_ENABLE_VERBOSE_LOGGING
        OutputDebugStringW(
            L"VideoCameraStreamer::SendFrame: No connection.\n");
#endif
        return;
    }
    if (!m_streamingEnabled)
    {
#if DBG_ENABLE_VERBOSE_LOGGING
        OutputDebugStringW(L"Streamer::SendFrame: Streaming disabled.\n");
#endif
        return;
    }

    // grab the frame info
    float fx = pFrame.VideoMediaFrame().CameraIntrinsics().FocalLength().x;
    float fy = pFrame.VideoMediaFrame().CameraIntrinsics().FocalLength().y;
    
    float ox = pFrame.VideoMediaFrame().CameraIntrinsics().PrincipalPoint().x;
    float oy = pFrame.VideoMediaFrame().CameraIntrinsics().PrincipalPoint().y;

    winrt::Windows::Foundation::Numerics::float4x4 PVtoWorldtransform;
    auto PVtoWorld =
        m_latestPvFrame.CoordinateSystem().TryGetTransformTo(m_worldCoordSystem);
    if (PVtoWorld)
    {
        PVtoWorldtransform = PVtoWorld.Value();
    }

    // grab the frame data
    SoftwareBitmap softwareBitmap = SoftwareBitmap::Convert(
        pFrame.VideoMediaFrame().SoftwareBitmap(), BitmapPixelFormat::Bgra8);

    int imageWidth = softwareBitmap.PixelWidth();
    int imageHeight = softwareBitmap.PixelHeight();

    int pixelStride = 4;
    int scaleFactor = 1;

    int rowStride = imageWidth * pixelStride;

    // Get bitmap buffer object of the frame
    BitmapBuffer bitmapBuffer = softwareBitmap.LockBuffer(BitmapBufferAccessMode::Read);

    // Get raw pointer to the buffer object
    uint32_t pixelBufferDataLength = 0;
    uint8_t* pixelBufferData;

    auto spMemoryBufferByteAccess{ bitmapBuffer.CreateReference()
        .as<::Windows::Foundation::IMemoryBufferByteAccess>() };

    try
    {
        spMemoryBufferByteAccess->
            GetBuffer(&pixelBufferData, &pixelBufferDataLength);
    }
    catch (winrt::hresult_error const& ex)
    {
#if DBG_ENABLE_ERROR_LOGGING
        winrt::hresult hr = ex.code(); // HRESULT_FROM_WIN32
        winrt::hstring message = ex.message();
        OutputDebugStringW(L"VideoCameraStreamer::SendFrame: Failed to get buffer with ");
        OutputDebugStringW(message.c_str());
        OutputDebugStringW(L"\n");
#endif
    }

    std::vector<uint8_t> imageBufferAsVector;
    for (int row = 0; row < imageHeight; row += scaleFactor)
    {
        for (int col = 0; col < rowStride; col += scaleFactor * pixelStride)
        {
            for (int j = 0; j < pixelStride - 1; j++)
            {
                imageBufferAsVector.emplace_back(
                    pixelBufferData[row * rowStride + col + j]);
            }
        }
    }


    if (m_writeInProgress)
    {
#if DBG_ENABLE_VERBOSE_LOGGING
        OutputDebugStringW(
            L"VideoCameraStreamer::SendFrame: Write in progress.\n");
#endif
        return;
    }
    m_writeInProgress = true;
    try
    {
        int outImageWidth = imageWidth / scaleFactor;
        int outImageHeight = imageHeight / scaleFactor;

        // pixel stride is reduced by 1 since we skip alpha channel
        int outPixelStride = pixelStride - 1;
        int outRowStride = outImageWidth * outPixelStride;


        // Write header
        m_writer.WriteUInt64(pTimestamp);
        m_writer.WriteInt32(outImageWidth);
        m_writer.WriteInt32(outImageHeight);
        m_writer.WriteInt32(outPixelStride);
        m_writer.WriteInt32(outRowStride);
        m_writer.WriteSingle(fx);
        m_writer.WriteSingle(fy);
        m_writer.WriteSingle(ox);
        m_writer.WriteSingle(oy);

        WriteMatrix4x4(PVtoWorldtransform);

        m_writer.WriteBytes(imageBufferAsVector);

#if DBG_ENABLE_VERBOSE_LOGGING
        OutputDebugStringW(L"VideoCameraStreamer::SendFrame: Trying to store writer...\n");
#endif
        m_writer.StoreAsync();
    }
    catch (winrt::hresult_error const& ex)
    {
        SocketErrorStatus webErrorStatus{ SocketError::GetStatus(ex.to_abi()) };
        if (webErrorStatus == SocketErrorStatus::ConnectionResetByPeer)
        {
            // the client disconnected!
            m_writer == nullptr;
            m_PvStreamSocket == nullptr;
            m_writeInProgress = false;
        }
#if DBG_ENABLE_ERROR_LOGGING
        winrt::hstring message = ex.message();
        OutputDebugStringW(L"RMCameraStreamer::SendFrame: Sending failed with ");
        OutputDebugStringW(message.c_str());
        OutputDebugStringW(L"\n");
#endif // DBG_ENABLE_ERROR_LOGGING
    }

    m_writeInProgress = false;

#if DBG_ENABLE_VERBOSE_LOGGING
    OutputDebugStringW(
        L"VideoCameraStreamer::SendFrame: Frame sent!\n");
#endif

}
//
//void VideoCameraStreamer::SendEyeGazeFrame(
//    HeTHaTEyeFrame pFrame, long long pTimestamp)
//{
//    OutputDebugStringW(L"EyeStreamer::SendFrame: Received eye for sending!\n");
//    if (!m_EyeGazeStreamSocket || !m_writer)
//    {
//        OutputDebugStringW(
//            L"VideoCameraStreamer::SendFrame: No connection.\n");
//    }
//    if (!m_streamingEnabled)
//    {
//        OutputDebugStringW(L"Streamer::SendFrame: Streaming disabled.\n");
//        return;
//    }
//    auto prevTimestamp = pFrame.timestamp;
//    auto absoluteTimestamp = m_converter.RelativeTicksToAbsoluteTicks(HundredsOfNanoseconds((long long)prevTimestamp)).count();
//
//
//    //m_writer.WriteBytes();
//    //std::vector<BYTE> EyeByteData;
//    auto ptr = reinterpret_cast<BYTE*>(&pFrame);
//    auto EyeByteData = std::vector<BYTE>(ptr, ptr + sizeof pFrame);
//    m_writeInProgress = false;
//
//    try
//    {
//        m_writer.WriteBytes(EyeByteData);
//        OutputDebugStringW(L"Streamer::SendFrame: Trying to store writer...\n");
//        m_writer.StoreAsync();
//    }
//    catch (winrt::hresult_error const& ex)
//    {
//        SocketErrorStatus webErrorStatus{ SocketError::GetStatus(ex.to_abi()) };
//        if (webErrorStatus == SocketErrorStatus::ConnectionResetByPeer)
//        {
//            // the client disconnected!
//            m_writer == nullptr;
//            m_EyeGazeStreamSocket == nullptr;
//            m_writeInProgress = false;
//        }
//        winrt::hstring message = ex.message();
//        OutputDebugStringW(L"Streamer::SendFrame: Sending failed with ");
//        OutputDebugStringW(message.c_str());
//        OutputDebugStringW(L"\n");
//    }
//    OutputDebugStringW(L"Streamer::SendFrame: Frame sent!\n");
//    //this is how we rebuild the data on the client side: auto p_obj = reinterpret_cast<obj_t*>(&buffer[0]);
//}




void VideoCameraStreamer::StreamingToggle()
{
#if DBG_ENABLE_INFO_LOGGING
    OutputDebugStringW(L"VideoCameraStreamer::StreamingToggle: Received!\n");
#endif
    if (m_streamingEnabled)
    {
        m_streamingEnabled = false;
    }
    else if (!m_streamingEnabled)
    {
        m_streamingEnabled = true;
    }
#if DBG_ENABLE_INFO_LOGGING
    OutputDebugStringW(L"VideoCameraStreamer::StreamingToggle: Done!\n");
#endif
}

void VideoCameraStreamer::WriteMatrix4x4(
    _In_ winrt::Windows::Foundation::Numerics::float4x4 matrix)
{
    m_writer.WriteSingle(matrix.m11);
    m_writer.WriteSingle(matrix.m12);
    m_writer.WriteSingle(matrix.m13);
    m_writer.WriteSingle(matrix.m14);

    m_writer.WriteSingle(matrix.m21);
    m_writer.WriteSingle(matrix.m22);
    m_writer.WriteSingle(matrix.m23);
    m_writer.WriteSingle(matrix.m24);

    m_writer.WriteSingle(matrix.m31);
    m_writer.WriteSingle(matrix.m32);
    m_writer.WriteSingle(matrix.m33);
    m_writer.WriteSingle(matrix.m34);

    m_writer.WriteSingle(matrix.m41);
    m_writer.WriteSingle(matrix.m42);
    m_writer.WriteSingle(matrix.m43);
    m_writer.WriteSingle(matrix.m44);
}
