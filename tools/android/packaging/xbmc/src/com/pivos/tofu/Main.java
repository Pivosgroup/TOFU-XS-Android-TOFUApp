package com.pivos.tofu;

import android.app.NativeActivity;
import android.content.Intent;
import android.os.Bundle;
import android.view.View;
import android.view.View.OnSystemUiVisibilityChangeListener;

public class Main extends NativeActivity implements View.OnSystemUiVisibilityChangeListener
{
  native void _onSystemUiVisibilityChange(int visibility);
  native void _onNewIntent(Intent intent);
  public Main() 
  {
    super();
  }

  @Override
  public void onCreate(Bundle savedInstanceState) 
  {
    super.onCreate(savedInstanceState);
    getWindow().getDecorView().setOnSystemUiVisibilityChangeListener(this);
  }

  @Override
  protected void onNewIntent(Intent intent)
  {
    super.onNewIntent(intent);
    _onNewIntent(intent);
  }

  @Override
  public void onSystemUiVisibilityChange(int visibility)
  {
    _onSystemUiVisibilityChange(visibility);
  }

}
