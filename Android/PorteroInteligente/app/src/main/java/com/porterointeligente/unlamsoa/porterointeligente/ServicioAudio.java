package com.porterointeligente.unlamsoa.porterointeligente;

import android.app.Service;
import android.content.Context;
import android.content.Intent;
import android.media.AudioManager;
import android.net.rtp.AudioCodec;
import android.net.rtp.AudioGroup;
import android.net.rtp.AudioStream;
import android.net.rtp.RtpStream;
import android.os.IBinder;
import android.os.StrictMode;
import android.support.annotation.Nullable;
import android.util.Log;
import android.widget.Toast;

import java.net.InetAddress;
import java.net.NetworkInterface;
import java.net.SocketException;
import java.net.UnknownHostException;
import java.util.Enumeration;

public class ServicioAudio extends Service {
    String ipRemota="";
    int puerto=0;
    AudioManager audio;
    AudioGroup audioGroup;
    AudioStream audioStream;
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

//        audioStream.getRemoteAddress()
    }

    public static byte[] getLocalIPAddress () {
        byte ip[]=null;
        try {
            for (Enumeration en = NetworkInterface.getNetworkInterfaces(); en.hasMoreElements();) {
                NetworkInterface intf = (NetworkInterface) en.nextElement();
                for (Enumeration enumIpAddr = intf.getInetAddresses(); enumIpAddr.hasMoreElements();) {
                    InetAddress inetAddress = (InetAddress) enumIpAddr.nextElement();
                    if (!inetAddress.isLoopbackAddress()) {
                        ip= inetAddress.getAddress();
                    }
                }
            }
        } catch (SocketException ex) {
            Log.i("SocketException ", ex.toString());
        }
        return ip;
    }

    @Override
    //Se ejecuta cuando se manda stopService
    public void onDestroy(){
        super.onDestroy();
        Toast.makeText(this,"Servicio detenido",Toast.LENGTH_SHORT).show();
//        AudioManager audio = (AudioManager)getSystemService(AUDIO_SERVICE);
//        audio.setSpeakerphoneOn(false);

        try {
            audioGroup.clear();
//            this.finalize();
            this.stopSelf();

        } catch (Throwable throwable) {
            throwable.printStackTrace();
        }

    }


    //Despues de onCreate automaticamente se ejecuta a OnstartCommand,
    //idArranque: Es el id del servicio a ejecutar
    @Override
    public int onStartCommand(Intent inten, int flags, int idArranque){
        ipRemota=inten.getExtras().getString("ipAudio");
        puerto=inten.getExtras().getInt("puerto");
        if(iniciarAudio()==true){
            audioStream.join(audioGroup);
            Toast.makeText(this,"Servicio onStartCommand",Toast.LENGTH_SHORT).show();

            return START_STICKY;
        }else {
            audioGroup.clear();
            try {
                this.finalize();
            } catch (Throwable throwable) {

            }
            return START_NOT_STICKY;
        }


    }

    protected boolean iniciarAudio(){
        StrictMode.ThreadPolicy policy = new StrictMode.ThreadPolicy.Builder().permitAll().build();
        StrictMode.setThreadPolicy(policy);
        try {
            audio =  (AudioManager) getSystemService(Context.AUDIO_SERVICE);
            audio.setMode(AudioManager.MODE_IN_COMMUNICATION);
            audioGroup = new AudioGroup();
            audioGroup.setMode(AudioGroup.MODE_NORMAL);
            audioStream = new AudioStream(InetAddress.getByAddress(getLocalIPAddress ()));
            audioStream.setCodec(AudioCodec.PCMU);

            audioStream.setMode(RtpStream.MODE_NORMAL);
            //poner la ip de la raspberry que tiene que recibir el audio.
//            audioStream.associate(InetAddress.getByAddress(new byte[] {(byte)192, (byte)168, (byte)1, (byte)132 }), 5050);
            byte[] ip=obtenerIpDeString(ipRemota);
            if(ip!=null){
                //asocio para el envio del audio.
                audioStream.associate(InetAddress.getByAddress(ip), puerto);
                return true;
            }else return false;

        }catch (Exception e) {
//            e.printStackTrace();
            Toast.makeText(this,"Servicio fallo",Toast.LENGTH_SHORT).show();
            return false;
        }
    }




    protected byte[] obtenerIpDeString(String ip) {
        byte[] ipSeparada = new byte[4];
        int[] ipEntera = new int[4];
        String[] separated = ip.split("\\.");
        if (separated.length != 4) {
            return null;
        }

        try {
            for (int i = 0; i < 4; i++) {
                Log.e("        IP  ",separated[i]+" ");
                ipEntera[i] = Integer.parseInt(separated[i]);
                ipSeparada[i] = (byte) ipEntera[i];
            }
        } catch (Exception e) {
            return null;
        }
        return ipSeparada;
    }
}
