TEMPLATE = subdirs

SUBDIRS += Core CemuDB CemuLibrary Network CemuCrypto

Core.depends = CemuDB
Core.depends = CemuLibrary
Core.depends = Network
Core.depends = CemuCrypto
