<?php

/**
 * @file
 * Hooks and utility functions for the theming layer.
 */

/**
 * Utility function for generating a link to apply an xquery.
 *
 * @param int $batch_id
 *   Batch identifier.
 *
 * @return string
 *   HTML link.
 */
function islandora_xquery_get_apply_link($batch_id) {
  return l(
    t("APPLY"),
    "islandora/xquery/preview/$batch_id/apply",
    array(
      'attributes' => array(
        'class' => array(
          'islandora-xquery-apply-link',
        ),
      ),
    )
  );
}

/**
 * Utility function for generating a link to cancel an xquery.
 *
 * @param int $batch_id
 *   Batch identifier.
 *
 * @return string
 *   HTML link.
 */
function islandora_xquery_get_cancel_link($batch_id) {
  return l(
    t("CANCEL"),
    "islandora/xquery/preview/$batch_id/cancel",
    array(
      'attributes' => array(
        'class' => array(
          'islandora-xquery-cancel-link',
        ),
      ),
    )
  );
}

/**
 * Utility function for generating a link to the unmodified datastream.
 *
 * @param string $pid
 *   Object identifier.
 * @param string $dsid
 *   Datastream identifier.
 *
 * @return string
 *   HTML link.
 */
function islandora_xquery_get_original_ds_link($pid, $dsid) {
  return l(
    t("@pid - @dsid", array("@pid" => $pid, "@dsid" => $dsid)),
    "islandora/object/$pid/datastream/$dsid/view",
    array(
      'attributes' => array(
        'class' => array(
          'islandora-xquery-original-ds-link',
        ),
      ),
    )
  );
}

/**
 * Uses a code highlighter to prettify a diff with green and red.
 *
 * @param string $diff
 *   A unified diff.
 *
 * @return string
 *   HTML of the diff that is formatted and highlighted.
 */
function islandora_xquery_highlight_diff($diff) {
  if (empty($diff)) {
    $diff = t("NO DIFF TO APPLY");
  }
  libraries_load("geshi");
  $geshi = new GeSHi($diff, 'diff');
  return $geshi->parse_code();
}
