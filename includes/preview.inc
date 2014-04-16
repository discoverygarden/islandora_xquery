<?php

/**
 * @file
 * Functions for previewing the results of an xquery.
 */

/**
 * Queries the db for pending results and displays them.
 *
 * @param int $batch_id
 *   Identifier for the batch that ran the xquery.
 *
 * @return string
 *   HTML markup for the preview.
 */
function islandora_xquery_preview_results($batch_id) {
  $results = db_select('islandora_xquery_diffs', 'diffs')
    ->fields('diffs', array('pid', 'dsid', 'diff'))
    ->condition('batch_id', $batch_id, '=')
    ->extend('PagerDefault')
    ->limit(10)
    ->execute();

  return theme('islandora_xquery_preview', array('results' => $results, 'batch_id' => $batch_id));
}
