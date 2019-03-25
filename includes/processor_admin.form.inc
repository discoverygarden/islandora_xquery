<?php

function islandora_xquery_processor_admin_form(array $form, array &$form_state) {
  form_load_include($form_state, 'inc', 'islandora_xquery', 'includes/processor_admin.form');
    module_load_include('inc', 'islandora_xquery', 'includes/zorba');
    module_load_include('inc', 'islandora_xquery', 'includes/basex');

  $form_state['storage']['implementations'] = IslandoraXquery::getImplementations();

  try {
    $default_value = IslandoraXquery::getImplementation()[0];
  }
  catch (IslandoraXqueryNoExecutableException $e) {
    // The one configured, even though it doesn't appear to be executable...
    $default_value = $e->getImplementationInfo()[0];
  }

  $form['#tree'] = TRUE;

  $form['processors'] = array(
    '#type' => 'radios',
    '#title' => t('Processors'),
    '#options' => drupal_map_assoc(array_keys($form_state['storage']['implementations'])),
    '#default_value' => $default_value,
  );

  $form['processor'] = array(
    '#type' => 'fieldset',
    '#title' => t('Processor config'),
  );

  foreach ($form_state['storage']['implementations'] as $name => $class) {
    $form['processor'][$name] = (is_callable(array($class, "getForm")) ?
      call_user_func_array(array($class, "getForm"), array(array('processor', $name), &$form_state)) :
      array(
        '#markup' => t('No configuration...'),
      )) + array(
        '#type' => 'item',
        '#states' => array(
          'visible' => array(
            ':input[name=processors]' => array(
              array('value' => $name),
            ),
          ),
        ),
      );
  }

  $form['actions'] = array(
    '#type' => 'actions',
    'submit' => array(
      '#type' => 'submit',
      '#value' => t('Save configuration'),
    ),
  );

  return $form;
}

function islandora_xquery_processor_admin_form_submit(array $form, &$form_state) {
  $selected = $form_state['values']['processors'];
  variable_set('islandora_xquery_implementation', $form_state['values']['processors']);
  $implementation = $form_state['storage']['implementations'][$selected];
  if (is_callable(array($implementation, "getForm"))) {
    call_user_func(array($implementation, 'submitForm'), drupal_array_get_nested_value($form_state, array(
      'values',
      'processor',
      $selected,
    )));
  }
  drupal_set_message(t('The configuration has been saved.'));
}
