################################################################################
# Package: PatAlgorithms
################################################################################
gaudi_subdir(PatAlgorithms v5r0)

gaudi_depends_on_subdirs(Det/OTDet
                         Det/STDet
                         Event/RecEvent
                         Event/TrackEvent
                         Kernel/PartProp
                         OT/OTDAQ
                         Tf/PatKernel
                         Tf/TfKernel
                         Tr/TrackInterfaces
                         Tr/TrackTools)

find_package(Boost)

gaudi_add_module(PatAlgorithms
                 src/*.cpp
                 INCLUDE_DIRS OT/OTDAQ Tf/PatKernel Tf/TfKernel Tr/TrackInterfaces
                 LINK_LIBRARIES OTDetLib STDetLib RecEvent TrackEvent PartPropLib)

gaudi_install_python_modules()

