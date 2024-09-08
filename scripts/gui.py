import tkinter as tk
from tkinter import ttk
from wifi_communicator import OutMessage
import time
from datetime import datetime
from datavis import excelLog

# Define os tempos!
tempoEnchimeto = 30000
tempoEsvaziamento = 30000
tempoAguardar = 300000
tempoMedir = 5000


class GUI(tk.Tk):
    def __init__(self, communicator):
        super().__init__()
        self.communicator = communicator
        self.title("EMEQ")
        self.geometry("400x300")

        # Visor
        self.visor_canvas = tk.Canvas(self, width=200, height=100)
        self.visor_canvas.pack(pady=10)
        self.visor_rect = self.visor_canvas.create_rectangle(10, 10, 190, 90, fill="green")
        self.visor_text = self.visor_canvas.create_text(100, 50, text="green", font=("Helvetica", 16), fill="white")

        # Chronometer
        self.chrono_label = ttk.Label(self, text="Temporizador", font=("Helvetica", 16))
        self.chrono_label.pack(pady=10)
        self.chrono_text = tk.StringVar(value="00:00:00")
        self.chrono_display = ttk.Label(self, textvariable=self.chrono_text, font=("Helvetica", 16))
        self.chrono_display.pack(pady=10)

        self.start_button = ttk.Button(self, text="Iniciar", command=self.start_chrono)
        self.start_button.pack(pady=10)

        self.stop_button = ttk.Button(self, text="Desligar", command=self.stop_chrono)
        self.stop_button.pack(pady=10)

        self.running = False
        self.start_time = 0
        self.starting = True

        # Color change and message sending
        self.colors = ["orange", "blue", "green", "red"]
        self.texts = ["AGUARDANDO", "ENCHENDO", "MEDINDO","ESVAZIANDO"]
        # As duracoes estao despareadas: A segunda duracao e do primeiro estado, e por ai vai
        self.durations = [tempoEsvaziamento, tempoAguardar, tempoEnchimeto, tempoMedir]  # Durations in milliseconds
        self.color_index = 0
        self.messages = ['s', 'f', 'm', 'e']
        self.message_index = 0

    def setEspera(self, espera):
        self.durations[1] = int(espera * 60000)

    def start_chrono(self):
        if not self.running:
            self.running = True
            if self.starting:
                self.start_time = time.time()
                self.starting = False
            self.update_chrono()
            self.change_status_and_send_message()

    def stop_chrono(self):
        # Manda parar
        if self.running:
            self.running = False
            msg = OutMessage(data='s')
            self.communicator.send_message(msg)
            self.quit()

    def reset_chrono(self):
        self.running = False
        self.start_time = 0
        self.chrono_text.set("00:00:00")

    def update_chrono(self):
        if self.running:
            elapsed_time = self.get_elapsed_time()
            self.chrono_text.set(self.format_time(elapsed_time))
            self.after(1000, self.update_chrono)

    def get_elapsed_time(self):
        return time.time() - self.start_time

    def format_time(self, elapsed):
        hours, rem = divmod(elapsed, 3600)
        minutes, seconds = divmod(rem, 60)
        return "{:02}:{:02}:{:02}".format(int(hours), int(minutes), int(seconds))

    def change_status_and_send_message(self):
        if not self.running:
            return

        # Change visor color
        current_color = self.colors[self.color_index]
        current_text = self.texts[self.color_index]
        self.visor_canvas.itemconfig(self.visor_rect, fill=current_color)
        self.visor_canvas.itemconfig(self.visor_text, text=current_text)

        # Send message
        message = self.messages[self.message_index]
        msg = OutMessage(data=message)
        self.communicator.send_message(msg)

        if(self.message_index == 2):
            data = datetime.now().strftime("%d-%m-%Y")
            hora = datetime.now().strftime("%H:%M:%S")
            pH = None
            pH = self.communicator.get_message().data
            print(f'pH: {pH}')
            tExt = None
            tExt = self.communicator.get_message().data
            print(f'Temp. externa: {tExt} Celsius')
            umidade = None
            umidade = self.communicator.get_message().data
            print(f'Umidade: {umidade}%')
            co = None
            co = self.communicator.get_message().data
            print(f'CO: {co} ppm')
            tAgua = None
            tAgua = self.communicator.get_message().data
            print(f'Temp. Agua: {tAgua} Celsius')
            particulas = None
            particulas = self.communicator.get_message().data
            print(f'TDS: {particulas} ppm')
            turbidez = None
            turbidez = self.communicator.get_message().data
            print(f'Turbidez: {turbidez}%\n\n')
            
            measured_data = {
                "Data": [data.replace(".", ",")],
                "Hora": [hora.replace(".", ",")],
                "Temperatura externa (°C)": [float(tExt)],
                "Umidade (%)": [float(umidade)],
                "Nível de CO (ppm)": [float(co)],
                "Temperatura da água (°C)": [float(tAgua)],
                "pH": [float(pH)],
                "Turbidez (%)": [float(turbidez)],
                "Partículas dissolvidas (ppm)": [float(particulas)]
            }

            excelLog('logs/RegistrosEMEQ.xlsx', measured_data)

        # Update indices
        self.color_index = (self.color_index + 1) % len(self.colors)
        self.message_index = (self.message_index + 1) % len(self.messages)

        # Schedule next change
        self.after(self.durations[self.color_index], self.change_status_and_send_message)