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
    if (module_exists('islandora_object_lock')) {
      module_load_include('inc', 'islandora_object_lock', 'includes/utilities');
      $locked = islandora_object_lock_get_locks(0, 10000);
      $found_pids = array();
      foreach ($results['response']['objects'] as $object) {
        $found_pids[] = $object['PID'];
      }
      $conflicts = array_intersect(array_keys($locked), $found_pids);
      if (!empty($conflicts)) {
        $output = count($conflicts) == 1 ? t('This object in the resultset is currently locked by other users') : t('These objects in the resultset are currently locked by other users');

        foreach ($conflicts as $conflict) {
          $output .= '<br />' . l($conflict, "islandora/object/$conflict");
        }
        return $output;
      }
    }
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
          isset($object_result['solr_doc'][$label_field]) ? $object_result['solr_doc'][$label_field] : $object_result['PID'], $object_result['object_url']
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
    $object_range = array_keys(array_filter($form_state['values']['islandora_xquery_table']));
  }

  // Get DSIDs to work on.
  $dsids = explode(',', $form_state['values']['xquery_controls']['dsids']);
  $dsids = array_map('trim', $dsids);

  // Get XQuery from either the text area or the db, whichever is appropriate.
  $query_is_from_text_area = isset($form_state['values']['xquery_controls']['xquery']) && strcmp($form_state['values']['xquery_controls']['xquery'], t('Custom')) == 0;
  if ($query_is_from_text_area) {
    $xquery = $form_state['values']['xquery_controls']['xquery_inputs'];
  }
  else {
    $xquery = islandora_xquery_get_xquery_from_machine_name($form_state['values']['xquery_controls']['xquery']);
  }

  // Substitute in the tokens.
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
        array($dsids, $object_range, $xquery, request_uri()),
      ),
    ),
  ));
}

/**
 * Populates insert query with find/replace diffs.
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
function islandora_xquery_run_query_batch_operation(array $dsids, $object_range, $xquery, $redirect, array &$context) {
  if (!isset($context['sandbox']['progress'])) {
    $context['sandbox']['progress'] = 0;
    $context['sandbox']['object_range'] = $object_range;
    $context['sandbox']['total'] = islandora_xquery_run_query_batch_operation_get_total($context);

    // Insert a record in the db to describe this batch operation.
    $vars = drupal_get_query_parameters();
    $context['results']['batch_id'] = $vars['id'];
    $context['results']['redirect'] = $redirect;

    $batch_insert_query = db_insert('islandora_xquery_batches');
    $batch_insert_query->fields(
        array(
          'batch_id',
          'batch_created',
          'batch_ended',
          'xquery',
          'redirect',
        )
    );
    $batch_insert_query->values(
        array(
          $vars['id'],
          time(),
          0,
          $xquery,
          $redirect,
        )
    );
    $batch_insert_query->execute();

    // Setup the query to insert each individual find/replace preview.
    $diff_insert_query = db_insert('islandora_xquery_diffs');
    $diff_insert_query->fields(array('batch_id', 'pid', 'dsid', 'diff'));
    $context['results']['diff_insert_query'] = $diff_insert_query;

    // Avoid divide by zero.
    if ($context['sandbox']['total'] == 0) {
      $context['finished'] = 1;
      return;
    }
  }

  // Run the find/replace on all applicable datastreams.
  $current_object = islandora_xquery_run_query_batch_operation_get_object($context);

  foreach ($current_object as $datastream) {
    if (array_intersect($dsids, array($datastream->id))) {
      islandora_xquery_run_query_batch_operation_generate_diff($current_object, $datastream, $xquery, $context);
    }
  }

  // Update progress.
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

  $results['diff_insert_query']->execute();
  db_update('islandora_xquery_batches')->fields(array('batch_ended' => time()))->condition('batch_id', $results['batch_id'], '=')->execute();

  // Redirect to the preview results screen.
  drupal_goto("islandora/xquery/preview/{$results['batch_id']}");
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
    // Bit of cludge here since this class isn't available when unserializing.
    // No idea why it's not being autoloaded.
    // Just make a new one and re-use the properties in the sandbox.  Then
    // replace it in the sandbox.  Its properties are needed elsewhere.
    $old_proc = $context['sandbox']['object_range']->islandoraSolrQueryProcessor;
    $processor = new IslandoraSolrQueryProcessor();
    $context['sandbox']['object_range']->islandoraSolrQueryProcessor = $processor;
    $processor->solrStart = $context['sandbox']['search_settings']['offset'];
    $processor->solrLimit = $context['sandbox']['search_settings']['step_size'];
    $processor->buildAndExecuteQuery($old_proc->solrQuery, $old_proc->solrParams);
    $solr_result = array_pop($processor->islandoraSolrResult['response']['objects']);
    $pid = $solr_result['PID'];
    $context['sandbox']['search_settings']['offset'] += $context['sandbox']['search_settings']['step_size'];
  }
  return islandora_object_load($pid);
}

/**
 * Adds a record to the diff insert query for the object/ds and xquery.
 *
 * @param Abstractobject $object
 *   The object to run the find/replace on.
 * @param Abstractdatastream $datastream
 *   The datastream to run the find/replace on.
 * @param string $xquery
 *   The find/replace xquery to execute.
 * @param array $context
 *   The context of the current batch.
 */
function islandora_xquery_run_query_batch_operation_generate_diff($object, $datastream, $xquery, $context) {
  module_load_include('inc', 'islandora_xquery', 'includes/zorba');

  // Make temp files for zorba to work on.
  $tmp_ds_file = islandora_xquery_get_temp_file('temporary://ds.xml');
  $tmp_ds_path = islandora_xquery_get_path_from_file($tmp_ds_file);

  $tmp_clean_ds_file = islandora_xquery_get_temp_file('temporary://clean_ds.xml');
  $tmp_clean_ds_path = islandora_xquery_get_path_from_file($tmp_clean_ds_file);

  $tmp_query_file = islandora_xquery_get_temp_file('temporary://query.xq');
  $tmp_query_path = islandora_xquery_get_path_from_file($tmp_query_file);

  $tmp_output_file = islandora_xquery_get_temp_file('temporary://zorba.out');
  $tmp_output_path = islandora_xquery_get_path_from_file($tmp_output_file);

  $tmp_diff_file = islandora_xquery_get_temp_file('temporary://out.diff');
  $tmp_diff_path = islandora_xquery_get_path_from_file($tmp_diff_file);

  $datastream->getContent($tmp_ds_path);

  // Bad encoding appears to be applied to only the 'DC' datastream.
  // Perphaps because of how fedora handles this, not exactly sure.
  // Example: Given '<dc:title>"my title"</dc:title>' would produce a false
  // negative that looks like '<dc:title>&quot;my title&quot;</dc:title>'.
  if (strcmp($datastream->label, "DC Record") == 0) {
    // Apply an identity transform on the datastream to have saxon parse it and
    // clean it up a little so the diffs don't get crazy with entity escaping.
    $xquery_object = islandora_xquery_get_query_object(
      $tmp_ds_path,
      $tmp_clean_ds_path,
      drupal_get_path('module', 'islandora_xquery') . '/queries/identity.xq'
    );
    $xquery_object->execute();
  }
  else {
    $tmp_clean_ds_file = $tmp_ds_file;
    $tmp_clean_ds_path = $tmp_ds_path;
  }

  // Now execute the query the user wants.
  file_put_contents($tmp_query_path, $xquery);
  $xquery_object = islandora_xquery_get_query_object(
    $tmp_clean_ds_path,
    $tmp_output_path,
    $tmp_query_path
  );
  $xquery_object->execute();

  // Pull out the results and diff them.
  xdiff_file_diff($tmp_clean_ds_path, $tmp_output_path, $tmp_diff_path, 3);

  // Insert a record into the preview table.
  $diff_insert_query = $context['results']['diff_insert_query'];
  $batch_id = $context['results']['batch_id'];
  $diff_insert_query->values(array(
    $batch_id,
    $object->id,
    $datastream->id,
    file_get_contents($tmp_diff_path),
  ));

  // Clean up the temp files.
  file_delete($tmp_ds_file);
  file_delete($tmp_clean_ds_file);
  file_delete($tmp_output_file);
  file_delete($tmp_query_file);
  file_delete($tmp_diff_file);
}

/**
 * Add a file as managed if is not already.
 *
 * @param string $file_uri
 *   The given file URI location.
 *
 * @return object
 *   The file, as returned from file_save().
 */
function islandora_xquery_get_temp_file($file_uri, $mime = NULL) {
  $query = new EntityFieldQuery();
  $result = $query
    ->entityCondition('entity_type', 'file')
    ->propertyCondition('uri', $file_uri)
    ->execute();
  if (isset($result['file'])) {
    $fid = current($result['file'])->fid;
    $file = file_load($fid);
  }
  else {
    $file = new stdClass();
    $file->uri = $file_uri;
    $file->filename = drupal_basename($file_uri);
    if (isset($mime)) {
      $file->filemime = $mime;
    }
  }
  $file->status = 0;
  return file_save($file);
}

/**
 * Gets the path to a file object using safe Drupal methods.
 *
 * @param StdClass $file
 *   The file object whose path we desire.
 *
 * @return string
 *   The path to the file.
 */
function islandora_xquery_get_path_from_file($file) {
  $uri = $file->uri;
  $wrapper = file_stream_wrapper_get_instance_by_uri($uri);
  return $wrapper->realpath();
}
