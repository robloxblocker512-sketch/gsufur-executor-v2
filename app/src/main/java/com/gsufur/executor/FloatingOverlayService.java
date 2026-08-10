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
    private float initialTouchX, initialTouchY;
    private int initialWindowX, initialWindowY;
    private boolean isDragging = false;

    @Override
    public void onCreate() {
        super.onCreate();

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

        // Drag support
        overlayView.setOnTouchListener((v, event) -> {
            switch (event.getAction()) {
                case MotionEvent.ACTION_DOWN:
                    initialTouchX = event.getRawX();
                    initialTouchY = event.getRawY();
                    initialWindowX = params.x;
                    initialWindowY = params.y;
                    isDragging = false;
                    return true;
                case MotionEvent.ACTION_MOVE:
                    float deltaX = event.getRawX() - initialTouchX;
                    float deltaY = event.getRawY() - initialTouchY;
                    if (Math.abs(deltaX) > 10 || Math.abs(deltaY) > 10) {
                        isDragging = true;
                    }
                    if (isDragging) {
                        params.x = initialWindowX + (int) deltaX;
                        params.y = initialWindowY + (int) deltaY;
                        windowManager.updateViewLayout(overlayView, params);
                    }
                    return true;
                case MotionEvent.ACTION_UP:
                    if (!isDragging) {
                        // It was a click, not a drag — focus the editor
                        scriptEditor.requestFocus();
                    }
                    return true;
            }
            return false;
        });

        // Focus editor on click
        scriptEditor.setOnClickListener(v -> {
            v.requestFocus();
        });

        executeBtn.setOnClickListener(v -> {
            String script = scriptEditor.getText().toString();
            if (!script.isEmpty()) {
                Toast.makeText(this, "✅ Script sent!", Toast.LENGTH_SHORT).show();
            } else {
                Toast.makeText(this, "⚠️ Script is empty", Toast.LENGTH_SHORT).show();
            }
        });

        clearBtn.setOnClickListener(v -> {
            scriptEditor.setText("");
            Toast.makeText(this, "🧹 Cleared", Toast.LENGTH_SHORT).show();
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
