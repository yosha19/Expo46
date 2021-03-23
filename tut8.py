from tkinter import*
from PIL import ImageTk,Image 
from tkinter import messagebox

root = Tk()
root.title('interface EXPO 46')
root.iconbitmap('c:/pythonzooi/y.ico')
root.geometry("1000x900")

batlure_img =ImageTk.PhotoImage(Image.open("apodemus.jpg"))
picture_label=Label(image=batlure_img)

def button_start():
	global Start 
	Start = 1
	Stop = 0
	Pause = 0
	messagebox.showinfo("Start", "The test has started")

def button_pause():
	global Pause 
	Pause = 1
	messagebox.showinfo("Pause", "The test has been paused")

def button_stop():
	global Stop 
	Stop = 1
	Start = 0
	Pause = 0
	messagebox.showinfo("Stop", "The test has stopt")

def button_results():
	messagebox.showinfo("Results", "The results will be showed here")

def button_send():
	messagebox.showinfo("send", "The new posisions have been send")
	#sending to the arduino
	#including the newpos and the start,stop, pause and autohome


def button_autohome():
	global Autohome 
	Autohome = 1
	messagebox.showinfo("Auto home", "The Autohoming has begon")

acurrentpos = StringVar()
acurrentpos.set("0")
tcurrentpos = StringVar()
tcurrentpos.set("0")

arecived = StringVar()
arecived.set("no")
trecived = StringVar()
trecived.set("no")

acompleted = StringVar()
acompleted.set("no")
tcompleted = StringVar()
tcompleted.set("no")

#all the buttons
button_start = Button(root, text = "Start mesurement", padx=45, pady= 20,command = button_start )
button_pause = Button(root, text=  "pause mesurement", padx=42, pady= 20, command = button_pause)
button_stop = Button(root, text=   "stop mesurement ", padx=44, pady= 20,  command = button_stop)
button_results = Button(root, text="results         ", padx=60, pady= 20,command = button_results)
button_send = Button(root, text="send", padx = 100, pady=20, command= button_send)
button_autohome = Button(root, text="Auto home", padx = 80, pady=20, command= button_autohome)

#all the text labels
angle_label = Label(root, text="angle" , font=("Helvetica",11 ), bd = 1, relief = "sunken", justify= "right")
acurrentpos_label = Label(root, text= "current posision", font=("Helvetica",11 ), bd = 1, relief = "sunken", justify= "right")
anewpos_label = Label(root, text="new posision", font=("Helvetica",11 ), bd = 1, relief = "sunken", justify= "right")
arecived_label = Label(root, text="recived", font=("Helvetica",11 ), bd = 1, relief = "sunken", justify= "right")
acompleted_label = Label(root, text="completed", font=("Helvetica",11 ), bd = 1, relief = "sunken", justify= "right")
tilt_label = Label(root, text="tilt", font=("Helvetica",11 ), bd = 1, relief = "sunken", justify= "right")
tcurrentpos_label = Label(root, text= "current posision", font=("Helvetica",11 ), bd = 1, relief = "sunken", justify= "right")
tnewpos_label = Label(root, text="new posision", font=("Helvetica",11 ), bd = 1, relief = "sunken", justify= "right")
trecived_label = Label(root, text="recived", font=("Helvetica",11 ), bd = 1, relief = "sunken", justify= "right")
tcompleted_label = Label(root, text="completed", font=("Helvetica",11 ), bd = 1, relief = "sunken", justify= "right")

#all the infill boxes
acurrentpos_text=  Label(root, textvariable= acurrentpos, font=("Helvetica",11 ), bd = 1, relief = "sunken", justify= "right")
anewpos_text= Text(root, width=5, height= 1)
arecived_text= Label(root, textvariable= arecived, font=("Helvetica",11 ), bd = 1, relief = "sunken", justify= "right")
acompleted_text= Label(root, textvariable= acompleted, font=("Helvetica",11 ), bd = 1, relief = "sunken", justify= "right")
tcurrentpos_text= Label(root, textvariable= tcurrentpos, font=("Helvetica",11 ), bd = 1, relief = "sunken", justify= "right")
tnewpos_text = Text(root, width=5, height= 1)
trecived_text= Label(root, textvariable= trecived, font=("Helvetica",11 ), bd = 1, relief = "sunken", justify= "right")
tcompleted_text= Label(root, textvariable= tcompleted, font=("Helvetica",11 ), bd = 1, relief = "sunken", justify= "right")

#Placing everything on the screen
#buttons on the beginning
button_start.grid(row = 1, column=1, columnspan= 3 )
button_pause.grid(row = 2, column= 1, columnspan= 3)
button_stop.grid(row = 3, column= 1,columnspan= 3)
button_results.grid(row = 5, column= 1, columnspan= 3)

#the logo
picture_label.grid( row = 0, column= 1, columnspan= 2)
#the angle collum
angle_label.grid (row = 6,column= 1, columnspan = 3, ipady= 20 , ipadx= 100)
acurrentpos_label.grid (row = 7,column= 1, columnspan = 3,  ipady= 20 , ipadx= 65)
anewpos_label.grid (row = 8,column= 1, columnspan = 3,  ipady= 20 , ipadx= 75)
arecived_label.grid (row = 9,column= 1, columnspan = 3,  ipady= 20 , ipadx= 95)
acompleted_label.grid (row = 10,column= 1, columnspan = 3,  ipady= 20 , ipadx= 85)
acurrentpos_text.grid(row = 7, column= 2,columnspan=4)
anewpos_text.grid(row = 8, column= 2,columnspan=4)
arecived_text.grid(row = 9, column=2,columnspan=4)
acompleted_text.grid(row = 10, column= 2,columnspan=4)
#the tilt collum
tilt_label.grid (row =6,column= 4, columnspan = 4, ipady= 20 , ipadx= 80)
tcurrentpos_label.grid (row = 7,column= 4, columnspan = 4,  ipady= 20 , ipadx= 50)
tnewpos_label.grid (row = 8,column= 4, columnspan = 4,  ipady= 20 , ipadx= 55)
trecived_label.grid (row =9,column= 4, columnspan = 4,  ipady= 20 , ipadx= 75)
tcompleted_label.grid (row = 10,column= 4, columnspan = 4,  ipady= 20 , ipadx= 65)
tcurrentpos_text.grid(row = 7, column= 7,columnspan=7)
tnewpos_text.grid(row = 8, column= 7,columnspan=7)
trecived_text.grid(row = 9, column= 7,columnspan=7)
tcompleted_text.grid(row = 10, column= 7,columnspan=7)

button_autohome.grid(row = 11, column= 1, columnspan= 3)
button_send.grid(row = 12, column= 1, columnspan= 3)
root.mainloop()
