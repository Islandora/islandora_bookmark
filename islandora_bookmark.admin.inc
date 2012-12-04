<?php

/**
 * @file
 * Contains all admin related functionality
 */

/**
 * Function to return admin settings form
 *
 * @param type $form
 * @param type $form_state
 * @return
 * The form array.
 */
function islandora_bookmark_admin_settings($form, &$form_state) {
  $form = array(
    'overview_page_elements' => array(
      '#type' => 'textfield',
      '#title' => t('Bookmarks Overview Table'),
      '#description' => t('Number of Bookmarks shown per page'),
      '#default_value' => variable_get('islandora_bookmark_overview_page_elements', 10),
      '#size' => 4,
    ),
    'detailed_page_elements' => array(
      '#type' => 'textfield',
      '#title' => t('Detailed Bookmark Table'),
      '#description' => t('Number of objects shown per page'),
      '#default_value' => variable_get('islandora_bookmark_detailed_page_elements', 10),
      '#size' => 4,
    ),
    'submit' => array(
      '#type' => 'submit',
      '#value' => t('Save configuration'),
    ),
  );
  
  return $form;
}

/**
 * Function to validate the admin settings form
 * 
 * @param type $form
 * @param type $form_state
 * 
 */
function islandora_bookmark_admin_settings_validate($form, &$form_state) {
  // Make sure our values are integers for the pager
  $overview_ele = $form_state['values']['overview_page_elements'];
  $detailed_ele = $form_state['values']['detailed_page_elements'];
  
  if (!is_numeric($overview_ele)) {
    form_set_error('overview_page_elements', t('The value for the Overview Table must be an integer.'));
  }
  
  if (!is_numeric($detailed_ele)) {
    form_set_error('detailed_page_elements', t('This value for the Detailed table must be an integer.'));
  }
}

/**
 * Function that sets the Drupal variables with user's input.
 * 
 * @param type $form
 * @param type $form_state
 */
function islandora_bookmark_admin_settings_submit($form, &$form_state) {
  drupal_set_message(t('The settings have been updated!'));
  variable_set('islandora_bookmark_overview_page_elements', floor($form_state['values']['overview_page_elements']));
  variable_set('islandora_bookmark_detailed_page_elements', floor($form_state['values']['detailed_page_elements']));
}

