package com.gsufur.executor;

import android.app.Service;
import android.content.Intent;
import android.graphics.PixelFormat;
import android.os.Build;
import android.os.IBinder;
import android.view.Gravity;
import android.view.LayoutInflater;
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

        overlayView = new LinearLayout(this);
        overlayView.setBackgroundColor(0xCC1a1a2e);
        overlayView.setPadding(16, 16, 16, 16);

        EditText editor = new EditText(this);
        editor.setHint("Enter script");
        editor.setTextColor(0xFF00ff88);
        editor.setBackgroundColor(0xFF0f0f1a);
        editor.setPadding(12, 12, 12, 12);
        LinearLayout.LayoutParams editorParams = new LinearLayout.LayoutParams(400, 300);
        editor.setLayoutParams(editorParams);

        Button runBtn = new Button(this);
        runBtn.setText("Run");
        runBtn.setBackgroundColor(0xFF00ff88);
        runBtn.setTextColor(0xFF000000);
        LinearLayout.LayoutParams btnParams = new LinearLayout.LayoutParams(
                LinearLayout.LayoutParams.MATCH_PARENT,
                LinearLayout.LayoutParams.WRAP_CONTENT
        );
        btnParams.topMargin = 8;
        runBtn.setLayoutParams(btnParams);

        runBtn.setOnClickListener(v -> {
            String script = editor.getText().toString();
            if (!script.isEmpty()) {
                Toast.makeText(this, "✅ Executed!", Toast.LENGTH_SHORT).show();
            }
        });

        overlayView.setOrientation(LinearLayout.VERTICAL);
        overlayView.addView(editor);
        overlayView.addView(runBtn);

        WindowManager.LayoutParams params = new WindowManager.LayoutParams(
                WindowManager.LayoutParams.WRAP_CONTENT,
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
