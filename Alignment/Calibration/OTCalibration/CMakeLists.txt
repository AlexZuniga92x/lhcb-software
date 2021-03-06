################################################################################
# Package: OTCalibration
################################################################################
gaudi_subdir(OTCalibration v1r12)

gaudi_depends_on_subdirs(Event/TrackEvent
                         Event/DAQEvent
                         GaudiAlg
                         Tr/TrackFitEvent
			 Alignment/AlignKernel
			 OT/OTDAQ)

find_package(GSL)
find_package(Boost)
find_package(ROOT COMPONENTS Hist Tree Matrix Graf Minuit)

gaudi_add_module(OTCalibration
                 src/*.cpp
                 INCLUDE_DIRS GSL ROOT
                 LINK_LIBRARIES Boost GSL ROOT DAQEventLib TrackEvent GaudiAlgLib TrackFitEvent AlignKernel)

gaudi_install_python_modules()
