
# Create index.html.h from index.html
with open("src/index.html.h", "w") as f, open("src/index.html", "r") as f2:
    f.write('''// THIS IS AN AUTOGENERATED FILE. DO NOT EDIT. CREATED BY ../preprocess.py

#include <avr/pgmspace.h>
const char INDEX_HTML[] PROGMEM = R"=====(

''')
    for line in f2:
        f.write(line)
    f.write('\n\n\n)=====";')

