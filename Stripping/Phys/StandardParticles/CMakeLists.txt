################################################################################
# Package: StandardParticles
################################################################################
gaudi_subdir(StandardParticles v2r3)

gaudi_depends_on_subdirs(Phys/CommonParticles)

find_package(pytools)

gaudi_install_python_modules()

