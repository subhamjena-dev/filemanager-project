savedcmd_filelogger.mod := printf '%s\n'   filelogger.o | awk '!x[$$0]++ { print("./"$$0) }' > filelogger.mod
