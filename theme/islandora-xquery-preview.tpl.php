<?php for ($i = 0; $i < $results->rowCount(); $i++): ?>
  <div class="islandora-xquery-diff">
    <?php $result = $results->fetchObject(); ?>
    <h2><?php print $result->pid; ?></h2>
    <pre><?php print $result->preview; ?></pre>
  </div>
  <?php if ($i < $results->rowCount() - 1): ?>
    <hr />
  <?php endif ?>
<?php endfor ?>
