BUILD STATUS
------------
Current build status:
[![Build Status](https://travis-ci.org/Islandora/islandora_bookmark.png?branch=7.x)](https://travis-ci.org/Islandora/islandora_bookmark)

CI Server:
http://jenkins.discoverygarden.ca

ISLANDORA BOOKMARK
==================

SUMMARY
-------

The Islandora Bookmark module allows users to build, manage, share and track lists of objects.
Manage your bookmarks under /islandora_bookmark. There's two ways to add bookmarks to your lists.
1) Through object pages: every object page will have a little tool at the bottom to bookmark the
current object.
2) Through Solr searches: Islandora Bookmark provides a solr display with functionality to
bookmark objects based on a custom Solr query.


REQUIREMENTS
------------

 * Islandora
 * Islandora Solr (optional)


CONFIGURATION
-------------

 * General settings can be configured on the admin page at admin/islandora/islandora_bookmark.
 * In the solr client admin page (admin/islandora/search/islandora_solr), enable the Islandora Bookmark
   display profile.
 * Set the permissions: there are permissions to administer, use and share bookmarks. Anonymous users can never
   share bookmarks.

CUSTOMIZATION
-------------

Table cells referencing the bookmarked objects in both bookmark lists and bookmark Solr results can be modified.
One way is by overriding the default template file located at theme/islandora-bookmark-object-display.tpl.php.
Another way is to use hook_islandora_bookmark_object_markup() to let another module define the content and markup.

The admin page has an option that allows for a singular, default Bookmark list be created for each individual user.
This list differs in the fact that it cannot be deleted.

TROUBLESHOOTING
---------------

 * How do I set up bookmarking through Solr?

Islandora Bookmark provides a Islandora Solr primary display which has to be enabled in the solr client admin page
(admin/islandora/search/islandora_solr). To set up Islandora Solr check the module's documentation. Islandora Solr comes
with a 'Display switch' block which lets you switch primary displays. When enabled, the bookmark display can be accessed
by appending ?display=bookmark to the Solr results URL.