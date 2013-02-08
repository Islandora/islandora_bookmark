<?php

/**
  * @file
  * Hooks provided by Islandora Bookmark.
  */

/**
  * Hook to collect exports functions available for use.
  *
  * @return
  *   Returns an array with the name of the module and the function name to call
  *   to handle exportation. In the form of 'module name' => 'export function'.
  */
function hook_islandora_bookmark_export_handler() {

}

/**
 * Hook to generate markup for displaying a Fedora object within a table.
 *
 * @return
 *   Returns an array containing the markup for displaying the Fedora object.
 */
function hook_islandora_bookmark_object_markup($fedora_object, $object_url_info) {

}

/**
 * Hook to generate markup for displaying a Fedora object within a table.
 * Content model specific.
 *
 * @return array
 *   Returns an array containing the markup for displaying the Fedora object.
 */
function hook_CMODEL_PID_islandora_bookmark_object_markup($fedora_object, $object_url_info) {

}

/**
 * Hook to collect additional style options used in exporting.
 *
 * @return array
 *   Returns an array containing the additional styles for export.
 */
function hook_islandora_bookmark_export_styles($option) {

}
