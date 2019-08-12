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
open62541.subdir              = $$PWD/libs/QUaServer.git/src/amalgamation
qadvanceddocking.subdir       = $$PWD/libs/QAdvancedDocking.git/src
01_basics.subdir              = $$PWD/examples/01_basics
02_xml.subdir                 = $$PWD/examples/02_xml
03_quausertable.subdir        = $$PWD/examples/03_quausertable
04_quaroletable.subdir        = $$PWD/examples/04_quaroletable
05_quapermissionstable.subdir = $$PWD/examples/05_quapermissionstable
06_quaacfullui.subdir         = $$PWD/examples/06_quaacfullui
# dependencies
01_basics.depends              = open62541
02_xml.depends                 = open62541
03_quausertable.depends        = open62541
04_quaroletable.depends        = open62541
05_quapermissionstable.depends = open62541
06_quaacfullui.depends         = open62541 qadvanceddocking