<?php

/**
 * @file
 * Drush helpers for Islandora XQuery.
 */

/**
 * Implements hook_drush_command().
 */
function islandora_xquery_drush_command() {
  $commands = array();
  $commands['islandora_xquery_batch_process_command'] = array(
    'description' => dt('Process an Islandora XQuery batch set.'),
    'options' => array(
      'batch_id' => array(
        'description' => dt('The batch set ID to process.'),
        'required' => TRUE,
      ),
    ),
    'aliases' => array('ixbp'),
  );
  return $commands;
}

/**
 * Command callback for processing XQuery batches.
 */
function drush_islandora_xquery_batch_process_command() {
  module_load_include('inc', 'islandora_xquery', 'includes/apply');

  // @XXX: Standard logged-in-user paranoia.
  drupal_static_reset('islandora_get_tuque_connection');
  $connection = islandora_get_tuque_connection();

  $batch_id = drush_get_option('batch_id');
  $batch = array(
    'title' => t('Applying XQuery'),
    'finished' => 'islandora_xquery_apply_query_batch_finished',
    'progress_message' => t('Time elapsed: @elapsed <br/>Estimated time remaining @estimate.'),
    'error_message' => t('An error has occurred.'),
    'file' => drupal_get_path('module', 'islandora_xquery') . '/includes/apply.inc',
    'operations' => array(
      array(
        'islandora_xquery_apply_query_batch_operation',
        array($batch_id, 'is_backend' => TRUE),
      ),
    ),
  );
  batch_set($batch);
  drush_backend_batch_process();

}
