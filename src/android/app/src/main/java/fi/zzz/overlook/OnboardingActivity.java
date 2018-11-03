package fi.zzz.overlook;

import android.app.Activity;
import android.content.Intent;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.os.Bundle;
import android.support.v7.widget.AppCompatImageView;
import android.view.View;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.EditText;
import android.widget.Spinner;
import android.widget.TextView;

import java.io.FileNotFoundException;
import java.io.InputStream;

public class OnboardingActivity extends Activity {

    public static final int PICK_IMAGE = 1;

    public OnboardingActivity() {

    }

    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.onboarding1);

        Button next1 = (Button)findViewById(R.id.next1);
        next1.setOnClickListener(new View.OnClickListener() {
            public void onClick(View v) {
                SetPage2();
            }
        });

    }


    void SetPage2() {
        setContentView(R.layout.onboarding2);

        EditText edit_port = findViewById(R.id.edit_port);
        edit_port.setText("17001");

        Button next2 = (Button) findViewById(R.id.next2);
        next2.setOnClickListener(new View.OnClickListener() {
            public void onClick(View v) {
                EditText edit_server = findViewById(R.id.edit_server);
                EditText edit_port = findViewById(R.id.edit_port);

                String server_str = edit_server.getText().toString();
                String port_str = edit_port.getText().toString();
                AppService.last.setupFinish(server_str, Integer.parseInt(port_str));
                finish();
            }
        });


    }
}
