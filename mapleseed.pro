TEMPLATE = subdirs

SUBDIRS += CemuDatabase CemuLibrary CemuCrypto Network Core

Core.depends = CemuDatabase CemuLibrary CemuCrypto Network

CONFIG += ordered