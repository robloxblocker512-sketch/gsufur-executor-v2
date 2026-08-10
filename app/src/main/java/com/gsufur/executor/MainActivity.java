package com.gsufur.executor;

import android.os.Bundle;
import android.widget.Button;
import android.widget.EditText;
import android.widget.Toast;
import androidx.appcompat.app.AppCompatActivity;

public class MainActivity extends AppCompatActivity {
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        EditText scriptEditor = findViewById(R.id.scriptEditor);
        Button executeBtn = findViewById(R.id.executeBtn);

        executeBtn.setOnClickListener(v -> {
            String script = scriptEditor.getText().toString();
            Toast.makeText(this, script.isEmpty() ? "Empty" : "Executed!", Toast.LENGTH_SHORT).show();
        });
    }
}
