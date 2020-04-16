TEMPLATE = subdirs

SUBDIRS += \
    IOInfra \
    UtilitiesInfra \
    TransportInfra \
    Driver


Driver.depends = IOInfra
Driver.depends = TransportInfra
UtilitiesInfra.depends = IOInfra
TransportInfra.depends = IoInfra
TransportInfra.depends = UtilitiesInfra
