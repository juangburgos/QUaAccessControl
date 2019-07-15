TEMPLATE = subdirs

CONFIG += debug_and_release build_all

SUBDIRS = \
$$PWD/../libs/QUaServer.git/src/amalgamation/open62541.pro \
$$PWD/01_basics/01_basics.pro \
$$PWD/02_xml/02_xml.pro \
$$PWD/03_quausertable/03_quausertable.pro
