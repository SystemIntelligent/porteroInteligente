import firebase_admin
from firebase_admin import credentials
from firebase_admin import db
import sys

cred = credentials.Certificate ('/home/pi/iot/cred.json')
# Initialize the app with a service account, granting admin privileges
firebase_admin.initialize_app(cred,{'databaseURL': 'https://porterointeligente-e1e18.firebaseio.com/'})

abrir_puerta = db.reference('abrirpuerta')
cerrar_puerta = db.reference('cerrarpuerta')
mic_android = db.reference('micandroid')


openDoor = sys.argv[1]
closeDoor = sys.argv[2]
mic = sys.argv[3]

abrir_puerta.set(openDoor)
cerrar_puerta.set(closeDoor)
mic_android.set(mic)

