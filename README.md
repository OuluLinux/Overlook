# Overlook
Wannabe MT4 clone, but which requires the MT4 running a connection script at background. This is my sandbox environment for profitable script searching currently.
The actual long-term usefulness is the problem which currently takes all the time.

To compile Overlook, you need to download and install Ultimate++ and learn to how to compile programs with it. Then, you clone this repository, add it to a new assembly or copy files to MyApps directory.
To run Overlook, you need to run MQL4 script Mt4Connection, which is in the src/plugin/MQL4 folder. The MT4Connection.mq4 goes to the Scripts folder and MT4ConnectionDll.dll goes to Libraries folder.
Run "Load Server History" in MT4 to have all data. Restart MT4 before starting Overlook to save history data to disk.

![Classic view](https://github.com/sppp/Overlook/raw/master/docs/classic.jpg)
