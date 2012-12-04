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
    module_load_include('module', 'islandora_bookmark');
    module_load_include('inc', 'fedora_repository', 'api/tuque');
    // add islandora solr theme css
    drupal_add_css(drupal_get_path('module', 'islandora_solr_config') . '/css/islandora_solr_config.theme.css');
    
    // set variable
    $output = '';
       
    if (empty($results)) {
      return t('no results');
    }
    
    return drupal_render(drupal_get_form('islandora_bookmark_results_form', $results->response->docs, $this->islandoraSolrQueryProcessor));
    
    if (variable_get('islandora_solr_debug_mode', 0)) { // debug dump
      $results_r .= "<pre>Results: " . print_r($results, TRUE) . "</pre>";
      $fieldset_r = array(
        '#title' => t("Raw Results"),
        '#collapsible' => TRUE,
        '#collapsed' => TRUE,
        '#value' => $results_r,
        );
      $output .= theme('fieldset', $fieldset_r);
    }
    return $output;
  }
}  

function islandora_bookmark_results_form($form, &$form_state, $docs, $query_processor) {
  
  $header = array(
    'markup' => '',
  );
  $rows = array();
  
  $form_state['islandora_solr_query_processor'] = $query_processor;
  
  foreach ($docs as $doc) {
    $temp_object = islandora_object_load($doc->PID);
    $rows[$doc->PID] = array(
      'markup' => islandora_bookmark_generate_markup($temp_object),
    );
  }
   
  if (count($rows) > 0 ) {
    $export = islandora_bookmark_get_export_options();
    $export_options['default'] = t('-- Select an option to export --'); 
    foreach ($export as $key => $value) {
      $export_options[$key] = $key;
      $form_state['islandora_bookmark_export'][$key] = $value;
    }
    $form['islandora_bookmark_export_fieldset'] = array(
      'fieldset' => array(
        '#type' => 'fieldset',
        '#title' => t('Export To File'),
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
  }
  
  $form['islandora_bookmark_save_fieldset'] = array(
      'fieldset' => array(
        '#type' => 'fieldset',
        '#title' => t('Save to Bookmark'),
        '#weight' => 0,
       ),
    );
  
  $form['islandora_bookmark_save_fieldset']['fieldset']['save_options'] = array(
      '#type' => 'select',
      '#options' => $export_options,
      '#weight' => 3,
    );
    $form['islandora_bookmark_save_fieldset']['fieldset']['save_selected_submit'] = array(
      '#type' => 'submit',
      '#value' => t('Save selected objects'),
      '#name' => 'export_selected',
      '#weight' => 4,
    );
    $form['islandora_bookmark_save_fieldset']['fieldset']['save_all_submit'] = array(
      '#type' => 'submit',
      '#value' => t('Save all results'),
      '#name' => 'export_all',
      '#weight' => 5,
    );
    $form['islandora_bookmark_save_fieldset']['fieldset']['go_to_bookmark'] = array(
      '#type' => 'submit',
      '#value' => t('Go to Bookmark'),
      '#name' => 'goto_bookmark',
      '#weight' => 6,
    );
  
  $form['islandora_bookmark_table'] = array(
      '#header' => $header,
      '#type' => 'tableselect',
      '#options' => $rows,
      '#empty' => t('No results found.'),
  );
  
  return $form;
}

function islandora_bookmark_results_form_submit($form, &$form_state) {
  dsm($form_state, 'fstate');
  
  if ($form_state['clicked_button']['#name'] === 'export_selected') {
    $pids = array();
    foreach ($form_state['values']['islandora_bookmark_table'] as $checkbox => $value) {
      if ($value !== 0) {
        $pids[] = $value;
      }
    }
    $key = $form_state['values']['export_options'];
    if (count($pids) > 0 && $key !== 'default') {
     call_user_func($form_state['islandora_bookmark_export'][$key], $pids);
    }
    elseif (count($pids) === 0) {
      drupal_set_message(t('Please select at least one pid!'), 'error'); 
    }
    else {
      drupal_set_message(t('Please select an option to export as!'), 'error'); 
    }
  }
  elseif ($form_state['clicked_button']['#name'] === 'export_all') {
    $query_proc = $form_state['islandora_solr_query_processor'];
    $query_proc->solrLimit = 100000;
    $query_proc->executeQuery();
    $solr_results = $query_proc->solrResult;
    $docs = $solr_results->response->docs;
    $all_pids = array();
    foreach ($docs as $doc) {
      $all_pids[$doc->PID] = $doc->PID;
    }
    $key = $form_state['values']['export_options'];
    
    if (count($all_pids) > 0 && $key !== 'default') {
     call_user_func($form_state['islandora_bookmark_export'][$key], $all_pids);
    }
    else {
      drupal_set_message(t('Please select an option to export as!'), 'error'); 
    }
  }
}