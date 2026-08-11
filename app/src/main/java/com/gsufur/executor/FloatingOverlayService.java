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
    private EditText scriptEditor;

    @Override
    public void onCreate() {
        super.onCreate();

        try {
            int result = NativeLib.initialize();
            if (result == 0) {
                Toast.makeText(this, "GSUFUR ready!", Toast.LENGTH_SHORT).show();
            }
        } catch (Exception e) {
            Toast.makeText(this, "Init error: " + e.getMessage(), Toast.LENGTH_SHORT).show();
        }

        windowManager = (WindowManager) getSystemService(WINDOW_SERVICE);

        LayoutInflater inflater = (LayoutInflater) getSystemService(LAYOUT_INFLATER_SERVICE);
        overlayView = (LinearLayout) inflater.inflate(R.layout.overlay_layout, null);

        WindowManager.LayoutParams params = new WindowManager.LayoutParams(
                400,
                WindowManager.LayoutParams.WRAP_CONTENT,
                Build.VERSION.SDK_INT >= Build.VERSION_CODES.O ?
                        WindowManager.LayoutParams.TYPE_APPLICATION_OVERLAY :
                        WindowManager.LayoutParams.TYPE_PHONE,
                WindowManager.LayoutParams.FLAG_NOT_TOUCH_MODAL |
                        WindowManager.LayoutParams.FLAG_WATCH_OUTSIDE_TOUCH,
                PixelFormat.TRANSLUCENT
        );
        params.gravity = Gravity.TOP | Gravity.START;
        params.x = 0;
        params.y = 100;

        windowManager.addView(overlayView, params);

        scriptEditor = overlayView.findViewById(R.id.scriptEditor);
        Button executeBtn = overlayView.findViewById(R.id.executeBtn);
        Button clearBtn = overlayView.findViewById(R.id.clearBtn);

        executeBtn.setOnClickListener(v -> {
            String script = scriptEditor.getText().toString();
            if (!script.isEmpty()) {
                int result = NativeLib.executeScript(script);
                Toast.makeText(this, result == 0 ? "✅ Executed!" : "❌ Failed", Toast.LENGTH_SHORT).show();
            } else {
                Toast.makeText(this, "⚠️ Empty", Toast.LENGTH_SHORT).show();
            }
        });

        clearBtn.setOnClickListener(v -> {
            scriptEditor.setText("");
            Toast.makeText(this, "🧹 Cleared", Toast.LENGTH_SHORT).show();
        });

        // Drag support
        overlayView.setOnTouchListener(new View.OnTouchListener() {
            private float startX, startY;
            private int initialX, initialY;

            @Override
            public boolean onTouch(View v, MotionEvent event) {
                switch (event.getAction()) {
                    case MotionEvent.ACTION_DOWN:
                        startX = event.getRawX();
                        startY = event.getRawY();
                        initialX = params.x;
                        initialY = params.y;
                        return true;
                    case MotionEvent.ACTION_MOVE:
                        params.x = initialX + (int)(event.getRawX() - startX);
                        params.y = initialY + (int)(event.getRawY() - startY);
                        windowManager.updateViewLayout(overlayView, params);
                        return true;
                    case MotionEvent.ACTION_UP:
                        return true;
                }
                return false;
            }
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
