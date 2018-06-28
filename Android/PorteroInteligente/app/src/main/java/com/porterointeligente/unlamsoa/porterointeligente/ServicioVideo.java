package com.porterointeligente.unlamsoa.porterointeligente;

import android.app.Service;
import android.content.Intent;
import android.os.IBinder;
import android.support.annotation.Nullable;

class ServicioVideo extends Service {
    @Nullable
    @Override
    //Se invoca a este metodo cuando ejecuta bindService
    public IBinder onBind(Intent intent) {

        return null;
    }

    @Override
    public void onCreate(){
        super.onCreate();

    }

    //Despues de onCreate automaticamente se ejecuta a OnstartCommand,
    //idArranque: Es el id del servicio a ejecutar
    @Override
    public int onStartCommand(Intent intenc, int flags, int idArranque){
    return 1;
    }

}
