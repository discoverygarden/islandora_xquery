<?php
/**
 * @file
 * Template to render out all diffs for a given xquery.
 */
?>

<?php libraries_load("geshi"); ?>

<div class="islandora-xquery-preview">
    <?php print l(t("APPLY"),
                  "islandora/xquery/preview/$batch_id/apply",
                  array(
                    'attributes' => array(
                      'class' => array(
                        'islandora-xquery-apply-link',
                      ),
                    ),
                  )
                 );
          print " ";
          print l(t("CANCEL"),
                  "islandora/xquery/preview/$batch_id/cancel",
                  array(
                    'attributes' => array(
                      'class' => array(
                        'islandora-xquery-cancel-link',
                      ),
                    ),
                  )
                 );
    ?>
    <?php foreach($results as $result): ?>
        <div class="islandora-xquery-diff">
          <h2><?php print $result->pid; ?></h2>
          <?php $geshi = new GeSHi($result->preview, 'diff');
                print $geshi->parse_code();
          ?>
        </div>
    <?php endforeach ?>
    <?php print theme('pager', array('tags' => array())); ?>
</div>
