from tkinter import *
from PIL import ImageTk, Image
from tkinter import messagebox
import serial

root = Tk()
root.title('interface EXPO 46')
root.iconbitmap('c:/pythonzooi/y.ico')
root.geometry("1000x900")
ser = serial.Serial('COM8', baudrate=115200, timeout=1)
batlure_img = ImageTk.PhotoImage(Image.open("apodemus.jpg"))
picture_label = Label(image=batlure_img)

global acurrentpos
global tcurrentpos
global apos
global tpos
global arecived
global trecived
global acompleted
global tcompleted
global Pause
global Stop


def button_start():
    messagebox.showinfo("Start", "The test has started")
    info_read("start")


def button_pause():
    messagebox.showinfo("Pause", "The test has been paused")
    info_read("Pause")


def button_stop():

    messagebox.showinfo("Stop", "The test has ended")
    info_read("Stop")


def button_results():
    messagebox.showinfo("Results", "The results will be showed here")


def button_send():
    anewpos = anewpos_text.get("1.0", END)
    tnewpos = tnewpos_text.get("1.0", END)
    if int(anewpos) > 90 or int(anewpos) < -90:
        messagebox.showinfo("send", "Enter a valid number for the new pan position")
    if int(tnewpos) > 90 or int(tnewpos) < -90:
        messagebox.showinfo("send", "Enter a valid number for the new tilt position")
    if 90 >= int(tnewpos) >= -90 and 90>= int(anewpos) >= -90:
        messagebox.showinfo("send", f"pan position = {anewpos} tilt position = {tnewpos}")
        info_write("p_target", anewpos)
        info_write("t_target", tnewpos)


    # sending message

    # sending to the arduino
    # including the newpos and the start,stop, pause and autohome


def button_autohome():

    messagebox.showinfo("Auto home", "The Autohoming has begon")
    info_read("autohome")


def info_write(key: str, value: str):
    to_write = f"{key}={value}\r\n".encode()
    print("writing", to_write)
    ser.write(to_write)
    print("readingwrite")
    response = ser.readline().decode()
    if (len(response) == 0):
        raise Exception("Empty response")
    print("ressponse=", response)
    response_split = response.split("=")
    waarde = response_split[0]
    print(waarde)
    return waarde


def info_read(key: str) -> str:
    ser.write(f"{key}?\r\n".encode())
    print("readingread")
    response = ser.readline().decode()
    print("done reading")
    if (len(response)== 0 ):
        raise Exception("Empty response")
    response_split = response.split("=")
    waarde = response_split[0]
    return waarde


# all the buttons
button_start = Button(root, text="Start measurement", padx=45, pady=20, command=button_start)
button_pause = Button(root, text="pause measurement", padx=42, pady=20, command=button_pause)
button_stop = Button(root, text="stop measurement ", padx=44, pady=20, command=button_stop)
button_results = Button(root, text="results         ", padx=60, pady=20, command=button_results)
button_send = Button(root, text="send", padx=100, pady=20, command=button_send)
button_autohome = Button(root, text="Auto home", padx=80, pady=20, command=button_autohome)

# all the text labels
angle_label = Label(root, text="pan", font=("Helvetica", 11), bd=1, relief="sunken", justify="right")
acurrentpos_label = Label(root, text="current position", font=("Helvetica", 11), bd=1, relief="sunken", justify="right")
anewpos_label = Label(root, text="new position", font=("Helvetica", 11), bd=1, relief="sunken", justify="right")
arecived_label = Label(root, text="received", font=("Helvetica", 11), bd=1, relief="sunken", justify="right")
acompleted_label = Label(root, text="completed", font=("Helvetica", 11), bd=1, relief="sunken", justify="right")
tilt_label = Label(root, text="tilt", font=("Helvetica", 11), bd=1, relief="sunken", justify="right")
tcurrentpos_label = Label(root, text="current position", font=("Helvetica", 11), bd=1, relief="sunken", justify="right")
tnewpos_label = Label(root, text="new position", font=("Helvetica", 11), bd=1, relief="sunken", justify="right")
trecived_label = Label(root, text="received", font=("Helvetica", 11), bd=1, relief="sunken", justify="right")
tcompleted_label = Label(root, text="completed", font=("Helvetica", 11), bd=1, relief="sunken", justify="right")



# all the infill boxes
tc = "0" #info_read("tcurrentpos")
ar = "0" #info_read("arecived")
acc = "0" #info_read("acompleted")
ac = "0" #info_read("acurrentpos")
tr = "0" #info_read("trecived")
tcc = "0" #info_read("tcompleted")

acurrentpos_text = Label(root, textvariable= ac, font=("Helvetica", 11), bd=1, relief="sunken",
                         justify="right")
anewpos_text = Text(root, width=5, height=1)
arecived_text = Label(root, textvariable= ar, font=("Helvetica", 11), bd=1, relief="sunken",
                      justify="right")
acompleted_text = Label(root, textvariable= acc, font=("Helvetica", 11), bd=1, relief="sunken",
                        justify="right")
tcurrentpos_text = Label(root, textvariable=tc, font=("Helvetica", 11), bd=1, relief="sunken",
                         justify="right")
tnewpos_text = Text(root, width=5, height=1)
trecived_text = Label(root, textvariable=tr , font=("Helvetica", 11), bd=1, relief="sunken",
                      justify="right")
tcompleted_text = Label(root, textvariable= tcc, font=("Helvetica", 11), bd=1, relief="sunken",
                        justify="right")

# Placing everything on the screen
# buttons on the beginning
button_start.grid(row=1, column=1, columnspan=3)
button_pause.grid(row=2, column=1, columnspan=3)
button_stop.grid(row=3, column=1, columnspan=3)
button_results.grid(row=5, column=1, columnspan=3)

# the logo
picture_label.grid(row=0, column=1, columnspan=2)
# the angle collum
angle_label.grid(row=6, column=1, columnspan=3, ipady=20, ipadx=100)
acurrentpos_label.grid(row=7, column=1, columnspan=3, ipady=20, ipadx=65)
anewpos_label.grid(row=8, column=1, columnspan=3, ipady=20, ipadx=75)
arecived_label.grid(row=9, column=1, columnspan=3, ipady=20, ipadx=95)
acompleted_label.grid(row=10, column=1, columnspan=3, ipady=20, ipadx=85)
acurrentpos_text.grid(row=7, column=2, columnspan=4)
anewpos_text.grid(row=8, column=2, columnspan=4)
arecived_text.grid(row=9, column=2, columnspan=4)
acompleted_text.grid(row=10, column=2, columnspan=4)
# the tilt collum
tilt_label.grid(row=6, column=4, columnspan=4, ipady=20, ipadx=80)
tcurrentpos_label.grid(row=7, column=4, columnspan=4, ipady=20, ipadx=50)
tnewpos_label.grid(row=8, column=4, columnspan=4, ipady=20, ipadx=55)
trecived_label.grid(row=9, column=4, columnspan=4, ipady=20, ipadx=75)
tcompleted_label.grid(row=10, column=4, columnspan=4, ipady=20, ipadx=65)
tcurrentpos_text.grid(row=7, column=7, columnspan=7)
tnewpos_text.grid(row=8, column=7, columnspan=7)
trecived_text.grid(row=9, column=7, columnspan=7)
tcompleted_text.grid(row=10, column=7, columnspan=7)

button_autohome.grid(row=11, column=1, columnspan=3)
button_send.grid(row=12, column=1, columnspan=3)

root.mainloop()
