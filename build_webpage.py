import os
import gzip
import minify_html

HEADER = """#ifndef VELLEZ_ESP8266_WEBPAGE_H
#define VELLEZ_ESP8266_WEBPAGE_H

"""

FOOTER = """

#endif // VELLEZ_ESP8266_WEBPAGE_H
"""

current_dir = os.getcwd()
src = os.path.join(current_dir, "web/index.html")
dst = os.path.join(current_dir, "", "src", "webpage.h")
with open(src, "r") as src_f:
    minified = minify_html.minify(src_f.read(), minify_css=True, minify_js=True)
    compressed = gzip.compress(minified.encode("utf-8"))
with open(dst, "w") as dst_f:
    dst_f.write(HEADER)
    value = ",".join("0x%02X" % x for x in compressed)
    dst_f.write("#define GZIPPED_WEBPAGE {%s}" % value)
    dst_f.write(FOOTER)
