<?php
/**
 * @file
 * Template management form for the Islandora XQuery module.
 * @todo
 *   Validate machine names.
 * @todo
 *   Adopt a string formatting language for token replacement.
 */

/**
 * Constructs the individual template management form.
 *
 * @param array $form
 *   An array representing a form in Drupal.
 * @param array $form_state
 *   An array containing the Drupal form state.
 * @param string $association_id
 *   The association ID that is being modified.
 *
 * @return array
 *   An array containing the form to be rendered.
 */
function islandora_xquery_template_management_form($form, &$form_state, $association_id) {
  module_load_include('inc', 'islandora_xquery', 'includes/db');
  $token_to_add = FALSE;
  $data = array();
  $xquery = '';
  if (isset($form_state['triggering_element'])) {
    $parents = $form_state['triggering_element']['#parents'];
    if ($parents == array('replacement_tokens', 'add_fieldset', 'add')) {
      $token_to_add = array(
        'token_name' => $form_state['values']['replacement_tokens']['add_fieldset']['token'],
        'token_description' => $form_state['values']['replacement_tokens']['add_fieldset']['description'],
        'token_default' => empty($form_state['values']['replacement_tokens']['add_fieldset']['default']) ? NULL : $form_state['values']['replacement_tokens']['add_fieldset']['default'],
      );
    }

    if ($parents == array('replacement_tokens', 'remove_selected')) {
      $rows_to_remove = array();
      foreach ($form_state['values']['replacement_tokens']['table'] as $key => $row) {
        if ($row == TRUE) {
          $rows_to_remove[] = $key;
        }
      }
      foreach ($rows_to_remove as $row_number) {
        unset($form_state['complete form']['replacement_tokens']['table']['#options'][$row_number]);
      }
    }

    // If there are values in the form_state use them for persistence in case of
    // AJAX callbacks, otherwise grab fresh values from the database.
    if (!empty($form_state['complete form']['replacement_tokens']['table']['#options'])) {
      $data = $form_state['complete form']['replacement_tokens']['table']['#options'];
      foreach ($data as &$val) {
        $val = islandora_xquery_token_database_structure($val);
      }
      unset($val);
    }

    if (isset($form_state['values']['xquery'])) {
      $xquery = $form_state['values']['xquery'];
    }
  }
  else {
    // Get stored values for this association.
    $data = islandora_xquery_get_tokens($association_id);
    $xquery = islandora_xquery_get_xquery($association_id);
    // Let's store these values now for use in submit in the future so
    // we don't have to make unneeded database requests.
    $form_state['tokens_database'] = $data;
    $form_state['xquery_database'] = $xquery;
  }

  $form = array(
    '#tree' => TRUE,
  );
  $form['association_id'] = array(
    '#type' => 'value',
    '#value' => $association_id,
  );

  if ($token_to_add !== FALSE) {
    $data[$token_to_add['token_name']] = $token_to_add;
  }
  $form['xquery'] = array(
    '#type' => 'textarea',
    '#title' => t('XQuery'),
    '#default_value' => $xquery,
  );
  $form['replacement_tokens'] = array(
    '#type' => 'fieldset',
    '#title' => t('Replacement Tokens'),
    '#collapsed' => TRUE,
    '#collapsible' => FALSE,
    '#prefix' => '<div id="islandora-xquery-tokens-wrapper">',
    '#suffix' => '</div>',
  );
  $form['replacement_tokens']['table'] = islandora_xquery_template_management_table($data);
  if (count($form['replacement_tokens']['table']['#options'])) {
    $form['replacement_tokens']['remove_selected'] = array(
      '#type' => 'button',
      '#value' => t('Remove selected'),
      '#ajax' => array(
        'callback' => 'islandora_xquery_ajax',
        'wrapper' => 'islandora-xquery-tokens-wrapper',
      ),
    );
  }
  $form['replacement_tokens']['add_fieldset'] = array(
    '#type' => 'fieldset',
    '#title' => t('Add replacement'),
    '#collapsed' => TRUE,
    '#collapsible' => FALSE,
  );
  $form['replacement_tokens']['add_fieldset']['token'] = array(
    '#type' => 'textfield',
    '#title' => t('Token'),
  );
  $form['replacement_tokens']['add_fieldset']['description'] = array(
    '#type' => 'textfield',
    '#title' => t('Description'),
  );
  $form['replacement_tokens']['add_fieldset']['default'] = array(
    '#type' => 'textfield',
    '#title' => t('Default'),
  );
  $form['replacement_tokens']['add_fieldset']['add'] = array(
    '#type' => 'button',
    '#value' => t('Add'),
    '#ajax' => array(
      'callback' => 'islandora_xquery_ajax',
      'wrapper' => 'islandora-xquery-tokens-wrapper',
    ),
  );
  $form['actions'] = array(
    '#type' => 'actions',
  );
  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save template'),
  );
  $form['actions']['delete'] = array(
    '#type' => 'submit',
    '#value' => t('Delete template'),
  );
  return $form;
}

/**
 * Validation for the template management form.
 *
 * @param array $form
 *   An array representing a form in Drupal.
 * @param array $form_state
 *   An array containing the Drupal form state.
 */
function islandora_xquery_template_management_form_validate($form, &$form_state) {
  $parents = $form_state['triggering_element']['#parents'];
  if ($parents == array('replacement_tokens', 'add_fieldset', 'add')) {
    if (empty($form_state['values']['replacement_tokens']['add_fieldset']['token'])) {
      form_error($form['replacement_tokens']['add_fieldset']['token'], t('The token field must not be empty.'));
    }
    // Check for token name uniqueness.
    else {
      $added_values = array();
      if (!empty($form_state['complete form']['replacement_tokens']['table']['#options'])) {
        foreach ($form_state['complete form']['replacement_tokens']['table']['#options'] as $token_name => $values) {
          $added_values[$token_name] = $token_name;
        }
      }
      $token = $form_state['values']['replacement_tokens']['add_fieldset']['token'];
      if (isset($added_values[$token])) {
        form_error($form['replacement_tokens']['add_fieldset']['token'], t('The token @token already exists in this template!', array('@token' => $token)));
      }
    }

    if (empty($form_state['values']['replacement_tokens']['add_fieldset']['description'])) {
      form_error($form['replacement_tokens']['add_fieldset']['description'], t('The description field must not be empty.'));
    }
  }

  if ($parents == array('actions', 'submit')) {
    // Check to see if the XQuery text area is empty.
    if (empty($form_state['values']['xquery'])) {
      form_error($form['xquery'], t('An XQuery must be defined!'));
    }
  }

  if ($parents == array('replacement_tokens', 'remove_selected')) {
    $rows_to_remove = array();
    foreach ($form_state['values']['replacement_tokens']['table'] as $key => $row) {
      if ($row['remove_field'] == TRUE) {
        $rows_to_remove[] = $key;
      }
    }
    if (count($rows_to_remove) === 0) {
      form_error($form['replacement_tokens']['table'], t('Must select at least one entry to remove!'));
    }
  }
}

/**
 * Submit handler for the template management form.
 *
 * @param array $form
 *   An array representing a form in Drupal.
 * @param array $form_state
 *   An array containing the Drupal form state.
 */
function islandora_xquery_template_management_form_submit($form, &$form_state) {
  $parents = $form_state['triggering_element']['#parents'];
  if ($parents == array('actions', 'submit')) {
    module_load_include('inc', 'islandora_xquery', 'includes/db');
    $association_id = $form_state['values']['association_id'];
    $xquery = $form_state['values']['xquery'];

    // See if it has changed from the database before we add.
    if (empty($form_state['xquery_database'])) {
      islandora_xquery_add_xquery($association_id, $xquery);
    }
    elseif ($xquery != $form_state['xquery_database']) {
      islandora_xquery_update_xquery($association_id, $xquery);
    }

    // Get all the values from the table and map them back to the db structure.
    $tokens_fs = array();
    foreach ($form_state['complete form']['replacement_tokens']['table']['#options'] as $value) {
      $tokens_fs[reset($value)] = islandora_xquery_token_database_structure($value);
    }
    $tokens_db = $form_state['tokens_database'];
    $tokens_insertion = array_diff_key($tokens_fs, $tokens_db);
    $tokens_deletion = array_diff_key($tokens_db, $tokens_fs);
    if (!empty($tokens_insertion)) {
      islandora_xquery_add_tokens($association_id, $tokens_insertion);
    }

    if (!empty($tokens_deletion)) {
      islandora_xquery_delete_tokens($association_id, $tokens_deletion);
    }
    drupal_set_message(t('The template changes have been saved!'));
  }
  if ($parents == array('actions', 'delete')) {
    $association_id = $form_state['values']['association_id'];
    drupal_goto("admin/islandora/tools/xquery/manage/template/$association_id/delete");
  }
}

/**
 * AJAX callback for the manage form.
 *
 * @param array $form
 *   An array representing a form in Drupal.
 * @param array $form_state
 *   An array containing the Drupal form state.
 *
 * @return array
 *   The portion of the form to be rendered.
 */
function islandora_xquery_ajax(&$form, &$form_state) {
  return $form['replacement_tokens'];
}


/**
 * Helper function to construct the tokens tableselect.
 *
 * @param array $data
 *   An array of data representing rows to be rendered in the tableselect.
 *
 * @return array
 *   An array representing the tableselect to be rendered.
 */
function islandora_xquery_template_management_table($data) {
  $rows = array();
  if ($data) {
    $map = function($token) {
      return array(
        $token['token_name'],
        $token['token_description'],
        $token['token_default'],
      );
    };
    $rows = array_map($map, $data);
  }
  // Add the table to the form.
  return array(
    '#type' => 'tableselect',
    '#header' => array(
      t('Token'),
      t('Description'),
      t('Default'),
    ),
    '#options' => $rows,
    '#empty' => t('No replacements currently set.'),
  );
}

/**
 * Takes an array of values and constructs into the database form from a row.
 *
 * @param array $value
 *   An array of numerical indexed values containing:
 *   - 0: The name of the token.
 *   - 1: The description of the token.
 *   - 2: The default value of the token.
 *
 * @return array
 *   An array in the database form we expect.
 */
function islandora_xquery_token_database_structure($value) {
  $keys = array('token_name', 'token_description', 'token_default');
  $token_db = array_combine($keys, $value);
  if (empty($token_db['token_default'])) {
    $token_db['token_default'] = NULL;
  }
  return $token_db;
}
