git describe --tags --dirty | awk '{print "#define VERSION " "\""$1"\"" }' 
