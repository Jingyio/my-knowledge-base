using System.Text;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;

namespace uicam
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {
        private WriteableBitmap mPreviewImage;
        private CameraDll.RenderCallback mCB;

        public MainWindow()
        {
            InitializeComponent();
            mCB = DrawPreview;


            CameraDll cameraDll = new CameraDll();
            UInt32 cameraCount = cameraDll.GetCameraCount();

            CameraDll.CameraInfo info = new CameraDll.CameraInfo();
            cameraDll.OpenCamera(0);
            cameraDll.QueryCameraInfo(0, ref info);

            CameraDll.CameraConfig config = new CameraDll.CameraConfig()
            {
                Type = 0,
                FormatIndex = 2,
                Action = 1,
                Callback = mCB,
            };
            cameraDll.ConfigureCamera(0, config);

            //cameraDll.CloseCamera(0);
        }

        public void DrawPreview(IntPtr data, CameraDll.MediaFormat format)
        {
            mPreviewImage = new WriteableBitmap((int)format.Width, (int)format.Height, 96, 96, PixelFormats.Bgr32, null);
            unsafe
            {
                mPreviewImage.Lock();
                IntPtr p = mPreviewImage.BackBuffer;

                Buffer.MemoryCopy(data.ToPointer(), p.ToPointer(), format.Width * format.Height * 4, format.Width * format.Height * 4);
                mPreviewImage.AddDirtyRect(new Int32Rect(0, 0, (int)format.Width, (int)format.Height));
                mPreviewImage.Unlock();
                mPreviewImage.Freeze();
            }

            App.Current.Dispatcher.Invoke(new Action(() =>
            {
                UiPreviewControl.Source = mPreviewImage;
            }));
        }
    }
}