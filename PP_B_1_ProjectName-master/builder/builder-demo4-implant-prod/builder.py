import json
import os
import zipfile

package = zipfile.ZipFile("package.zip", "r")

manifest_raw = package.read("manifest.json")
manifest_json = manifest_raw.decode('utf8')
manifest = json.loads(manifest_json)

agent = package.read("agent.exe")

agent_output = open("agent_output.exe", "wb")
agent_output.write(agent)
agent_output.close()

agent_output = open("agent_output.exe", "r+b")

for variable in manifest["configurables"]:
    print("Name: %s" % variable["name"])
    print("Description: %s" % variable["description"])
    value = input("Value:")

    for relocation in manifest["relocations"]:
        if relocation["varid"] == variable["name"]:
            print(value)
            s = value
            b = bytearray()
            b.extend(map(ord, s))

            agent_output.seek(int(relocation["offset"], 16))
            agent_output.write(b)
   
agent_output.close()
package.close()
