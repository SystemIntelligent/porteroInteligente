package com.porterointeligente.unlamsoa.porterointeligente;

import android.content.Intent;
import android.os.Bundle;
import android.support.v7.app.AppCompatActivity;
import android.view.View;
import android.widget.EditText;

public class SetUrl extends AppCompatActivity {
    EditText path;


    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.seturl);
        path=(EditText) findViewById(R.id.editText);
    }


    public void terminar(View view){
        Intent intent = new Intent();
        intent.putExtra("url",path.getText().toString());

        setResult(RESULT_OK, intent);
        finish(); //termino la actividad y vuelve a la que la llamo
    }
    @Override
    public void onSaveInstanceState(Bundle estado){
//        Con el Bundle guardo el estado de la aplicacion, ante un reinicio, rotacion o inactividad prolongada.
        estado.putString("url",path.getText().toString());
        super.onSaveInstanceState(estado);
    }

    @Override
    public void onRestoreInstanceState(Bundle estado){
        super.onRestoreInstanceState(estado);
        path.setText(estado.getString("url").toString());
    }




}
