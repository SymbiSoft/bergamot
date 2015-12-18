## Requirements ##

  * Download RSS, list podcasts
  * Download podcasts
  * Manage podcasts
    * Delete old/played
    * Organize
  * Call on music and video players to play content
  * Show podcast information
    * Including cover art?

  * Maintaining engine compatibility with UIQ would be nice, but is hardly a requirement

## View and error handling ##

  * See [ErrorHandling](ErrorHandling.md)

## Implementation plan ##

All the below issues are also [listed in the issue tracker with the tag "Implementation"](http://code.google.com/p/bergamot/issues/list?q=label:Implementation).

  * Fix backend issues (pointers versus SQLite)
  * Switch to Symbian SQLite instead of our own version
  * Scrap player view
    * Disable audio books (won't work without our own player)
    * Integrate with Music player
  * Fix UI issues
    * Single/multi tap inconsistencies
      * But consider that S^3 has changed behavior here (See [the PDFs here](http://developer.symbian.org/wiki/index.php/Prop/Improve_Single_Tap_Support))
    * Screen rotation in all views
    * Common icons with S60
    * Rearrange menu items
  * Fix user reported bugs
  * Remove list of shows on phone
  * Update settings
    * Make it simpler for users?
  * Rename to Podcasting
  * [Change license to EPL](EscarpodLicenseText.md)
  * Move to Symbian Mercurial repositories
  * Implement podcast browsing and search
  * Create daemon that starts with your phone?
  * Create Qt GUI


### Icons ###

We need icons for toolbars. Here we list which icons can be used from other system apps, or if we have to make new ones:

|**Button**|**Icon from**|
|:---------|:------------|
|update all|             |
|add       |             |
|import    |             |
|export    |             |
|settings  |             |
|update    |             |
|delete    |from messenger app|
|mark old  |             |
|suspend   |             |
|resume    |             |
|remove    |same as delete|
|clear     |             |
