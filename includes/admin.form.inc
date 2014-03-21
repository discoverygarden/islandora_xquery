<?php
/**
 * @file
 * Management overview form for the Islandora XQuery module.
 */

/**
 * Builds the overview form for Islandora XQuery.
 *
 * @param array $form
 *   An array representing a form in Drupal.
 * @param array $form_state
 *   An array containing the Drupal form state.
 *
 * @return array
 *   An array containing the form to be rendered.
 */
function islandora_xquery_admin_form($form, &$form_state) {
  module_load_include('inc', 'islandora', 'includes/utilities');
  module_load_include('inc', 'islandora_xquery', 'includes/utilities');
  module_load_include('inc', 'islandora_xquery', 'includes/db');
  $rows = array();
  // Get all the already defined templates.
  $associations = islandora_xquery_get_associations();
  foreach ($associations as $association) {
    $association_id = $association['association_id'];
    $rows[] = array(
      l($association['template_name'], "admin/islandora/xquery/manage/template/$association_id"),
      $association['machine_name'],
      $association['template_description'],
    );
  }
  $form['configure_zorba'] = array(
    '#type' => 'fieldset',
    '#title' => t('Configure Zorba'),
    '#collapsed' => FALSE,
    '#collapsible' => FALSE,
  );
  $zorba = isset($form_state['values']['zorba']) ? $form_state['values']['zorba'] : variable_get('islandora_xquery_zorba', ISLANDORA_XQUERY_ZORBA_DEFAULT_LOCATION);
  $version = islandora_xquery_get_zorba_version($zorba);
  $form['configure_zorba']['zorba'] = array(
    '#prefix' => '<div id="zorba-executable-wrapper">',
    '#suffix' => '</div>',
    '#type' => 'textfield',
    '#title' => t('Zorba'),
    '#description' => t('Zorba is used to execute XQuery queries.<br/>!msg', array(
      '!msg' => islandora_executable_available_message($zorba, $version, ISLANDORA_XQUERY_ZORBA_REQUIRED_VERSION))
    ),
    '#default_value' => $zorba,
    '#ajax' => array(
      'callback' => 'islandora_xquery_admin_form_ajax_callback',
      'wrapper' => 'zorba-executable-wrapper',
      'effect' => 'fade',
      'event' => 'change',
    ),
  );
  $form['configure_zorba']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Submit'),
    '#submit' => array('islandora_xquery_admin_form_zorba_configure_submit'),
    '#limit_validation_errors' => array(
      array('zorba'),
    ),
  );
  $form['table'] = array(
    '#theme' => 'table',
    '#header' => array(
      t('Name'),
      t('Machine Name'),
      t('Description'),
    ),
    '#rows' => $rows,
    '#empty' => t('No templates currently present.'),
  );
  $form['add_template'] = array(
    '#type' => 'fieldset',
    '#title' => t('Add a template'),
    '#collapsed' => TRUE,
    '#collapsible' => FALSE,
  );
  $form['add_template']['template_name'] = array(
    '#type' => 'textfield',
    '#size' => 100,
    '#title' => t('Template name'),
    '#required' => TRUE,
  );
  $form['add_template']['machine_name'] = array(
    '#type' => 'textfield',
    '#size' => 100,
    '#title' => t('Machine name'),
    '#description' => t('A unique machine name used in the exportation of features'),
    '#required' => TRUE,
    '#element_validate' => array('islandora_xquery_admin_form_validate_machine_name'),
  );
  $form['add_template']['template_description'] = array(
    '#type' => 'textarea',
    '#title' => t('Description'),
  );
  $form['add_template']['save_content_model'] = array(
    '#type' => 'submit',
    '#value' => t('Add template'),
  );
  return $form;
}

/**
 * Validation for the machine name.
 *
 * @param array $element
 *   An array representing a the machine name element.
 * @param array $form_state
 *   An array representing the Drupal form state.
 * @param array $form
 *   An array representing a Drupal form.
 */
function islandora_xquery_admin_form_validate_machine_name($element, &$form_state, $form) {
  module_load_include('inc', 'islandora_xquery', 'includes/db');
  $config_exists = islandora_xquery_retrieve_association_from_machine_name($element['#value']);
  if ($config_exists) {
    form_error($element, t('A configuration with this machine name already exists!'));
  }
}

/**
 * Submit handler for the template overview form.
 *
 * @param array $form
 *   An array representing a Drupal form.
 * @param array $form_state
 *   An array representing the Drupal form state.
 */
function islandora_xquery_admin_form_submit($form, &$form_state) {
  module_load_include('inc', 'islandora_xquery', 'includes/db');
  islandora_xquery_add_association($form_state['values']['template_name'], $form_state['values']['machine_name'], $form_state['values']['template_description']);
  drupal_set_message(t('The template has been added!'));
}

/**
 * Submit handler for the template overview form.
 *
 * @param array $form
 *   An array representing a Drupal form.
 * @param array $form_state
 *   An array representing the Drupal form state.
 */
function islandora_xquery_admin_form_zorba_configure_submit($form, &$form_state) {
  variable_set('islandora_xquery_zorba', $form_state['values']['zorba']);
}

/**
 * Ajax callback for the Tesseract textfield.
 *
 * @param array $form
 *   The Drupal form definition.
 * @param array $form_state
 *   The Drupal form state.
 *
 * @return array
 *   The element to render as part the ajax callback.
 */
function islandora_xquery_admin_form_ajax_callback(array $form, array $form_state) {
  return $form['configure_zorba']['zorba'];
}