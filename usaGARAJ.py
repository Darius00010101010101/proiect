import paho.mqtt.client as mqtt
import datetime
import time

def on_message(client, userdata, msg):
    mesaj = msg.payload.decode()
    ora = datetime.datetime.now().strftime("%H:%M:%S")
    print(f"[{ora}] Mesaj primit: {mesaj}")
    with open("baza_date_garaj.txt", "a") as f:
        f.write(f"{ora} - {mesaj}\n")

# Configurarea pentru a evita erorile de versiune (Paho 2.0)
try:
    client = mqtt.Client(mqtt.CallbackAPIVersion.VERSION1)
except:
    client = mqtt.Client()

client.on_message = on_message

print("Incercare conectare...")
try:
    client.connect("broker.hivemq.com", 1883, 60)
    client.subscribe("proiect_ps/garaj/stare")
    print("Conectat! Astept date... (Apasă Ctrl+C pentru a opri)")
    client.loop_forever()
except Exception as e:
    print(f"Eroare la conectare: {e}")
    time.sleep(10) # Tine fereastra deschisa 10 secunde daca pica