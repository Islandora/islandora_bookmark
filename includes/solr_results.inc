<?php

/**
 * @file
 * Contains methods to search solr and display results. Depends on
 * Apache_Solr_Php client.
 */

/**
 * Extention of IslandoraSolrResults to create an alternative display type.
 */
class IslandoraSolrResultsBookmark extends IslandoraSolrResults {
  /**
   * Constructor.
   */
  public function __construct() {
    parent::__construct();
  }

  /**
   * Renders the Solr results as a table.
   *
   * @see displayResults()
   *
   * @param array $results
   *   The raw Solr results from IslandoraSolrQueryProcessor::solrResult
   *
   * @return array
   *   Rendered Solr results
   */
  public function printResults($results) {
    drupal_add_css(drupal_get_path('module', 'islandora_solr_config') . '/css/islandora_solr_config.theme.css');

    if (empty($results)) {
      return t('No results');
    }
    $results_form = drupal_get_form('islandora_bookmark_results_form', $results['response']['objects'], $this->islandoraSolrQueryProcessor);
    return drupal_render($results_form);
  }
}

/**
 * Creates a form that will be rendered with the Solr search results.
 *
 * Rendered as a tableselect form.
 *
 * @param array $form
 *   The Drupal form definition.
 * @param array $form_state
 *   The Drupal form state.
 * @param array $object_results
 *   The object results from the Islandora Solr processor created in the
 *   printResults function.
 * @param IslandoraSolrQueryProcessor $query_processor
 *   The Islandora Solr Query Processor used to modify the existing query
 *   to retrieve all results.
 *
 * @return array
 *   The Drupal form definition.
 */
function islandora_bookmark_results_form(array $form, array &$form_state, $object_results, $query_processor) {
  module_load_include('inc', 'islandora_bookmark', 'includes/api');

  form_load_include($form_state, 'inc', 'islandora_bookmark', 'IslandoraSolrResultsBookmark');

  if (module_exists('citation_exporter')) {
    if (CitationExporter::ReadyToExport()) {
      $citation_export_head = array(
        '#type' => 'html_tag',
        '#tag' => 'meta',
        '#attributes' => array(
          'http-equiv' => 'refresh',
          'content' => '1.0;' . url(MENU_CITATION_EXPORTER_EXPORT, array('absolute' => TRUE)),
        ),
      );
      drupal_add_html_head($citation_export_head, 'citation_exporter_head');
    }
  }

  $header = array(
    'markup' => t('Label'),
  );
  $rows = array();
  $form_state['islandora_solr_query_processor'] = $query_processor;
  foreach ($object_results as $object_result) {
    $doc = $object_result['solr_doc'];
    $markup = '';
    if (isset($doc['object_label'])) {
      $markup = $doc['object_label'];
    }
    else {
      $markup = $doc['PID'];
    }
    $object_url_info = array(
      'path' => $object_result['object_url'],
      'params' => $object_result['object_url_params'],
      'markup' => $markup,
    );
    $rows[$doc['PID']] = array(
      'markup' => islandora_bookmark_generate_markup($doc['PID'], $object_url_info),
    );
  }
  if (count($rows) > 0) {
    $export = islandora_bookmark_get_export_options();
    $export_options['default'] = t('- Export type -');
    $export_styles = array();
    foreach ($export as $module_name => $export_array) {
      foreach ($export_array as $name => $export_func) {
        $export_options[$module_name][$name] = $name;
        $form_state['islandora_bookmark_export'][$name] = $export_func;
        $export_styles[$name] = islandora_bookmark_get_export_format_options($name);
      }
    }

    $form['islandora_bookmark_export_fieldset'] = array(
      'fieldset' => array(
        '#type' => 'fieldset',
        '#title' => t('Export to file'),
        '#weight' => 0,
      ),
    );

    $form['islandora_bookmark_export_fieldset']['fieldset']['export_styles'] = array(
      '#weight' => 4,
    );
    foreach ($export_styles as $key => $value) {
      if (!empty($export_styles[$key])) {
        $form['islandora_bookmark_export_fieldset']['fieldset']['export_styles'][$key] = array(
          '#type' => 'select',
          '#title' => t('Style'),
          '#options' => $export_styles[$key],
          '#weight' => 4,
          '#name' => 'islandora_bookmark_export_styles_' . $key,
          '#states' => array(
            'visible' => array(
              ':input[name="islandora_bookmark_export_options_select"]' => array('value' => $key),
            ),
          ),
        );
      }
    }

    $form['islandora_bookmark_export_fieldset']['fieldset']['export_options'] = array(
      '#type' => 'select',
      '#options' => $export_options,
      '#weight' => 3,
      '#title' => t('Format'),
      '#name' => 'islandora_bookmark_export_options_select',
    );

    $form['islandora_bookmark_export_fieldset']['fieldset']['export_selected_submit'] = array(
      '#type' => 'submit',
      '#value' => t('Export selected objects'),
      '#name' => 'export_selected',
      '#weight' => 5,
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
        $save_options[$bookmark->bookmarkId] = $bookmark->bookmarkName;
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
 * @param array $form
 *   The Drupal form definition.
 * @param array $form_state
 *   The Drupal form state.
 */
function islandora_bookmark_results_form_submit(array $form, array &$form_state) {
  $queries = $_GET;
  unset($queries['q']);
  $form_state['redirect'] = array(request_path(), array('query' => $queries));
  module_load_include('inc', 'islandora_bookmark', 'includes/api');

  // Grab and modify the query proc's limit to 100000 incase we need
  // to retrieve all objects.
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
    $key = $form_state['input']['islandora_bookmark_export_options_select'];
    if (count($pids) > 0 && $key !== 'default') {
      if (array_key_exists('islandora_bookmark_export_styles_' . $key, $form_state['input'])) {
        call_user_func($form_state['islandora_bookmark_export'][$key], $pids, $form_state['input']['islandora_bookmark_export_styles_' . $key]);
      }
      else {
        call_user_func($form_state['islandora_bookmark_export'][$key], $pids);
      }
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
    $solr_results = $query_proc->islandoraSolrResult;
    $object_results = $solr_results['response']['objects'];
    $all_pids = array();
    $broke_pids = '';
    $broke_count = 0;

    foreach ($object_results as $object_result) {
      $doc = $object_result['solr_doc'];
      if (islandora_object_load($doc['PID'])) {
        $all_pids[$doc['PID']] = $doc['PID'];
      }
      else {
        $broke_pids .= $doc['PID'] . ' ';
        $broke_count++;
      }
    }
    $key = $form_state['input']['islandora_bookmark_export_options_select'];

    if ($key !== 'default' && count($all_pids > 0)) {
      if (array_key_exists('islandora_bookmark_export_styles_' . $key, $form_state['input'])) {
        call_user_func($form_state['islandora_bookmark_export'][$key], $all_pids, $form_state['input']['islandora_bookmark_export_styles_' . $key]);
      }
      else {
        call_user_func($form_state['islandora_bookmark_export'][$key], $all_pids);
      }
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
      // the selected.
      $bookmark_object = islandora_bookmark_get_bookmark_by_number($key);

      $pids_to_add = array();
      $skipped_pids = '';
      $skipped_count = 0;
      foreach ($pids as $pid) {
        if (!in_array($pid, $bookmark_object->pidList)) {
          $pids_to_add[] = $pid;
        }
        else {
          $skipped_pids .= $pid . ' ';
          $skipped_count++;
        }
      }

      if (count($pids_to_add) > 0) {
        try {
          $bookmark_object->addPids($pids_to_add);
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
                '@listname' => $bookmark_object->bookmarkName,
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
                          array('!skipped_pids' => trim($skipped_pids), '!listname' => $bookmark_object->bookmarkName)
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
    $solr_results = $query_proc->islandoraSolrResult;
    $object_results = $solr_results['response']['objects'];

    $all_pids = array();
    $broke_pids = '';
    $broke_count = 0;

    foreach ($object_results as $object_result) {
      $doc = $object_result['solr_doc'];
      if (islandora_object_load($doc['PID'])) {
        $all_pids[$doc['PID']] = $doc['PID'];
      }
      else {
        $broke_pids .= $doc['PID'] . ' ';
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
      // the selected.
      $bookmark_object = islandora_bookmark_get_bookmark_by_number($key);

      $pids_to_add = array();
      $skipped_pids = '';
      $skipped_count = 0;
      foreach ($all_pids as $pid) {
        if (!in_array($pid, $bookmark_object->pidList)) {
          $pids_to_add[] = $pid;
        }
        else {
          $skipped_pids .= $pid . ' ';
          $skipped_count++;
        }
      }
      if (count($pids_to_add) > 0) {
        try {
          $bookmark_object->addPids($pids_to_add);
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
                '@listname' => $bookmark_object->bookmarkName,
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
                          array('!skipped_pids' => trim($skipped_pids), '!listname' => $bookmark_object->bookmarkName)
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
