## How do I send a bug report? ##

There are still bugs in Escarpod that may cause the program to crash in some situations. If you find a series of steps that will reliably crash Escarpod, please help us to improve it by sending us a log and a description of what caused the problem using the [Issue tracker](http://code.google.com/p/bergamot/issues/entry).

Escarpod maintains an action log in the file `C:\logs\escarpod\debug.log`, assuming the directory `c:\logs\escarpod` exists. The built-in UIQ file manager will not let you access this directory, so you will need a third party tool like [Swiss Manager](http://cellphonesoft.com/1down.php) to do this.

To obtain a log, complete the following steps

  1. Create the directory `C:\logs\escarpod`
  1. Run Escarpod, reproduce the crash
  1. Copy the file `C:\logs\escarpod\debug.log` to a folder you can access from other UIQ programs, for example the folder `C:\Media Files\other` or your memory stick at `D:` or `E:`
  1. Send the file to your PC using bluetooth, the serial cable, email or by moving your memory stick
  1. Attach the file to your issue report.
  1. Feel good that you contributed to Escarpod!

## How do I add new feeds? ##

Escarpod needs URLs to RSS files to access feeds. If Escarpod fails to load a feed, make sure you entered the URL of an RSS file, and not an HTML file. RSS files commonly end with .rss or .xml.

## Why do my downloads fail? ##

Strange behavior occurs when the disk is full. Make sure you didn't fill up your flash card with podcasts, as can easily happen with Escarpod. Use the purge feed option to clean a folder from within the program, or remove your downloaded files manually through the file manager.

## Can I use my Bluetooth Headsets with Escarpod? ##

You can use Bluetooth headsets with Escarpod, at least with Sony Ericsson phones. The problem is that there is no known public way to enable the headset. However, if you start the media player and use Transfer sound to redirect your sound to your bluetooth headset, the sound will stay in the headset until you disable it. This includes sound played through Escarpod.

## Are There any Hardware Button Shortcuts? ##

Yes, you can:

  * _Delete shows_ using the backspace/clear button on your phone.
  * _Jump back and ahead_ while playing a show using sideways buttons.
  * _Mark played/unplayed_ using the asterisk (`*`)

## Can I use a Remote Control? ##

From version 0.94, Escarpod supports remote control for pause and playback, both for wired and Bluetooth headsets.

UIQ only allows one program to listen to remote control events, so if you want Escarpod to respond to play and pause, please make sure the built-in media player is not running before you start Escarpod.

## How do I Import Feeds? ##

Escarpod supports OPML lists of feeds from version 0.94 onwards. Use Import feeds from the menu.

An example OPML file may look like this:

```
<?xml version="1.0"?>
<opml version="1.1">
<body>
  <outline text="Stephen Fry's Podgrams" xmlUrl="http://www.stephenfry.com/podcasts/rss.xml" />
  <outline text="LugRadio" xmlUrl="http://lugradio.org/episodes.rss"/>
  <outline text="TWiT" xmlUrl="http://leo.am/podcasts/twit" />
</body>
</opml>
```

The text field is optional.

Save it as a .xml file, for instance myopml.xml, copy it to your phone and use Import feeds from the menu.

_Tip! If you place an OPML file named feeds.xml in your podcasts directory, it will be automatically imported when Escarpod starts!_