################################################################################
# Package: DecayTreeTupleHerschel
################################################################################
gaudi_subdir(DecayTreeTupleHerschel v1r2)
 
gaudi_depends_on_subdirs(Phys/DecayTreeTupleBase)
 
gaudi_add_module(DecayTreeTupleHerschel
                 src/*.cpp
                 LINK_LIBRARIES DecayTreeTupleBaseLib) 
