Import("env")

try:
    import minify_html
except ImportError:
    env.Execute("$PYTHONEXE -m pip install minify_html")
