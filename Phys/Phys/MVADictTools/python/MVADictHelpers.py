# Helper functions to configure complete MVAclassifier tool chains

# TMVA Value (to be used for cutting)
def addTMVAclassifierValue(Component, XMLFile, Variables, ToolName) :
    from Configurables import LoKi__Hybrid__DictValue as DictValue
    Component.addTool(DictValue,ToolName)
    MVAResponse = getattr(Component,ToolName)
    Key = "BDT"
    MVAResponse.Key = Key
    MVAResponse.Source = "LoKi::Hybrid::DictTransform<TMVATransform>/TMVA"
    Options = {
        "XMLFile"    : XMLFile,
        "Name"       : Key,
        "KeepVars"   : "0"}

    from Configurables import LoKi__Hybrid__DictTransform_TMVATransform_ as TMVAtransform
    from Configurables import LoKi__Hybrid__DictOfFunctors

    MVAResponse.addTool(TMVAtransform,"TMVA")
    MVAResponse.TMVA.Options = Options
    MVAResponse.TMVA.Source = "LoKi::Hybrid::DictOfFunctors/MVAdict"
    MVAResponse.TMVA.addTool(LoKi__Hybrid__DictOfFunctors,"MVAdict")
    MVAResponse.TMVA.MVAdict.Variables = Variables
    # end of function

# TMVA tuple (to be added to the HybridTupleTool)
def addTMVAclassifierTuple(Branch, XMLFile, Variables,
                           Name="BDT", Keep=False, Preambulo=[]) :
    from Configurables import LoKi__Hybrid__Dict2Tuple as Dict2Tuple
    Branch.addTupleTool(Dict2Tuple,Name)
    MVAResponse = getattr(Branch,Name)
    #Key = "BDT"
    #MVAResponse.Key = Key

    MVAResponse.Source = "LoKi::Hybrid::DictTransform<TMVATransform>/TMVA"
    Options = {
        "XMLFile"    : XMLFile,
        "Name"       : Name,
        "KeepVars"   : "1" if Keep else "0"}

    from Configurables import LoKi__Hybrid__DictTransform_TMVATransform_ as TMVAtransform
    from Configurables import LoKi__Hybrid__DictOfFunctors

    MVAResponse.addTool(TMVAtransform,"TMVA")
    MVAResponse.TMVA.Options = Options
    MVAResponse.TMVA.Source = "LoKi::Hybrid::DictOfFunctors/MVAdict"
    MVAResponse.TMVA.addTool(LoKi__Hybrid__DictOfFunctors,"MVAdict")
    MVAResponse.TMVA.MVAdict.Preambulo = Preambulo
    MVAResponse.TMVA.MVAdict.Variables = Variables
    # end of function

# Matrixnet Value (to be used for cutting)
def addMatrixnetclassifierValue(Component, MatrixnetFile, Variables, ToolName) :
    from Configurables import LoKi__Hybrid__DictValue as DictValue
    Component.addTool(DictValue,ToolName)
    MVAResponse = getattr(Component,ToolName)
    Key = "BDT"
    MVAResponse.Key = Key
    MVAResponse.Source = "LoKi::Hybrid::DictTransform<MatrixnetTransform>/Matrixnet"
    Options = {
        "MatrixnetFile"    : MatrixnetFile,
        "Name"       : Key,
        "KeepVars"   : "0"}

    from Configurables import LoKi__Hybrid__DictTransform_MatrixnetTransform_ as Matrixnettransform
    from Configurables import LoKi__Hybrid__DictOfFunctors

    MVAResponse.addTool(Matrixnettransform,"Matrixnet")
    MVAResponse.Matrixnet.Options = Options
    MVAResponse.Matrixnet.Source = "LoKi::Hybrid::DictOfFunctors/MVAdict"
    MVAResponse.Matrixnet.addTool(LoKi__Hybrid__DictOfFunctors,"MVAdict")
    MVAResponse.Matrixnet.MVAdict.Variables = Variables
    # end of function

# Matrixnet tuple (to be added to the HybridTupleTool)
def addMatrixnetclassifierTuple(Branch, MatrixnetFile, Variables,
                           Name="BDT", Keep=False, Preambulo=[]) :
    from Configurables import LoKi__Hybrid__Dict2Tuple as Dict2Tuple
    Branch.addTupleTool(Dict2Tuple,Name)
    MVAResponse = getattr(Branch,Name)
    #Key = "BDT"
    #MVAResponse.Key = Key

    MVAResponse.Source = "LoKi::Hybrid::DictTransform<MatrixnetTransform>/Matrixnet"
    Options = {
        "MatrixnetFile"    : MatrixnetFile,
        "Name"       : Name,
        "KeepVars"   : "1" if Keep else "0"}

    from Configurables import LoKi__Hybrid__DictTransform_MatrixnetTransform_ as Matrixnettransform
    from Configurables import LoKi__Hybrid__DictOfFunctors

    MVAResponse.addTool(Matrixnettransform,"Matrixnet")
    MVAResponse.Matrixnet.Options = Options
    MVAResponse.Matrixnet.Source = "LoKi::Hybrid::DictOfFunctors/MVAdict"
    MVAResponse.Matrixnet.addTool(LoKi__Hybrid__DictOfFunctors,"MVAdict")
    MVAResponse.Matrixnet.MVAdict.Preambulo = Preambulo
    MVAResponse.Matrixnet.MVAdict.Variables = Variables
    # end of function

