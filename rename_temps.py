import os
import fnmatch

for f in os.listdir('.'):
    if fnmatch.fnmatch(f, '*.h') or fnmatch.fnmatch(f, '*.c'):
        os.remove(f)

for f in os.listdir('.'):
    if fnmatch.fnmatch(f, '*.tmp'):
        head, tail = os.path.splitext(f)
        os.rename(f, head)

