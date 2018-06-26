package com.porterointeligente.unlamsoa.porterointeligente;

import android.content.Intent;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.webkit.WebSettings;
import android.webkit.WebView;
import android.webkit.WebViewClient;
import android.widget.Button;
import android.widget.Toast;

import java.io.File;

public class MainActivity extends AppCompatActivity {
    Button btnVideo;
    Button btnAudio;
    WebView ViewVideo;
    String url="http://192.168.10.178:8080/?action=stream";
    Boolean videoActivo=true;
    Boolean microfonoActivo=false;
//    File file = new File(context.getFilesDir(), filename);

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        Log.i("Ejecuto","onCreate.");
        setContentView(R.layout.activity_main);

        btnVideo=(Button) findViewById(R.id.btnVideo);
        btnAudio=(Button) findViewById(R.id.btnAudio);
        ViewVideo=(WebView) findViewById(R.id.viewVideo);
        ViewVideo.setWebViewClient(new WebViewClient());
        WebSettings settings= ViewVideo.getSettings();
        settings.setSupportZoom(true);
        settings.setBuiltInZoomControls(true);
//        settings.setJavaScriptEnabled(true);
//        settings.setLoadsImagesAutomatically(true);
        cargarVideo(null);

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
}
