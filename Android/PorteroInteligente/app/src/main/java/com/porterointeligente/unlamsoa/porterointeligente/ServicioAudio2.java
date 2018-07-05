package com.porterointeligente.unlamsoa.porterointeligente;

import android.app.Service;
import android.content.Context;
import android.content.Intent;
import android.media.AudioManager;
import android.media.MediaPlayer;
import android.net.Uri;
import android.net.rtp.AudioCodec;
import android.net.rtp.AudioGroup;
import android.net.rtp.AudioStream;
import android.net.rtp.RtpStream;
import android.os.IBinder;
import android.os.StrictMode;
import android.support.annotation.Nullable;
import android.widget.Toast;

import java.io.IOException;
import java.net.InetAddress;

public class ServicioAudio2 extends Service {
    String ip="10.41.101.10";
    AudioManager audio;
    AudioGroup audioGroup;
    AudioStream audioStream;
    MediaPlayer reproductor;
    @Nullable
    @Override
    //Se invoca a este metodo cuando ejecuta bindService.
    // El método OnBind es un método abstracto el cual nos regresa el canal de comunicación del servicio
    // y sirve para establecer comunicación entre servicios (para esto es necesario definir un AIDL).
    // Colocamos null para declarar que en esta ocasión no estableceremos esa comunicación.
    public IBinder onBind(Intent intent) {
        Toast.makeText(this,"Servicio onBind",Toast.LENGTH_SHORT).show();
        return null;
    }

    @Override
    //Se ejecuta primero, luego On StartCommand
    public void onCreate(){
        super.onCreate();
        Toast.makeText(this,"Servicio iniciado",Toast.LENGTH_SHORT).show();
        Uri audioUri = Uri.parse("rtp://192.168.1.137:5050");
//        reproductor = MediaPlayer.create(this, R.raw.imperial);
        reproductor = new MediaPlayer();
        try {
            reproductor.setDataSource("rtp://192.168.1.137:5050");
            reproductor.prepare();
            reproductor.start();
        } catch (IOException e) {
            e.printStackTrace();
        }
        audio =  (AudioManager) getSystemService(Context.AUDIO_SERVICE);
            audio.setSpeakerphoneOn(true);
//        reproductor.setLooping(true);
//        StrictMode.ThreadPolicy policy = new StrictMode.ThreadPolicy.Builder().permitAll().build();
//        StrictMode.setThreadPolicy(policy);
//        try {
//             audio =  (AudioManager) getSystemService(Context.AUDIO_SERVICE);
//            audio.setSpeakerphoneOn(true);
//            audio.setMode(AudioManager.MODE_IN_COMMUNICATION);
//             audioGroup = new AudioGroup();
//            audioGroup.setMode(AudioGroup.MODE_NORMAL);
//             audioStream = new AudioStream(InetAddress.getByAddress(new byte[] {(byte)10, (byte)41, (byte)101, (byte)10 }));
//            audioStream.setCodec(AudioCodec.PCMU);
////            audioStream.setCodec(AudioCodec.AMR);
//
//            audioStream.setMode(RtpStream.MODE_RECEIVE_ONLY);
//            //set receiver(vlc player) machine ip address(please update with your machine ip)
//            audioStream.associate(InetAddress.getByAddress(new byte[] {(byte)10, (byte)41, (byte)101, (byte)10 }), 1234);
//            audioStream.join(audioGroup);
//
//        }catch (Exception e) {
//            e.printStackTrace();
//            Toast.makeText(this,"Servicio fallo",Toast.LENGTH_SHORT).show();
//        }
    }

    @Override
    //Se ejecuta cuando se manda stopService
    public void onDestroy(){
        super.onDestroy();
        Toast.makeText(this,"Servicio detenido",Toast.LENGTH_SHORT).show();
//        AudioManager audio = (AudioManager)getSystemService(AUDIO_SERVICE);
        audio.setSpeakerphoneOn(false);
        reproductor.stop();

    }


    //Despues de onCreate automaticamente se ejecuta a OnstartCommand,
    //idArranque: Es el id del servicio a ejecutar
    @Override
    public int onStartCommand(Intent intenc, int flags, int idArranque){
        Toast.makeText(this,"Servicio onStartCommand",Toast.LENGTH_SHORT).show();
//        reproductor.start();
        return START_STICKY;
    }

}
