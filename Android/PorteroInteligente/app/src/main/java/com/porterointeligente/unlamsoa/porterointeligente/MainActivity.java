package com.porterointeligente.unlamsoa.porterointeligente;

import android.app.NotificationManager;
import android.app.PendingIntent;
import android.content.Intent;
import android.content.SharedPreferences;
import android.hardware.Sensor;
import android.hardware.SensorEvent;
import android.hardware.SensorEventListener;
import android.hardware.SensorManager;
import android.media.AudioManager;
import android.preference.PreferenceManager;
import android.support.v4.app.NotificationCompat;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.webkit.WebSettings;
import android.webkit.WebView;
import android.webkit.WebViewClient;
import android.widget.Button;
import android.widget.TextView;
import android.widget.Toast;

import com.google.firebase.database.DatabaseReference;
import com.google.firebase.database.FirebaseDatabase;


public class MainActivity extends AppCompatActivity implements SensorEventListener {
    Button btnVideo;
    NotificationManager mNotifyMgr;
    NotificationCompat.Builder mBuilder;
    FirebaseDatabase database;
    DatabaseReference myRefMic;
    DatabaseReference myRefvid;
    Button btnAudio;
    WebView ViewVideo;
    TextView textVieww;
    Intent intentService=null;
    //si no se inicializó nunca va a tener estos valores.
    String url="";
    String ip="";
    int puertoVideo=8080;
    int puertoAudio=5050;
    Boolean videoActivo=true;
    Boolean microfonoActivo=false;
    private SensorManager      sensor;
    private final static float ACC = 30;
    SharedPreferences dato;

    boolean sensores[];
    boolean videoApagadoPorSensores;


//    File file = new File(context.getFilesDir(), filename);

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        //obtengo el Shared Preferences de mi aplicacion.
        sensores=new boolean[3];
        for(int i=0;i<3;i++){
            sensores[i]=false;
        }
        videoApagadoPorSensores =false;
        dato= PreferenceManager.getDefaultSharedPreferences(this);
        //Obtengo datos del Shared Preference. El 1 parametro es la clave, el 2 es el valor por defecto si no lo encuentra.
        url=dato.getString("url",url);
        ip=dato.getString("ip",ip);
        videoActivo=!dato.getBoolean("videoActivo",videoActivo);
        microfonoActivo=false;
        database= FirebaseDatabase.getInstance();
        myRefMic =database.getReference("microfono");
        myRefvid =database.getReference("video");
        Log.i("Ejecuto","onCreate.");
        setContentView(R.layout.activity_main);
        sensor = (SensorManager) getSystemService(SENSOR_SERVICE);
        btnVideo=(Button) findViewById(R.id.btnVideo);
        btnAudio=(Button) findViewById(R.id.btnAudio);
        ViewVideo=(WebView) findViewById(R.id.viewVideo);
        textVieww= findViewById(R.id.textView2);
        ViewVideo.setWebViewClient(new WebViewClient());
        WebSettings settings= ViewVideo.getSettings();
        settings.setSupportZoom(true);
        settings.setBuiltInZoomControls(true);
        settings.setJavaScriptEnabled(true);
        settings.setLoadsImagesAutomatically(true);
        cargarVideo(null);

        sensor = (SensorManager) getSystemService(SENSOR_SERVICE);

        boolean done;
        done = sensor.registerListener(this, sensor.getDefaultSensor(Sensor.TYPE_ACCELEROMETER), SensorManager.SENSOR_DELAY_NORMAL);
        sensor.registerListener(this,sensor.getDefaultSensor(Sensor.TYPE_PROXIMITY), SensorManager.SENSOR_DELAY_NORMAL);
        sensor.registerListener(this,sensor.getDefaultSensor(Sensor.TYPE_LIGHT), SensorManager.SENSOR_DELAY_NORMAL);
        if (!done)
        {
            Toast.makeText(this, "No soporta Shake", Toast.LENGTH_SHORT).show();

        }else Toast.makeText(this, "Soporta Shake", Toast.LENGTH_SHORT).show();

    }

    public void config(View view){
        Intent intent=new Intent(this,SetUrl.class);
        intent.putExtra("ip",ip);
        startActivityForResult(intent,1);
    }

    @Override
    protected void onActivityResult (int requestCode, int resultCode, Intent data) {
        if (requestCode == 1 & resultCode == RESULT_OK) {
            this.url = "http://" + data.getExtras().getString("url") + ":" + puertoVideo + "/stream/webrtc" ;
            this.ip = data.getExtras().getString("url");
            cargarVideo(null);
//            Toast.makeText(this,url,Toast.LENGTH_SHORT).show();
        }
    }

    @Override
    protected void onResume() {
        super.onResume();
    }

    @Override
    protected void onPause() {
        super.onPause();
        //Hago editable este objeto
        SharedPreferences.Editor miEditor=dato.edit();
        //pongo la información que quiero editar.
        miEditor.putString("url",url);
        miEditor.putString("ip",ip);
        miEditor.putBoolean("videoActivo",videoActivo);
        //Guardo los cambios de miEditor en el Shared Preference.
        miEditor.apply();
    }


    public void cargarVideo(View view){
        if(this.videoActivo==true){
            apagarVideo();
        }else{
            prenderVideo();

        }
    }

    protected void prenderVideo(){
        this.videoActivo=true;
        ViewVideo.setVisibility(View.VISIBLE);
        ViewVideo.loadUrl(url);
        AudioManager audio = (AudioManager)getSystemService(AUDIO_SERVICE);
        audio.setSpeakerphoneOn(true);
    }

    protected void apagarVideo(){
        videoActivo=false;
        ViewVideo.setVisibility(View.INVISIBLE);
        ViewVideo.stopLoading();
        AudioManager audio = (AudioManager)getSystemService(AUDIO_SERVICE);
        audio.setSpeakerphoneOn(false);
    }
    public void PresionAudio(View view){
        if(intentService==null){
            intentService=new Intent(MainActivity.this, ServicioAudio.class);
        }

        if(microfonoActivo==true){
            microfonoActivo=false;
            stopService(intentService);
            intentService=null;
            //apago Notificacion
            mNotifyMgr.cancel(1);
            //mando a Firebase
            myRefMic.setValue("microfonoApagado");

        }else{
            if(!this.ip.equals("")) {
                intentService.putExtra("ipAudio", ip);
                intentService.putExtra("puerto", puertoAudio);
                microfonoActivo = true;
                myRefMic.setValue("microfonoActivo");
                lanzarNotificacionMicActivo();
                startService(intentService);
            }
        }
    }

    protected void lanzarNotificacionMicActivo(){
        mNotifyMgr =(NotificationManager) getApplicationContext().getSystemService(NOTIFICATION_SERVICE);

        Intent i=new Intent(MainActivity.this, MainActivity.class);
        PendingIntent pendingIntent = PendingIntent.getActivity(MainActivity.this, 0, i, 0);

        mBuilder =new NotificationCompat.Builder(getApplicationContext())
                .setContentIntent(pendingIntent)
                .setSmallIcon(R.drawable.icono)
                .setContentTitle("Portero Microfono Activado")
                .setContentText("Microfono activado hacia el portero.")
                .setVibrate(new long[] {100, 250, 100, 500})
                .setAutoCancel(false);  //con esto le digo que no se cierre al apretar en la notificacion
        mBuilder.setOngoing(true); //con esto hago que no se pueda cerrar la notificacion.

        mNotifyMgr.notify(1, mBuilder.build());

    }


    @Override
    public void onSaveInstanceState(Bundle estado){
//        Con el Bundle guardo el estado de la aplicacion, ante un reinicio, rotacion o inactividad prolongada.
        estado.putString("url",url);
        estado.putString("ip",ip);
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
        ip=estado.getString("ip");
        videoActivo= estado.getBoolean("videoActivo");
        microfonoActivo= estado.getBoolean("microfonoActivo");
//        porcentaje=estado.getDouble("porcentaje");
//        resu.setText(estado.getString("resultado"));
        if(videoActivo==true){
            ViewVideo.setVisibility(View.VISIBLE);
            ViewVideo.loadUrl(url);
        }else ViewVideo.setVisibility(View.INVISIBLE);
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
                sensores[0]=true;
                //esta boca abajo
                textVieww.setText("Apago"+values[0]+"   "+values[1]+"   "+values[2]+"   ");
            }else sensores[0]=false;

        }else if(sensorType == Sensor.TYPE_PROXIMITY){
            //0=hay objeto,  5 no hay objeto.
            if(event.values[0]==0){
                sensores[1]=true;
                textVieww.setText(""+event.values[0]);
            }else sensores[1]=false;

        }else if(sensorType == Sensor.TYPE_LIGHT){
            if(event.values[0]==0){
                sensores[2]=true;
                textVieww.setText("Sin luz"+event.values[0]);
            }else sensores[2]=false;
        }

        if(isSensorAllTrue()==true){
            if(videoActivo==true){
                apagarVideo();
                videoApagadoPorSensores=true;
            }

        }else{
            if(videoActivo==false && videoApagadoPorSensores==true){
                prenderVideo();
                videoApagadoPorSensores=false;
            }

        }
    }

    protected boolean isSensorAllTrue(){
        for(int i=0;i<sensores.length;i++){
            if(sensores[i]==false)
                return false;
        }
        return true;
    }

    @Override
    public void onAccuracyChanged(Sensor sensor, int accuracy) {    }
}
