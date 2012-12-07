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

INSTALLATION
------------


CONFIGURATION
-------------

 * General settings can be configured on the admin page at admin/islandora/islandora_bookmark.
 * In the solr client admin page (admin/islandora/search/islandora_solr), enable the Islandora Bookmark
   display profile.


CUSTOMIZATION
-------------


TROUBLESHOOTING
---------------

 * How do I set up bookmarking through Solr?
 
Islandora Bookmark provides a Islandora Solr primary display which has to be enabled in the solr client admin page
(admin/islandora/search/islandora_solr). To set up Islandora Solr check the module's documentation. Islandora Solr comes
with a 'Display switch' block which lets you switch primary displays. When enabled, the bookmark display can be accessed
by appending ?display=bookmark to the Solr results URL.



F.A.Q.
------