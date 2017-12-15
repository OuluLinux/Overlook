# Overlook
MT4 clone, which requires the MT4 running a connection script at background.

To compile Overlook, you need to download and install Ultimate++ and learn to how to compile programs with it. Then, you clone this repository, add it to a new assembly or copy files to MyApps directory.
To run Overlook, you need to run MQL4 script Mt4Connection, which is in the src/plugin/MQL4 folder. The MT4Connection.mq4 goes to the Scripts folder and MT4ConnectionDll.dll goes to Libraries folder.
Check that your system clock matches to your MT4 broker's clock. Change your PC timezone if it is not the same.
This repository is kept in working state all the time, but Overlook expert advisors are very experiemental.
Run "Load Server History" in MT4 to have all data. Restart MT4 before starting Overlook to save history data to disk.
Overlook history data tends to corrupt easily, so remove history and corecache when that happens. The current MT4 data loader is a mess, but will be improved eventually.

[Development blog](https://makingoverlook.blogspot.fi/)

![Classic view](https://github.com/sppp/Overlook/raw/master/docs/classic.jpg)
