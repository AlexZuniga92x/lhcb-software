from PyQt4.QtCore import QThread
from PyQt4 import QtCore

"""The RefreshWorker updates the GUI in a background thread"""
class RefreshWorker(QThread):
    def __init__(self, equipDB, confDB, spareDB, devicesInfo, sparesInfo, parentController):
        QThread.__init__(self)
        self.equipDB = equipDB
        self.confDB = confDB
        self.spareDB = spareDB
        self.devicesInfo = devicesInfo
        self.sparesInfo = sparesInfo
        self.parentController = parentController
        self.connect(self, QtCore.SIGNAL("devicesInfoChanged()"), self.parentController.onRefreshDevicesWidget)
        self.connect(self, QtCore.SIGNAL("sparesInfoChanged()"), self.parentController.onRefreshSparesWidget)
    def run(self):
        print "RefreshWorker.run() start"
        equipDBSystem = self.parentController.equipDBSystem
        changedSystem = self.confDB.getChangedSystems(equipDBSystem)
        #devicesInfo = DevicesInfo()
        if self.devicesInfo is not None:
            self.devicesInfo.setNewDevicesWithDHCPData(len(changedSystem[0].getAllDevices()))
            self.devicesInfo.setNewDevicesWithoutDHCPData(len(changedSystem[1].getAllDevices()))
            self.devicesInfo.setChangedDevices(len(changedSystem[2].getAllDevices())+len(changedSystem[3].getAllDevices()))
            self.devicesInfo.setDevicesUpToDate(len(changedSystem[4].getAllDevices()))
            self.emit(QtCore.SIGNAL("devicesInfoChanged()"))
        if self.sparesInfo is not None:
            self.sparesInfo.setSparesInEquipDB = (self.spareDB.getAllSparesEquipDBCount())
            self.sparesInfo.setSparesInConfDB(self.spareDB.getAllSparesConfDBCount())
            self.sparesInfo.setChangedSpares(len(self.spareDB.getChangedSpares()))
            self.sparesInfo.setNewSpares(len(self.spareDB.getNewSpares()))
            self.emit(QtCore.SIGNAL("sparesInfoChanged()"))
        print "RefreshWorker.run() end"