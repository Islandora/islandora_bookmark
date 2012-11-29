<?php

/**
 * @file
 * 
 * This is a template for generating the default markup for a Fedora object
 * in conjunction with Islandora Bookmark.
 * 
 * The 'variables' array contains a Fedora Tuque object see 
 * islandora-object.tpl.php of the core Islandora module for further details.
 * 
 */
?>

<?php 
print l($fedora_object->label, 'islandora/object/' . $variables['fedora_object']->id, array(
              'attributes' => array(
                'target' => '_blank'
                )));
?>

