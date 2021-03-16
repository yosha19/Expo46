from tkinter import   * 
from PIL import ImageTk,Image 



root = Tk()
root.title('interface EXPO 46')
root.iconbitmap('c:/pythonzooi/y.ico')
root.geometry("1000x900")



batlure_img = ImageTk.PhotoImage(Image.open("apodemus.jpg"))
picture_label= Label(image=batlure_img)

def button_start():
	return

def button_pause():
	return

def button_stop():
	return

def button_results():
	return


button_start = Button(root, text = "Start mesurement", padx=45, pady= 20,command = button_start )
button_pause = Button(root, text=  "pause mesurement", padx=42, pady= 20, command = button_pause)
button_stop = Button(root, text=   "stop mesurement ", padx=44, pady= 20,  command = button_stop)
button_results = Button(root, text="results         ", padx=60, pady= 20,command = button_results)


button_start.grid(row = 1, column=1, columnspan= 3 )
button_pause.grid(row = 2, column= 1, columnspan= 3)
button_stop.grid(row = 3, column= 1,columnspan= 3)
button_results.grid(row = 5, column= 1, columnspan= 3)
picture_label.grid( row = 0, column= 1, columnspan= 2)





root.mainloop()