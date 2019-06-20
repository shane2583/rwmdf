# -*- coding: utf-8 -*-

# pylint: disable=invalid-name
# pylint: disable=missing-docstring

# from PySide2.QtWidgets import QMainWindow
# from PyQt5.QtCore import QAbstractItemModel
from PyQt5.QtWidgets import QMainWindow, QHeaderView, QAbstractItemView
from PyQt5.QtGui import QStandardItemModel, QStandardItem
from PyQt5.QtCore import QStringListModel
# import numpy as np
from asammdf import MDF, Signal
import ui_mainwin as mainwin


class MainWin(QMainWindow, mainwin.Ui_MainWindow):

    def __init__(self):
        super(MainWin, self).__init__()
        self.setupUi(self)
        self.init()

    def init(self):
        # self.treeView.setFixedWidth(80)
        # self.treeView.setMaximumWidth(200)
        # self.listView.setMaximumHeight(100)
        # self.treeView.width = 30
        # self.listView.height = 50
        # self.graphicsView.setFixedSize(400,300)
        self.init_tree()

    def plot(self,sig):
        self.graphicsView.plotItem.clear()
        self.graphicsView.plotItem.plot(sig.samples, pen='r')
        # x = np.random.random(50)
        # pItem = self.graphicsView.addItem(title='graph view')
        # pItem.plot(x)

    def init_tree(self):
        # signals = ["EngineSpeed","VehSpeed","OutputSpeedAccel"]
        self.__model = QStandardItemModel(self.treeView)
        self.__model.setHorizontalHeaderLabels(["Name","Discription"])
        self.__rwmdf = MDF(r'D:/TestData/DataSpySampleDataFile.mf4')
        
        chs = self.__rwmdf.channels_db
        for _ch in chs.keys():
            sig = self.__rwmdf.get(_ch)
            cmt = sig.comment
            cmt = "" if cmt.find("\n") != -1 else cmt
            # self.__model.appendRow([QStandardItem(sig.name),QStandardItem("")])
            self.__model.insertRow(0, QStandardItem(sig.name))
            self.__model.insertRow(1, QStandardItem(cmt))
        
        self.treeView.setModel(self.__model)
        self.treeView.setEditTriggers(QAbstractItemView.NoEditTriggers)
        self.treeView.setSortingEnabled(True)
        self.treeView.header().setSectionResizeMode(QHeaderView.ResizeToContents)

        self.init_list_view([])

    def init_list_view(self,lst):
        self.__lstmodel = QStringListModel()
        _sig = self.__rwmdf.get('EngineSpeed')
        _lst = []
        for val in _sig.samples:
            _lst.append(str(val))
        self.__lstmodel.setStringList(_lst)
        self.listView.setModel(self.__lstmodel)

    def on_tree_view_db_clicked(self, q_index):
        try:
            print("on_tree_view_db_clicked...")
            # q_it = self.__model.itemFromIndex(q_index)
            # print(q_index.column(), q_it.text())
            # if q_index.column() == 0:
            #     q_it = self.__model.itemFromIndex(q_index)
            #     sig = self.__rwmdf._mdf.get(q_it.text())
            #     if sig:
            #         self.plot(sig)
                    # self.initListView(sig.samples)
        except Exception as err:
            print(err)
