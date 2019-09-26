<?php

/**
 * @file
 * Processor admin form.
 */

/**
 * Form building function.
 */
function islandora_xquery_general_admin_form(array $form, array &$form_state) {
  $form['islandora_xquery_show_batch_id'] = array(
    '#type' => 'checkbox',
    '#title' => t('Show batch ID'),
    '#default_value' => variable_get('islandora_xquery_show_batch_id', FALSE),
    '#description' => t('Show the batch ID when previewing XQuery results.'),
  );
  return system_settings_form($form);
}
