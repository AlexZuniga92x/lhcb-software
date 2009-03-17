from Gaudi.Configuration import *
from Configurables import CreateHistDBPages
createHistAlg = CreateHistDBPages("createHistAlg")

createHistAlg.PageBase = "/TT"
createHistAlg.HistoBase = "TTNZSAna"

createHistAlg.OutputLevel = INFO

app = ApplicationMgr()
app.TopAlg.append(createHistAlg)
