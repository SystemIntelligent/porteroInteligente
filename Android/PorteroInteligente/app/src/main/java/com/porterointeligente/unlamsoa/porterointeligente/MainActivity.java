package com.porterointeligente.unlamsoa.porterointeligente;

import android.content.Intent;
import android.hardware.Sensor;
import android.hardware.SensorEvent;
import android.hardware.SensorEventListener;
import android.hardware.SensorListener;
import android.hardware.SensorManager;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.webkit.WebSettings;
import android.webkit.WebView;
import android.webkit.WebViewClient;
import android.widget.Button;
import android.widget.CompoundButton;
import android.widget.TextView;
import android.widget.Toast;

import java.io.File;

public class MainActivity extends AppCompatActivity implements SensorEventListener {
    Button btnVideo;
    Button btnAudio;
    WebView ViewVideo;
    TextView textVieww;
    String url="http://192.168.10.178:8080/?action=stream";
    Boolean videoActivo=true;
    Boolean microfonoActivo=false;
    private SensorManager      sensor;
    private final static float ACC = 30;

//    File file = new File(context.getFilesDir(), filename);

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        Log.i("Ejecuto","onCreate.");
        setContentView(R.layout.activity_main);
        sensor = (SensorManager) getSystemService(SENSOR_SERVICE);
        btnVideo=(Button) findViewById(R.id.btnVideo);
        btnAudio=(Button) findViewById(R.id.btnAudio);
        ViewVideo=(WebView) findViewById(R.id.viewVideo);
        textVieww=(TextView) findViewById(R.id.textView2);
        ViewVideo.setWebViewClient(new WebViewClient());
        WebSettings settings= ViewVideo.getSettings();
        settings.setSupportZoom(true);
        settings.setBuiltInZoomControls(true);
//        settings.setJavaScriptEnabled(true);
//        settings.setLoadsImagesAutomatically(true);
        cargarVideo(null);

        sensor = (SensorManager) getSystemService(SENSOR_SERVICE);

        boolean done;
        done = sensor.registerListener(this, sensor.getDefaultSensor(Sensor.TYPE_ACCELEROMETER), SensorManager.SENSOR_DELAY_NORMAL);
        sensor.registerListener(this,sensor.getDefaultSensor(Sensor.TYPE_PROXIMITY), SensorManager.SENSOR_DELAY_NORMAL);
        sensor.registerListener(this,sensor.getDefaultSensor(Sensor.TYPE_LIGHT), SensorManager.SENSOR_DELAY_NORMAL);
        if (!done)
        {
            Toast.makeText(this, "No soporta Shake", Toast.LENGTH_SHORT).show();

        }
        Toast.makeText(this, "Soporta Shake", Toast.LENGTH_SHORT).show();
    }

    public void config(View view){
        Intent intent=new Intent(this,SetUrl.class);
        startActivityForResult(intent,1);
    }

    @Override
    protected void onActivityResult (int requestCode, int resultCode, Intent data) {
        if (requestCode == 1 & resultCode == RESULT_OK) {
            this.url = data.getExtras().getString("url");
            cargarVideo(null);
//            Toast.makeText(this,url,Toast.LENGTH_SHORT).show();
        }
    }
    @Override
    protected void onResume() {
        super.onResume();



    }


    public void cargarVideo(View view){
        if(videoActivo==true){
            videoActivo=false;
            ViewVideo.setVisibility(View.INVISIBLE);
        }else{
            videoActivo=true;
            ViewVideo.setVisibility(View.VISIBLE);
            ViewVideo.loadUrl(url);
        }
    }


    @Override
    public void onSaveInstanceState(Bundle estado){
//        Con el Bundle guardo el estado de la aplicacion, ante un reinicio, rotacion o inactividad prolongada.
        estado.putString("url",url);
//        estado.putDouble("porcentaje",porcentaje);
//        estado.putString("resultado",resu.getText().toString());
        estado.putBoolean("videoActivo",videoActivo);
        estado.putBoolean("microfonoActivo",microfonoActivo);
        super.onSaveInstanceState(estado);
    }

    @Override
    public void onRestoreInstanceState(Bundle estado){
        super.onRestoreInstanceState(estado);
        url=estado.getString("url");
        videoActivo= estado.getBoolean("videoActivo");
        microfonoActivo= estado.getBoolean("microfonoActivo");
//        porcentaje=estado.getDouble("porcentaje");
//        resu.setText(estado.getString("resultado"));
        ViewVideo.loadUrl(url);
    }

    @Override
    public void onSensorChanged(SensorEvent event) {
        int sensorType = event.sensor.getType();

        float[] values = event.values;

        if (sensorType == Sensor.TYPE_ACCELEROMETER)
        {
            //es Shake?
            if ((Math.abs(values[0]) > ACC || Math.abs(values[1]) > ACC || Math.abs(values[2]) > ACC))
            {
                //Accion al hacer Shake.
                cargarVideo(null);
            }

            if(values[2]<-8.7 ){
                //esta boca abajo
                textVieww.setText("Apago"+values[0]+"   "+values[1]+"   "+values[2]+"   ");
            }
        }else if(sensorType == Sensor.TYPE_PROXIMITY){
            //0=hay objeto,  5 no hay objeto.
            if(event.values[0]==0){
                textVieww.setText(""+event.values[0]);
            }

        }else if(sensorType == Sensor.TYPE_LIGHT){
            if(event.values[0]==0){
                textVieww.setText("Sin luz"+event.values[0]);
            }

        }
    }

    @Override
    public void onAccuracyChanged(Sensor sensor, int accuracy) {    }
}
