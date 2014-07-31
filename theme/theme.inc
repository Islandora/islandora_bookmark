<?php

/**
 * @file
 * Theme and preprocess functions.
 */

/**
 * Implements hook_preprocess_theme().
 */
function islandora_bookmark_preprocess_islandora_bookmark_list_columns(array &$variables) {
  $variables['column_count'] = 3;
  $list_links = theme(
    'islandora_bookmark_list_links',
    array('id' => $variables['id'], 'current' => 'manage')
  );
  $variables['list_links'] = drupal_render($list_links);
  drupal_add_css(drupal_get_path('module', 'islandora_bookmark') . '/css/list-columns.css');

  module_load_include('inc', 'islandora_bookmark', 'includes/api');
  $pids = islandora_bookmark_list_pids_query($variables['id']);
  $pid_count = 0;
  foreach ($pids as $pid) {
    // Drop the PIDs into baskets corresponding to columns.
    $column = $pid_count % $variables['column_count'] + 1;
    $pid_count++;
    $variables['objects'][$column][$pid] = array();
    // Build markup for objects.
    $object_markup = &$variables['objects'][$column][$pid];
    $object = islandora_object_load($pid);
    $object_markup['image'] = islandora_datastream_access(ISLANDORA_VIEW_OBJECTS, $object['TN']) ?
      theme(
        'image',
        array('path' => url("islandora/object/$pid/datastream/TN/view"))) :
      '';

    $display_label = isset($object->label) ? $object->label : $pid;
    $object_markup['label'] = l(
      t("<strong>@display_label</strong>", array('@display_label' => $display_label)),
      "islandora/object/$pid",
      array('html' => TRUE)
    );

    if (islandora_datastream_access(ISLANDORA_VIEW_OBJECTS, $object['DC'])) {
      $dc_dom = new DOMDocument();
      $dc_dom->loadXML($object['DC']->content);
      $dc_xpath = new DOMXpath($dc_dom);
      $dc_elements = $dc_xpath->query("//*[local-name()='description']");
      if ($dc_elements->item(0)) {
        $object_markup['description'] = "<div>{$dc_elements->item(0)->nodeValue}</div>";
      }
    }
  }
}

/**
 * Implements theme_hook().
 */
function theme_islandora_bookmark_list_links(array &$variables) {
  $links = array('#weight' => -1);
  $module_path = drupal_get_path('module', 'islandora_bookmark');
  if ($variables['current'] != '') {
    $links['manage'] = array(
      '#markup' => l(
        t('Admin'),
        "islandora-bookmark/listid/{$variables['id']}/manage",
        array('attributes' => array('class' => array('list-links')))
      ),
    );
  }
  if ($variables['current'] != 'islandora_bookmark_list_columns') {
    $links['islandora_bookmark_list_columns'] = array(
      '#markup' => l(
        t('View'),
        "islandora-bookmark/listid/{$variables['id']}/view",
        array('attributes' => array('class' => array('list-links')))
      ),
    );
  }
  // This is the RSS icon link.
  $links['bookmark_rss'] = array(
    '#markup' => l(theme('image', array(
      'path' => $module_path . '/images/rss.png',
      'title' => t('Present Bookmark list in RSS form'),
      'attributes' => array(),
        )), "islandora-bookmark/listid/{$variables['id']}/rss", array('html' => TRUE)),
    '#prefix' => '<div id="islandora-bookmark-rss-format">',
    '#suffix' => '</div>',
  );
  drupal_add_css($module_path . '/css/list-links.css');
  return $links;
}
