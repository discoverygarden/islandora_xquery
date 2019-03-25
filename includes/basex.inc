<?php

namespace Islandora\Xquery;

use IslandoraXqueryConfigurableProcessorInterface;

class BaseX implements IslandoraXqueryConfigurableProcessorInterface {

  public function __construct($context, $output, $query) {
    $this->context = $context;
    $this->output = $output;
    $this->query = $query;
  }

  public static function createBasicQuery($context, $output, $query) {
    return new static(
      $context,
      $output,
      $query
    );
  }

  public static function getExecutable() {
    return variable_get('islandora_xquery_basex_executable', 'basex');
  }

  public static function hasExecutable() {
    return is_executable(static::getExecutable());
  }

  public function execute() {

    $command_bits = array(
      static::getExecutable(),
      '-i' . drupal_realpath($this->context),
      '-o' . drupal_realpath($this->output),
      drupal_realpath($this->query),
    );

    $command = implode(array_map('escapeshellarg', $command_bits));
    $output = array();
    $return = 0;
    exec($command, $output, $return);
    return $return == 0;

  }

  public function validate() {
    $command_bits = array(
      static::getExecutable(),
      '-i' . drupal_realpath($this->context),
      '-o' . drupal_realpath($this->output),
      '-Rfalse',
      drupal_realpath($this->query),
    );

    $command = implode(array_map('escapeshellarg', $command_bits));
    $output = array();
    $return = 0;
    exec($command, $output, $return);
    return $return == 0;

  }

  /**
   * {@inheritdoc}
   */
  public static function getForm($offset, &$form_state) {
    form_load_include($form_state, 'inc', 'islandora_xquery', 'includes/basex');
    form_load_include($form_state, 'inc', 'islandora_xquery', 'includes/utilities');
    form_load_include($form_state, 'inc', 'islandora_xquery', 'includes/admin.form');
    form_load_include($form_state, 'inc', 'islandora', 'includes/utilities');

    $executable = drupal_array_get_nested_value($form_state, array_merge(array('values'), $offset, array('executable')));
    if (!isset($executable)) {
      $executable = static::getExecutable();
    }

    $id = 'basex-executable-wrapper';

    return array(
      'executable' => array(
        '#prefix' => "<div id='$id'>",
        '#suffix' => '</div>',
        '#type' => 'textfield',
        '#title' => t('BaseX exectuable'),
        '#description' => t('BaseX will be used to execute XQuery queries.<br/>!msg', array(
          '!msg' => islandora_executable_available_message($executable))
        ),
        '#default_value' => $executable,
        '#ajax' => array(
          'callback' => 'islandora_xquery_admin_form_ajax_callback',
          'wrapper' => $id,
          'effect' => 'fade',
          'event' => 'change',
        ),
      ),
    );
  }

  /**
   * {@inheritdoc}
   */
  public static function submitForm(array $values) {
    variable_set('islandora_xquery_basex_executable', $values['executable']);
  }

}
