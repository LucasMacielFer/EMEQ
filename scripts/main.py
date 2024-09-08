from gui import GUI
from wifi_communicator import WiFiCommunicator

def main():
    espera = input("Tempo de espera (minutos) entre medições: ")
    if espera != "":
        espera = float(espera)
    else:
        espera = 0
        
    if espera < 1 or espera > 60:
        espera = 5

    communicator = WiFiCommunicator(max_buffer_sz=128)
    gui = GUI(communicator=communicator)
    gui.setEspera(0.1)
    gui.mainloop()

if __name__ == '__main__':
    main()
