
IF(DYNAMIC_OPENSCENEGRAPH)
    ADD_DEFINITIONS(-DOSGMODELING_LIBRARY)
ELSE()
    ADD_DEFINITIONS(-DOSG_LIBRARY_STATIC)
ENDIF()

SET(LIB_NAME osgModeling)
SET(HEADER_PATH ${PROJECT_SOURCE_DIR}/include/${LIB_NAME})
SET(TARGET_H
    ${HEADER_PATH}/Export
    ${HEADER_PATH}/Algorithm
    ${HEADER_PATH}/Curve
    ${HEADER_PATH}/Model
    ${HEADER_PATH}/ModelVisitor
    ${HEADER_PATH}/NormalVisitor
    ${HEADER_PATH}/TexCoordVisitor
    ${HEADER_PATH}/Utilities
    ${HEADER_PATH}/Extrude
    ${HEADER_PATH}/Lathe
    ${HEADER_PATH}/Loft
    ${HEADER_PATH}/Helix
    ${HEADER_PATH}/Bezier
    ${HEADER_PATH}/Nurbs
    ${HEADER_PATH}/Subdivision
    ${HEADER_PATH}/BspTree
    ${HEADER_PATH}/BoolOperator
    ${HEADER_PATH}/PolyMesh
)

# FIXME: For OS X, need flag for Framework or dylib
SET(TARGET_SRC
    Curve.cpp
    ModelVisitor.cpp
    NormalVisitor.cpp
    TexCoordVisitor.cpp
    Utilities.cpp
    Extrude.cpp
    Lathe.cpp
    Loft.cpp
    Helix.cpp
    BezierCurve.cpp
    BezierSurface.cpp
    NurbsCurve.cpp
    NurbsSurface.cpp
    Subdivision.cpp
    BspTree.cpp
    BoolOperator.cpp
    PolyMesh.cpp
    ${OPENSCENEGRAPH_VERSIONINFO_RC}
)

SET(TARGET_LIBRARIES
    osgDB
    osg
    osgUtil
    OpenThreads
)

SETUP_LIBRARY(${LIB_NAME})

