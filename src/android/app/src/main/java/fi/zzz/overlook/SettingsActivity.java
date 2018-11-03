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

import java.io.FileNotFoundException;
import java.io.InputStream;

public class SettingsActivity extends Activity {

    private Bitmap profile_image;
    public static final int PICK_IMAGE = 1;

    public SettingsActivity() {


    }

    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.settings);

        EditText edit_server = findViewById(R.id.edit_server2);
        EditText edit_port = findViewById(R.id.edit_port2);
        edit_server.setText(AppService.setup_server);
        edit_port.setText(Integer.toString(AppService.setup_port));

        Button save = (Button)findViewById(R.id.save);
        save.setOnClickListener(new View.OnClickListener() {
            public void onClick(View v) {
                save();
            }
        });


    }

    private void save() {

        EditText edit_server = findViewById(R.id.edit_server2);
        EditText edit_port = findViewById(R.id.edit_port2);

        String server_str = edit_server.getText().toString();
        String port_str = edit_port.getText().toString();
        AppService.last.settingsFinish(
                server_str,
                Integer.parseInt(port_str));
        finish();
    }


}
