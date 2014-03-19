<?php
/**
 * @file
 * An primary Solr display for running XQuery queries.
 * @todo
 *   Remind the user that Solr may not be up to date.
 */

/**
 * Extention of IslandoraSolrResults to create an alternative display type.
 */
class IslandoraSolrResultsXquery extends IslandoraSolrResults {
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
   *   The Solr results from IslandoraSolrQueryProcessor::solrResult
   *
   * @return array
   *   Rendered Solr results
   */
  public function printResults($results) {
    if (empty($results)) {
      return t('No results.');
    }
    $results_form = drupal_get_form('islandora_xquery_results_form', $results['response']['objects'], $this);
    return drupal_render($results_form);
  }
}

/**
 * Form building function.
 *
 * @param array $form
 *   The Drupal form definition.
 * @param array $form_state
 *   The Drupal form state.
 * @param array $object_results
 *   The object results from the Islandora Solr processor created in the
 *   printResults function.
 * @param IslandoraSolrResultsXquery $solr
 *   Solr results object, used in submit handler.
 *
 * @return array
 *   An array containing the form structure.
 */
function islandora_xquery_results_form($form, &$form_state, $object_results, $solr) {
  form_load_include($form_state, 'inc', 'islandora_xquery', 'includes/db');
  // This form gets called in a roundabout fashion and needs to include itself.
  form_load_include($form_state, 'inc', 'islandora_xquery', 'includes/solr_results');
  $header = array('markup' => t('Object'));
  $rows = array();
  $label_field = variable_get('islandora_solr_object_label_field', 'fgs_label_s');
  foreach ($object_results as $object_result) {
    $rows[$object_result['PID']] = array(
      'markup' => l(
        isset($object_result['solr_doc'][$label_field]) ? $object_result['solr_doc'][$label_field] : $object_result['PID'],
        $object_result['object_url']
      ),
    );
  }
  // TRUE because struggling with ajax isn't worth it.
  $form['#tree'] = TRUE;
  $form['islandora_xquery_table'] = array(
    '#header' => $header,
    '#type' => 'tableselect',
    '#options' => $rows,
    '#attributes' => array('class' => array('islandora-xquery-solr-results')),
    '#empty' => t('No results found.'),
  );

  if (count($rows) > 0) {
    $xquery_options = array();
    $associations = islandora_xquery_get_associations();
    foreach ($associations as $association) {
      $xquery_options[$association['machine_name']] = $association['template_name'];
    }
    $xquery_options['Custom'] = t('Custom');

    $form['xquery_controls'] = array(
      'xquery' => array(
        '#type' => 'select',
        '#title' => t('XQuery'),
        '#required' => TRUE,
        '#description' => t('Select the XQuery to run.'),
        '#options' => $xquery_options,
        '#ajax' => array(
          'wrapper' => 'xquery_inputs',
          'callback' => 'islandora_xquery_xquery_inputs_ajax',
        ),
      ),
      'xquery_inputs' => array(
        '#type' => 'item',
        '#prefix' => '<div id="xquery_inputs">',
        '#suffix' => '</div>',
      ),
      'dsids' => array(
        '#type' => 'textfield',
        '#title' => t('DSIDs'),
        '#description' => t('A comma seperated list of DSIDs to run against.'),
        '#size' => 10,
        '#required' => TRUE,
      ),
      'submit_selected' => array(
        '#type' => 'submit',
        '#value' => t('Run Selected'),
      ),
      'submit_all' => array(
        '#type' => 'submit',
        '#value' => t('Run All'),
      ),
    );
    if (isset($form_state['values']['xquery_controls']['xquery'])) {
      if ($form_state['values']['xquery_controls']['xquery'] == 'Custom') {
        $form['xquery_controls']['xquery_inputs']['query'] = array(
          '#type' => 'textarea',
          '#rows' => 10,
          '#cols' => 30,
          '#required' => TRUE,
          '#title' => t('Query'),
          '#description' => t('Enter a custom XQuery to run.'),
        );
      }
      else {
        $tokens = islandora_xquery_get_tokens_from_machine_name(
          $form_state['values']['xquery_controls']['xquery']
        );
        $form['xquery_controls']['xquery_inputs']['tokens'] = array();
        foreach ($tokens as $token) {
          $form['xquery_controls']['xquery_inputs']['tokens'][$token['token_name']] = array(
            '#type' => 'textfield',
            '#default_value' => $token['token_default'],
            '#title' => check_plain($token['token_name']),
            '#description' => check_plain($token['token_description']),
          );
        }
      }
    }
  }
  return $form;
}

/**
 * Ajax callback to render inputs when XQuery changes.
 */
function islandora_xquery_xquery_inputs_ajax($form, $form_state) {
  return $form['xquery_controls']['xquery_inputs'];
}

/**
 * Submit handler for the islandora_xquery_results form.
 *
 * @param array $form
 *   The Drupal form definition.
 * @param array $form_state
 *   The Drupal form state.
 */
function islandora_xquery_results_form_submit(array $form, array &$form_state) {
  form_load_include($form_state, 'inc', 'islandora_xquery', 'includes/db');
  // Get objects to work on.
  if (end($form_state['clicked_button']['#parents']) == 'submit_all') {
    $object_range = $form_state['build_info']['args'][1];
  }
  else {
    $object_range = array();
    foreach ($form_state['values']['islandora_xquery_table'] as $pid => $checked) {
      if ($checked) {
        $object_range[] = $pid;
      }
    }
  }
  // Get DSIDs to work on.
  $dsids = explode(',', $form_state['values']['xquery_controls']['dsids']);
  $dsids = array_map('trim', $dsids);

  // Get XQuery combined with tokens.
  if (isset($form_state['values']['xquery_controls']['query'])) {
    $xquery = $form_state['values']['xquery_controls']['query'];
  }
  else {
    $xquery = islandora_xquery_get_xquery_from_machine_name($form_state['values']['xquery_controls']['xquery']);
  }
  $tokens = array();
  $replacements = array();
  if (isset($form_state['values']['xquery_controls']['xquery_inputs']['tokens'])) {
    foreach ($form_state['values']['xquery_controls']['xquery_inputs']['tokens'] as $token => $replacement) {
      $tokens[] = $token;
      $replacements[] = $replacement;
    }
  }
  $xquery = str_replace($tokens, $replacements, $xquery);
  // Set batch.
  batch_set(array(
    'title' => t('Running XQuery'),
    'finished' => 'islandora_xquery_run_query_batch_finished',
    'progress_message' => t('Time elapsed: @elapsed <br/>Estimated time remaining @estimate.'),
    'error_message' => t('An error has occurred.'),
    'file' => drupal_get_path('module', 'islandora_xquery') . '/includes/solr_results.inc',
    'operations' => array(
      array(
        'islandora_xquery_run_query_batch_operation',
        array($dsids, $object_range, $xquery),
      ),
    ),
  ));
}

/**
 * @todo comment what this does when it does something.
 *
 * @param array $dsids
 *   An array of dsids to perform the operations on.
 * @param mixed $object_range
 *   Objects to work on, either an array or an IslandoraSolrResultsXquery.
 * @param string $xquery
 *   The XQuery to run on the datastreams.
 * @param array $context
 *   The batch context.
 */
function islandora_xquery_run_query_batch_operation(array $dsids, $object_range, $xquery, array &$context) {
  if (!isset($context['sandbox']['progress'])) {
    $context['sandbox']['progress'] = 0;
    $context['sandbox']['object_range'] = $object_range;
    $context['sandbox']['total'] = islandora_xquery_run_query_batch_operation_get_total($context);
    // Avoid divide by zero.
    if ($context['sandbox']['total'] == 0) {
      $context['finished'] = 1;
      return;
    }
  }
  $current_object = islandora_xquery_run_query_batch_operation_get_object($context);
  // @todo Do operation here.

  $context['sandbox']['progress']++;
  $context['finished'] = $context['sandbox']['progress'] / $context['sandbox']['total'];
}

/**
 * The batch 'finished' callback.
 */
function islandora_xquery_run_query_batch_finished($success, $results, $operations) {
  if (!$success) {
    $error_operation = reset($operations);
    $message = t('An error occurred while processing %error_operation with arguments: @arguments', array(
                 '%error_operation' => $error_operation[0],
                 '@arguments' => print_r($error_operation[1], TRUE)));
    drupal_set_message($message, 'error');
  }
}

/**
 * Gets total number of objects to work on from the batch context.
 *
 * This function is useful because the Object information may be stored as an
 * array of PIDs or as a Solr query.
 *
 * @param array $context
 *   The context of the current batch.
 *
 * @return int
 *   The number of operations.
 */
function islandora_xquery_run_query_batch_operation_get_total(&$context) {
  if (is_array($context['sandbox']['object_range'])) {
    return count($context['sandbox']['object_range']);
  }
  $solr = $context['sandbox']['object_range'];
  return $context['sandbox']['object_range']
    ->islandoraSolrQueryProcessor
    ->islandoraSolrResult['response']['numFound'];
}

/**
 * Gets an object to work on from the batch context.
 *
 * This function is useful because the Object information may be stored as an
 * array of PIDs or as a Solr query.
 *
 * @param array $context
 *   The context of the current batch.
 *
 * @return AbstractObject
 *   The current object to work on.
 */
function islandora_xquery_run_query_batch_operation_get_object(&$context) {
  if (is_array($context['sandbox']['object_range'])) {
    return islandora_object_load(array_pop($context['sandbox']['object_range']));
  }
  // Init search.
  if (!isset($context['sandbox']['search_settings'])) {
    $context['sandbox']['search_settings']['step_size'] = 100;
    $context['sandbox']['search_settings']['offset'] = count(
      $context['sandbox']['object_range']
        ->islandoraSolrQueryProcessor
        ->islandoraSolrResult['response']['objects']
    );
  }
  // Get a PID if available.
  $solr_result = array_pop($context['sandbox']['object_range']
    ->islandoraSolrQueryProcessor
    ->islandoraSolrResult['response']['objects']);
  if ($solr_result) {
    $pid = $solr_result['PID'];
  }
  else {
    // Populate PIDs if necessary to get one.
    $solr_query = $context['sandbox']['object_range']->islandoraSolrQueryProcessor;
    $solr_query->solrStart = $sandbox['sandbox']['search_settings']['offset'];
    $solr_query->solrLimit = $context['sandbox']['search_settings']['step_size'];
    $solr_query->executeQuery();
    $solr_result = array_pop($solr_query->islandoraSolrResult['response']['objects']);
    $pid = $solr_result['PID'];
    $sandbox['sandbox']['search_settings']['offset'] += $context['sandbox']['search_settings']['step_size'];
  }
  return islandora_object_load($pid);
}
