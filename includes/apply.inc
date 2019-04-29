<?php

/**
 * @file
 * Functions for committing xquery results to Fedora.
 */

/**
 * Kicks off a batch to apply the results from a previous xquery batch.
 *
 * @param int $batch_id
 *   Identifier for the batch that previously ran the xquery.
 */
function islandora_xquery_apply_results($batch_id) {
  // Set batch.
  batch_set(array(
    'title' => t('Applying XQuery'),
    'finished' => 'islandora_xquery_apply_query_batch_finished',
    'progress_message' => t('Time elapsed: @elapsed <br/>Estimated time remaining @estimate.'),
    'error_message' => t('An error has occurred.'),
    'file' => drupal_get_path('module', 'islandora_xquery') . '/includes/apply.inc',
    'operations' => array(
      array(
        'islandora_xquery_apply_query_batch_operation',
        array($batch_id),
      ),
    ),
  ));

  batch_process('islandora/xquery/preview' . $batch_id);
}

/**
 * Queries the db for a handful of diffs and applies them.
 *
 * This operation will get run over and over again until there are no more
 * pending results.  This will avoid exceeding the memory limit or timeout
 * for the session.
 *
 * @param int $batch_id
 *   Identifier for the batch that previously ran the xquery.
 * @param array $context
 *   The batch context.
 */
function islandora_xquery_apply_query_batch_operation($batch_id, array &$context) {
  module_load_include('inc', 'islandora_object_lock', 'includes/utilities');

  if (!isset($context['sandbox']['progress'])) {
    // Init progress.
    $context['sandbox']['progress'] = 0;
    $context['sandbox']['total'] = islandora_xquery_apply_query_batch_get_total($batch_id);

    // Jam the batch id into the results array for later use.
    $context['results']['batch_id'] = $batch_id;

    // Avoid divide by zero.
    if ($context['sandbox']['total'] == 0) {
      $context['finished'] = 1;
      return;
    }
  }

  // Query for remaining diffs.
  $results = db_select('islandora_xquery_diffs', 'diffs')
      ->fields(
          'diffs', array(
            'id',
            'pid',
            'dsid',
            'diff',
            'batch_id',
          )
      )
      ->condition('batch_id', $batch_id, '=')
      ->condition('status', 'PENDING', '=')
      ->range(0, 100)
      ->execute();

  $batch_times = db_select('islandora_xquery_batches', 'ixd')
      ->fields(
          'ixd', array(
            'batch_created',
          )
      )
      ->condition('batch_id', $batch_id)
      ->execute()
      ->fetch();
  $batch_time = $batch_times->batch_created;

  $ignored_objects = array();

  foreach ($results as $result) {
    $include = TRUE;
    $fedora_object = islandora_object_load($result->pid);
    $locked = islandora_object_lock_is_locked($fedora_object);
    $created_date = strtotime($fedora_object[$result->dsid]->createdDate);
    if ($locked) {
      $ignored_objects[] = $result->pid;
      $include = FALSE;
    }
    elseif ($created_date > $batch_time) {
      $ignored_objects[] = $result->pid;
      $include = FALSE;
    }
    islandora_object_lock_set_object_lock($result->pid);
    islandora_xquery_apply_query_batch_apply_diff($result, $context);
  }

  $context['results']['ignored_objects'] = $ignored_objects;
}

/**
 * Gets the total amount of diffs left to apply.
 *
 * @param int $batch_id
 *   Identifier for the batch that previously ran the xquery.
 *
 * @return int
 *   The number of diffs to apply.
 */
function islandora_xquery_apply_query_batch_get_total($batch_id) {
  return db_select('islandora_xquery_diffs', 'diffs')
          ->condition('batch_id', $batch_id, '=')
          ->condition('status', 'PENDING', '=')
          ->countQuery()
          ->execute()
          ->fetchField();
}

/**
 * Applies a diff to a datastream and updates its status in the db.
 *
 * @param object $result
 *   A single record from the db.
 * @param array $context
 *   The batch context.
 */
function islandora_xquery_apply_query_batch_apply_diff($result, &$context) {
  $object = islandora_object_load($result->pid);

  // Exit early if object fails to load from Fedora.
  if (!$object) {
    islandora_xquery_apply_query_batch_update_progress($result, ISLANDORA_XQUERY_STATUS_OBJECT_LOAD_FAIL, $context);
    return;
  }

  $datastream = $object[$result->dsid];

  // Exit early if datastream does not exist.
  if (!$datastream) {
    islandora_xquery_apply_query_batch_update_progress($result, ISLANDORA_XQUERY_STATUS_DATASTREAM_LOAD_FAIL, $context);
    return;
  }

  $diff = $result->diff;

  $old = $datastream->content;
  $new = xdiff_string_patch($old, $diff);

  // Exit early if patch fails.
  if (!$new) {
    islandora_xquery_apply_query_batch_update_progress($result, ISLANDORA_XQUERY_STATUS_PATCH_FAIL, $context);
    return;
  }

  // Exit early if datastream update fails.
  try {
    $datastream->setContentFromString($new);
    if (module_exists('islandora_object_lock')) {
      module_load_include('inc', 'islandora_object_lock', 'includes/utilities');
      islandora_object_lock_remove_object_lock($result->pid);
    }
  }
  catch (Exception $e) {
    islandora_xquery_apply_query_batch_update_progress($result, ISLANDORA_XQUERY_STATUS_DATSTREAM_UPDATE_FAIL, $context);
    throw $e;
  }

  // Success!
  islandora_xquery_apply_query_batch_update_progress($result, ISLANDORA_XQUERY_STATUS_APPLIED, $context);
}

/**
 * Updates the status of a particular diff's application and batch progress.
 *
 * @param object $result
 *   A single record from the db.
 * @param string $status
 *   The status message to write to the database.
 * @param array $context
 *   The batch context.
 */
function islandora_xquery_apply_query_batch_update_progress($result, $status, &$context) {
  // Setup update query.
  db_update('islandora_xquery_diffs')
      ->fields(array('status' => $status))
      ->condition('id', $result->id, '=')
      ->execute();

  // Update progress.
  $context['sandbox']['progress']++;
  $context['finished'] = $context['sandbox']['progress'] / $context['sandbox']['total'];
}

/**
 * The batch 'finished' callback.
 */
function islandora_xquery_apply_query_batch_finished($success, $results, $operations) {
  module_load_include('inc', 'islandora_xquery', 'includes/cancel');

  if (!$success) {
    $error_operation = reset($operations);
    $message = t('An error occurred while processing %error_operation with arguments: @arguments', array(
      '%error_operation' => $error_operation[0],
      '@arguments' => print_r($error_operation[1], TRUE)));
    drupal_set_message($message, 'error');
  }
  if (isset($results['ignored_objects']) && count($results['ignored_objects']) > 0) {
    $output = t('The following object(s) were skipped because they have been locked or altered since the query was run.');
    foreach ($results['ignored_objects'] as $ignored_object) {
      $output .= ' ' . l($ignored_object, "islandora/object/$ignored_object") . ' ';
    }
    drupal_set_message(filter_xss($output));
  }
  // Delete all appropriate db records and redirect.
  islandora_xquery_cancel_results($results['batch_id']);
}
