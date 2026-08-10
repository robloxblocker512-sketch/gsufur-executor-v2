package com.gsufur.executor;

import android.os.Bundle;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;
import android.widget.LinearLayout;
import android.widget.Toast;
import androidx.appcompat.app.AppCompatActivity;
import androidx.viewpager2.widget.ViewPager2;
import com.google.android.material.tabs.TabLayout;
import com.google.android.material.tabs.TabLayoutMediator;

public class MainActivity extends AppCompatActivity {

    private EditText scriptEditor;
    private Button executeBtn, clearBtn, saveBtn, loadBtn;
    private TabLayout tabLayout;
    private LinearLayout executeTab, scriptsTab, filesTab, settingsTab;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        scriptEditor = findViewById(R.id.scriptEditor);
        executeBtn = findViewById(R.id.executeBtn);
        clearBtn = findViewById(R.id.clearBtn);
        saveBtn = findViewById(R.id.saveBtn);
        loadBtn = findViewById(R.id.loadBtn);
        tabLayout = findViewById(R.id.tabLayout);
        executeTab = findViewById(R.id.executeTab);
        scriptsTab = findViewById(R.id.scriptsTab);
        filesTab = findViewById(R.id.filesTab);
        settingsTab = findViewById(R.id.settingsTab);

        // Tab switching
        tabLayout.addOnTabSelectedListener(new TabLayout.OnTabSelectedListener() {
            @Override
            public void onTabSelected(TabLayout.Tab tab) {
                int position = tab.getPosition();
                executeTab.setVisibility(position == 0 ? View.VISIBLE : View.GONE);
                scriptsTab.setVisibility(position == 1 ? View.VISIBLE : View.GONE);
                filesTab.setVisibility(position == 2 ? View.VISIBLE : View.GONE);
                settingsTab.setVisibility(position == 3 ? View.VISIBLE : View.GONE);
            }

            @Override
            public void onTabUnselected(TabLayout.Tab tab) {}

            @Override
            public void onTabReselected(TabLayout.Tab tab) {}
        });

        // Execute button
        executeBtn.setOnClickListener(v -> {
            String script = scriptEditor.getText().toString();
            if (!script.isEmpty()) {
                Toast.makeText(this, "✅ Script Executed!", Toast.LENGTH_SHORT).show();
                // Injection goes here
            } else {
                Toast.makeText(this, "⚠️ Script is empty", Toast.LENGTH_SHORT).show();
            }
        });

        // Clear button
        clearBtn.setOnClickListener(v -> {
            scriptEditor.setText("");
            Toast.makeText(this, "🧹 Cleared", Toast.LENGTH_SHORT).show();
        });

        // Save button
        saveBtn.setOnClickListener(v -> {
            Toast.makeText(this, "💾 Saved", Toast.LENGTH_SHORT).show();
        });

        // Load button
        loadBtn.setOnClickListener(v -> {
            Toast.makeText(this, "📂 Loaded", Toast.LENGTH_SHORT).show();
        });
    }
}
