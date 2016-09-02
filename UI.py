import matplotlib

matplotlib.use("TkAgg")

from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg, NavigationToolbar2TkAgg
from matplotlib.figure import Figure
import matplotlib.animation as animation
from matplotlib import style

import urllib
import json
import pandas as pd
import numpy as np

import Tkinter as tk
import ttk
from Tkinter import *

LARGE_FONT = ["Verdana", 12]
style.use("ggplot")

f = Figure(figsize=(5, 5), dpi=100)
a = f.add_subplot(111)


def animate(i):
    pullData = open("sampleData.txt", "r").read()
    dataList = pullData.split('\n')
    xlist = []
    ylist = []
    for eachLine in dataList:
        if len(eachLine) > 1:
            x, y = eachLine.split(',')
            xlist.append(int(x))
            ylist.append(int(y))
    a.clear()
    a.plot(xlist, ylist)


class GuiForDRMD(tk.Tk):
    def __init__(self, *args, **kwargs):
        tk.Tk.__init__(self, *args, **kwargs)
        # tk.Tk.iconbitmap(self, default="pineapple.bmp")
        tk.Tk.wm_title(self, "DRMD")

        container = tk.Frame(self)
        container.pack(side="top", fill="both", expand=True)
        container.grid_rowconfigure(0, weight=1)
        container.grid_columnconfigure(0, weight=1)

        self.frames = {}

        for f in (StartPage, runExperiment, CreateExperiment):
            frame = f(container, self)

            self.frames[f] = frame

            frame.grid(row=0, column=0, sticky="nsew")

        self.show_frame(StartPage)

    def show_frame(self, cont):
        frame = self.frames[cont]
        frame.tkraise()


class StartPage(tk.Frame):
    def __init__(self, parent, controller):
        tk.Frame.__init__(self, parent)
        label = tk.Label(self, text="welcome to DRMD", font=LARGE_FONT)
        label.pack(pady=10, padx=10)

        button1 = ttk.Button(self, text="Create new experiment",
                             command=lambda: controller.show_frame(CreateExperiment))
        button1.pack()

        button2 = ttk.Button(self, text="Use existing ",
                             command=quit)
        button2.pack()

runstate = DISABLED

class CreateExperiment(tk.Frame):
    def __init__(self, parent, controller):
        tk.Frame.__init__(self, parent)

        nameFrame = Frame(self)
        nameFrame.pack()
        name = ""
        label = ttk.Label(nameFrame, text="Experiment Name", font=LARGE_FONT)
        label.pack(pady=10, padx=10, side=LEFT)
        E1 = ttk.Entry(nameFrame, textvariable=name, state=NORMAL)
        E1.pack(side=RIGHT)

        timeFrame = Frame(self)
        timeFrame.pack(side=RIGHT)
        L1 = ttk.Label(timeFrame, text="Run Time", font=LARGE_FONT)
        L1.pack(pady=10, padx=10, side=LEFT)
        timeOp = StringVar()
        timeOp.set("10 hrs")
        times = ["30 Min", "45 Min", "1 hrs", "1.5 hrs", "2 hrs", "2.5 hrs", "3 hrs", "3.5 hrs",
                 "4 hrs", "5 hrs", "6 hrs", "7.5 hrs", "10 hrs", "12 hrs", "18 hrs", "24 hrs", "36 hrs",
                 "48 hrs", "72 hrs"]
        timeDropDown = OptionMenu(timeFrame, timeOp, *times).pack(side=LEFT)

        sampleFrame = Frame(self)
        sampleFrame.pack(side=LEFT)
        L2 = ttk.Label(sampleFrame, text="How often samples are taken", font=LARGE_FONT)
        L2.pack(pady=10, padx=10, side=LEFT)
        sampleRate = StringVar()
        sampleRate.set("15 Sec")
        rates = ["5 Sec", "10 Sec", "15 Sec", "20 Sec", "30 Sec", "45 Sec", "1 Min", "1.5 Min",
                 "2 Min", "2.5 Min", "3 Min", "5 Min", "6 Min", "7.5 Min", "10 Min", "12 Min", "15 Min",
                 "30 Min"]
        ratesDropDown = OptionMenu(sampleFrame, sampleRate, *rates).pack(side=RIGHT)

        button1 = tk.Button(self, text="cancel",
                            command=lambda: controller.show_frame(StartPage))
        button1.pack(side=BOTTOM)

        button2 = tk.Button(self, text="Create Experment",
                            command=lambda: self.createFile(name, self.gettime(timeOp.get()), self.gettime(sampleRate.get()) ))
        button2.pack(side=BOTTOM)

        button3 = tk.Button(self, text="run Experment",state=runstate,
                            command=lambda: controller.show_frame(runExperiment))
        button3.pack(side=BOTTOM)


    def createFile(self, name, time, sRate):
        file = open(name+".txt", "w")
        data = name + "/n" + time + "/n" + sRate
        file.write(data)
        file.close()
        runstate = ACTIVE


    def gettime(self, strg):
        q = strg.split(' ')
        if q[1] is "sec":
            return q[0]
        elif q[1] is "Min":
            return q[0] * 60
        elif q[1] is "hrs":
            return q[0] * 60 * 60
        else:
            return "error"


class runExperiment(tk.Frame):
    def __init__(self, parent, controller):
        tk.Frame.__init__(self, parent)
        label = ttk.Label(self, text="Run Exper", font=LARGE_FONT)
        label.pack(pady=10, padx=10)

        button1 = tk.Button(self, text="BAclk to home ",
                            command=lambda: controller.show_frame(StartPage))
        button1.pack()

        canvas = FigureCanvasTkAgg(f, self)
        canvas.show()
        canvas.get_tk_widget().pack(side=tk.TOP, fill=tk.BOTH, expand=True)

        toolbar = NavigationToolbar2TkAgg(canvas, self)
        toolbar.update()
        canvas._tkcanvas.pack(side=tk.TOP, fill=tk.BOTH, expand=True)


app = GuiForDRMD()
ani = animation.FuncAnimation(f, animate, interval=1000)
app.mainloop()
