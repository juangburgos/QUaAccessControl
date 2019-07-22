TEMPLATE = subdirs

CONFIG += debug_and_release build_all
# names
SUBDIRS = \
open62541 \
qadvanceddocking \
01_basics \
02_xml \
03_quausertable \
04_quaroletable \
05_quapermissionstable \
06_quaacfullui
# directories
open62541.subdir              = $$PWD/../libs/QUaServer.git/src/amalgamation/open62541.pro
qadvanceddocking.subdir       = $$PWD/../libs/QAdvancedDocking.git/src/src.pro
01_basics.subdir              = $$PWD/01_basics/01_basics.pro
02_xml.subdir                 = $$PWD/02_xml/02_xml.pro
03_quausertable.subdir        = $$PWD/03_quausertable/03_quausertable.pro
04_quaroletable.subdir        = $$PWD/04_quaroletable/04_quaroletable.pro
05_quapermissionstable.subdir = $$PWD/05_quapermissionstable/05_quapermissionstable.pro
06_quaacfullui.subdir         = $$PWD/06_quaacfullui/06_quaacfullui.pro
# dependencies
01_basics.depends              = open62541
02_xml.depends                 = open62541
03_quausertable.depends        = open62541
04_quaroletable.depends        = open62541
05_quapermissionstable.depends = open62541
06_quaacfullui.depends         = open62541 qadvanceddocking