<?php

/**
 * @file
 * Hooks provided by Islandora Bookmark.
 */

/**
 * Hook to collect exports functions available for use.
 *
 * @return array
 *   Returns an array with the name of the module and the function name to call
 *   to handle exportation. In the form of 'module name' => 'export function'.
 */
function hook_islandora_bookmark_export_handler() {
}

/**
 * Hook to alter the options returned for exportation.
 *
 * @return array
 *   Returns an array with the name of the module and the function name to call
 *   to handle exportation. In the form of 'module name' => 'export function'.
 */
function hook_islandora_bookmark_export_handler_alter(&$output) {
}

/**
 * Hook to generate markup for displaying a Fedora object within a table.
 *
 * @return array
 *   Returns an array containing the markup for displaying the Fedora object.
 */
function hook_islandora_bookmark_object_markup($fedora_object, $object_url_info) {
}

/**
 * Hook to generate markup for displaying a Fedora object within a table.
 *
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

/**
 * Hook to change or add values to RSS fields.
 *
 * Somtimes you might want to alter fields for an rss item.
 * 
 * @param AbstractObject $object
 *   The bookmarked object
 * 
 * @return array
 *   Returns an array containing the additional changes to the rss item.
 */
function hook_islandora_bookmark_rss_item(AbstractObject $object) {

  // Create an associative array for the required elements
  // for a valid bookmark RSS item.
  $rss_item = array();
  // The title of the item.
  $rss_item['title'] = 'Altered Title';;
  // The link of the item.
  $rss_item['link'] = 'Altered Link';
  // The description of the item.
  $rss_item['description'] = 'Altered description';

  // Set the source attribute.
  $rss_item['items'] = array(
    array(
      'key' => 'source',
      'value' => 'source value', 'attributes' => array('url' => 'url')),
  );

  // Return the RSS item.
  return $rss_item;
}
