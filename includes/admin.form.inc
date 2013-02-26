<?php

/**
 * @file
 * Handles the display/submission of the admin settings form for this module.
 */

/**
 * Defines the admin settings form.
 *
 * @param array $form
 *   The Drupal form definition.
 * @param array $form_state
 *   The Drupal form state.
 *
 * @return array
 *   The Drupal form definition.
 */
function islandora_bookmark_admin_settings(array $form, array &$form_state) {
  $form = array(
    'overview_page_elements' => array(
      '#type' => 'textfield',
      '#title' => t('Bookmarks overview table'),
      '#description' => t('Number of Bookmarks shown per page'),
      '#default_value' => variable_get('islandora_bookmark_overview_page_elements', 10),
      '#element_validate' => array('element_validate_integer_positive'),
      '#size' => 4,
    ),
    'detailed_page_elements' => array(
      '#type' => 'textfield',
      '#title' => t('Detailed bookmark table'),
      '#description' => t('Number of objects shown per page'),
      '#default_value' => variable_get('islandora_bookmark_detailed_page_elements', 10),
      '#element_validate' => array('element_validate_integer_positive'),
      '#size' => 4,
    ),
    'submit' => array(
      '#type' => 'submit',
      '#value' => t('Save configuration'),
    ),
    'reset' => array(
      '#type' => 'submit',
      '#value' => t('Reset to defaults'),
    ),
  );

  return $form;
}

/**
 * Function that sets the Drupal variables with user's input.
 *
 * @param array $form
 *   The Drupal form definition.
 * @param array $form_state
 *   The Drupal form state.
 */
function islandora_bookmark_admin_settings_submit(array $form, array &$form_state) {
  drupal_set_message(t('The settings have been updated!'));
  $id = $form_state['triggering_element']['#id'];
  switch ($id) {
    case 'edit-submit':
      variable_set('islandora_bookmark_overview_page_elements', $form_state['values']['overview_page_elements']);
      variable_set('islandora_bookmark_detailed_page_elements', $form_state['values']['detailed_page_elements']);
      break;

    case 'edit-reset':
      variable_del('islandora_bookmark_overview_page_elements');
      variable_del('islandora_bookmark_detailed_page_elements');
      break;
  }
}
