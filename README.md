#Weather Wiz

Weather Wiz is a weather indicator for Linux, displaying
local weather info. The indicator resides in the indicator panel
(top right in Unity panel and bottom right in KDE panel).
It is in its initial development stage right now and not all
of it's potential features have been fully implemented yet.
If you still want to check it out, read the build instructions below.

####Prerequisites for building from source
* libcurl3
* libcurl4-openssl-dev
* libgtk2.0-dev
* libjansson4
* libjansson-dev
* libappindicator1
* libappindicator-dev

Some of these packages already come pre-installed in some Linux distros,
others can be installed from your distribution repositories.

####Building
Make sure you have the prerequisites mentioned above.
Once you've downloaded the zip, open a terminal and `cd` to the directory
the zip file is located in and enter the following:
```
unzip weather-wiz-master
cd weather-wiz-master
make
```
This will create the necessary binary `weather-wiz` in the `bin` directory.
You can excecute it like any normal executable (through GUI or the terminal).

You can set the indicator to autorun on system startup, through Startup Applications,
or by manually writing a startup configuration file for **Upstart**.
