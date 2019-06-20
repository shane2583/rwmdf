# -*- coding: utf-8 -*-
# pylint: disable=invalid-name
# pylint: disable=missing-docstring

import sys
from mainwin import MainWin
# from PyQt5 import QtCore, QtGui, QtWidgets
from PyQt5.QtWidgets import QApplication
# import pandas as pd
import threading
import time
# import numpy as np
# import pyqtgraph as pg
# import pyqtgraph.ptime as ptime


# from pyqtgraph.Qt import QtGui, QtCore, USE_PYSIDE, USE_PYQT5
# import pyqtgraph.examples
# pyqtgraph.examples.run()

# thread =  threading.Event()

def _running(i):
    time.sleep(0.001)
    app = QApplication(sys.argv)
    win = MainWin()
    win.setWindowTitle('MDF Reader')
    win.show()
    app.exec_()
    print("MainWin is showing...")


if __name__ == '__main__':
    app = QApplication(sys.argv)
    # th = threading.Thread(target=_running, args="3")
    # th.start()
    win = MainWin()
    win.setWindowTitle('MDF Reader')
    win.show()
    sys.exit(app.exec_())

# class MainWidget(QtWidgets.QMainWindow):
#     def __init__(self):
#         super().__init__()
#         self.setWindowTitle("上证指数收盘价历史走势|州的先生zmister.com") # 设置窗口标题

#         main_widget = QtWidgets.QWidget()           # 实例化一个widget部件
#         main_layout = QtWidgets.QGridLayout()       # 实例化一个网格布局层
#         main_widget.setLayout(main_layout)          # 设置主widget部件的布局为网格布局
#         x = np.random.random(50)
#         a = np.random.random(8)
#         pw = pg.PlotWidget()                        # 实例化一个绘图部件
#         pw.plot(x)                                  # 在绘图部件中绘制折线图
#         main_layout.addWidget(pw)                   # 添加绘图部件到网格布局层
        
#         self.setCentralWidget(main_widget)          # 设置窗口默认部件为主widget


# if __name__ == '__main__':
#     app = QtWidgets.QApplication(sys.argv)
#     gui = MainWidget()
#     gui.show()
#     sys.exit(app.exec_())

# python3 -m PyQt5.uic.pyuic main_window.ui -o main_window.py 
# 或者
# pyuic5 main_window.ui -o main_window.py
