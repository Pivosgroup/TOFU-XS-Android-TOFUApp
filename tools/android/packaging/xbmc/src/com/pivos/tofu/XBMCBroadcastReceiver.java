package com.pivos.tofu;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.util.Log;

public class XBMCBroadcastReceiver extends BroadcastReceiver
{
  native void _onReceive(Intent intent);

  @Override
  public void onReceive(Context context, Intent intent)
  {
    Log.d("XBMCBroadcastReceiver", "Received Intent");
    _onReceive(intent);
  }
}
