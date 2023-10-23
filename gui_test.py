import tkinter as tk
from tkinter import ttk

def get_started():
    NewWindow(home_window)


class NewWindow(tk.Toplevel):
    def __init__(self, master=None):
        super().__init__(master=master)
        self.title("Information")
        self.geometry("600x600")
        label = ttk.Label(self, text="Enter your information")
        label.pack()


home_window = tk.Tk()
home_window.title("Rota System")
home_window.geometry("600x600")

welcome = ttk.Label(home_window, text="Rota System")
welcome.config(font=("Courier", 30))
welcome.pack(side="top", pady=10)

byTom = ttk.Label(home_window, text="Created by Tom")
byTom.config(font=("Courier", 10))
byTom.pack(side="top", pady=10)

# Use `tk.Button` instead of `ttk.Button`
getStarted = tk.Button(home_window, text="Click here to get started",
                        command=get_started)
getStarted.config(font=("Courier", 15))
getStarted.place(x=25, y=500, height=50, width=550)

home_window.mainloop()