using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace uicam
{
    public class CameraDll
    {
        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        public delegate void RenderCallback(IntPtr data, MediaFormat format);

        [StructLayout(LayoutKind.Sequential)]
        public struct CameraConfig
        {
            public UInt64 Type;
            public UInt64 FormatIndex;
            public UInt64 Action;
            public RenderCallback Callback;
            public IntPtr FilePath;

            public CameraConfig(UInt64 type, UInt64 formatIndex, UInt64 action, RenderCallback cb, IntPtr path)
            {
                Type = type;
                FormatIndex = formatIndex;
                Action = action;
                Callback = cb;
                FilePath = path;
            }
        }

        [StructLayout(LayoutKind.Sequential)]
        public struct MediaFormat
        {
            public UInt64 Format;
            public UInt64 Width;
            public UInt64 Height;
            public Int64  Stride;
            public UInt64 FrameNumerator;
            public UInt64 FrameDenominator;

            public MediaFormat(UInt64 format, UInt64 width, UInt64 height, Int64 stride, UInt64 frameNum, UInt64 frameDen)
            {
                Format = format;
                Width = width;
                Height = height;
                Stride = stride;
                FrameNumerator = frameNum;
                FrameDenominator = frameDen;
            }
        }

        [StructLayout(LayoutKind.Sequential)]
        public struct CameraInfo
        {
            [MarshalAs(UnmanagedType.ByValArray, SizeConst = 128)]
            public MediaFormat[] PreviewFormatArray;

            [MarshalAs(UnmanagedType.ByValArray, SizeConst = 128)]
            public MediaFormat[] RecordFormatArray;

            [MarshalAs(UnmanagedType.ByValArray, SizeConst = 128)]
            public MediaFormat[] PhotoFormatArray;

            UInt64 PreviewFormatCount;
            UInt64 RecordFormatCount;
            UInt64 PhotoFormatCount;

            MediaFormat CurrentPreviewFormat;
            MediaFormat CurrentRecordFormat;
            MediaFormat CurrentPhotoFormat;
        }

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        public delegate void DrawPreview(IntPtr data, uint width, uint height);

        [DllImport("libcamcore.dll", EntryPoint = "GetCameraCount", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern UInt64 DllGetCameraCount(ref UInt32 pCount);

        [DllImport("libcamcore.dll", EntryPoint = "OpenCamera", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern UInt64 DllOpenCamera(UInt32 cameraIndex, ref IntPtr pHandle);

        [DllImport("libcamcore.dll", EntryPoint = "CloseCamera", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern UInt64 DllCloseCamera(IntPtr handle);

        [DllImport("libcamcore.dll", EntryPoint = "ConfigureCamera", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern UInt64 DllConfigureCamera(IntPtr handle, CameraConfig cameraConfig);

        [DllImport("libcamcore.dll", EntryPoint = "QueryCameraInfo", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern UInt64 DllQueryCameraInfo(IntPtr handle, ref CameraInfo pCameraInfo);


        private Dictionary<UInt32, IntPtr> mCameraHandleMapper;

        public CameraDll()
        {
            mCameraHandleMapper = new Dictionary<UInt32, IntPtr>();
        }

        public UInt32 GetCameraCount()
        {
            UInt32 count = 0;
            if (DllGetCameraCount(ref count) == 0)
                return count;
            else
                return 0;
        }

        public UInt64 OpenCamera(UInt32 cameraIndex)
        {
            IntPtr handle = IntPtr.Zero;
            UInt64 res = DllOpenCamera(cameraIndex, ref handle);

            if (res == 0)
                mCameraHandleMapper.Add(cameraIndex, handle);

            return res;
        }

        public UInt64 CloseCamera(UInt32 cameraIndex)
        {
            IntPtr handle = IntPtr.Zero;

            if (mCameraHandleMapper.TryGetValue(cameraIndex, out handle) == true)
            {
                mCameraHandleMapper.Remove(cameraIndex);
                return DllCloseCamera(handle);
            }

            return 0;
        }

        public UInt64 ConfigureCamera(UInt32 cameraIndex, CameraConfig config)
        {
            IntPtr handle = IntPtr.Zero;

            if (mCameraHandleMapper.TryGetValue(cameraIndex, out handle) == false)
                return 4;

            return DllConfigureCamera(handle, config);
        }

        public UInt64 QueryCameraInfo(UInt32 cameraIndex, ref CameraInfo info)
        {
            IntPtr handle = IntPtr.Zero;

            if (mCameraHandleMapper.TryGetValue(cameraIndex, out handle) == false)
                return 4;

            return DllQueryCameraInfo(handle, ref info);
        }
    }
}
