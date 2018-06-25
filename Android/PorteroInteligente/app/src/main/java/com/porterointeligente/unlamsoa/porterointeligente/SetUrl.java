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
}
