import firebase_admin
from firebase_admin import credentials
from firebase_admin import db

cred = credentials.Certificate ('/home/pi/iot/cred.json')
# Initialize the app with a service account, granting admin privileges
firebase_admin.initialize_app(cred,{'databaseURL': 'https://porterointeligente-e1e18.firebaseio.com/'})

abrir_puerta = db.reference('abrirpuerta')
cerrar_puerta = db.reference('cerrarpuerta')
mic_android = db.reference('micandroid')

datos = []

datos.append(abrir_puerta.get())
datos.append(cerrar_puerta.get())
datos.append(mic_android.get())

index = 0
while index < len(datos):
  print datos[index],
  index=index+1






