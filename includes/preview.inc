<?php

function islandora_xquery_preview_results($batch_id) {
  libraries_load("geshi");

  $query = db_select('islandora_xquery_previews', 'previews');
  $query->fields('previews', array('pid', 'dsid', 'preview'));
  $query->condition('batch_id', $batch_id, '=');
  $query = $query->extend('PagerDefault');
  $query->limit(10);

  $results = $query->execute();

  $html = "";

  for ($i = 0; $i < $results->rowCount(); $i++) {
    $result = $results->fetchObject();
    $result->GeSHi = new GeSHi($result->preview, 'diff');
    $html .= theme('islandora_xquery_preview', array('result' => $result));

    if ($i < $results->rowCount() - 1) {
      $html .= "<hr />";
    }
  }

  $html .= theme('pager', array('tags' => array()));

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
