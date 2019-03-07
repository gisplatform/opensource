cmake_minimum_required( VERSION 2.6 )


# Target.
add_library(gpqtrunner SHARED gp-qtrunner.cpp )
target_link_libraries(gpqtrunner QtCore QtGui)

