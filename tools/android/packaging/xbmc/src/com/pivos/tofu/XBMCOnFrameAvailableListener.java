package com.pivos.tofu;

import android.graphics.SurfaceTexture;
import android.graphics.SurfaceTexture.OnFrameAvailableListener;

public class XBMCOnFrameAvailableListener implements OnFrameAvailableListener
{
  native void _onFrameAvailable(SurfaceTexture surfaceTexture);

  private synchronized void signalNewFrame(SurfaceTexture surfaceTexture)
  {
    _onFrameAvailable(surfaceTexture);
  }

  @Override
  public void onFrameAvailable(SurfaceTexture surfaceTexture)
  {
    signalNewFrame(surfaceTexture);
  }
}
