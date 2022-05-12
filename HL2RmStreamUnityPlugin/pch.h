#pragma once

#define NOMINMAX

#include "targetver.h"

#include <stdio.h>
#include <wchar.h>
#include <comdef.h>
#include <MemoryBuffer.h>
#include <deque>
#include <codecvt>

#include <.\Generated Files\winrt\base.h>
#include <.\Generated Files\winrt\Windows.Foundation.h>
#include <.\Generated Files\winrt\Windows.Foundation.Collections.h>
#include <.\Generated Files\winrt\Windows.Networking.Sockets.h>
#include <.\Generated Files\winrt\Windows.Storage.Streams.h>
#include <.\Generated Files\winrt\Windows.Perception.Spatial.h>
#include <.\Generated Files\winrt\Windows.Perception.Spatial.Preview.h>
#include <.\Generated Files\winrt\Windows.Media.Capture.Frames.h>
#include <.\Generated Files\winrt\Windows.Media.Devices.Core.h>
#include <.\Generated Files\winrt\Windows.Graphics.Imaging.h>

#include "TimeConverter.h"
#include "ResearchModeApi.h"
#include "IResearchModeFrameSink.h"
#include "ResearchModeFrameProcessor.h"
#include "VideoCameraStreamer.h"
#include "Streamer.h"
#include "EyeGazeHandler.h"
