CONFIG -= qt

TEMPLATE = lib
DEFINES += IIO_LIBRARY

CONFIG += c++11

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

INCLUDEPATH += include/ include/drivers/

SOURCES += \
    src/AnalogIn.cpp \
    src/AnalogOut.cpp \
    src/Channel.cpp \
    src/DigitalIn.cpp \
    src/DigitalOut.cpp \
    src/EncoderCounter.cpp \
    src/HTTPClient.cpp \
    src/HTTPScript.cpp \
    src/HTTPServer.cpp \
    src/HighpassFilter.cpp \
    src/LowpassFilter.cpp \
    src/Module.cpp \
    src/Mutex.cpp \
    src/RealtimeThread.cpp \
    src/Thread.cpp \
    src/Timer.cpp \
    src/XMLParser.cpp \
    src/drivers/AdvantechPCIe1680.cpp \
    src/drivers/BeagleBone.cpp \
    src/drivers/BeckhoffBK5151.cpp \
    src/drivers/BeckhoffEL1000.cpp \
    src/drivers/BeckhoffEL2000.cpp \
    src/drivers/BeckhoffEL3102.cpp \
    src/drivers/BeckhoffEL3104.cpp \
    src/drivers/BeckhoffEL3255.cpp \
    src/drivers/BeckhoffEL4004.cpp \
    src/drivers/BeckhoffEL4732.cpp \
    src/drivers/BeckhoffEL5101.cpp \
    src/drivers/BeckhoffEL7332.cpp \
    src/drivers/BeckhoffEL7342.cpp \
    src/drivers/CAN.cpp \
    src/drivers/CANMessage.cpp \
    src/drivers/CANopen.cpp \
    src/drivers/CoE.cpp \
    src/drivers/DS406Encoder.cpp \
    src/drivers/ElmoWhistle.cpp \
    src/drivers/EtherCAT.cpp \
    src/drivers/Ethernet.cpp \
    src/drivers/GrivixAutoCharge.cpp \
    src/drivers/HetronicRC.cpp \
    src/drivers/HokuyoUST10LX.cpp \
    src/drivers/IMSServocontroller.cpp \
    src/drivers/Intel82541.cpp \
    src/drivers/Intel82574.cpp \
    src/drivers/MaxonEPOS4.cpp \
    src/drivers/MaxonIDX.cpp \
    src/drivers/Mecca500.cpp \
    src/drivers/PCANpci.cpp \
    src/drivers/PCANpcie.cpp \
    src/drivers/PCI.cpp \
    src/drivers/PhoenixCanBK.cpp \
    src/drivers/RPLidar.cpp \
    src/drivers/RPLidarA2.cpp \
    src/drivers/RtelligentECR60.cpp \
    src/drivers/SMCServoJXCE1.cpp \
    src/drivers/SchneiderAltivar31.cpp \
    src/drivers/SchneiderTesysT.cpp \
    src/drivers/Serial.cpp \
    src/drivers/SocketCAN.cpp \
    src/drivers/SpaceMouseWireless.cpp \
    src/drivers/SpaceNavigator.cpp \
    src/drivers/SpaceTraveler.cpp \
    src/drivers/TPMC901.cpp

HEADERS += \
    include/AnalogIn.h \
    include/AnalogOut.h \
    include/Channel.h \
    include/DigitalIn.h \
    include/DigitalOut.h \
    include/EncoderCounter.h \
    include/HTTPClient.h \
    include/HTTPScript.h \
    include/HTTPServer.h \
    include/HighpassFilter.h \
    include/LowpassFilter.h \
    include/Module.h \
    include/Mutex.h \
    include/RealtimeThread.h \
    include/Thread.h \
    include/Timer.h \
    include/XMLParser.h \
    include/drivers/AdvantechPCIe1680.h \
    include/drivers/BeagleBone.h \
    include/drivers/BeckhoffBK5151.h \
    include/drivers/BeckhoffEL1000.h \
    include/drivers/BeckhoffEL2000.h \
    include/drivers/BeckhoffEL3102.h \
    include/drivers/BeckhoffEL3104.h \
    include/drivers/BeckhoffEL3255.h \
    include/drivers/BeckhoffEL4004.h \
    include/drivers/BeckhoffEL4732.h \
    include/drivers/BeckhoffEL5101.h \
    include/drivers/BeckhoffEL7332.h \
    include/drivers/BeckhoffEL7342.h \
    include/drivers/CAN.h \
    include/drivers/CANMessage.h \
    include/drivers/CANopen.h \
    include/drivers/CoE.h \
    include/drivers/DS406Encoder.h \
    include/drivers/ElmoWhistle.h \
    include/drivers/EtherCAT.h \
    include/drivers/Ethernet.h \
    include/drivers/GrivixAutoCharge.h \
    include/drivers/HetronicRC.h \
    include/drivers/HokuyoUST10LX.h \
    include/drivers/IMSServocontroller.h \
    include/drivers/Intel82541.h \
    include/drivers/Intel82574.h \
    include/drivers/MaxonEPOS4.h \
    include/drivers/MaxonIDX.h \
    include/drivers/Mecca500.h \
    include/drivers/PCANpci.h \
    include/drivers/PCANpcie.h \
    include/drivers/PCI.h \
    include/drivers/PhoenixCanBK.h \
    include/drivers/RPLidar.h \
    include/drivers/RPLidarA2.h \
    include/drivers/RtelligentECR60.h \
    include/drivers/SMCServoJXCE1.h \
    include/drivers/SchneiderAltivar31.h \
    include/drivers/SchneiderTesysT.h \
    include/drivers/Serial.h \
    include/drivers/SocketCAN.h \
    include/drivers/SpaceMouseWireless.h \
    include/drivers/SpaceNavigator.h \
    include/drivers/SpaceTraveler.h \
    include/drivers/TPMC901.h

# Default rules for deployment.
unix {
    target.path = /usr/lib
}
!isEmpty(target.path): INSTALLS += target

DISTFILES +=
