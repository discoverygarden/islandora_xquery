<?php
/**
 * @file
 * Template to render out all diffs for a given xquery.
 */
?>

<div class="islandora-xquery-preview">
  <?php if(variable_get('islandora_xquery_show_batch_id', FALSE)): ?>
    <div class="islandora-xquery-batch-id">
      <p><?php print "Batch ID: " . $batch_id?></p>
    </div>
  <?php endif ?>
  <div class="islandora-xquery-preview-controls">
    <ul class="islandora-xquery-preview-control-list">
      <li class="islandora-xquery-preview-control-li"><?php print islandora_xquery_get_apply_link($batch_id); ?></li>
      <li class="islandora-xquery-preview-control-li"><?php print islandora_xquery_get_cancel_link($batch_id); ?></li>
    </ul>
  </div>
  <div class="islandora-xquery-diffs">
    <?php foreach($results as $result): ?>
      <div class="islandora-xquery-diff">
        <?php print islandora_xquery_get_original_ds_link($result->pid, $result->dsid); ?>
        <?php print islandora_xquery_highlight_diff($result->diff); ?>
      </div>
    <?php endforeach ?>
  </div>
  <?php print theme('pager', array('tags' => array())); ?>
</div>
