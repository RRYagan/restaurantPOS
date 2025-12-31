qt_add_qml_module(POS_Components
    URI "POS.Components"
    VERSION 1.0
    SOURCES
        # SalesModel.h SalesModel.cpp
    QML_FILES
        # SalesScreen.qml
)

# Link to core logic so the SalesModel can call OrderService
target_link_libraries(POS_ComponentsPRIVATE Qt6::Quick pos_core)
