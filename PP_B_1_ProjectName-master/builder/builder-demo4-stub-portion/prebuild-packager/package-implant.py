import json
import os
import pefile
import re
import zipfile

'''
    TODO: Documentation
'''
class VSMapsFileParser:

    def __init__(self, maps_file, pe):
        self.maps_file = maps_file
        self.pe = pe

    def GetVAFromMaps(self, SymName):
        #TODO: Document
        SymName = "_" + SymName
        regex = re.compile(r"\s*([0-9]{4}):([0-9a-f]{8})\s*([^\s]+)\s*([^\s]+)\s*[^\s]*")

        for line in self.maps_file:
            match = regex.match(line) #.decode('utf-8'))
            if match:
                if match.group(3) == SymName:
                    return int(match.group(4), 16)

    def GetFileOffsetFromMaps(self, SymName):
        va = self.GetVAFromMaps(SymName)
        rva = va - pe.OPTIONAL_HEADER.ImageBase
        return self.GetFileOffsetFromRVA(rva)

    def GetFileOffsetFromRVA(self, rva):
        for section in pe.sections:
            if section.contains_rva(rva):
                return section.get_offset_from_rva(rva)


class ConfigurablesParser:

    def __init__(self, configurables_path):
        self.configurables_file = open(configurables_path)
        self.configurables = json.load(configurables_file)

        # TODO: Check configurables header

    def GetBinaryConfVariables(binary):
        print("fuck")


configurables_file = open("configurables.json")
configurables = json.load(configurables_file)

manifest = {}
relocations = []

# Generate Relocations

for binary in configurables["binaries"]:
    pe = pefile.PE(binary["bin"])
    maps_file = open(binary["map"], "r")
    maps = VSMapsFileParser(maps_file, pe)
    for variable in binary["variables"]:
        relocations_entry = {}

        offset = maps.GetFileOffsetFromMaps(variable["name"])

        relocations_entry["id"] = binary["id"]
        relocations_entry["varid"] = variable["name"]
        relocations_entry["offset"] = hex(offset)

        relocations.append(relocations_entry)
    


# END GENERATE RELOCATIONS

# Create Info BLock

info = {}
info["type"] = "manifest"
info["version"] = "1.0"


# End Create Info BLock

manifest["info"] = info
manifest["binaries"]      = configurables["binaries"]
manifest["configurables"] = configurables["configurables"]
manifest["relocations"]   = relocations

# print(json.dumps(manifest, indent=4))

# TODO: Write manifest to package without writing to disk
with open('manifest.json', 'w') as outfile:
    json.dump(manifest, outfile, indent = 4)

package = zipfile.ZipFile("package.zip", "w")

package.write("manifest.json")


for binary in configurables["binaries"]:
    package.write(binary["bin"])

package.close()

