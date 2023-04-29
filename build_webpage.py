import os
import gzip

HEADER = """#ifndef VELLEZ_ESP8266_WEBPAGE_H
#define VELLEZ_ESP8266_WEBPAGE_H

"""

FOOTER = """

#endif // VELLEZ_ESP8266_WEBPAGE_H
"""

current_dir = os.getcwd()
src = os.path.join(current_dir, "web/index.html")
dst = os.path.join(current_dir, "", "src", "webpage.h")
with open(src, "rb") as src_f:
    compressed = gzip.compress(src_f.read())
with open(dst, "w") as dst_f:
    dst_f.write(HEADER)
    value = ",".join("0x%02X" % x for x in compressed)
    dst_f.write("#define GZIPPED_WEBPAGE {%s}" % value)
    dst_f.write(FOOTER)
