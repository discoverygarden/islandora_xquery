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
 * @param bool $http_redirect
 *   Whether to issue an HTTP redirect after cancelling pending results.
 */
function islandora_xquery_cancel_results($batch_id, $http_redirect = TRUE) {
  // Get the redirect url.
  if ($http_redirect) {
    $redirect = db_select('islandora_xquery_batches', 'batches')
      ->fields('batches', array('redirect'))
      ->condition('batch_id', $batch_id, '=')
      ->execute()
      ->fetchField();
  }

  // Clear out the db.
  db_delete('islandora_xquery_diffs')
    ->condition('batch_id', $batch_id, '=')
    ->execute();

  db_delete('islandora_xquery_batches')
    ->condition('batch_id', $batch_id, '=')
    ->execute();

  // Redirect.
  if ($http_redirect) {
    $url = drupal_parse_url($redirect);
    drupal_goto($url['path'], $url);
  }
}
