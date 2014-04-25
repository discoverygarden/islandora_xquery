<?php
/**
 * @file
 * Houses the confirm_form for the Islandora XQuery configuration form.
 */

/**
 * The delete confirm_form for the Islandora XQuery module.
 *
 * @param array $form
 *   An array representing a Drupal form.
 * @param array $form_state
 *   An array containing the Drupal form state.
 * @param int $association_id
 *   The ID of the configuration we are currently editing.
 *
 * @return array
 *   An array representing the configuration form.
 */
function islandora_xquery_template_delete_confirm_form($form, &$form_state, $association_id) {
  module_load_include('inc', 'islandora_xquery', 'includes/db');
  $form_state['association_id'] = $association_id;
  $association_name = islandora_xquery_retrieve_association_name($association_id);
  $form_state['association_name'] = $association_name;
  return confirm_form($form,
    t('Are you sure you want to delete the @association_name display configuration?', array('@association_name' => $association_name)),
    "admin/islandora/tools/xquery/manage/template/$association_id",
    t('This action cannot be undone.'),
    t('Delete'),
    t('Cancel')
  );
}

/**
 * Submit handler for the islandora_solr_metadata delete confirm_form.
 *
 * @param array $form
 *   An array representing a Drupal form.
 * @param array $form_state
 *   An array containing the Drupal form state.
 */
function islandora_xquery_template_delete_confirm_form_submit($form, &$form_state) {
  module_load_include('inc', 'islandora_xquery', 'includes/db');
  islandora_xquery_delete_association($form_state['association_id']);
  $form_state['redirect'] = 'admin/islandora/tools/xquery/manage';
  drupal_set_message(t('The @association_name display configuration has been deleted!', array('@association_name' => $form_state['association_name'])));
}
