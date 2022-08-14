from collections import namedtuple, deque
from eye_stream_header import EYE_FRAME_STREAM
from eye_stream_header_recorder_format import RECORDER_EYE_FRAME_STREAM


def transform_eye_frame_to_recorder_frame(eye_frame:EYE_FRAME_STREAM, recorder_eye_frame: RECORDER_EYE_FRAME_STREAM):

    for name in recorder_eye_frame._fields:


