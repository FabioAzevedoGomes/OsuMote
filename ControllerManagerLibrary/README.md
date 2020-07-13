
# Controller manager library
This is the code for the library that handles wiimote events and translates them to game inputs.

Compile as a shared object (libcontrollermanager.so) as follows and put into the same folder as the executable file (Bin/Debug or Bin/Release).
```
g++ -Wall -fPIC -I./wiic -c controllermanager.cpp
 
g++ -shared controllermanager.o -lwiicpp -lX11 -lXtst -o libcontrollermanager.so
```

