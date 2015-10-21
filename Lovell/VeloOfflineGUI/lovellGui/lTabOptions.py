from PyQt4.QtCore import *
from PyQt4.QtGui import *
from lFuncs import *
import lInterfaces

class lTabOpsState():
	def __init__(self, moduleID, displayRefs, overLayRef, refDiff, refRatio, 
		runNum, refRunNum, run_data_dir):
		self.run_data_dir = run_data_dir
		self.moduleID = moduleID
		self.displayRefs = displayRefs
		self.overLayRef = overLayRef
		self.refDiff = refDiff
		self.refRatio = refRatio
		self.runNum = runNum
		self.refRunNum = refRunNum


	def outline(self):
		print "Module:", self.moduleID
		print "Display refs:", self.displayRefs
		print "Overlaying refs:", self.overLayRef
		print "Reference diff:", self.refDiff
		print "Reference ratio:", self.refRatio
		print "Run number:", self.runNum
		print "Ref run number:", self.refRunNum


class lTabOptions(QGroupBox):
	state_change = pyqtSignal()
	def __init__(self, parent, run_data_dir):
		self.run_data_dir = run_data_dir
		QGroupBox.__init__(self, "Global Tab Options", parent)
		self.grid_layout = QGridLayout(self)
		title = QLabel('LovellGUI')
		font = QFont()
		font.setPointSize(20)
		font.setBold(True)
		title.setFont(font)

		self.grid_layout.addWidget(title, self.rowCount(), 0, 1, 2)
		setPadding(self.grid_layout)
		self.add_group_selector()
		self.add_ID_selector()
		self.add_next_prev_buttons()
		self.add_reference_buttons()
		self.add_run_num_buttons()


	def add_group_selector(self):
		self.module_group_box = QComboBox(self)
		self.module_group_box.currentIndexChanged.connect(self.state_changed)
		self.grid_layout.addWidget(QLabel("Module Group:"), self.rowCount(), 0)
		self.grid_layout.addWidget(self.module_group_box, self.rowCount()-1, 1)
		self.module_group_box.addItem("R (0)")
		self.module_group_box.addItem("Phi (1)")

	
	def add_ID_selector(self):
		self.module_ID_box = QComboBox(self)
		self.module_ID_box.currentIndexChanged.connect(self.state_changed)
		self.grid_layout.addWidget(QLabel("Module ID:"), self.rowCount(), 0)
		self.grid_layout.addWidget(self.module_ID_box, self.rowCount()-1, 1)
		for i in range(0, 42): self.module_ID_box.addItem(str(i))
		for i in range(64, 106): self.module_ID_box.addItem(str(i))


	def add_next_prev_buttons(self):
		self.next_button = QPushButton("Next")
		self.grid_layout.addWidget(self.next_button, self.rowCount(), 0)
		self.prev_button = QPushButton("Prev")
		self.grid_layout.addWidget(self.prev_button, self.rowCount()-1, 1)
		self.next_button.clicked.connect(self.next_clicked)
		self.prev_button.clicked.connect(self.prev_clicked)


	def add_run_num_buttons(self):
		self.add_bar()
		self.run_num_box = QComboBox(self)
		self.run_num_box.currentIndexChanged.connect(self.state_changed)
		self.grid_layout.addWidget(QLabel("Run number:"), self.rowCount(), 0, 1, 2)
		self.grid_layout.addWidget(self.run_num_box, self.rowCount(), 0, 1, 2)

		# Reference
		self.run_numRef_box = QComboBox(self)
		self.run_numRef_box.currentIndexChanged.connect(self.state_changed)
		self.grid_layout.addWidget(QLabel("Reference number:"), self.rowCount(), 0, 1, 2)
		self.grid_layout.addWidget(self.run_numRef_box, self.rowCount(), 0, 1, 2)
		
		self.run_numRef_box.addItem('Auto')
		for run in lInterfaces.run_list(self.run_data_dir):
			self.run_num_box.addItem(str(run))
			self.run_numRef_box.addItem(str(run))


	def add_bar(self):
		bar = QLabel('_______________')
		bar.setAlignment(Qt.AlignHCenter)
		bar.setAlignment(Qt.AlignVCenter)
		self.grid_layout.addWidget(bar, self.rowCount(), 0, 1, 2)


	def add_reference_buttons(self):
		self.add_bar()
		self.showing_ref_box = QCheckBox("Display References")
		self.showing_ref_box.setChecked(False)
		self.overlay_ref_box = QRadioButton("Overlay", self)
		self.overlay_ref_box.setChecked(True)
		self.overlay_refDiff_box = QRadioButton("Data - Ref", self)
		self.overlay_refRatio_box = QRadioButton("Data/Ref", self)

		self.grid_layout.addWidget(self.showing_ref_box, self.rowCount(), 0, 1, 2)
		self.grid_layout.addWidget(self.overlay_ref_box, self.rowCount(), 0, 1, 2)
		self.grid_layout.addWidget(self.overlay_refDiff_box, self.rowCount(), 0, 1, 2)
		self.grid_layout.addWidget(self.overlay_refRatio_box, self.rowCount(), 0, 1, 2)

		# Slots.
		self.showing_ref_box.clicked.connect(self.state_changed)
		self.overlay_ref_box.clicked.connect(self.state_changed)
		self.overlay_refDiff_box.clicked.connect(self.state_changed)
		self.overlay_refRatio_box.clicked.connect(self.state_changed)


	def rowCount(self):
		return self.grid_layout.rowCount()


	def state_changed(self):
		self.state_change.emit()


	def state(self):
		g = self.module_group_box.currentIndex()
		moduleID = g*self.module_ID_box.count() + self.module_ID_box.currentIndex()
		state = lTabOpsState(moduleID, self.showing_ref_box.isChecked(), self.overlay_ref_box.isChecked(), 
			self.overlay_refDiff_box.isChecked(), self.overlay_refRatio_box.isChecked(), 
			self.run_num_box.currentText(), self.run_numRef_box.currentText(), self.run_data_dir) 
		return state


	def next_clicked(self):
		currentGroup = self.module_group_box.currentIndex()
		currentModule = self.module_ID_box.currentIndex()

		# Check if the last one.
		if currentModule != self.module_ID_box.count() - 1:
			self.module_ID_box.setCurrentIndex(currentModule + 1)
		else:
			if currentGroup != self.module_group_box.count() - 1:
				self.module_group_box.setCurrentIndex(currentGroup + 1)
			else: 
				self.module_group_box.setCurrentIndex(0)
			self.module_ID_box.setCurrentIndex(0)


	def prev_clicked(self):
		currentGroup = self.module_group_box.currentIndex()
		currentModule = self.module_ID_box.currentIndex()

		# Check if the first one.
		if currentModule != 0:
			self.module_ID_box.setCurrentIndex(currentModule - 1)
		else:
			if currentGroup != 0:
				self.module_group_box.setCurrentIndex(currentGroup - 1)
			else: 
				self.module_group_box.setCurrentIndex(self.module_group_box.count()-1)
			self.module_ID_box.setCurrentIndex(self.module_ID_box.count()-1)