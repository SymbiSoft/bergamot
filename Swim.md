![http://bergamot.googlecode.com/svn/wiki/swimicon.jpg](http://bergamot.googlecode.com/svn/wiki/swimicon.jpg)

# Swim #

Swim is a utility for automatic periodic synchronization of data with internet servers such as Google Calendar (through [GooSync](http://www.goosync.com/)), [Mobical](http://www.mobical.net), [Zyb](http://zyb.com), [ScheduleWorld](http://www.scheduleworld.com), [syncgcal](http://www.syncgcal.com/), [Ovi](http://ovi.com/) or any system running [Funambol](http://www.funambol.com). It makes use of the existing SyncML support in the system, and adds timing capabilities, something both UIQ and S60 mysteriously lack! You can set it to sync your data every 15 minutes, every hour, every 4 hours, every 12 hours, daily, or weekly.

Swim is now available for UIQ 3, S60 3rd, _3rd FP2 and S60 5th edition (Symbian^1)_. Thanks to _kuldippujara_ for hints how to do it.

You can get Swim for Nokia phones [in the Ovi store](http://store.ovi.com/content/114419?clickSource=publisher+channel).

## Screenshots ##

### On Sony Ericsson G900 ###

![http://bergamot.googlecode.com/svn/wiki/swimg900_1.png](http://bergamot.googlecode.com/svn/wiki/swimg900_1.png)
![http://bergamot.googlecode.com/svn/wiki/swimg900_2.png](http://bergamot.googlecode.com/svn/wiki/swimg900_2.png)

### On Nokia N95 ###

![http://bergamot.googlecode.com/svn/wiki/Swim_N95_1.jpg](http://bergamot.googlecode.com/svn/wiki/Swim_N95_1.jpg) ![http://bergamot.googlecode.com/svn/wiki/Swim_N95_2.jpg](http://bergamot.googlecode.com/svn/wiki/Swim_N95_2.jpg)

## How To Install ##

To use Swim, you need to do three things:

  1. Download the latest unsigned Swim SIS file from our [download page](http://code.google.com/p/bergamot/downloads/list) **onto your computer**.
    * There are 3 versions of Swim
      * S60 3rd edition FP2 and 5th edition (FP2 & 5th for short)
      * S60 3rd edition FP1
      * UIQ 3
    * Most new Nokia phones will need the FP2 & 5th edition version
    * If that doesn't work, try the FP1 version
    * Samsung i8910 and Sony Ericsson Satio need the FP2 & 5th edition version
    * And if you have a UIQ phone, you obviously need that version
  1. Sign Swim using Symbian [Open Signed Online](https://www.symbiansigned.com/app/page/public/openSignedOnline.do).
    * Get your IMEI by typing `*#06#` on your phone
    * Enter your email address
    * Upload the Swim SIS file that you downloaded from this site
    * You will get an email with a verification link. Click it. Then you get a second email with a link to the signed SIS file. Download the signed SIS file.
  1. Install the signed SIS file on your phone.

**In addition, many E-series phones have the option to allow self-signed programs disabled. To enable it, go to Application manager > Options > Settings > Software installation and change it to 'All'**

## How to Use ##

  1. Set up one or more SyncML accounts:
    * UIQ: Go to Agenda manager (the calendar app)
    * S60: Settings/Connections/Sync, Tools/Sync or Connections/Sync
    * Tip: In the SyncML settings you can choose which internet account to use, so the system won't ask you every time the sync is triggered by Swim
  1. Run Swim, and set up a timer
  1. Swim now automatically starts with the phone, and runs invisibly in the background (The process is SyncServer, the UI does not need to run)
  1. Enjoy your synchronized data!

The sync timer is set relative to the time of day when you enable it. So if you enable hourly sync at 12:03, it will sync at 13:03, 14:03 etc. Similarly, if you set it for daily sync, it will sync at 12:03 every day. This part could certainly be improved :)


## How to specify data connection for an account ##

Many people are asking where to specify which data connection to use.

### S60 3rd edition ###

  * Have to check

### 5th edition ###

  1. Go to _Settings_ > _Connectivity_ > _Data transfer_ > _Sync_
  1. Select the profile you want to change through _Options_ > _Change active profile_.
  1. Select _Options_ > _Edit profile_
  1. Under _Network connection_ you can set _User defined_ and then select your preferred connection.

## Releases ##

The complete source code is found in the [subversion repository](http://bergamot.googlecode.com/svn/Sync/trunk). Compiled binaries for each release can be found on our [Downloads page](http://code.google.com/p/bergamot/downloads/list). Note that the unsigned binaries are unusable without signing them yourself!

**You can get Swim for Nokia phones [in the Ovi store](http://store.ovi.com/content/114419?clickSource=publisher+channel).**

  * 0.50 (2009-12-22) [Source](http://bergamot.googlecode.com/svn/Sync/tags/v050)
    * **Support for S60 3rd FP2 and S60 5th ed**

  * 0.41 (S60 Only) (2008-11-17) [Source](http://bergamot.googlecode.com/svn/Sync/tags/v041)
    * Fix for S60 problem with sync every 30 minutes
    * **Note that this version seems not to be compatible with Feature Pack 2**

  * 0.40 (2008-11-06) [Source](http://bergamot.googlecode.com/svn/Sync/tags/v040)
    * First S60 version
    * Bug fix for UIQ
    * **Note that the S60 version of 0.40 has a bug that causes syncing every 30 minutes**

  * 0.30 (2008-10-04) [Source](http://bergamot.googlecode.com/svn/Sync/tags/v030)
    * Finally changed UIDs so Swim can be signed with Symbian [Open Signed Online](https://www.symbiansigned.com/app/page/public/openSignedOnline.do)!
    * Added options to synchronize every 12 hours and weekly

  * 0.20 (2008-02-25) [Source](http://bergamot.googlecode.com/svn/Sync/tags/beta2)
    * Autostart now works on newer firmware

  * 0.10 (2007-11-04)
    * Client loads list of sync accounts every time it's shown
    * KitchenSync is now Swim, and KitchenSync.exe is SyncClient.exe, while KitchenSyncServer.exe is SyncServer.exe
    * Note! Since the executable name has changed since last version, while the app ID remains, KitchenSync must be uninstalled before this version can be installed