<?php

/**
 * @file
 * Functions for canceling the results of an xquery.
 */

/**
 * Queries the db for pending results and cancels them.
 *
 * @param int $batch_id
 *   Identifier for the batch that ran the xquery.
 */
function islandora_xquery_cancel_results($batch_id) {
  // Get the redirect url.
  $redirect = db_select('islandora_xquery_batches', 'batches')
    ->fields('batches', array('redirect'))
    ->condition('batch_id', $batch_id, '=')
    ->execute()
    ->fetchField();

  // Clear out the db.
  db_delete('islandora_xquery_previews')
    ->condition('batch_id', $batch_id, '=')
    ->execute();

  db_delete('islandora_xquery_batches')
    ->condition('batch_id', $batch_id, '=')
    ->execute();

  // Redirect.
  drupal_goto($redirect);
}
