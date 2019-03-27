<?php

/**
 * BaseX processor implementation.
 */
class IslandoraXqueryBaseX implements IslandoraXqueryConfigurableProcessorInterface {

  /**
   * Constructor; build out our object.
   */
  protected function __construct($context, $output, $query) {
    $this->context = $context;
    $this->output = $output;
    $this->query = $query;
  }

  /**
   * {@inheritdoc}
   */
  public static function createBasicQuery($context, $output, $query) {
    return new static(
      $context,
      $output,
      $query
    );
  }

  /**
   * Helper; get the name of the executable.
   *
   * @return string
   *   The name of the executable to use.
   */
  public static function getExecutable() {
    return variable_get('islandora_xquery_basex_executable', '/usr/bin/basex');
  }

  /**
   * {@inheritdoc}
   */
  public static function hasExecutable() {
    return is_executable(static::getExecutable());
  }

  /**
   * {@inheritdoc}
   */
  public function execute() {
    return static::execCommand(array(
      static::getExecutable(),
      "-i{$this->context}",
      "-o{$this->output}",
      $this->query,
    ));
  }

  /**
   * {@inheritdoc}
   */
  public function validate() {
    return static::execCommand(array(
      static::getExecutable(),
      "-i{$this->context}",
      "-o{$this->output}",
      '-R false',
      $this->query,
    ));
  }

  /**
   * Helper; execute the given command.
   *
   * @param array $command_bits
   *   An array of command parts... we'll escape them before calling.
   *
   * @return bool
   *   TRUE if the command exited with exit-code 0; otherwise, FALSE.
   */
  protected static function execCommand(array $command_bits) {
    $command = implode(' ', array_map('escapeshellarg', $command_bits));

    $output = array();
    $return = 0;

    exec($command, $output, $return);

    return $return == 0;
  }

  /**
   * {@inheritdoc}
   */
  public static function getForm(array $offset, array &$form_state) {
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
          '!msg' => islandora_executable_available_message($executable),
        )),
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
