package com.gsufur.executor;

import android.app.Service;
import android.content.Intent;
import android.graphics.PixelFormat;
import android.os.Build;
import android.os.IBinder;
import android.view.Gravity;
import android.view.LayoutInflater;
import android.view.MotionEvent;
import android.view.View;
import android.view.WindowManager;
import android.widget.Button;
import android.widget.EditText;
import android.widget.LinearLayout;
import android.widget.Toast;

public class FloatingOverlayService extends Service {

    private WindowManager windowManager;
    private LinearLayout overlayView;

    @Override
    public void onCreate() {
        super.onCreate();

        windowManager = (WindowManager) getSystemService(WINDOW_SERVICE);

        LayoutInflater inflater = (LayoutInflater) getSystemService(LAYOUT_INFLATER_SERVICE);
        overlayView = (LinearLayout) inflater.inflate(R.layout.overlay_layout, null);

        WindowManager.LayoutParams params = new WindowManager.LayoutParams(
                WindowManager.LayoutParams.WRAP_CONTENT,
                WindowManager.LayoutParams.WRAP_CONTENT,
                Build.VERSION.SDK_INT >= Build.VERSION_CODES.O ?
                        WindowManager.LayoutParams.TYPE_APPLICATION_OVERLAY :
                        WindowManager.LayoutParams.TYPE_PHONE,
                WindowManager.LayoutParams.FLAG_NOT_FOCUSABLE,
                PixelFormat.TRANSLUCENT
        );
        params.gravity = Gravity.TOP | Gravity.START;
        params.x = 0;
        params.y = 100;

        windowManager.addView(overlayView, params);

        EditText scriptEditor = overlayView.findViewById(R.id.scriptEditor);
        Button executeBtn = overlayView.findViewById(R.id.executeBtn);
        Button clearBtn = overlayView.findViewById(R.id.clearBtn);

        // Make EditText work
        scriptEditor.setOnTouchListener((v, event) -> {
            if (event.getAction() == MotionEvent.ACTION_UP) {
                v.performClick();
                v.requestFocus();
                return true;
            }
            return false;
        });

        scriptEditor.requestFocus();

        executeBtn.setOnClickListener(v -> {
            String script = scriptEditor.getText().toString();
            if (!script.isEmpty()) {
                Toast.makeText(this, "Script Executed!", Toast.LENGTH_SHORT).show();
            } else {
                Toast.makeText(this, "Script is empty", Toast.LENGTH_SHORT).show();
            }
        });

        clearBtn.setOnClickListener(v -> {
            scriptEditor.setText("");
            Toast.makeText(this, "Cleared", Toast.LENGTH_SHORT).show();
        });
    }

    @Override
    public IBinder onBind(Intent intent) {
        return null;
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
        if (overlayView != null && windowManager != null) {
            windowManager.removeView(overlayView);
        }
    }
}
