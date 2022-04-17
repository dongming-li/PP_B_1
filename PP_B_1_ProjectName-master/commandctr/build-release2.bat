cl /MT implant.cpp /subsystem:windows
link implant.obj /map:agent.map /out:agent.exe 
