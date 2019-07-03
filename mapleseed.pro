TEMPLATE = subdirs

SUBDIRS += CemuDB CemuLibrary CemuCrypto Network Core

Core.depends = CemuDB CemuLibrary CemuCrypto Network

# where to find the sub projects - give the folders
CemuDB.subdir  = ./CemuDB
CemuLibrary.subdir  = ./CemuLibrary
CemuCrypto.subdir  = ./CemuCrypto
Network.subdir  = ./Network
Core.subdir = ./Core

CONFIG += ordered