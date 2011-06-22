import os
import fnmatch
import tempfile

for f in os.listdir('.'):
    if fnmatch.fnmatch(f, '*.c') or fnmatch.fnmatch(f, '*.h'):
        newfilename = f + '.' + tempfile.gettempprefix()
        if os.fork() == 0:
            os.execl('./replacer', 'replacer', 'dict.txt', f, newfilename)

