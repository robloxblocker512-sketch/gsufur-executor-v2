package com.gsufur.executor;

import android.app.Notification;
import android.app.NotificationChannel;
import android.app.NotificationManager;
import android.app.Service;
import android.content.Context;
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
import android.widget.ImageButton;
import android.widget.LinearLayout;
import android.widget.TextView;
import android.widget.Toast;
import androidx.core.app.NotificationCompat;

public class FloatingOverlayService extends Service {

    private WindowManager windowManager;
    private LinearLayout overlayView;
    private View collapsedView;
    private View expandedView;
    private EditText scriptEditor;
    private Button executeBtn, clearBtn;
    private ImageButton closeBtn, minimizeBtn;
    private TextView statusText;
    private boolean isExpanded = true;

    private float initialTouchX, initialTouchY;
    private int initialWindowX, initialWindowY;
    private boolean isDragging = false;

    @Override
    public void onCreate() {
        super.onCreate();
        createNotificationChannel();
        startForeground(1, createNotification());

        windowManager = (WindowManager) getSystemService(WINDOW_SERVICE);

        LayoutInflater inflater = (LayoutInflater) getSystemService(LAYOUT_INFLATER_SERVICE);
        overlayView = (LinearLayout) inflater.inflate(R.layout.overlay_layout, null);

        setupViews();

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
        setupDrag(overlayView, params);
    }

    private void setupViews() {
        expandedView = overlayView.findViewById(R.id.expandedView);
        collapsedView = overlayView.findViewById(R.id.collapsedView);
        scriptEditor = overlayView.findViewById(R.id.scriptEditor);
        executeBtn = overlayView.findViewById(R.id.executeBtn);
        clearBtn = overlayView.findViewById(R.id.clearBtn);
        closeBtn = overlayView.findViewById(R.id.closeBtn);
        minimizeBtn = overlayView.findViewById(R.id.minimizeBtn);
        statusText = overlayView.findViewById(R.id.statusText);

        expandedView.setVisibility(View.VISIBLE);
        collapsedView.setVisibility(View.GONE);

        executeBtn.setOnClickListener(v -> {
            String script = scriptEditor.getText().toString();
            if (!script.isEmpty()) {
                statusText.setText("✅ Executed!");
                Toast.makeText(this, "✅ Script Executed!", Toast.LENGTH_SHORT).show();
            } else {
                statusText.setText("⚠️ Empty script");
                Toast.makeText(this, "⚠️ Script is empty", Toast.LENGTH_SHORT).show();
            }
        });

        clearBtn.setOnClickListener(v -> {
            scriptEditor.setText("");
            statusText.setText("🧹 Cleared");
        });

        closeBtn.setOnClickListener(v -> stopSelf());

        minimizeBtn.setOnClickListener(v -> toggleExpand());

        collapsedView.setOnClickListener(v -> toggleExpand());
    }

    private void toggleExpand() {
        isExpanded = !isExpanded;
        if (isExpanded) {
            expandedView.setVisibility(View.VISIBLE);
            collapsedView.setVisibility(View.GONE);
            statusText.setText("Expanded");
        } else {
            expandedView.setVisibility(View.GONE);
            collapsedView.setVisibility(View.VISIBLE);
            TextView collapsedStatus = collapsedView.findViewById(R.id.collapsedStatus);
            collapsedStatus.setText("GSUFUR");
        }
    }

    private void setupDrag(View view, WindowManager.LayoutParams params) {
        view.setOnTouchListener((v, event) -> {
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
                    return true;
            }
            return false;
        });
    }

    private void createNotificationChannel() {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
            NotificationChannel channel = new NotificationChannel(
                    "overlay_channel",
                    "GSUFUR Overlay",
                    NotificationManager.IMPORTANCE_LOW
            );
            channel.setDescription("GSUFUR floating overlay service");
            NotificationManager manager = getSystemService(NotificationManager.class);
            manager.createNotificationChannel(channel);
        }
    }

    private Notification createNotification() {
        return new NotificationCompat.Builder(this, "overlay_channel")
                .setContentTitle("GSUFUR")
                .setContentText("Floating overlay is running")
                .setSmallIcon(android.R.drawable.ic_menu_edit)
                .setPriority(NotificationCompat.PRIORITY_LOW)
                .build();
    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        return START_STICKY;
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
