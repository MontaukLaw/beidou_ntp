import tkinter as tk
from datetime import datetime
from tkinter import ttk

# create a window
root = tk.Tk()
root.title("Time")
root.attributes('-fullscreen', True)

screen_width = root.winfo_screenwidth()
screen_height = root.winfo_screenheight()

# create label
# date_label = tk.Label(root, text="", font=("Helvetica", 80))
# date_label.pack(pady=(screen_width - 720) // 2, padx=(screen_height - 1280) // 2)

style = ttk.Style()
style.configure("LargeFont.TLabel", font=("Helvetica", 800))  # 自定义样式
date_label = ttk.Label(root, style="LargeFont.TLabel")
date_label.pack(padx=20, pady=20)

def update_date():
    current_date = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
    date_label.config(text=current_date)
    root.after(1000, update_date) 

update_date()

root.mainloop()
