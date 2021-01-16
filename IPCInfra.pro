TEMPLATE = subdirs

SUBDIRS += \
    Configuration \
    IOInfra \
    UtilitiesInfra \
    TransportInfra \
    Driver


Driver.depends = IOInfra
Driver.depends = TransportInfra
Driver.depends = Configuration
Configuration.depends = UtilitiesInfra
UtilitiesInfra.depends = IOInfra
TransportInfra.depends = IoInfra
TransportInfra.depends = UtilitiesInfra
TransportInfra.depends = Configuration
