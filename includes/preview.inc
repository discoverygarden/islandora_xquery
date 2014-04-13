<?php

function islandora_xquery_preview_results($batch_id) {
  $query = db_select('islandora_xquery_previews', 'previews');
  $query->fields('previews', array('pid', 'dsid', 'preview'));
  $query->condition('batch_id', $batch_id, '=');
  $query = $query->extend('PagerDefault');
  $query->limit(10);

  $results = $query->execute();

  $html = theme('islandora_xquery_preview', array('results' => $results));

  //$header = array(
    //array('data' => 'Object', 'field' => 'pid'),
    //array('data' => 'Datastream', 'field' => 'dsid'),
    //array('data' => 'Preview', 'field' => 'preview'),
  //);

  //foreach ($results as $result)
  //{
    //$rows[] = array(
      //'data' => array(
        //$result->pid,
        //$result->dsid,
        //'<pre>' . $result->preview . '</pre>',
      //),
    //);
  //}

  //$html = theme('table', array(
    //'header' => $header,
    //'rows' => $rows,
    //'empty' => 'No previews found for batch ' . $batch_id,
    //'sticky' => FALSE,
  //));

  //$html .= theme('pager', array('tags' => array()));

  return $html;
}
