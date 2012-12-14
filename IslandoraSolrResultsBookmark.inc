<?php

/*
 * @file
 * Contains methods to search solr and display results. Depends on
 * Apache_Solr_Php client.
 */

/**
 * Extention of IslandoraSolrResults to create an alternative display type.
 */
class IslandoraSolrResultsBookmark extends IslandoraSolrResults {

  function IslandoraSolrResultsBookmark() {
    parent::__construct();
  }

  /**
   * Renders the Solr results as a table.
   *
   * @param type $results
   *   The raw Solr results from IslandoraSolrQueryProcessor::solrResult
   * @return
   *   Rendered Solr results
   *
   * @see IslandoraSolrResults::displayResults()
   */
  function printResults($results) {
    // add islandora solr theme css
    drupal_add_css(drupal_get_path('module', 'islandora_solr_config') . '/css/islandora_solr_config.theme.css');

    if (empty($results)) {
      return t('No results');
    }
    $results_form = drupal_get_form('islandora_bookmark_results_form', $results->response->docs, $this->islandoraSolrQueryProcessor);
    return drupal_render($results_form);
  }
}

/**
 * Creates a form that will be rendered with the Solr search results in
 * tableselect form.
 *
 * @param type $form
 * @param type $form_state
 * @param type $docs
 *   The Solr docs created in the printResults function.
 * @param type $query_processor
 *   The Islandora Solr Query Processor used to modify the existing query
 *   to retrieve all results.
 * @return type
 */
function islandora_bookmark_results_form($form, &$form_state, $docs, $query_processor) {
  module_load_include('inc', 'islandora_bookmark', 'islandora_bookmark_api');
  module_load_include('inc', 'islandora_bookmark', 'Bookmark');
  module_load_include('inc', 'islandora', 'includes/IslandoraTuque');

  $header = array(
    'markup' => t('Label'),
  );
  $rows = array();

  $form_state['islandora_solr_query_processor'] = $query_processor;

  foreach ($docs as $doc) {

    $fedora_object = islandora_object_load($doc->PID);
    if ($fedora_object) {
      $rows[$doc->PID] = array(
        'markup' => islandora_bookmark_generate_markup($fedora_object),
      );
    }
    else {
      $rows[$doc->PID] = array(
        'markup' => '<div class="islandora-bookmark-error-message">' . t('Unable to load object @pid.',
          array('@pid' => $doc->PID)) . '</div>',
      );
    }
  }

  if (count($rows) > 0) {
    $export = islandora_bookmark_get_export_options();
    $export_options['default'] = t('- Export type -');

    foreach ($export as $key => $value) {
      $export_options[$key] = $key;
      $form_state['islandora_bookmark_export'][$key] = $value;
    }

    $form['islandora_bookmark_export_fieldset'] = array(
      'fieldset' => array(
        '#type' => 'fieldset',
        '#title' => t('Export to file'),
        '#weight' => 0,
      ),
    );
    $form['islandora_bookmark_export_fieldset']['fieldset']['export_options'] = array(
      '#type' => 'select',
      '#options' => $export_options,
      '#weight' => 3,
    );
    $form['islandora_bookmark_export_fieldset']['fieldset']['export_selected_submit'] = array(
      '#type' => 'submit',
      '#value' => t('Export selected objects'),
      '#name' => 'export_selected',
      '#weight' => 4,
    );
    $form['islandora_bookmark_export_fieldset']['fieldset']['export_all_submit'] = array(
      '#type' => 'submit',
      '#value' => t('Export all results'),
      '#name' => 'export_all',
      '#weight' => 5,
    );

    $user_bookmarks = islandora_bookmark_get_user_owned_bookmarks();

    if (count($user_bookmarks) > 0) {
      $save_options = array();
      $save_options['default'] = t('- Select bookmark list -');

      foreach ($user_bookmarks as $bookmark) {
        $save_options[$bookmark->bookmark_id] = $bookmark->bookmark_name;
      }

      $form['islandora_bookmark_save_fieldset'] = array(
        'fieldset' => array(
          '#type' => 'fieldset',
          '#title' => t('Bookmark'),
          '#weight' => 0,
        ),
      );
      $form['islandora_bookmark_save_fieldset']['fieldset']['save_options'] = array(
        '#type' => 'select',
        '#options' => $save_options,
        '#weight' => 3,
      );
      $form['islandora_bookmark_save_fieldset']['fieldset']['save_selected_submit'] = array(
        '#type' => 'submit',
        '#value' => t('Bookmark selected objects'),
        '#name' => 'save_selected',
        '#weight' => 4,
      );
      $form['islandora_bookmark_save_fieldset']['fieldset']['save_all_submit'] = array(
        '#type' => 'submit',
        '#value' => t('Bookmark all results'),
        '#name' => 'save_all',
        '#weight' => 5,
      );
      $form['islandora_bookmark_save_fieldset']['fieldset']['go_to_bookmark'] = array(
        '#type' => 'submit',
        '#value' => t('View list'),
        '#name' => 'goto_bookmark',
        '#weight' => 6,
      );
    }
  }
  $form['islandora_bookmark_table'] = array(
      '#header' => $header,
      '#type' => 'tableselect',
      '#options' => $rows,
      '#attributes' => array('class' => array('islandora-bookmark-solr-results')),
      '#empty' => t('No results found.'),
  );
  return $form;
}

/**
 * Submit handler for the islandora_bookmark_results form.
 *
 * @param type $form
 * @param type $form_state
 */
function islandora_bookmark_results_form_submit($form, &$form_state) {
  module_load_include('inc', 'islandora_bookmark', 'islandora_bookmark_api');
  module_load_include('inc', 'islandora_bookmark', 'Bookmark');

  // Grab and modify the query proc's limit to 100000 incase we need
  // to retrieve all objects
  $query_proc = $form_state['islandora_solr_query_processor'];
  $query_proc->solrLimit = 100000;

  if ($form_state['clicked_button']['#name'] === 'export_selected') {
    $pids = array();
    $broke_pids = '';
    $broke_count = 0;

    foreach ($form_state['values']['islandora_bookmark_table'] as $checkbox => $value) {
      if ($value !== 0) {
        if (islandora_object_load($value)) {
          $pids[] = $value;
        }
        else {
          $broke_pids .= $value . ' ';
          $broke_count++;
        }
      }
    }
    $key = $form_state['values']['export_options'];
    if (count($pids) > 0 && $key !== 'default') {
      call_user_func($form_state['islandora_bookmark_export'][$key], $pids);
    }
    elseif (count($pids) === 0 && $broke_count === 0) {
      drupal_set_message(t('Please select at least one object.'), 'error');
    }
    elseif ($key === 'default') {
      drupal_set_message(t('Please select an export type.'), 'error');
    }
    else {
      if ($broke_count > 0) {
        $broken_pids = format_plural($broke_count, 'The following object !broke_pids was unable to be exported as it does not exist.',
          'The following objects !broke_pids were unable to be exported as they do not exist.',
          array('!broke_pids' => trim($broke_pids)));
        drupal_set_message($broken_pids, 'error');
      }
    }
  }
  elseif ($form_state['clicked_button']['#name'] === 'export_all') {
    $query_proc->executeQuery();
    $solr_results = $query_proc->solrResult;
    $docs = $solr_results->response->docs;
    $all_pids = array();
    $broke_pids = '';
    $broke_count = 0;

    foreach ($docs as $doc) {
      if (islandora_object_load($doc->PID)) {
          $all_pids[$doc->PID] = $doc->PID;
      }
      else {
        $broke_pids .= $doc->PID . ' ';
        $broke_count++;
      }
    }
    $key = $form_state['values']['export_options'];

    if ($key !== 'default' && count($all_pids > 0)) {
      call_user_func($form_state['islandora_bookmark_export'][$key], $all_pids);
    }
    elseif ($key === 'default') {
      drupal_set_message(t('Please select an option to export as!'), 'error');
    }
    else {
      if ($broke_count > 0) {
        $broken_pids = format_plural($broke_count, 'The following object !broke_pids was unable to be exported as it does not exist.',
          'The following objects !broke_pids were unable to be exported as they do not exist.',
          array('!broke_pids' => trim($broke_pids)));
        drupal_set_message($broken_pids, 'error');
      }
    }
  }
  elseif ($form_state['clicked_button']['#name'] === 'save_selected') {
    $pids = array();
    $broke_pids = '';
    $broke_count = 0;
    foreach ($form_state['values']['islandora_bookmark_table'] as $checkbox => $value) {
      if ($value !== 0) {
        if (islandora_object_load($value)) {
          $pids[] = $value;
        }
        else {
          $broke_pids .= $value . ' ';
          $broke_count++;
        }
      }
    }
    if ($broke_count > 0) {
      $broken_pids = format_plural($broke_count, 'The following object !broke_pids was unable to be bookmarked as it does not exist.',
        'The following objects !broke_pids were unable to be bookmarked as they do not exist.',
        array('!broke_pids' => trim($broke_pids)));
      drupal_set_message($broken_pids, 'error');
    }
    $key = $form_state['values']['save_options'];
    if ($key !== 'default') {
      // Instantiate a Bookmark so we can do some comparisons before we add
      // the selected
      $bookmark_object = islandora_bookmark_get_bookmark_by_number($key);

      $pids_to_add = array();
      $skipped_pids = '';
      $skipped_count = 0;
      foreach ($pids as $pid) {
        if (!in_array($pid, $bookmark_object->pid_list)) {
          $pids_to_add[] = $pid;
        }
        else {
          $skipped_pids .= $pid . ' ';
          $skipped_count++;
        }
      }

      if (count($pids_to_add) > 0) {
        try {
          $bookmark_object->add_pids($pids_to_add);
          $plural_add = format_plural(count($pids), 'selected object has', 'selected objects have');

          $plural_skipped = '';

          if ($skipped_count > 0) {
            $plural_skipped = format_plural($skipped_count, 'The selected object (!skipped_pids) was not added as it already exists in the bookmark list.',
              'The selected objects (!skipped_pids) were not added as they already exist in the bookmark list.',
              array('@skipped_pids' => trim($skipped_pids)));
          }
          drupal_set_message(t('The @plural_add been added to @listname. @plural_skipped',
            array(
              '@plural_add' => $plural_add,
              '@listname' => $bookmark_object->bookmark_name,
              '@plural_skipped' => $plural_skipped,
            )
          ));
        }
        catch (Exception $e) {
          drupal_set_message($e->getMessage(), 'error');
        }
      }
      else {
        $plural_skipped = format_plural($skipped_count, 'The selected object (!skipped_pids) was not added as it already exists in !listname.',
            'The selected objects (!skipped_pids) were not added as they already exist in !listname.',
            array('!skipped_pids' => trim($skipped_pids), '!listname' => $bookmark_object->bookmark_name)
        );
        drupal_set_message($plural_skipped, 'warning');
      }
    }
    elseif (count($pids) === 0) {
      drupal_set_message(t('Please select at least one object.'), 'error');
    }
    else {
      drupal_set_message(t('Please select a bookmark list.'), 'error');
    }
  }
  elseif ($form_state['clicked_button']['#name'] === 'save_all') {
    $query_proc->executeQuery();
    $solr_results = $query_proc->solrResult;
    $docs = $solr_results->response->docs;

    $all_pids = array();
    $broke_pids = '';
    $broke_count = 0;

    foreach ($docs as $doc) {
      if (islandora_object_load($doc->PID)) {
          $all_pids[$doc->PID] = $doc->PID;
      }
      else {
        $broke_pids .= $doc->PID . ' ';
        $broke_count++;
      }
    }

    if ($broke_count > 0) {
      $broken_pids = format_plural($broke_count, 'The following object !broke_pids was unable to be added as it does not exist.',
        'The following objects !broke_pids were unable to be added as they do not exist.',
        array('!broke_pids' => trim($broke_pids)));
      drupal_set_message($broken_pids, 'error');
    }
    $key = $form_state['values']['save_options'];

    if ($key !== 'default') {
     // Instantiate a Bookmark so we can do some comparisons before we add
      // the selected
      $bookmark_object = islandora_bookmark_get_bookmark_by_number($key);

      $pids_to_add = array();
      $skipped_pids = '';
      $skipped_count = 0;
      foreach ($all_pids as $pid) {
        if (!in_array($pid, $bookmark_object->pid_list)) {
          $pids_to_add[] = $pid;
        }
        else {
          $skipped_pids .= $pid . ' ';
          $skipped_count++;
        }
      }
      if (count($pids_to_add) > 0) {
        try {
          $bookmark_object->add_pids($pids_to_add);
          $plural_add = format_plural(count($pids_to_add), 'result has', 'results have');

          $plural_skipped = '';

          if ($skipped_count > 0) {
            $plural_skipped = format_plural($skipped_count, 'The object (!skipped_pids) was not added as it already exists in the bookmark list.',
                'The objects (!skipped_pids) were not added as they already exist in the bookmark list.',
                array('!skipped_pids' => trim($skipped_pids)));
          }
          drupal_set_message(t('The @plural_add been added to @listname. @plural_skipped',
            array(
              '@plural_add' => $plural_add,
              '@listname' => $bookmark_object->bookmark_name,
              '@plural_skipped' => $plural_skipped,
            )
          ));
        }
        catch (Exception $e) {
          drupal_set_message($e->getMessage(), 'error');
        }
      }
      else {
        $plural_skipped = format_plural($skipped_count, 'The pid (!skipped_pids) was not added as it already exists in !listname.',
          'The pids (!skipped_pids) were not added as they already exist in !listname',
          array('!skipped_pids' => trim($skipped_pids), '!listname' => $bookmark_object->bookmark_name)
        );
        drupal_set_message($plural_skipped, 'warning');
      }
    }
    else {
      drupal_set_message(t('Please select a bookmark list.'), 'error');
    }
  }
  else {
    $key = $form_state['values']['save_options'];

    if ($key !== 'default') {
      drupal_goto('islandora-bookmark/listid/' . $key);
    }
    else {
      drupal_set_message(t('Please select a bookmark list.'), 'error');
    }
  }
}